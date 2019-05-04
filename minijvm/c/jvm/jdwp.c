//
// Created by gust on 2017/9/20.
//
#include "../utils/d_type.h"
#include "jdwp.h"
#include "jvm_util.h"
#include "java_native_io.h"
#include "garbage.h"

JdwpServer jdwpserver;

void jdwp_post_events(JdwpClient *client);

void jdwp_eventset_post(JdwpClient *client, EventSet *set, EventInfo *event);

void event_on_debug_step(Runtime *step_runtime);

void jdwp_eventset_destory(EventSet *set);

void jdwppacket_destory(JdwpPacket *packet);

JdwpClient *jdwp_client_create(s32 sockfd);

void jdwp_client_destory(JdwpClient *client);

//==================================================    server    ==================================================

void jdwp_put_client(ArrayList *clients, JdwpClient *client) {
    arraylist_push_back(clients, client);
}

s32 jdwp_thread_listener(void *para) {
    JdwpServer *srv = (JdwpServer *) para;
    s32 srvsock = sock_open();
    sock_bind(srvsock, srv->ip, srv->port);
    srv->srvsock = srvsock;
    sock_listen(srvsock);
    srv->mode |= JDWP_MODE_LISTEN;

    while (!srv->exit) {
        s32 sockfd = sock_accept(srvsock);
        if (sockfd == -1) {
            srv->exit = 1;
            break;
        }
        sock_option(sockfd, SOCK_OP_TYPE_NON_BLOCK, SOCK_OP_VAL_NON_BLOCK, 0);
        JdwpClient *client = jdwp_client_create(sockfd);
        jdwp_put_client(srv->clients, client);
    }

    return 0;
}

s32 jdwp_thread_dispacher(void *para) {
    JdwpServer *srv = (JdwpServer *) para;
    Runtime *runtime = runtime_create(NULL);
    srv->runtime = runtime;
    srv->mode |= JDWP_MODE_DISPATCH;
    s32 i;
    while (!srv->exit) {
        for (i = 0; i < srv->clients->length; i++) {
            JdwpClient *client = arraylist_get_value(srv->clients, i);
            jdwp_client_process(client, runtime);
            jdwp_post_events(client);
            if (client->closed) {
                jdwp_client_destory(client);
                arraylist_remove(srv->clients, client);
            }
        }
        jthread_sleep(runtime, 20);
    }
    check_suspend_and_pause(runtime);//for no destory when gc()
    runtime_destory(runtime);
    srv->runtime = NULL;

    return 0;
}

s32 jdwp_start_server() {
    if (!JDWP_DEBUG)return 0;
    jdwpserver.ip = utf8_create();
    jdwpserver.port = JDWP_TCP_PORT;
    jdwpserver.exit = 0;
    jdwpserver.clients = arraylist_create(0);
    jdwpserver.events = arraylist_create(0);
    jdwpserver.event_sets = hashtable_create(DEFAULT_HASH_FUNC, DEFAULT_HASH_EQUALS_FUNC);

    thrd_create(&jdwpserver.pt_listener, jdwp_thread_listener, &jdwpserver);
    thrd_create(&jdwpserver.pt_dispacher, jdwp_thread_dispacher, &jdwpserver);
    return 0;
}

s32 jdwp_stop_server() {
    if (!JDWP_DEBUG)return 0;
    while (jdwpserver.mode != JDWP_MODE_LISTEN + JDWP_MODE_DISPATCH) {
        threadSleep(10);
    }
    jdwpserver.exit = 1;
    sock_close(jdwpserver.srvsock);
    s32 i;
    //
    for (i = 0; i < jdwpserver.clients->length; i++) {
        JdwpClient *client = arraylist_get_value(jdwpserver.clients, i);
        jdwp_client_destory(client);
    }
    arraylist_destory(jdwpserver.clients);
    //
    for (i = 0; i < jdwpserver.events->length; i++) {
        EventInfo *event = arraylist_get_value(jdwpserver.events, i);
        jvm_free(event);
    }
    arraylist_destory(jdwpserver.events);
    //
    HashtableIterator hti;
    hashtable_iterate(jdwpserver.event_sets, &hti);
    for (; hashtable_iter_has_more(&hti);) {
        __refer k = hashtable_iter_next_key(&hti);
        EventSet *set = hashtable_get(jdwpserver.event_sets, k);
        jdwp_eventset_destory(set);
    }
    hashtable_destory(jdwpserver.event_sets);
    //

    //
    utf8_destory(jdwpserver.ip);
    thrd_detach(jdwpserver.pt_listener);
    thrd_detach(jdwpserver.pt_dispacher);
    return 0;
}


JdwpClient *jdwp_client_create(s32 sockfd) {
    JdwpClient *client = jvm_calloc(sizeof(JdwpClient));
    client->closed = 0;
    client->conn_first = 1;
    client->sockfd = sockfd;
    client->rcvp = NULL;
    client->temp_obj_holder = hashset_create();
    return client;
}

void jdwp_client_destory(JdwpClient *client) {
    if (client->rcvp) {
        jdwppacket_destory(client->rcvp);
    }
    //release all hold object
    HashsetIterator hi;
    hashset_iterate(client->temp_obj_holder, &hi);
    while (hashset_iter_has_more(&hi)) {
        HashsetKey k = hashset_iter_next_key(&hi);
        gc_refer_release(k);
    }
    hashset_destory(client->temp_obj_holder);
    client->temp_obj_holder = NULL;
    jvm_free(client);
}

void jdwp_client_hold_obj(JdwpClient *client, Runtime *runtime, __refer obj) {
    hashset_put(client->temp_obj_holder, obj);
    gc_refer_hold(obj);
    gc_refer_reg(runtime, obj);
}

void jdwp_client_release_obj(JdwpClient *client, __refer obj) {
    hashset_remove(client->temp_obj_holder, obj, 1);
    gc_refer_release(obj);
}
//==================================================    packet    ==================================================



JdwpPacket *jdwppacket_create() {
    JdwpPacket *packet = jvm_calloc(sizeof(JdwpPacket));
    packet->alloc = 32;
    packet->readPos = 11;
    packet->writePos = 11;
    packet->data = jvm_calloc(packet->alloc);
    return packet;
}

JdwpPacket *jdwppacket_create_data(c8 *data, s32 len) {
    JdwpPacket *packet = jvm_calloc(sizeof(JdwpPacket));
    packet->data = data;
    packet->alloc = len;
    return packet;
}

void jdwppacket_destory(JdwpPacket *packet) {
    jvm_free(packet->data);
    jvm_free(packet);
}

s8 jdwppacket_read_byte(JdwpPacket *packet) {
    return packet->data[packet->readPos++];
}

s16 jdwppacket_read_short(JdwpPacket *packet) {
    s16 s = (short) (((packet->data[packet->readPos + 1] & 0xFF) << 0) +
                     ((packet->data[packet->readPos + 0] & 0xFF) << 8));
    packet->readPos += 2;
    return s;
}


s32 jdwppacket_read_int(JdwpPacket *packet) {
    s32 i = ((packet->data[packet->readPos + 3] & 0xFF) << 0)
            + ((packet->data[packet->readPos + 2] & 0xFF) << 8)
            + ((packet->data[packet->readPos + 1] & 0xFF) << 16)
            + ((packet->data[packet->readPos + 0] & 0xFF) << 24);
    packet->readPos += 4;
    return i;
}


s64 jdwppacket_read_long(JdwpPacket *packet) {
    s64 l = (((s64) packet->data[packet->readPos + 7] & 0xFFL) << 0)
            + (((s64) packet->data[packet->readPos + 6] & 0xFFL) << 8)
            + (((s64) packet->data[packet->readPos + 5] & 0xFFL) << 16)
            + (((s64) packet->data[packet->readPos + 4] & 0xFFL) << 24)
            + (((s64) packet->data[packet->readPos + 3] & 0xFFL) << 32)
            + (((s64) packet->data[packet->readPos + 2] & 0xFFL) << 40)
            + (((s64) packet->data[packet->readPos + 1] & 0xFFL) << 48)
            + (((s64) packet->data[packet->readPos + 0] & 0xFFL) << 56);
    packet->readPos += 8;
    return l;
}


Utf8String *jdwppacket_read_utf(JdwpPacket *packet) {
    Utf8String *ustr = utf8_create();
    s32 len = jdwppacket_read_int(packet);
    s32 i;
    for (i = 0; i < len; i++) {
        utf8_pushback(ustr, jdwppacket_read_byte(packet));
    }
    return ustr;
}

__refer jdwppacket_read_refer(JdwpPacket *packet) {
    if (sizeof(__refer) > 4) {
        return (__refer) (intptr_t) jdwppacket_read_long(packet);
    } else {
        return (__refer) (intptr_t) jdwppacket_read_int(packet);
    }
}

void jdwppacket_ensureCapacity(JdwpPacket *packet, s32 length) {
    s32 newcount = packet->writePos + length;
    if (newcount >= packet->alloc) {
        newcount = newcount > packet->alloc << 1 ? newcount : packet->alloc << 1;
        c8 *tmp = jvm_calloc(newcount);
        memcpy(tmp, packet->data, packet->alloc);
        jvm_free(packet->data);
        packet->data = tmp;
        packet->alloc = newcount;
    }
}

void jdwppacket_write_byte(JdwpPacket *packet, s8 val) {
    jdwppacket_ensureCapacity(packet, 1);
    packet->data[packet->writePos] = val;
    packet->writePos++;
}

void jdwppacket_write_short(JdwpPacket *packet, s16 val) {
    jdwppacket_ensureCapacity(packet, 2);
    packet->data[packet->writePos + 1] = (u8) (val >> 0);
    packet->data[packet->writePos + 0] = (u8) (val >> 8);
    packet->writePos += 2;
}


void jdwppacket_write_int(JdwpPacket *packet, s32 val) {
    jdwppacket_ensureCapacity(packet, 4);
    packet->data[packet->writePos + 3] = (u8) (val >> 0);
    packet->data[packet->writePos + 2] = (u8) (val >> 8);
    packet->data[packet->writePos + 1] = (u8) (val >> 16);
    packet->data[packet->writePos + 0] = (u8) (val >> 24);
    packet->writePos += 4;
}

void jdwppacket_write_long(JdwpPacket *packet, s64 val) {
    jdwppacket_ensureCapacity(packet, 8);
    packet->data[packet->writePos + 7] = (u8) (val >> 0);
    packet->data[packet->writePos + 6] = (u8) (val >> 8);
    packet->data[packet->writePos + 5] = (u8) (val >> 16);
    packet->data[packet->writePos + 4] = (u8) (val >> 24);
    packet->data[packet->writePos + 3] = (u8) (val >> 32);
    packet->data[packet->writePos + 2] = (u8) (val >> 40);
    packet->data[packet->writePos + 1] = (u8) (val >> 48);
    packet->data[packet->writePos + 0] = (u8) (val >> 56);
    packet->writePos += 8;
}

void jdwppacket_write_buf(JdwpPacket *packet, c8 *val, s32 len) {
    jdwppacket_ensureCapacity(packet, len);
    memcpy(packet->data + packet->writePos, val, len);
    packet->writePos += len;
}

void jdwppacket_write_utf(JdwpPacket *packet, Utf8String *val) {
    jdwppacket_write_int(packet, val->length);
    jdwppacket_write_buf(packet, utf8_cstr(val), val->length);
}

void jdwppacket_write_refer(JdwpPacket *packet, __refer val) {
    if (sizeof(__refer) > 4) {
        jdwppacket_write_long(packet, (s64) (intptr_t) val);
    } else {
        jdwppacket_write_int(packet, (s32) (intptr_t) val);
    }
}

s32 jdwppacket_getbypos(JdwpPacket *packet, s32 pos, s32 n) {
    s32 len = 0;
    s32 last = pos + n;
    s32 i;
    for (i = pos; i < last; i++) {
        len <<= 8;
        len += (u8) packet->data[i];
    }
    return len;
}

s16 jdwppacket_get_cmd_err(JdwpPacket *packet) {
    return (s16) jdwppacket_getbypos(packet, 9, 2);
}

u8 jdwppacket_get_flag(JdwpPacket *packet) {
    return (u8) jdwppacket_getbypos(packet, 8, 1);
}

s32 jdwppacket_get_id(JdwpPacket *packet) {
    return jdwppacket_getbypos(packet, 4, 4);
}

s32 jdwppacket_get_length(JdwpPacket *packet) {
    return jdwppacket_getbypos(packet, 0, 4);
}

void jdwppacket_set_flag(JdwpPacket *packet, u8 flag) {
    s32 pos = 8;
    packet->data[pos++] = flag;
}

void jdwppacket_set_id(JdwpPacket *packet, s32 id) {
    s32 pos = 4;
    packet->data[pos++] = (u8) (id >> 24);
    packet->data[pos++] = (u8) (id >> 16);
    packet->data[pos++] = (u8) (id >> 8);
    packet->data[pos++] = (u8) (id);
}

void jdwppacket_set_length(JdwpPacket *packet, s32 len) {
    s32 pos = 0;
    packet->data[pos++] = (u8) (len >> 24);
    packet->data[pos++] = (u8) (len >> 16);
    packet->data[pos++] = (u8) (len >> 8);
    packet->data[pos++] = (u8) (len);
}

void jdwppacket_set_cmd(JdwpPacket *packet, u16 cmd) {
    s32 pos = 9;
    packet->data[pos++] = (c8) (cmd >> 8);
    packet->data[pos++] = (c8) (cmd >> 0);
}

void jdwppacket_set_err(JdwpPacket *packet, u16 err) {
    s32 pos = 9;
    packet->data[pos++] = (c8) (err >> 8);
    packet->data[pos++] = (c8) (err >> 0);
}

s32 jdwp_read_fully(JdwpClient *client, c8 *buf, s32 need) {
    s32 got = 0, len = 0;
    while (len != -1) {
        got += len;
        if (got == need)break;
        len = sock_recv(client->sockfd, buf + got, need - got);
        if (len == -2)len = 0;
    }
    if (len == -1)return len;
    return got;
}

s32 jdwp_write_fully(JdwpClient *client, c8 *buf, s32 need) {
    s32 sent = 0, len = 0;
    while (len != -1) {
        sent += len;
        if (sent == need)break;
        len = sock_send(client->sockfd, buf + sent, need - sent);
        if (len == -2)len = 0;
    }
    if (len == -1)return len;
    return sent;
}

JdwpPacket *jdwp_readpacket(JdwpClient *client) {
    if (!client->conn_first) {
        if (!client->rcvp) {//上个包已收完
            client->rcvp = jdwppacket_create();
            client->rcvp->_req_len = 4;
            client->rcvp->_rcv_len = 0;
            client->rcvp->_4len = 1;//标志先接收的是长度信息
        }
        if (client->rcvp) {//上个包收到一半，有两种情况，先收4字节，再收剩余部分
            if (client->rcvp->_4len) {
                s32 len = sock_recv(client->sockfd, client->rcvp->data + client->rcvp->_rcv_len,
                                    client->rcvp->_req_len - client->rcvp->_rcv_len);
                if (len == -1)client->closed = 1;
                if (len == -2)len = 0;
                client->rcvp->_rcv_len += len;
                if (client->rcvp->_rcv_len == client->rcvp->_req_len) {
                    client->rcvp->_4len = 0;
                    client->rcvp->_req_len = jdwppacket_get_length(client->rcvp) - 4;//下次进入时直接收包体
                    client->rcvp->_rcv_len = 0;
                    jdwppacket_ensureCapacity(client->rcvp, client->rcvp->_req_len);
                }
            } else {
                s32 len = sock_recv(client->sockfd, client->rcvp->data + 4 + client->rcvp->_rcv_len,
                                    client->rcvp->_req_len - client->rcvp->_rcv_len);
                if (len == -1)client->closed = 1;
                if (len == -2)len = 0;
                client->rcvp->_rcv_len += len;
                if (client->rcvp->_rcv_len == client->rcvp->_req_len) {
                    JdwpPacket *p = client->rcvp;
                    client->rcvp = NULL;
                    return p;
                }
            }
        }
    } else {//首次连接
        c8 buf[14];
        s32 len = jdwp_read_fully(client, (c8 *) &buf, 14);
        if (len == -1) {
            client->closed = 1;
        }
        len = (s32) strlen(JDWP_HANDSHAKE);
        s32 i;
        for (i = 0; i < len; i++) {
            if (JDWP_HANDSHAKE[i] != buf[i]) {
                client->closed = 1;
                return NULL;
            }
        }
        len = jdwp_write_fully(client, JDWP_HANDSHAKE, 14);
        if (len < 0) {
            client->closed = 1;
        }
        client->conn_first = 0;
    }
    return NULL;
}


s32 jdwp_writepacket(JdwpClient *client, JdwpPacket *packet) {
    jdwppacket_set_length(packet, packet->writePos);
    s32 len = jdwp_write_fully(client, packet->data, packet->writePos);
    jdwppacket_destory(packet);
    if (len < 0) {
        client->closed = 1;
        return 1;
    }
    return 0;
}

//==================================================    toolkit    ==================================================

Runtime *getRuntimeOfThread(Instance *jthread) {
    Runtime *r = NULL;
    s32 i;
    for (i = 0; i < thread_list->length; i++) {
        Runtime *rt = threadlist_get(i);
        if (rt && rt->threadInfo->jthread == jthread)r = rt;
    }
    return r;
}

void signatureToName(Utf8String *signature) {
    if (utf8_char_at(signature, 0) == 'L') {
        utf8_substring(signature, 1, signature->length - 1);
    }
}

void nameToSignature(Utf8String *name) {
    utf8_insert(name, 0, 'L');
    utf8_append_c(name, ";");
}

u8 getClassStatus(JClass *clazz) {
//    return JDWP_CLASS_STATUS_INITIALIZED;
    return clazz->status >= CLASS_STATUS_CLINITED ?
           JDWP_CLASS_STATUS_INITIALIZED | JDWP_CLASS_STATUS_PREPARED | JDWP_CLASS_STATUS_VERIFIED
                                                  : (clazz->status >= CLASS_STATUS_PREPARED ?
                                                     JDWP_CLASS_STATUS_PREPARED | JDWP_CLASS_STATUS_VERIFIED
                                                                                            : JDWP_CLASS_STATUS_VERIFIED);
}

CodeAttribute *getCodeAttribute(MethodInfo *method) {
    return method->converted_code;
}


s32 getClassType(JClass *clazz) {
    if (clazz->mb.arr_type_index) {
        return JDWP_TYPETAG_ARRAY;
    } else if (clazz->cff.access_flags & ACC_INTERFACE) {
        return JDWP_TYPETAG_INTERFACE;
    } else {
        return JDWP_TYPETAG_CLASS;
    }
}


c8 getSimpleTag(u8 type) {
    char bytes = '0';
    switch (type) {
        case JDWP_TAG_BYTE:
        case JDWP_TAG_BOOLEAN:
            bytes = '1';
            break;
        case JDWP_TAG_SHORT:
        case JDWP_TAG_CHAR:
            bytes = '2';
            break;
        case JDWP_TAG_INT:
        case JDWP_TAG_FLOAT:
            bytes = '4';
            break;
        case JDWP_TAG_LONG:
        case JDWP_TAG_DOUBLE:
            bytes = '8';
            break;
        case JDWP_TAG_ARRAY:
        case JDWP_TAG_OBJECT:
        case JDWP_TAG_STRING:
        case JDWP_TAG_THREAD:
        case JDWP_TAG_THREAD_GROUP:
        case JDWP_TAG_CLASS_LOADER:
        case JDWP_TAG_CLASS_OBJECT:
            bytes = 'R';
            break;
        case JDWP_TAG_VOID:
            bytes = '0';
            break;
    }
    return bytes;
}

c8 getInstanceOfClassTag(Instance *ins) {
    if (!ins)return JDWP_TAG_OBJECT;
    if (ins->mb.type == MEM_TYPE_CLASS)return JDWP_TAG_CLASS_OBJECT;
    JClass *clazz = ins->mb.clazz;
    if (clazz->mb.arr_type_index)return JDWP_TAG_ARRAY;
    if (utf8_equals_c(clazz->name, STR_CLASS_JAVA_LANG_THREAD))return JDWP_TAG_THREAD;
    if (utf8_equals_c(clazz->name, STR_CLASS_JAVA_LANG_STRING))return JDWP_TAG_STRING;
    return JDWP_TAG_OBJECT;
}

c8 getJdwpTag(Utf8String *ustr) {
    if (utf8_equals_c(ustr, STR_INS_JAVA_LANG_STRING)) {
        return JDWP_TAG_STRING;
    } else if (utf8_equals_c(ustr, STR_INS_JAVA_LANG_CLASS)) {
        return JDWP_TAG_CLASS_OBJECT;
    } else if (utf8_equals_c(ustr, STR_INS_JAVA_LANG_THREAD)) {
        return JDWP_TAG_THREAD;
    }
    return utf8_char_at(ustr, 0);
}

void writeValueType(JdwpPacket *res, ValueType *vt) {
    jdwppacket_write_byte(res, vt->type);
    switch (getSimpleTag(vt->type)) {
        case '1':
            jdwppacket_write_byte(res, (s8) vt->value);
            break;
        case '2':
            jdwppacket_write_short(res, (short) vt->value);
            break;
        case '4':
            jdwppacket_write_int(res, (int) vt->value);
            break;
        case '8':
            jdwppacket_write_long(res, vt->value);
            break;
        case 'R':
            jdwppacket_write_refer(res, (__refer) (intptr_t) vt->value);
            break;
    }
}

void readValueType(JdwpPacket *req, ValueType *vt) {
    vt->type = jdwppacket_read_byte(req);
    switch (getSimpleTag(vt->type)) {
        case '1':
            vt->value = jdwppacket_read_byte(req);
            break;
        case '2':
            vt->value = jdwppacket_read_short(req);
            break;
        case '4':
            vt->value = jdwppacket_read_int(req);
            break;
        case '8':
            vt->value = jdwppacket_read_long(req);
            break;
        case 'R':
            vt->value = (s64) (intptr_t) jdwppacket_read_refer(req);
            break;
    }
}

void writeLocation(JdwpPacket *res, Location *loc) {
    jdwppacket_write_byte(res, loc->typeTag);
    jdwppacket_write_refer(res, loc->classID);
    jdwppacket_write_refer(res, loc->methodID);
    jdwppacket_write_long(res, loc->execIndex);
}

void readLocation(JdwpPacket *req, Location *loc) {
    loc->typeTag = jdwppacket_read_byte(req);
    loc->classID = jdwppacket_read_refer(req);
    loc->methodID = jdwppacket_read_refer(req);
    loc->execIndex = jdwppacket_read_long(req);
}

s64 getPtrValue(u8 type, c8 *ptr) {
    s64 value = 0;
    switch (getSimpleTag(type)) {
        case '1':
            value = getFieldByte(ptr);
            break;
        case '2':
            value = getFieldShort(ptr);
            break;
        case '4':
            value = getFieldInt(ptr);
            break;
        case '8':
            value = getFieldLong(ptr);
            break;
        case 'R':
            value = (s64) (intptr_t) getFieldRefer(ptr);
            break;
    }
    return value;
}


void writeArrayRegion(JdwpPacket *res, Instance *arr, s32 firstIndex, s32 length) {
    c8 arr_type = utf8_char_at(arr->mb.clazz->name, 1);
    jdwppacket_write_byte(res, arr_type);
    jdwppacket_write_int(res, length);
    c8 tag = getSimpleTag(arr_type);
    s32 i;
    //原子类型不用写标志，非原子类型则需要是ValueType
    for (i = 0; i < length; i++) {
        switch (tag) {
            case '1':
                jdwppacket_write_byte(res, getFieldByte(&arr->arr_body[(firstIndex + i)]));
                break;
            case '2':
                jdwppacket_write_short(res, getFieldShort(&arr->arr_body[(firstIndex + i) * 2]));
                break;
            case '4':
                jdwppacket_write_int(res, getFieldInt(&arr->arr_body[(firstIndex + i) * 4]));
                break;
            case '8':
                jdwppacket_write_long(res, getFieldLong(&arr->arr_body[(firstIndex + i) * 8]));
                break;
            case 'R': {
                Instance *elem = getFieldRefer(&arr->arr_body[(firstIndex + i) * sizeof(__refer)]);
                if (elem)
                    jdwppacket_write_byte(res, getInstanceOfClassTag(elem));
                else
                    jdwppacket_write_byte(res, 'L');
                jdwppacket_write_refer(res, elem);
                break;
            }
        }
    }
}

void getClassSignature(JClass *clazz, Utf8String *ustr) {
    if (clazz->mb.arr_type_index) {
        utf8_append(ustr, clazz->name);
    } else {
        utf8_append_c(ustr, "L");
        utf8_append(ustr, clazz->name);
        utf8_append_c(ustr, ";");
    }
}

s32 location_equals(Location *loc1, Location *loc2) {
    if ((!loc1 && loc2) || (!loc2 && loc1))return 0; //其中一个为NULL，另一个不空
    if (loc1->typeTag == loc2->typeTag
        && loc1->classID == loc2->classID
        && loc1->methodID == loc2->methodID
        && loc1->execIndex == loc2->execIndex
            )
        return 1;
    return 0;
}


void jdwp_print_packet(JdwpPacket *packet) {
    s32 i;
    for (i = 0; i < packet->writePos; i++) {
        if (i % 5 == 0) { jvm_printf("      "); }
        if (i % 10 == 0) { jvm_printf("\n"); }
        jvm_printf(" %c[%2x]", packet->data[i] < ' ' || packet->data[i] > '~' ? ' ' : packet->data[i],
                   (u8) packet->data[i]);
    }
    jvm_printf("\n------------------------------\n");
}

void jdwp_check_breakpoint(Runtime *runtime) {
    u32 index = (u32) (runtime->pc - runtime->ca->code);
    MethodInfo *method = runtime->method;
    if ((method->breakpoint) && pairlist_getl(method->breakpoint, index)) {
        event_on_breakpoint(runtime);//
        s32 i;
        for (i = 0; i < thread_list->length; i++) {
            Runtime *t = threadlist_get(i);
            if (t) {
                jthread_suspend(t);
            }
        }
    }
}

void jdwp_check_debug_step(Runtime *runtime) {
    JdwpStep *step = &(runtime->threadInfo->jdwp_step);
    s32 suspend = 0;
    switch (step->next_type) {
        case NEXT_TYPE_SINGLE:
            if (step->bytecode_count >= step->next_stop_bytecode_count) {
                suspend = 1;
            }
            break;
        case NEXT_TYPE_OVER:
            if (runtime->ca) {
                s32 depth = getRuntimeDepth(runtime->threadInfo->top_runtime);
                if (depth == step->next_stop_runtime_depth) {
                    if (getLineNumByIndex(runtime->ca, (s32) (runtime->pc - runtime->ca->code)) !=
                        step->next_stop_bytecode_index) {
                        suspend = 1;
                    }
                } else if (depth < step->next_stop_runtime_depth) {
                    suspend = 1;
                }
            }
            break;
        case NEXT_TYPE_INTO:
            if (getRuntimeDepth(runtime->threadInfo->top_runtime) <= step->next_stop_runtime_depth) {
                suspend = 1;
            }
            break;
        case NEXT_TYPE_OUT:
            if (getRuntimeDepth(runtime->threadInfo->top_runtime) == step->next_stop_runtime_depth) {
                suspend = 1;
            }
            break;
    }
    if (suspend) {
        event_on_debug_step(runtime);
        jthread_suspend(runtime);
    }
}
//==================================================    event    ==================================================

void jdwp_event_put(EventInfo *event) {
    arraylist_push_back(jdwpserver.events, event);
}

EventInfo *jdwp_event_get() {
    return arraylist_pop_front(jdwpserver.events);
}

void jdwp_eventset_put(EventSet *set) {
    hashtable_put((jdwpserver.event_sets), (__refer) (intptr_t) set->requestId, set);
}

EventSet *jdwp_eventset_get(s32 id) {
    return hashtable_get((jdwpserver.event_sets), (__refer) (intptr_t) id);
}

void jdwp_post_events(JdwpClient *client) {
    EventInfo *event;
    while ((event = jdwp_event_get()) != NULL) {
        s32 i = 0;
        HashtableIterator hti;
        hashtable_iterate(jdwpserver.event_sets, &hti);
        for (; hashtable_iter_has_more(&hti);) {
            __refer k = hashtable_iter_next_key(&hti);
            EventSet *set = hashtable_get(jdwpserver.event_sets, k);
            if (set->eventKind == event->eventKind) {
                jdwp_eventset_post(client, set, event);
            }
            i++;
        }
        jvm_free(event);
    }
}

void event_on_class_prepar(Runtime *runtime, JClass *clazz) {
    EventInfo *ei = jvm_calloc(sizeof(EventInfo));
    ei->eventKind = JDWP_EVENTKIND_CLASS_PREPARE;
    ei->thread = runtime->threadInfo->jthread;
    ei->refTypeTag = getClassType(clazz);
    ei->typeID = clazz;
    ei->signature = clazz->name;
    ei->status = getClassStatus(clazz);
    jdwp_event_put(ei);
}

void event_on_class_unload(Runtime *runtime, JClass *clazz) {
    EventInfo *ei = jvm_calloc(sizeof(EventInfo));
    ei->eventKind = JDWP_EVENTKIND_CLASS_UNLOAD;
    ei->signature = clazz->name;
    jdwp_event_put(ei);
}

void event_on_thread_start(Instance *jthread) {
    EventInfo *ei = jvm_calloc(sizeof(EventInfo));
    ei->eventKind = JDWP_EVENTKIND_THREAD_START;
    ei->thread = jthread;
    jdwp_event_put(ei);
}

void event_on_thread_death(Instance *jthread) {
    EventInfo *ei = jvm_calloc(sizeof(EventInfo));
    ei->eventKind = JDWP_EVENTKIND_THREAD_DEATH;
    ei->thread = jthread;
    jdwp_event_put(ei);
}

void event_on_breakpoint(Runtime *breakpoint_runtime) {
    EventInfo *ei = jvm_calloc(sizeof(EventInfo));
    ei->eventKind = JDWP_EVENTKIND_BREAKPOINT;
    ei->thread = breakpoint_runtime->threadInfo->jthread;
    ei->loc.typeTag = getClassType(breakpoint_runtime->clazz);
    ei->loc.classID = breakpoint_runtime->clazz;
    ei->loc.methodID = breakpoint_runtime->method;
    ei->loc.execIndex = (u64) (intptr_t) breakpoint_runtime->pc - (u64) (intptr_t) breakpoint_runtime->ca->code;
    jdwp_event_put(ei);
}

void event_on_debug_step(Runtime *step_runtime) {
    EventInfo *ei = jvm_calloc(sizeof(EventInfo));
    ei->eventKind = JDWP_EVENTKIND_SINGLE_STEP;
    ei->thread = step_runtime->threadInfo->jthread;
    ei->loc.typeTag = getClassType(step_runtime->clazz);
    ei->loc.classID = step_runtime->clazz;
    ei->loc.methodID = step_runtime->method;
    ei->loc.execIndex = (u64) (intptr_t) step_runtime->pc - (u64) (intptr_t) step_runtime->ca->code;
    jdwp_event_put(ei);
}

s32 jdwp_set_breakpoint(s32 setOrClear, JClass *clazz, MethodInfo *methodInfo, s64 execIndex) {
    if (!methodInfo->breakpoint) {
        methodInfo->breakpoint = pairlist_create(4);
    }
    if (methodInfo->converted_code) {
        if (setOrClear) {
            pairlist_putl(methodInfo->breakpoint, (intptr_t) execIndex, 1);
            return JDWP_ERROR_NONE;
        } else {
            pairlist_removel(methodInfo->breakpoint, (intptr_t) execIndex);
            if (methodInfo->breakpoint->count == 0) {
                jvm_free(methodInfo->breakpoint);
                methodInfo->breakpoint = NULL;
            }
            return JDWP_ERROR_NONE;
        }
    }

    return JDWP_ERROR_INVALID_LOCATION;
}


s32 jdwp_set_debug_step(s32 setOrClear, Instance *jthread, s32 size, s32 depth) {
    /**
     * 由于方法调用层级不同，则runtime的son的层级不同，由此控制虚机方法step_into ,step_out
     */
    Runtime *r = jthread_get_stackframe_value(jthread);
    JdwpStep *step = &r->threadInfo->jdwp_step;
    if (setOrClear) {
        step->active = 1;
        if (depth == JDWP_STEPDEPTH_INTO) {
            step->next_type = NEXT_TYPE_INTO;
            step->next_stop_runtime_depth = getRuntimeDepth(r->threadInfo->top_runtime) + 1;
        } else if (depth == JDWP_STEPDEPTH_OUT) {
            step->next_type = NEXT_TYPE_OUT;
            step->next_stop_runtime_depth = getRuntimeDepth(r->threadInfo->top_runtime) - 1;
        } else {
            if (size == JDWP_STEPSIZE_LINE) {
                Runtime *last = getLastSon(r);//当前runtime
                s32 nextPc = getLineNumByIndex(last->ca, (s32) (last->pc - last->ca->code));
                step->next_type = NEXT_TYPE_OVER;
                step->next_stop_bytecode_index = nextPc;
                step->next_stop_runtime_depth = getRuntimeDepth(r->threadInfo->top_runtime);
            } else {
                step->next_type = NEXT_TYPE_SINGLE;
                step->next_stop_bytecode_count = 1;
            }
        }
    } else {
        step->active = 0;
    }
    return JDWP_ERROR_NONE;
}

EventSet *jdwp_create_eventset(JdwpPacket *req) {
    EventSet *set = jvm_calloc(sizeof(EventSet));
    set->requestId = jdwp_eventset_requestid++;
    set->eventKind = jdwppacket_read_byte(req);
    set->suspendPolicy = jdwppacket_read_byte(req);
    set->modifiers = jdwppacket_read_int(req);
    set->mods = jvm_calloc(set->modifiers * sizeof(EventSetMod));
    s32 i;
    for (i = 0; i < set->modifiers; i++) {
        EventSetMod *mod = &set->mods[i];
        u8 imod = jdwppacket_read_byte(req);
        mod->mod_type = imod;
        switch (imod) {
            case 1:
                mod->count = jdwppacket_read_int(req);
                break;
            case 2:
                mod->exprID = jdwppacket_read_int(req);
                break;
            case 3:
                mod->thread = jdwppacket_read_refer(req);
                break;
            case 4:
                mod->clazz = jdwppacket_read_refer(req);
                break;
            case 5:
            case 6:
                mod->classPattern = jdwppacket_read_utf(req);
                break;
            case 7:
                readLocation(req, &mod->loc);
                break;
            case 8:
                mod->exceptionOrNull = jdwppacket_read_refer(req);
                mod->caught = jdwppacket_read_byte(req);
                mod->uncaught = jdwppacket_read_byte(req);
                break;
            case 9:
                mod->declaring = jdwppacket_read_refer(req);
                mod->fieldID = jdwppacket_read_refer(req);
                break;
            case 10:
                mod->thread = jdwppacket_read_refer(req);
                mod->size = jdwppacket_read_int(req);
                mod->depth = jdwppacket_read_int(req);
                break;
            case 11:
                mod->instance = jdwppacket_read_refer(req);
                break;
            case 12:
                mod->sourceNamePattern = jdwppacket_read_utf(req);
                break;
        }
    }
    return set;
}

void jdwp_eventset_destory(EventSet *set) {
    if (set->mods) {
        s32 i;
        for (i = 0; i < set->modifiers; i++) {
            EventSetMod *mod = &set->mods[i];
            if (mod->sourceNamePattern)utf8_destory(mod->sourceNamePattern);
            if (mod->classPattern)utf8_destory(mod->classPattern);
        }
        jvm_free(set->mods);
    }
    jvm_free(set);
}

s16 jdwp_eventset_set(EventSet *set) {
    if (set) {
        switch (set->eventKind) {
            case JDWP_EVENTKIND_VM_DISCONNECTED: {
                break;
            }
            case JDWP_EVENTKIND_VM_START: {
                break;
            }
            case JDWP_EVENTKIND_THREAD_DEATH: {
                break;
            }
            case JDWP_EVENTKIND_SINGLE_STEP: {
                s32 i;
                for (i = 0; i < set->modifiers; i++) {
                    EventSetMod *mod = &set->mods[i];
                    if (10 == mod->mod_type) {
                        jdwp_set_debug_step(JDWP_EVENTSET_SET, mod->thread,
                                            mod->size, mod->depth);
                    }
                }
                break;
            }
            case JDWP_EVENTKIND_BREAKPOINT: {
                s32 i;
                for (i = 0; i < set->modifiers; i++) {
                    EventSetMod *mod = &set->mods[i];
                    if (mod->mod_type == 7) {
                        jdwp_set_breakpoint(JDWP_EVENTSET_SET, mod->loc.classID, mod->loc.methodID,
                                            mod->loc.execIndex);
                    }
                }
                break;
            }
            case JDWP_EVENTKIND_FRAME_POP: {
                break;
            }
            case JDWP_EVENTKIND_EXCEPTION: {
                break;
            }
            case JDWP_EVENTKIND_USER_DEFINED: {
                break;
            }
            case JDWP_EVENTKIND_THREAD_START: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_PREPARE: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_UNLOAD: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_LOAD: {
                break;
            }
            case JDWP_EVENTKIND_FIELD_ACCESS: {
                break;
            }
            case JDWP_EVENTKIND_FIELD_MODIFICATION: {
                break;
            }
            case JDWP_EVENTKIND_EXCEPTION_CATCH: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_ENTRY: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_EXIT: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_EXIT_WITH_RETURN_VALUE: {
                break;
            }
            case JDWP_EVENTKIND_VM_DEATH: {
                break;
            }
            default: {
                break;
            }
        }
    }
    return JDWP_ERROR_NONE;
}

s16 jdwp_eventset_clear(s32 id) {
    EventSet *set = jdwp_eventset_get(id);
    if (set) {
        switch (set->eventKind) {
            case JDWP_EVENTKIND_VM_DISCONNECTED: {
                break;
            }
            case JDWP_EVENTKIND_VM_START: {
                break;
            }
            case JDWP_EVENTKIND_THREAD_DEATH: {
                break;
            }
            case JDWP_EVENTKIND_SINGLE_STEP: {
                s32 i;
                for (i = 0; i < set->modifiers; i++) {
                    EventSetMod *mod = &set->mods[i];
                    if (10 == mod->mod_type) {
                        jdwp_set_debug_step(JDWP_EVENTSET_CLEAR, mod->thread,
                                            mod->size, mod->depth);
                    }
                }
                break;
            }
            case JDWP_EVENTKIND_BREAKPOINT: {
                s32 i;
                for (i = 0; i < set->modifiers; i++) {
                    EventSetMod *mod = &set->mods[i];
                    if (7 == mod->mod_type) {
                        jdwp_set_breakpoint(JDWP_EVENTSET_CLEAR, mod->loc.classID, mod->loc.methodID,
                                            mod->loc.execIndex);
                    }
                }
                break;
            }
            case JDWP_EVENTKIND_FRAME_POP: {
                break;
            }
            case JDWP_EVENTKIND_EXCEPTION: {
                break;
            }
            case JDWP_EVENTKIND_USER_DEFINED: {
                break;
            }
            case JDWP_EVENTKIND_THREAD_START: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_PREPARE: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_UNLOAD: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_LOAD: {
                break;
            }
            case JDWP_EVENTKIND_FIELD_ACCESS: {
                break;
            }
            case JDWP_EVENTKIND_FIELD_MODIFICATION: {
                break;
            }
            case JDWP_EVENTKIND_EXCEPTION_CATCH: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_ENTRY: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_EXIT: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_EXIT_WITH_RETURN_VALUE: {
                break;
            }
            case JDWP_EVENTKIND_VM_DEATH: {
                break;
            }
            default: {
                break;
            }
        }
    }
    hashtable_remove(jdwpserver.event_sets, (__refer) (intptr_t) id, 0);
    jdwp_eventset_destory(set);
    return JDWP_ERROR_NONE;
}

void jdwp_eventset_post(JdwpClient *client, EventSet *set, EventInfo *event) {
    if (set) {
        switch (set->eventKind) {
            case JDWP_EVENTKIND_VM_DISCONNECTED: {
                break;
            }
            case JDWP_EVENTKIND_VM_START: {
                break;
            }
            case JDWP_EVENTKIND_THREAD_DEATH: {
                break;
            }
            case JDWP_EVENTKIND_SINGLE_STEP: {
                s32 i;
                for (i = 0; i < set->modifiers; i++) {
                    EventSetMod *mod = &set->mods[i];
                    if (10 == mod->mod_type) {
                        JdwpPacket *req = jdwppacket_create();
                        jdwppacket_set_id(req, jdwp_eventset_commandid++);
                        jdwppacket_set_cmd(req, JDWP_CMD_Event_Composite);
                        jdwppacket_write_byte(req, set->suspendPolicy);
                        jdwppacket_write_int(req, 1);
                        jdwppacket_write_byte(req, set->eventKind);
                        jdwppacket_write_int(req, set->requestId);
                        jdwppacket_write_refer(req, event->thread);
                        writeLocation(req, &event->loc);
                        jdwp_writepacket(client, req);
                    }
                }
                break;
            }
            case JDWP_EVENTKIND_BREAKPOINT: {
                s32 i;
                for (i = 0; i < set->modifiers; i++) {
                    EventSetMod *mod = &set->mods[i];
                    if (7 == mod->mod_type)
                        if (location_equals(&mod->loc, &event->loc)) {
                            JdwpPacket *req = jdwppacket_create();
                            jdwppacket_set_id(req, jdwp_eventset_commandid++);
                            jdwppacket_set_cmd(req, JDWP_CMD_Event_Composite);
                            jdwppacket_write_byte(req, set->suspendPolicy);
                            jdwppacket_write_int(req, 1);
                            jdwppacket_write_byte(req, set->eventKind);
                            jdwppacket_write_int(req, set->requestId);
                            jdwppacket_write_refer(req, event->thread);
                            writeLocation(req, &event->loc);
                            jdwp_writepacket(client, req);
                        }
                }
                break;
            }
            case JDWP_EVENTKIND_FRAME_POP: {
                break;
            }
            case JDWP_EVENTKIND_EXCEPTION: {
                break;
            }
            case JDWP_EVENTKIND_USER_DEFINED: {
                break;
            }
            case JDWP_EVENTKIND_THREAD_START: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_PREPARE: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_UNLOAD: {
                break;
            }
            case JDWP_EVENTKIND_CLASS_LOAD: {
                break;
            }
            case JDWP_EVENTKIND_FIELD_ACCESS: {
                break;
            }
            case JDWP_EVENTKIND_FIELD_MODIFICATION: {
                break;
            }
            case JDWP_EVENTKIND_EXCEPTION_CATCH: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_ENTRY: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_EXIT: {
                break;
            }
            case JDWP_EVENTKIND_METHOD_EXIT_WITH_RETURN_VALUE: {
                break;
            }
            case JDWP_EVENTKIND_VM_DEATH: {
                break;
            }
            default: {
                break;
            }
        }
        //printf("send event composiet :%x\n", event->requestId);
    }
}

//==================================================    process packet    ==================================================

s32 jdwp_client_process(JdwpClient *client, Runtime *runtime) {
    JdwpPacket *req = NULL;
    while ((req = jdwp_readpacket(client)) != NULL) {

        u16 cmd = jdwppacket_get_cmd_err(req);
        //jvm_printf("jdwp receiv cmd: %x\n", cmd);
        JdwpPacket *res = jdwppacket_create();
        jdwppacket_set_flag(res, JDWP_PACKET_RESPONSE);
        jdwppacket_set_id(res, jdwppacket_get_id(req));
        switch (cmd) {
//set 1
            case JDWP_CMD_VirtualMachine_Version: {//1.1
                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                Utf8String *ustr = utf8_create();
                utf8_append_c(ustr, "MINI_JVM");
                jdwppacket_write_utf(res, ustr);
                jdwppacket_write_int(res, 1);
                jdwppacket_write_int(res, 0);
                utf8_clear(ustr);
                utf8_append_c(ustr, "0.1");
                jdwppacket_write_utf(res, ustr);
                utf8_clear(ustr);
                utf8_append_c(ustr, "Mini jvm");
                jdwppacket_write_utf(res, ustr);
                jdwp_writepacket(client, res);
                utf8_destory(ustr);
                break;
            }
            case JDWP_CMD_VirtualMachine_ClassesBySignature: {//1.2
                Utf8String *signature = jdwppacket_read_utf(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                signatureToName(signature);
                JClass *cl = classes_load_get(signature, runtime);
                if (cl == NULL) {
                    jdwppacket_write_int(res, 0);
                } else {
                    jdwppacket_write_int(res, 1);
                    jdwppacket_write_byte(res, getClassType(cl));
                    jdwppacket_write_refer(res, cl);
                    jdwppacket_write_int(res, getClassStatus(cl));
                }
                //jvm_printf("VirtualMachine_ClassesBySignature:%s ,%lld\n", utf8_cstr(signature), (s64) (intptr_t) cl);
                jdwp_writepacket(client, res);
                utf8_destory(signature);
                break;
            }
            case JDWP_CMD_VirtualMachine_AllClasses: {//1.3
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_int(res, (s32) sys_classloader->classes->entries);

                Utf8String *ustr = utf8_create();
                HashtableIterator hti;
                hashtable_iterate(sys_classloader->classes, &hti);
                for (; hashtable_iter_has_more(&hti);) {
                    Utf8String *k = hashtable_iter_next_key(&hti);
                    JClass *cl = hashtable_get(sys_classloader->classes, k);

                    jdwppacket_write_byte(res, getClassType(cl));
                    jdwppacket_write_refer(res, cl);
                    utf8_clear(ustr);
                    utf8_append(ustr, cl->name);
                    nameToSignature(ustr);
                    jdwppacket_write_utf(res, ustr);
                    jdwppacket_write_int(res, getClassStatus(cl));
                }
                utf8_destory(ustr);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_AllThreads: {//1.4
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_int(res, thread_list->length);
                s32 i;
                for (i = 0; i < thread_list->length; i++) {
                    Runtime *t = threadlist_get(i);
                    if (t) {
                        jdwppacket_write_refer(res, t->threadInfo->jthread);
                        //jvm_printf("VirtualMachine_AllThreads: %llx\n", (s64) (intptr_t) t);
                        Instance *jarr_name = jthread_get_name_value(t->threadInfo->jthread);
                        Utf8String *ustr = utf8_create();
                        unicode_2_utf8((u16 *) jarr_name->arr_body, ustr, jarr_name->arr_length);
                        //printf("%s\n", utf8_cstr(ustr));
                        utf8_destory(ustr);
                    }
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_TopLevelThreadGroups: {//1.5

                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwppacket_write_int(res, 0);
                //jdwppacket_write_refer(res, 0);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_Dispose: {//1.6

                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwp_writepacket(client, res);
                client->closed = 1;
                break;
            }
            case JDWP_CMD_VirtualMachine_IDSizes: {//1.7

                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwppacket_write_int(res, sizeof(__refer));
                jdwppacket_write_int(res, sizeof(__refer));
                jdwppacket_write_int(res, sizeof(__refer));
                jdwppacket_write_int(res, sizeof(__refer));
                jdwppacket_write_int(res, sizeof(__refer));
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_Suspend: {//1.8
                s32 i;
                for (i = 0; i < thread_list->length; i++) {
                    Runtime *t = threadlist_get(i);
                    if (t)jthread_suspend(t);
                    //jvm_printf("VirtualMachine_Suspend: %lld\n" + (s64) (intptr_t) t);
                }
                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_Resume: {//1.9
                s32 i;
                for (i = 0; i < thread_list->length; i++) {
                    Runtime *t = threadlist_get(i);
                    if (t)jthread_resume(t);
                    //jvm_printf("VirtualMachine_Resume: %llx\n", (s64) (intptr_t) t);
                }
                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_Exit: {//1.10
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_VirtualMachine_CreateString: {//1.11
                Utf8String *str = jdwppacket_read_utf(req);
                Instance *jstr = jstring_create(str, runtime);
                jdwp_client_hold_obj(client, runtime, jstr);//防止回收此处需要hold
                utf8_destory(str);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_refer(res, jstr);
                //jvm_printf("VirtualMachine_CreateString: %s , rid: %llx\n", utf8_cstr(str), (s64) (intptr_t) jstr);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_Capabilities: {//1.12
//                jdwppacket_set_err(res, JDWP_ERROR_NONE);
//                s32 i;
//                for (i = 0; i < 7; i++) {
//                    jdwppacket_write_byte(res, 0);
//                }
//                jdwp_writepacket(client, res);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_byte(res, 0);//canWatchFieldModification
                jdwppacket_write_byte(res, 0);//canWatchFieldAccess
                jdwppacket_write_byte(res, 0);//canGetBytecodes
                jdwppacket_write_byte(res, 0);//canGetSyntheticAttribute
                jdwppacket_write_byte(res, 0);//canGetOwnedMonitorInfo
                jdwppacket_write_byte(res, 0);//canGetCurrentContendedMonitor
                jdwppacket_write_byte(res, 0);//canGetMonitorInfo
                //jvm_printf("VirtualMachine_Capabilities");
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_ClassPaths: {//1.13
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_VirtualMachine_DisposeObjects: {//1.14
                s32 requests = jdwppacket_read_int(req);
                s32 i;
                for (i = 0; i < requests; i++) {
                    __refer ins = jdwppacket_read_refer(req);
                    memoryblock_destory(ins);
                    s32 count = jdwppacket_read_int(req);
                    jdwp_client_release_obj(client, ins);//release obj
                }
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_HoldEvents: {//1.15
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_VirtualMachine_ReleaseEvents: {//1.16
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_VirtualMachine_CapabilitiesNew: {//1.17
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                s32 i;
                for (i = 0; i < 32; i++) {
                    jdwppacket_write_byte(res, 0);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_VirtualMachine_RedefineClasses: {//1.18
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_VirtualMachine_SetDefaultStratum: {//1.19
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_VirtualMachine_AllClassesWithGeneric: {//1.20
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_int(res, (s32) sys_classloader->classes->entries);

                Utf8String *ustr = utf8_create();
                HashtableIterator hti;
                hashtable_iterate(sys_classloader->classes, &hti);
                for (; hashtable_iter_has_more(&hti);) {
                    Utf8String *k = hashtable_iter_next_key(&hti);
                    JClass *cl = hashtable_get(sys_classloader->classes, k);

                    jdwppacket_write_byte(res, getClassType(cl));
                    jdwppacket_write_refer(res, cl);
                    utf8_clear(ustr);
                    utf8_append(ustr, cl->name);
                    nameToSignature(ustr);
                    jdwppacket_write_utf(res, ustr);
                    utf8_clear(ustr);
                    jdwppacket_write_utf(res, ustr);
                    jdwppacket_write_int(res, getClassStatus(cl));
                }
                utf8_destory(ustr);
                jdwp_writepacket(client, res);
                break;
            }
//set 2

            case JDWP_CMD_ReferenceType_Signature: {//2.1
                JClass *ref = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                Utf8String *str = utf8_create();
                getClassSignature(ref, str);
                jdwppacket_write_utf(res, str);
                jdwp_writepacket(client, res);
                //jvm_printf("ReferenceType_Signature:%llx , %s \n", (s64) (intptr_t) ref, utf8_cstr(str));
                utf8_destory(str);
                break;
            }
            case JDWP_CMD_ReferenceType_ClassLoader: {//2.2
                JClass *ref = jdwppacket_read_refer(req);

                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwppacket_write_refer(res, 0);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_Modifiers: {//2.3
                JClass *ref = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwppacket_write_int(res, ref->cff.access_flags);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_Fields: {//2.4
                JClass *ref = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                s32 len = ref->fieldPool.field_used;
                jdwppacket_write_int(res, len);
                s32 i;
                for (i = 0; i < len; i++) {
                    ////jvm_printf("method[" + i + "]" + ref.methods[i]);
                    jdwppacket_write_refer(res, &ref->fieldPool.field[i]);
                    jdwppacket_write_utf(res, ref->fieldPool.field[i].name);
                    jdwppacket_write_utf(res, ref->fieldPool.field[i].descriptor);
                    jdwppacket_write_int(res, ref->fieldPool.field[i].access_flags);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_Methods: {//2.5
                JClass *ref = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                s32 len = ref->methodPool.method_used;
                jdwppacket_write_int(res, len);
                s32 i;
                for (i = 0; i < len; i++) {
                    ////jvm_printf("method[" + i + "]" + ref.methods[i]);
                    jdwppacket_write_refer(res, &ref->methodPool.method[i]);
                    jdwppacket_write_utf(res, ref->methodPool.method[i].name);
                    jdwppacket_write_utf(res, ref->methodPool.method[i].descriptor);
                    jdwppacket_write_int(res, ref->methodPool.method[i].access_flags);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_GetValues: {//2.6
                JClass *ref = jdwppacket_read_refer(req);
                s32 fields = jdwppacket_read_int(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_int(res, fields);
                s32 i;
                for (i = 0; i < fields; i++) {
                    FieldInfo *fi = jdwppacket_read_refer(req);
                    ValueType vt;
                    vt.type = getJdwpTag(fi->descriptor);
                    c8 *ptr = getFieldPtr_byName(NULL, ref->name, fi->name, fi->descriptor, runtime);
                    vt.value = getPtrValue(vt.type, ptr);
                    writeValueType(res, &vt);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_SourceFile: {//2.7
                JClass *ref = jdwppacket_read_refer(req);

                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwppacket_write_utf(res, ref->source);
                //jvm_printf("ReferenceType_SourceFile:%s\n", utf8_cstr(ref->source));
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_NestedTypes: {//2.8
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_ReferenceType_Status: {//2.9
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_ReferenceType_Interfaces: {//2.10
                JClass *ref = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                s32 len = ref->interfacePool.clasz_used;
                jdwppacket_write_int(res, len);
                s32 i;
                for (i = 0; i < len; i++) {
                    JClass *cl = classes_load_get(ref->interfacePool.clasz[i].name, runtime);
                    jdwppacket_write_refer(res, cl);
                }
                //jvm_printf("ReferenceType_Interfaces:%s\n", utf8_cstr(ref->name));
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_ClassObject: {//2.11
                JClass *ref = jdwppacket_read_refer(req);

                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwppacket_write_refer(res, ref);
                //jvm_printf("ReferenceType_ClassObject:%s\n", utf8_cstr(ref->name));
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_SourceDebugExtension: {//2.12
                jdwppacket_set_err(res, JDWP_ERROR_ABSENT_INFORMATION);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_SignatureWithGeneric: {//2.13
                JClass *ref = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                Utf8String *str = utf8_create();
                getClassSignature(ref, str);
                jdwppacket_write_utf(res, str);
                utf8_clear(str);
                jdwppacket_write_utf(res, str);
                jdwp_writepacket(client, res);
                //jvm_printf("JDWP_CMD_ReferenceType_SignatureWithGeneric:%llx , %s \n", (s64) (intptr_t) ref, utf8_cstr(str));
                utf8_destory(str);
                break;
            }
            case JDWP_CMD_ReferenceType_FieldsWithGeneric: {//2.14
                JClass *ref = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                s32 len = ref->fieldPool.field_used;
                jdwppacket_write_int(res, len);
                Utf8String *ustr = utf8_create();
                s32 i;
                for (i = 0; i < len; i++) {
                    jdwppacket_write_refer(res, &ref->fieldPool.field[i]);
                    jdwppacket_write_utf(res, ref->fieldPool.field[i].name);
                    jdwppacket_write_utf(res, ref->fieldPool.field[i].descriptor);
                    jdwppacket_write_utf(res, ustr);
                    jdwppacket_write_int(res, ref->fieldPool.field[i].access_flags);
                }
                utf8_destory(ustr);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ReferenceType_MethodsWithGeneric: {//2.15
                JClass *ref = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                s32 len = ref->methodPool.method_used;
                jdwppacket_write_int(res, len);
                Utf8String *ustr = utf8_create();
                s32 i;
                for (i = 0; i < len; i++) {
                    jdwppacket_write_refer(res, &ref->methodPool.method[i]);
                    jdwppacket_write_utf(res, ref->methodPool.method[i].name);
                    jdwppacket_write_utf(res, ref->methodPool.method[i].descriptor);
                    jdwppacket_write_utf(res, ustr);
                    jdwppacket_write_int(res, ref->methodPool.method[i].access_flags);
                }
                utf8_destory(ustr);
                jdwp_writepacket(client, res);
                break;
            }
//set 3

            case JDWP_CMD_ClassType_Superclass: {//3.1
                __refer refType = jdwppacket_read_refer(req);
                JClass *ref = (JClass *) (refType);

                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwppacket_write_refer(res, getSuperClass(ref));
                //jvm_printf("JDWP_CMD_ClassType_Superclass:%s\n", utf8_cstr(ref->name));
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ClassType_SetValues: {//3.2
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_ClassType_InvokeMethod: {//3.3
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_ClassType_NewInstance: {//3.4
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
//set 4

            case JDWP_CMD_ArrayType_NewInstance: {//4.1
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
//set 5
//set 6

            case JDWP_CMD_Method_LineTable: {//6.1
                __refer refType = jdwppacket_read_refer(req);
                JClass *ref = (JClass *) (refType);
                MethodInfo *method = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                CodeAttribute *ca = getCodeAttribute(method);
                if (method->access_flags & ACC_NATIVE) {
                    jdwppacket_write_long(res, -1);
                    jdwppacket_write_long(res, -1);
                } else {
                    jdwppacket_write_long(res, 0);
                    jdwppacket_write_long(res, ca->code_length);
                }
                jdwppacket_write_int(res, ca->line_number_table_length);
                s32 i;
                for (i = 0; i < ca->line_number_table_length; i++) {
                    jdwppacket_write_long(res, ca->line_number_table[i].start_pc);
                    jdwppacket_write_int(res, ca->line_number_table[i].line_number);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_Method_VariableTable: {//6.2
                __refer refType = jdwppacket_read_refer(req);
                JClass *ref = (JClass *) (refType);
                MethodInfo *method = jdwppacket_read_refer(req);
                CodeAttribute *ca = getCodeAttribute(method);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_int(res, method->para_slots);  //slot count
                jdwppacket_write_int(res, ca->local_var_table_length);// para count
                s32 i;
                for (i = 0; i < ca->local_var_table_length; i++) {
                    LocalVarTable *tab = &ca->local_var_table[i];
                    jdwppacket_write_long(res, tab->start_pc);
                    jdwppacket_write_utf(res, class_get_utf8_string(ref, tab->name_index));
                    jdwppacket_write_utf(res, class_get_utf8_string(ref, tab->descriptor_index));
                    jdwppacket_write_int(res, tab->length);
                    jdwppacket_write_int(res, tab->index);
                }
                //jvm_printf("Method_VariableTable:\n");
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_Method_Bytecodes: {//6.3
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_Method_IsObsolete: {//6.4
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_Method_VariableTableWithGeneric: {//6.5
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
//set 8
//set 9

            case JDWP_CMD_ObjectReference_ReferenceType: {//9.1
                Instance *obj = (Instance *) jdwppacket_read_refer(req);
                JClass *ref = obj->mb.clazz;
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_byte(res, getClassType(ref));
                if (obj->mb.type == MEM_TYPE_CLASS) {//类对象
                    ref = classes_load_get_c(STR_CLASS_JAVA_LANG_CLASS, runtime);
                }
                jdwppacket_write_refer(res, ref);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ObjectReference_GetValues: {//9.2
                Instance *obj = (Instance *) jdwppacket_read_refer(req);
                JClass *ref = obj->mb.clazz;
                s32 fields = jdwppacket_read_int(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_int(res, fields);
                s32 i;
                for (i = 0; i < fields; i++) {
                    FieldInfo *fi = jdwppacket_read_refer(req);
                    ValueType vt;
                    vt.type = getJdwpTag(fi->descriptor);
                    c8 *ptr = getFieldPtr_byName(obj, obj->mb.clazz->name, fi->name, fi->descriptor, runtime);
                    vt.value = getPtrValue(vt.type, ptr);
                    writeValueType(res, &vt);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ObjectReference_SetValues: {//9.3
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_ObjectReference_MonitorInfo: {//9.5
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_ObjectReference_InvokeMethod: {//9.6
                Instance *object = jdwppacket_read_refer(req);
                Instance *thread = jdwppacket_read_refer(req);
                JClass *clazz = jdwppacket_read_refer(req);
                MethodInfo *methodInfo = jdwppacket_read_refer(req);
                s32 arguments = jdwppacket_read_int(req);

                s32 stacksize = stack_size(runtime->stack);
                if (!(methodInfo->access_flags & ACC_STATIC)) {
                    push_ref(runtime->stack, object);
                }
                runtime->clazz = clazz;
                Long2Double l2d;
                s32 i;
                for (i = 0; i < arguments; i++) {
                    ValueType vt;
                    readValueType(req, &vt);

                    switch (getSimpleTag(vt.type)) {
                        case '8':
                            push_long(runtime->stack, vt.value);
                            break;
                        case 'R':
                            push_ref(runtime->stack, (__refer) (intptr_t) vt.value);
                            break;
                        default:
                            push_int(runtime->stack, (s32) vt.value);
                    }
                }
                s32 ret = execute_method_impl(methodInfo, runtime);
                if (ret != RUNTIME_STATUS_NORMAL) {
                    print_exception(runtime);
                }
                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                ValueType vt;
                memset(&vt, 0, sizeof(ValueType));
                if (stack_size(runtime->stack) > stacksize) {
                    Utf8String *us = utf8_create_copy(methodInfo->descriptor);
                    utf8_substring(us, utf8_indexof_c(us, ")") + 1, us->length);
                    vt.type = getJdwpTag(us);
                    switch (getSimpleTag(vt.type)) {
                        case '8':
                            vt.value = pop_long(runtime->stack);
                            break;
                        case 'R': {
                            __refer r = pop_ref(runtime->stack);
                            vt.type = getInstanceOfClassTag(r);//recorrect type, may be Arraylist<String>
                            vt.value = (s64) (intptr_t) r;
                            jdwp_client_hold_obj(client, runtime, r);

//                            if (vt.type == 's') {
//                                s32 debug = 1;
//                                Utf8String *ustr = utf8_create();
//                                jstring_2_utf8((Instance *) r, ustr);
//                                utf8_destory(ustr);
//                            }
                            break;
                        }
                        default:
                            vt.value = pop_int(runtime->stack);
                    }
                    utf8_destory(us);
                }
                writeValueType(res, &vt);
                vt.type = 'L';
                vt.value = 0;
                writeValueType(res, &vt);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ObjectReference_DisableCollection: {//9.7
                Instance *obj = (Instance *) jdwppacket_read_refer(req);
                jdwp_client_hold_obj(client, runtime, obj);

                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwp_writepacket(client, res);
//                //jvm_printf("ObjectReference_DisableCollection:" + obj);
                break;
            }
            case JDWP_CMD_ObjectReference_EnableCollection: {//9.8
                Instance *obj = (Instance *) jdwppacket_read_refer(req);
                //Eclipse have a nonhuman opration that access a object after enable collection the same one
                //so this garbage_derefer may be release the object
//                garbage_refer_count_dec(obj);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwp_writepacket(client, res);
//                //jvm_printf("ObjectReference_EnableCollection:" + obj);
                break;
            }
            case JDWP_CMD_ObjectReference_IsCollected: {//9.9
                Instance *obj = (Instance *) jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_byte(res, !gc_is_alive(obj));
                jdwp_writepacket(client, res);
                break;
            }
//set 10

            case JDWP_CMD_StringReference_Value: {//10.1
                Instance *jstr = jdwppacket_read_refer(req);
                Utf8String *ustr = utf8_create();
                jstring_2_utf8(jstr, ustr);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwppacket_write_utf(res, ustr);
                jdwp_writepacket(client, res);
                //jvm_printf("ThreadReference_Name:%s\n", utf8_cstr(ustr));
                utf8_destory(ustr);
                break;
            }
//set 11

            case JDWP_CMD_ThreadReference_Name: {//11.1
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    Instance *jarr_name = jthread_get_name_value(jthread);
                    Utf8String *ustr = utf8_create();
                    unicode_2_utf8((u16 *) jarr_name->arr_body, ustr, jarr_name->arr_length);

                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_utf(res, ustr);
                    utf8_destory(ustr);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                //jvm_printf("ThreadReference_Name:%s\n", utf8_cstr(ustr));
                break;
            }
            case JDWP_CMD_ThreadReference_Suspend: {//11.2
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    jthread_suspend(r);
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_Resume: {//11.3
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    if (r->threadInfo->suspend_count > 0)jthread_resume(r);;
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_Status: {//11.4
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_int(res, r->threadInfo->thread_status);
                    jdwppacket_write_int(res, r->threadInfo->is_suspend || r->threadInfo->is_blocking
                                              ? JDWP_SUSPEND_STATUS_SUSPENDED : 0);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }

            case JDWP_CMD_ThreadReference_ThreadGroup: {//11.5
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_refer(res, 0);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_Frames: {//11.6
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *rt = getRuntimeOfThread(jthread);
                s32 startFrame = jdwppacket_read_int(req);
                s32 length = jdwppacket_read_int(req);
                if (rt) {
                    //jvm_printf("ThreadReference_Frames: startFrame=%d, len=%d\n", startFrame, length);
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    s32 deepth = getRuntimeDepth(rt);
                    if (length == -1) {//等于-1返回所有剩下的
                        length = deepth - startFrame;
                    }
                    jdwppacket_write_int(res, length);
                    Runtime *r = getLastSon(rt);
                    ////jvm_printf("deepth:" + frame.getDeepth());
                    s32 i;
                    for (i = 0; i < deepth; i++) {
                        if (i >= startFrame && i < startFrame + length) {//返回指定层级的stackframe
                            jdwppacket_write_refer(res, r);
                            Location loc;
                            loc.typeTag = getClassType(r->clazz);
                            loc.classID = r->clazz;
                            loc.methodID = r->method;
                            if (r->ca)
                                loc.execIndex = (s64) (intptr_t) r->pc - (s64) (intptr_t) r->ca->code;
                            else
                                loc.execIndex = 0;
                            writeLocation(res, &loc);
                        }
                        r = r->parent;
                        if (r == NULL) {
                            break;
                        }
                    }
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_FrameCount: {//11.7
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_int(res, getRuntimeDepth(r));
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                //jvm_printf("ThreadReference_FrameCount:%d\n", getRuntimeDepth(r));
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_OwnedMonitors: {//11.8
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_int(res, 0);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_CurrentContendedMonitor: {//11.9
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_refer(res, NULL);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_Stop: {//11.10
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    thrd_detach(r->threadInfo->pthread);//todo need release all lock
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_Interrupt: {//11.11
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    //todo
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ThreadReference_SuspendCount: {//11.12
                Instance *jthread = jdwppacket_read_refer(req);
                Runtime *r = getRuntimeOfThread(jthread);
                if (r) {
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_int(res, r->threadInfo->suspend_count);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_THREAD);
                }
                //jvm_printf("ThreadReference_SuspendCount:%llx,%d\n", (s64) (intptr_t) jthread,r->threadInfo->suspend_count);
                jdwp_writepacket(client, res);
                break;
            }
                //set 12

            case JDWP_CMD_ThreadGroupReference_Name: {//12.1
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_ThreadGroupReference_Parent: {//12.2
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
            case JDWP_CMD_ThreadGroupReference_Children: {//12.3
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
//set 13
            case JDWP_CMD_ArrayReference_Length: {//13.1
                Instance *arr = jdwppacket_read_refer(req);
                if (gc_is_alive(arr)) {
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_int(res, arr->arr_length);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_ARRAY);
                }
                //jvm_printf("ArrayReference_Length:%d\n", arr->arr_length);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_ArrayReference_GetValues: {//13.2
                Instance *arr = jdwppacket_read_refer(req);
                if (gc_is_alive(arr)) {
                    s32 firstIndex = jdwppacket_read_int(req);
                    s32 length = jdwppacket_read_int(req);
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    writeArrayRegion(res, arr, firstIndex, length);
                } else {
                    jdwppacket_set_err(res, JDWP_ERROR_INVALID_ARRAY);
                }
                jdwp_writepacket(client, res);
                //jvm_printf("ArrayReference_GetValues:%llx\n", arr);
                break;
            }
            case JDWP_CMD_ArrayReference_SetValues: {//13.3
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
//set 14
            case JDWP_CMD_ClassLoaderReference_VisibleClasses: {//14.1
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
//set 15
            case JDWP_CMD_EventRequest_Set: {//15.1
                EventSet *eventSet = jdwp_create_eventset(req);
                jdwp_eventset_put(eventSet);
                s16 ret = jdwp_eventset_set(eventSet);


                if (ret == JDWP_ERROR_NONE) {
                    jdwppacket_set_err(res, JDWP_ERROR_NONE);
                    jdwppacket_write_int(res, eventSet->requestId);
                } else {
                    jdwppacket_set_err(res, ret);
                }
                jdwp_writepacket(client, res);
                break;
            }//
            case JDWP_CMD_EventRequest_Clear: {//15.2
                u8 eventKind = jdwppacket_read_byte(req);
                s32 requestID = jdwppacket_read_int(req);
                //jvm_printf("EventRequest_Clear:eventKind=%d, requestID=%d\n", eventKind, requestID);
                jdwp_eventset_clear(requestID);


                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_EventRequest_ClearAllBreakpoints: {//15.3`


                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwp_writepacket(client, res);
                break;
            }
//set 16
            case JDWP_CMD_StackFrame_GetValues: {//16.1
                Instance *thread = jdwppacket_read_refer(req);
                Runtime *frame = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                s32 slots = jdwppacket_read_int(req);
                jdwppacket_write_int(res, slots);
                Long2Double l2d;
                s32 i;
                for (i = 0; i < slots; i++) {
                    s32 slot = jdwppacket_read_int(req);
                    ValueType vt;
                    vt.type = jdwppacket_read_byte(req);
                    if (slot < frame->ca->max_locals) {
                        switch (getSimpleTag(vt.type)) {
                            case 'R': {
                                Instance *ins = localvar_getRefer(frame->localvar, slot);
                                vt.type = getInstanceOfClassTag(ins);
                                vt.value = (s64) (intptr_t) ins;
                                break;
                            }
                            case '8':
                                l2d.l = localvar_getLong(frame->localvar, slot);
//                                l2d.i2l.i1 = localvar_getInt(frame->localvar, slot);
//                                l2d.i2l.i0 = localvar_getInt(frame->localvar, slot + 1);
                                vt.value = l2d.l;
                                i++;
                                break;
                            case '4':
                            case '2':
                            case '1':
                                vt.value = localvar_getInt(frame->localvar, slot);
                                break;
                        }
                    }
                    writeValueType(res, &vt);
                    //jvm_printf("JDWP_CMD_StackFrame_GetValues,thead=%llx , frame=%llx, val=%llx\n", (s64) (intptr_t) thread,
                    //(s64) (intptr_t) frame, vt.value);
                }
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_StackFrame_SetValues: {//16.2
                Instance *thread = jdwppacket_read_refer(req);
                Runtime *frame = jdwppacket_read_refer(req);

                s32 slotValues = jdwppacket_read_int(req);
                Long2Double l2d;
                s32 i;
                for (i = 0; i < slotValues; i++) {
                    s32 slot = jdwppacket_read_int(req);
                    ValueType vt;
                    readValueType(req, &vt);
                    if (slot < frame->ca->max_locals) {
                        switch (getSimpleTag(vt.type)) {
                            case 'R':
                                localvar_setRefer(frame->localvar, slot, (__refer) (intptr_t) vt.value);
                                break;
                            case '8':
                                l2d.l = vt.value;
                                localvar_setLong(frame->localvar, slot, l2d.l);
//                                localvar_setInt(frame->localvar, slot, l2d.i2l.i0);
//                                localvar_setInt(frame->localvar, slot + 1, l2d.i2l.i1);
                                i++;
                                break;
                            case '4':
                            case '2':
                            case '1':
                                localvar_setInt(frame->localvar, slot, (s32) vt.value);
                                break;
                        }
                    }

                    //jvm_printf("StackFrame_SetValues,thead=%llx , frame=%llx, val=%llx\n", (s64) (intptr_t) thread,
                    //(s64) (intptr_t) frame, vt.value);
                }


                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                jdwp_writepacket(client, res);
                break;
            }
            case JDWP_CMD_StackFrame_ThisObject: {//16.3
                Instance *thread = jdwppacket_read_refer(req);
                Runtime *frame = jdwppacket_read_refer(req);
                jdwppacket_set_err(res, JDWP_ERROR_NONE);
                ValueType vt;
                if (frame->method->access_flags & ACC_STATIC || frame->method->access_flags & ACC_NATIVE) {
                    vt.type = JDWP_TAG_OBJECT;
                    vt.value = 0;
                } else {
                    Instance *ins = localvar_getRefer(frame->localvar, 0);
                    vt.type = getInstanceOfClassTag(ins);
                    vt.value = (s64) (intptr_t) ins;
                }
                writeValueType(res, &vt);
                jdwp_writepacket(client, res);
                //jvm_printf("StackFrame_ThisObject,thead=%llx , frame=%llx \n", (s64) (intptr_t) thread,
                //(s64) (intptr_t) frame);
                break;
            }
            case JDWP_CMD_StackFrame_PopFrames: {//16.4
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
//set 17
            case JDWP_CMD_ClassObjectReference_ReflectedType: {//17.1
                JClass *classObject = jdwppacket_read_refer(req);

                jdwppacket_set_err(res, JDWP_ERROR_NONE);

                jdwppacket_write_byte(res, getClassType(classObject));
                jdwppacket_write_refer(res, classObject);
                //jvm_printf("ClassObjectReference_ReflectedType:%s\n", utf8_cstr(classObject->mb.clazz->name));
                jdwp_writepacket(client, res);
                break;
            }
//set 64
            case JDWP_CMD_Event_Composite: {
                jvm_printf("%x not support\n", jdwppacket_get_cmd_err(req));
                break;
            }
        }
        jdwppacket_destory(req);
    }
    return 0;
}

