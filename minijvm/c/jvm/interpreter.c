

#include "../utils/d_type.h"
#include "jvm.h"
#include "java_native_std.h"
#include "jvm_util.h"
#include "garbage.h"


/* ==================================opcode implementation =============================*/



enum {
    /* 0x00 */ op_nop,
    /* 0x01 */ op_aconst_null,
    /* 0x02 */ op_iconst_m1,
    /* 0x03 */ op_iconst_0,
    /* 0x04 */ op_iconst_1,
    /* 0x05 */ op_iconst_2,
    /* 0x06 */ op_iconst_3,
    /* 0x07 */ op_iconst_4,
    /* 0x08 */ op_iconst_5,
    /* 0x09 */ op_lconst_0,
    /* 0x0A */ op_lconst_1,
    /* 0x0B */ op_fconst_0,
    /* 0x0C */ op_fconst_1,
    /* 0x0D */ op_fconst_2,
    /* 0x0E */ op_dconst_0,
    /* 0x0F */ op_dconst_1,
    /* 0x10 */ op_bipush,
    /* 0x11 */ op_sipush,
    /* 0x12 */ op_ldc,
    /* 0x13 */ op_ldc_w,
    /* 0x14 */ op_ldc2_w,
    /* 0x15 */ op_iload,
    /* 0x16 */ op_lload,
    /* 0x17 */ op_fload,
    /* 0x18 */ op_dload,
    /* 0x19 */ op_aload,
    /* 0x1A */ op_iload_0,
    /* 0x1B */ op_iload_1,
    /* 0x1C */ op_iload_2,
    /* 0x1D */ op_iload_3,
    /* 0x1E */ op_lload_0,
    /* 0x1F */ op_lload_1,
    /* 0x20 */ op_lload_2,
    /* 0x21 */ op_lload_3,
    /* 0x22 */ op_fload_0,
    /* 0x23 */ op_fload_1,
    /* 0x24 */ op_fload_2,
    /* 0x25 */ op_fload_3,
    /* 0x26 */ op_dload_0,
    /* 0x27 */ op_dload_1,
    /* 0x28 */ op_dload_2,
    /* 0x29 */ op_dload_3,
    /* 0x2A */ op_aload_0,
    /* 0x2B */ op_aload_1,
    /* 0x2C */ op_aload_2,
    /* 0x2D */ op_aload_3,
    /* 0x2E */ op_iaload,
    /* 0x2F */ op_laload,
    /* 0x30 */ op_faload,
    /* 0x31 */ op_daload,
    /* 0x32 */ op_aaload,
    /* 0x33 */ op_baload,
    /* 0x34 */ op_caload,
    /* 0x35 */ op_saload,
    /* 0x36 */ op_istore,
    /* 0x37 */ op_lstore,
    /* 0x38 */ op_fstore,
    /* 0x39 */ op_dstore,
    /* 0x3A */ op_astore,
    /* 0x3B */ op_istore_0,
    /* 0x3C */ op_istore_1,
    /* 0x3D */ op_istore_2,
    /* 0x3E */ op_istore_3,
    /* 0x3F */ op_lstore_0,
    /* 0x40 */ op_lstore_1,
    /* 0x41 */ op_lstore_2,
    /* 0x42 */ op_lstore_3,
    /* 0x43 */ op_fstore_0,
    /* 0x44 */ op_fstore_1,
    /* 0x45 */ op_fstore_2,
    /* 0x46 */ op_fstore_3,
    /* 0x47 */ op_dstore_0,
    /* 0x48 */ op_dstore_1,
    /* 0x49 */ op_dstore_2,
    /* 0x4A */ op_dstore_3,
    /* 0x4B */ op_astore_0,
    /* 0x4C */ op_astore_1,
    /* 0x4D */ op_astore_2,
    /* 0x4E */ op_astore_3,
    /* 0x4F */ op_iastore,
    /* 0x50 */ op_lastore,
    /* 0x51 */ op_fastore,
    /* 0x52 */ op_dastore,
    /* 0x53 */ op_aastore,
    /* 0x54 */ op_bastore,
    /* 0x55 */ op_castore,
    /* 0x56 */ op_sastore,
    /* 0x57 */ op_pop,
    /* 0x58 */ op_pop2,
    /* 0x59 */ op_dup,
    /* 0x5A */ op_dup_x1,
    /* 0x5B */ op_dup_x2,
    /* 0x5C */ op_dup2,
    /* 0x5D */ op_dup2_x1,
    /* 0x5E */ op_dup2_x2,
    /* 0x5F */ op_swap,
    /* 0x60 */ op_iadd,
    /* 0x61 */ op_ladd,
    /* 0x62 */ op_fadd,
    /* 0x63 */ op_dadd,
    /* 0x64 */ op_isub,
    /* 0x65 */ op_lsub,
    /* 0x66 */ op_fsub,
    /* 0x67 */ op_dsub,
    /* 0x68 */ op_imul,
    /* 0x69 */ op_lmul,
    /* 0x6A */ op_fmul,
    /* 0x6B */ op_dmul,
    /* 0x6C */ op_idiv,
    /* 0x6D */ op_ldiv,
    /* 0x6E */ op_fdiv,
    /* 0x6F */ op_ddiv,
    /* 0x70 */ op_irem,
    /* 0x71 */ op_lrem,
    /* 0x72 */ op_frem,
    /* 0x73 */ op_drem,
    /* 0x74 */ op_ineg,
    /* 0x75 */ op_lneg,
    /* 0x76 */ op_fneg,
    /* 0x77 */ op_dneg,
    /* 0x78 */ op_ishl,
    /* 0x79 */ op_lshl,
    /* 0x7A */ op_ishr,
    /* 0x7B */ op_lshr,
    /* 0x7C */ op_iushr,
    /* 0x7D */ op_lushr,
    /* 0x7E */ op_iand,
    /* 0x7F */ op_land,
    /* 0x80 */ op_ior,
    /* 0x81 */ op_lor,
    /* 0x82 */ op_ixor,
    /* 0x83 */ op_lxor,
    /* 0x84 */ op_iinc,
    /* 0x85 */ op_i2l,
    /* 0x86 */ op_i2f,
    /* 0x87 */ op_i2d,
    /* 0x88 */ op_l2i,
    /* 0x89 */ op_l2f,
    /* 0x8A */ op_l2d,
    /* 0x8B */ op_f2i,
    /* 0x8C */ op_f2l,
    /* 0x8D */ op_f2d,
    /* 0x8E */ op_d2i,
    /* 0x8F */ op_d2l,
    /* 0x90 */ op_d2f,
    /* 0x91 */ op_i2b,
    /* 0x92 */ op_i2c,
    /* 0x93 */ op_i2s,
    /* 0x94 */ op_lcmp,
    /* 0x95 */ op_fcmpl,
    /* 0x96 */ op_fcmpg,
    /* 0x97 */ op_dcmpl,
    /* 0x98 */ op_dcmpg,
    /* 0x99 */ op_ifeq,
    /* 0x9A */ op_ifne,
    /* 0x9B */ op_iflt,
    /* 0x9C */ op_ifge,
    /* 0x9D */ op_ifgt,
    /* 0x9E */ op_ifle,
    /* 0x9F */ op_if_icmpeq,
    /* 0xA0 */ op_if_icmpne,
    /* 0xA1 */ op_if_icmplt,
    /* 0xA2 */ op_if_icmpge,
    /* 0xA3 */ op_if_icmpgt,
    /* 0xA4 */ op_if_icmple,
    /* 0xA5 */ op_if_acmpeq,
    /* 0xA6 */ op_if_acmpne,
    /* 0xA7 */ op_goto,
    /* 0xA8 */ op_jsr,
    /* 0xA9 */ op_ret,
    /* 0xAA */ op_tableswitch,
    /* 0xAB */ op_lookupswitch,
    /* 0xAC */ op_ireturn,
    /* 0xAD */ op_lreturn,
    /* 0xAE */ op_freturn,
    /* 0xAF */ op_dreturn,
    /* 0xB0 */ op_areturn,
    /* 0xB1 */ op_return,
    /* 0xB2 */ op_getstatic,
    /* 0xB3 */ op_putstatic,
    /* 0xB4 */ op_getfield,
    /* 0xB5 */ op_putfield,
    /* 0xB6 */ op_invokevirtual,
    /* 0xB7 */ op_invokespecial,
    /* 0xB8 */ op_invokestatic,
    /* 0xB9 */ op_invokeinterface,
    /* 0xBA */ op_invokedynamic,
    /* 0xBB */ op_new,
    /* 0xBC */ op_newarray,
    /* 0xBD */ op_anewarray,
    /* 0xBE */ op_arraylength,
    /* 0xBF */ op_athrow,
    /* 0xC0 */ op_checkcast,
    /* 0xC1 */ op_instanceof,
    /* 0xC2 */ op_monitorenter,
    /* 0xC3 */ op_monitorexit,
    /* 0xC4 */ op_wide,
    /* 0xC5 */ op_multianewarray,
    /* 0xC6 */ op_ifnull,
    /* 0xC7 */ op_ifnonnull,
    /* 0xC8 */ op_goto_w,
    /* 0xC9 */ op_jsr_w,
    /* 0xCA */ op_breakpoint,
};

static inline void _op_load_1_slot(RuntimeStack *stack, LocalVarItem *localvar, Runtime *runtime, s32 i) {
    push_entry(stack, localvar_getEntry(localvar, i));
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
    StackEntry entry;
    peek_entry(stack->sp - 1, &entry);
    invoke_deepth(runtime);
    jvm_printf("load_1slot : load localvar[%d] value %lld/[%llx] into stack\n", i, entry.lvalue, entry.rvalue);
#endif
}

static inline void _op_load_2_slot(RuntimeStack *stack, LocalVarItem *localvar, Runtime *runtime, s32 i) {
    s64 v = localvar_getLong(localvar, i);
    push_long(stack, v);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
    StackEntry entry;
    peek_entry(stack->sp - 1, &entry);
    invoke_deepth(runtime);
    jvm_printf("load_2slot : load localvar[%d] value %lld/[%llx] into stack\n", i, entry.lvalue, entry.rvalue);
#endif
}


static inline void _op_store_1_slot(RuntimeStack *stack, LocalVarItem *localvar, Runtime *runtime, s32 i) {
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
    StackEntry entry;
    peek_entry(stack->sp - 1, &entry);
    invoke_deepth(runtime);
    jvm_printf("store_1slot : save %llx/%lld into localvar[%d]\n", entry.rvalue, entry.lvalue, i);
#endif
    localvar_setEntry(localvar, i, (--stack->sp));
}


static inline void _op_store_2_slot(RuntimeStack *stack, LocalVarItem *localvar, Runtime *runtime, s32 i) {
    s64 v = pop_long(stack);
    localvar_setLong(localvar, i, v);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
    StackEntry entry;
    peek_entry(stack->sp - 2, &entry);
    invoke_deepth(runtime);
    jvm_printf("store_2slot : save %llx/%lld into localvar[%d]\n", entry.rvalue, entry.lvalue, i);
#endif
}

static inline void _op_ldc_impl(RuntimeStack *stack, JClass *clazz, Runtime *runtime, s32 index) {

    ConstantItem *item = class_get_constant_item(clazz, index);
    switch (item->tag) {
        case CONSTANT_INTEGER:
        case CONSTANT_FLOAT: {
            s32 v = class_get_constant_integer(clazz, index);
            push_int(stack, v);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
            invoke_deepth(runtime);
            jvm_printf("ldc: [%x] \n", v);
#endif
            break;
        }
        case CONSTANT_STRING_REF: {
            ConstantUTF8 *cutf = class_get_constant_utf8(clazz, class_get_constant_stringref(clazz, index)->stringIndex);
            push_ref(stack, (__refer) cutf->jstr);


#if _JVM_DEBUG_BYTECODE_DETAIL > 5
            invoke_deepth(runtime);
            jvm_printf("ldc: [%llx] =\"%s\"\n", (s64) (intptr_t) cutf->jstr, utf8_cstr(cutf->utfstr));
#endif
            break;
        }
        case CONSTANT_CLASS: {
            JClass *cl = classes_load_get(class_get_constant_classref(clazz, index)->name, runtime);
            if (!cl->ins_class) {
                cl->ins_class = insOfJavaLangClass_create_get(runtime, cl);
            }
            push_ref(stack, cl->ins_class);
            break;
        }
        default: {
            push_long(stack, 0);
            jvm_printf("ldc: something not implemention \n");
        }
    }
}

static inline void _op_iconst_n(RuntimeStack *stack, Runtime *runtime, s32 i) {
    push_int(stack, i);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
    invoke_deepth(runtime);
    jvm_printf("iconst_%d: push %d into stack\n", i, i);
#endif
}

static inline void _op_dconst_n(RuntimeStack *stack, Runtime *runtime, f64 d) {
    push_double(stack, d);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
    invoke_deepth(runtime);
    jvm_printf("dconst_%d: push %lf into stack\n", (s32) (d), d);
#endif
}

static inline void _op_fconst_n(RuntimeStack *stack, Runtime *runtime, f32 f) {
    push_float(stack, f);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
    invoke_deepth(runtime);
    jvm_printf("fconst_%f: push %f into stack\n", (s32) f, f);
#endif
}

static inline void _op_lconst_n(RuntimeStack *stack, Runtime *runtime, s64 i) {

    push_long(stack, i);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
    invoke_deepth(runtime);
    jvm_printf("lconst_%lld: push %lld into stack\n", i, i);
#endif
}


void _op_notsupport(u8 *opCode, Runtime *runtime) {
    invoke_deepth(runtime);
    jvm_printf("not support instruct [%x]\n", opCode[0]);
    exit(-3);
}

//----------------------------------  tool func  ------------------------------------------

static ExceptionTable *
_find_exception_handler(Runtime *runtime, Instance *exception, CodeAttribute *ca, s32 offset, __refer exception_ref) {
    Instance *ins = (Instance *) exception_ref;

    s32 i;
    ExceptionTable *e = ca->exception_table;
    for (i = 0; i < ca->exception_table_length; i++) {

        if (offset >= (e + i)->start_pc
            && offset <= (e + i)->end_pc) {
            if (!(e + i)->catch_type) {
                return e + i;
            }
            ConstantClassRef *ccr = class_get_constant_classref(runtime->clazz, (e + i)->catch_type);
            JClass *catchClass = classes_load_get(ccr->name, runtime);
            if (instance_of(catchClass, exception, runtime))
                return e + i;
        }
    }
    return NULL;
}


static inline s32 exception_handle(RuntimeStack *stack, Runtime *runtime) {

    StackEntry entry;
    peek_entry(stack->sp - 1, &entry);
    Instance *ins = entry_2_refer(&entry);

#if _JVM_DEBUG_BYTECODE_DETAIL > 3
    JClass *clazz = runtime->clazz;
    s32 lineNum = getLineNumByIndex(runtime->ca, runtime->pc - runtime->ca->code);
    printf("   at %s.%s(%s.java:%d)\n",
           utf8_cstr(clazz->name), utf8_cstr(runtime->method->name),
           utf8_cstr(clazz->name),
           lineNum
    );
#endif
    ExceptionTable *et = _find_exception_handler(runtime, ins, runtime->ca, (s32) (runtime->pc - runtime->ca->code), ins);
    if (et == NULL) {
        Instance *ins = pop_ref(stack);
        localvar_dispose(runtime);
        push_ref(stack, ins);
        return 1;
    } else {
#if _JVM_DEBUG_BYTECODE_DETAIL > 3
        jvm_printf("Exception : %s\n", utf8_cstr(ins->mb.clazz->name));
#endif
        runtime->pc = (runtime->ca->code + et->handler_pc);
        return 0;
    }

}

static s32 filterClassName(Utf8String *clsName) {
    if (utf8_indexof_c(clsName, "com/sun") < 0
        && utf8_indexof_c(clsName, "java/") < 0
        && utf8_indexof_c(clsName, "javax/") < 0) {
        return 1;
    }
    return 0;
}

static void _printCodeAttribute(CodeAttribute *ca, JClass *p) {

    jvm_printf("attribute name : %s\n", utf8_cstr(class_get_utf8_string(p, ca->attribute_name_index)));
    jvm_printf("attribute arr_length: %d\n", ca->attribute_length);

    jvm_printf("max_stack: %d\n", ca->max_stack);
    jvm_printf("max_locals: %d\n", ca->max_locals);
    jvm_printf("code_length: %d\n", ca->code_length);
}

/**
* 从栈中取得实例对象，中间穿插着调用参数
* @param cmr cmr
* @param stack stack
* @return ins
*/
static inline Instance *getInstanceInStack(ConstantMethodRef *cmr, RuntimeStack *stack) {
    return (stack->sp - 1 - cmr->para_slots)->rvalue;
}

/**
* 把堆栈中的方法调用参数存入方法本地变量
* 调用方法前，父程序把函数参数推入堆栈，方法调用时，需要把堆栈中的参数存到本地变量
* @param method  method
* @param father  runtime of father
* @param son     runtime of son
*/



static inline void _synchronized_lock_method(MethodInfo *method, Runtime *runtime) {
    //synchronized process
    {
        if (method->access_flags & ACC_STATIC) {
            runtime->lock = (MemoryBlock *) runtime->clazz;
        } else {
            runtime->lock = (MemoryBlock *) localvar_getRefer(runtime->localvar, 0);
        }
        jthread_lock(runtime->lock, runtime);
    }
}

static inline void _synchronized_unlock_method(MethodInfo *method, Runtime *runtime) {
    //synchronized process
    {
        jthread_unlock(runtime->lock, runtime);
        runtime->lock = NULL;
    }
}

#if __JVM_OS_VS__ || __JVM_OS_MINGW__ || __JVM_OS_CYGWIN__
//#  define STORE_ADDRESS(index,label) __asm lea eax, label __asm mov edx,opcode_addr  __asm mov [edx][index * TYPE opcode_addr],eax
//#  define JUMP_TO_IP(cur_inst) { void* addr = *opcode_addr[cur_inst]; __asm jmp addr }
#  define STORE_ADDRESS(label) while(0){}
#  define GET_LABEL_ADDRESS(label) 0
#  define JUMP_TO_IP(cur_inst) while(0){}
#else
#  define STORE_ADDRESS(label) opcode_addr[index] = &&label
#  define GET_LABEL_ADDRESS(label) &&label
#  define JUMP_TO_IP(cur_inst) goto *opcode_addr[cur_inst]
#endif


s32 execute_method_impl(MethodInfo *method, Runtime *pruntime) {

    static __refer opcode_addr[0xCB] = {
            GET_LABEL_ADDRESS(label_nop),
            GET_LABEL_ADDRESS(label_aconst_null),
            GET_LABEL_ADDRESS(label_iconst_m1),
            GET_LABEL_ADDRESS(label_iconst_0),
            GET_LABEL_ADDRESS(label_iconst_1),
            GET_LABEL_ADDRESS(label_iconst_2),
            GET_LABEL_ADDRESS(label_iconst_3),
            GET_LABEL_ADDRESS(label_iconst_4),
            GET_LABEL_ADDRESS(label_iconst_5),
            GET_LABEL_ADDRESS(label_lconst_0),
            GET_LABEL_ADDRESS(label_lconst_1),
            GET_LABEL_ADDRESS(label_fconst_0),
            GET_LABEL_ADDRESS(label_fconst_1),
            GET_LABEL_ADDRESS(label_fconst_2),
            GET_LABEL_ADDRESS(label_dconst_0),
            GET_LABEL_ADDRESS(label_dconst_1),
            GET_LABEL_ADDRESS(label_bipush),
            GET_LABEL_ADDRESS(label_sipush),
            GET_LABEL_ADDRESS(label_ldc),
            GET_LABEL_ADDRESS(label_ldc_w),
            GET_LABEL_ADDRESS(label_ldc2_w),
            GET_LABEL_ADDRESS(label_iload),
            GET_LABEL_ADDRESS(label_lload),
            GET_LABEL_ADDRESS(label_fload),
            GET_LABEL_ADDRESS(label_dload),
            GET_LABEL_ADDRESS(label_aload),
            GET_LABEL_ADDRESS(label_iload_0),
            GET_LABEL_ADDRESS(label_iload_1),
            GET_LABEL_ADDRESS(label_iload_2),
            GET_LABEL_ADDRESS(label_iload_3),
            GET_LABEL_ADDRESS(label_lload_0),
            GET_LABEL_ADDRESS(label_lload_1),
            GET_LABEL_ADDRESS(label_lload_2),
            GET_LABEL_ADDRESS(label_lload_3),
            GET_LABEL_ADDRESS(label_fload_0),
            GET_LABEL_ADDRESS(label_fload_1),
            GET_LABEL_ADDRESS(label_fload_2),
            GET_LABEL_ADDRESS(label_fload_3),
            GET_LABEL_ADDRESS(label_dload_0),
            GET_LABEL_ADDRESS(label_dload_1),
            GET_LABEL_ADDRESS(label_dload_2),
            GET_LABEL_ADDRESS(label_dload_3),
            GET_LABEL_ADDRESS(label_aload_0),
            GET_LABEL_ADDRESS(label_aload_1),
            GET_LABEL_ADDRESS(label_aload_2),
            GET_LABEL_ADDRESS(label_aload_3),
            GET_LABEL_ADDRESS(label_iaload),
            GET_LABEL_ADDRESS(label_laload),
            GET_LABEL_ADDRESS(label_faload),
            GET_LABEL_ADDRESS(label_daload),
            GET_LABEL_ADDRESS(label_aaload),
            GET_LABEL_ADDRESS(label_baload),
            GET_LABEL_ADDRESS(label_caload),
            GET_LABEL_ADDRESS(label_saload),
            GET_LABEL_ADDRESS(label_istore),
            GET_LABEL_ADDRESS(label_lstore),
            GET_LABEL_ADDRESS(label_fstore),
            GET_LABEL_ADDRESS(label_dstore),
            GET_LABEL_ADDRESS(label_astore),
            GET_LABEL_ADDRESS(label_istore_0),
            GET_LABEL_ADDRESS(label_istore_1),
            GET_LABEL_ADDRESS(label_istore_2),
            GET_LABEL_ADDRESS(label_istore_3),
            GET_LABEL_ADDRESS(label_lstore_0),
            GET_LABEL_ADDRESS(label_lstore_1),
            GET_LABEL_ADDRESS(label_lstore_2),
            GET_LABEL_ADDRESS(label_lstore_3),
            GET_LABEL_ADDRESS(label_fstore_0),
            GET_LABEL_ADDRESS(label_fstore_1),
            GET_LABEL_ADDRESS(label_fstore_2),
            GET_LABEL_ADDRESS(label_fstore_3),
            GET_LABEL_ADDRESS(label_dstore_0),
            GET_LABEL_ADDRESS(label_dstore_1),
            GET_LABEL_ADDRESS(label_dstore_2),
            GET_LABEL_ADDRESS(label_dstore_3),
            GET_LABEL_ADDRESS(label_astore_0),
            GET_LABEL_ADDRESS(label_astore_1),
            GET_LABEL_ADDRESS(label_astore_2),
            GET_LABEL_ADDRESS(label_astore_3),
            GET_LABEL_ADDRESS(label_iastore),
            GET_LABEL_ADDRESS(label_lastore),
            GET_LABEL_ADDRESS(label_fastore),
            GET_LABEL_ADDRESS(label_dastore),
            GET_LABEL_ADDRESS(label_aastore),
            GET_LABEL_ADDRESS(label_bastore),
            GET_LABEL_ADDRESS(label_castore),
            GET_LABEL_ADDRESS(label_sastore),
            GET_LABEL_ADDRESS(label_pop),
            GET_LABEL_ADDRESS(label_pop2),
            GET_LABEL_ADDRESS(label_dup),
            GET_LABEL_ADDRESS(label_dup_x1),
            GET_LABEL_ADDRESS(label_dup_x2),
            GET_LABEL_ADDRESS(label_dup2),
            GET_LABEL_ADDRESS(label_dup2_x1),
            GET_LABEL_ADDRESS(label_dup2_x2),
            GET_LABEL_ADDRESS(label_swap),
            GET_LABEL_ADDRESS(label_iadd),
            GET_LABEL_ADDRESS(label_ladd),
            GET_LABEL_ADDRESS(label_fadd),
            GET_LABEL_ADDRESS(label_dadd),
            GET_LABEL_ADDRESS(label_isub),
            GET_LABEL_ADDRESS(label_lsub),
            GET_LABEL_ADDRESS(label_fsub),
            GET_LABEL_ADDRESS(label_dsub),
            GET_LABEL_ADDRESS(label_imul),
            GET_LABEL_ADDRESS(label_lmul),
            GET_LABEL_ADDRESS(label_fmul),
            GET_LABEL_ADDRESS(label_dmul),
            GET_LABEL_ADDRESS(label_idiv),
            GET_LABEL_ADDRESS(label_ldiv),
            GET_LABEL_ADDRESS(label_fdiv),
            GET_LABEL_ADDRESS(label_ddiv),
            GET_LABEL_ADDRESS(label_irem),
            GET_LABEL_ADDRESS(label_lrem),
            GET_LABEL_ADDRESS(label_frem),
            GET_LABEL_ADDRESS(label_drem),
            GET_LABEL_ADDRESS(label_ineg),
            GET_LABEL_ADDRESS(label_lneg),
            GET_LABEL_ADDRESS(label_fneg),
            GET_LABEL_ADDRESS(label_dneg),
            GET_LABEL_ADDRESS(label_ishl),
            GET_LABEL_ADDRESS(label_lshl),
            GET_LABEL_ADDRESS(label_ishr),
            GET_LABEL_ADDRESS(label_lshr),
            GET_LABEL_ADDRESS(label_iushr),
            GET_LABEL_ADDRESS(label_lushr),
            GET_LABEL_ADDRESS(label_iand),
            GET_LABEL_ADDRESS(label_land),
            GET_LABEL_ADDRESS(label_ior),
            GET_LABEL_ADDRESS(label_lor),
            GET_LABEL_ADDRESS(label_ixor),
            GET_LABEL_ADDRESS(label_lxor),
            GET_LABEL_ADDRESS(label_iinc),
            GET_LABEL_ADDRESS(label_i2l),
            GET_LABEL_ADDRESS(label_i2f),
            GET_LABEL_ADDRESS(label_i2d),
            GET_LABEL_ADDRESS(label_l2i),
            GET_LABEL_ADDRESS(label_l2f),
            GET_LABEL_ADDRESS(label_l2d),
            GET_LABEL_ADDRESS(label_f2i),
            GET_LABEL_ADDRESS(label_f2l),
            GET_LABEL_ADDRESS(label_f2d),
            GET_LABEL_ADDRESS(label_d2i),
            GET_LABEL_ADDRESS(label_d2l),
            GET_LABEL_ADDRESS(label_d2f),
            GET_LABEL_ADDRESS(label_i2b),
            GET_LABEL_ADDRESS(label_i2c),
            GET_LABEL_ADDRESS(label_i2s),
            GET_LABEL_ADDRESS(label_lcmp),
            GET_LABEL_ADDRESS(label_fcmpl),
            GET_LABEL_ADDRESS(label_fcmpg),
            GET_LABEL_ADDRESS(label_dcmpl),
            GET_LABEL_ADDRESS(label_dcmpg),
            GET_LABEL_ADDRESS(label_ifeq),
            GET_LABEL_ADDRESS(label_ifne),
            GET_LABEL_ADDRESS(label_iflt),
            GET_LABEL_ADDRESS(label_ifge),
            GET_LABEL_ADDRESS(label_ifgt),
            GET_LABEL_ADDRESS(label_ifle),
            GET_LABEL_ADDRESS(label_if_icmpeq),
            GET_LABEL_ADDRESS(label_if_icmpne),
            GET_LABEL_ADDRESS(label_if_icmplt),
            GET_LABEL_ADDRESS(label_if_icmpge),
            GET_LABEL_ADDRESS(label_if_icmpgt),
            GET_LABEL_ADDRESS(label_if_icmple),
            GET_LABEL_ADDRESS(label_if_acmpeq),
            GET_LABEL_ADDRESS(label_if_acmpne),
            GET_LABEL_ADDRESS(label_goto),
            GET_LABEL_ADDRESS(label_jsr),
            GET_LABEL_ADDRESS(label_ret),
            GET_LABEL_ADDRESS(label_tableswitch),
            GET_LABEL_ADDRESS(label_lookupswitch),
            GET_LABEL_ADDRESS(label_ireturn),
            GET_LABEL_ADDRESS(label_lreturn),
            GET_LABEL_ADDRESS(label_freturn),
            GET_LABEL_ADDRESS(label_dreturn),
            GET_LABEL_ADDRESS(label_areturn),
            GET_LABEL_ADDRESS(label_return),
            GET_LABEL_ADDRESS(label_getstatic),
            GET_LABEL_ADDRESS(label_putstatic),
            GET_LABEL_ADDRESS(label_getfield),
            GET_LABEL_ADDRESS(label_putfield),
            GET_LABEL_ADDRESS(label_invokevirtual),
            GET_LABEL_ADDRESS(label_invokespecial),
            GET_LABEL_ADDRESS(label_invokestatic),
            GET_LABEL_ADDRESS(label_invokeinterface),
            GET_LABEL_ADDRESS(label_invokedynamic),
            GET_LABEL_ADDRESS(label_new),
            GET_LABEL_ADDRESS(label_newarray),
            GET_LABEL_ADDRESS(label_anewarray),
            GET_LABEL_ADDRESS(label_arraylength),
            GET_LABEL_ADDRESS(label_athrow),
            GET_LABEL_ADDRESS(label_checkcast),
            GET_LABEL_ADDRESS(label_instanceof),
            GET_LABEL_ADDRESS(label_monitorenter),
            GET_LABEL_ADDRESS(label_monitorexit),
            GET_LABEL_ADDRESS(label_wide),
            GET_LABEL_ADDRESS(label_multianewarray),
            GET_LABEL_ADDRESS(label_ifnull),
            GET_LABEL_ADDRESS(label_ifnonnull),
            GET_LABEL_ADDRESS(label_goto_w),
            GET_LABEL_ADDRESS(label_jsr_w),
            GET_LABEL_ADDRESS(label_breakpoint)
    };


    s32 ret = RUNTIME_STATUS_NORMAL;

    Runtime *runtime = runtime_create_inl(pruntime);
    JClass *clazz = method->_this_class;
    runtime->method = method;
    runtime->clazz = clazz;
    while (clazz->status < CLASS_STATUS_CLINITING) {
        class_clinit(clazz, runtime);
    }
    s32 method_sync = method->access_flags & ACC_SYNCHRONIZED;

    //    if (utf8_equals_c(method->name, "lambda$t1$1")
    ////        && utf8_equals_c(clazz->name, "java/lang/String")
    //            ) {
    //        s32 debug = 1;
    //    }

    RuntimeStack *stack = runtime->stack;

    if (!(method->access_flags & ACC_NATIVE)) {
        CodeAttribute *ca = method->converted_code;
        if (ca) {
            if (ca->code_length == 1 && *ca->code == op_return) {//empty method, do nothing
                s32 paras = method->para_slots;
                switch (*pruntime->pc) {//only static and special can be optimize , invokevirtual may call by diff instance
                    case op_invokestatic:
                    case op_invokespecial: {
                        if (paras == 0) {
                            *pruntime->pc = op_nop;
                            *(pruntime->pc + 1) = op_nop;
                            *(pruntime->pc + 2) = op_nop;
                        } else if (paras == 1) {
                            *pruntime->pc = op_pop;
                            *(pruntime->pc + 1) = op_nop;
                            *(pruntime->pc + 2) = op_nop;
                        } else if (paras == 2) {
                            *pruntime->pc = op_pop2;
                            *(pruntime->pc + 1) = op_nop;
                            *(pruntime->pc + 2) = op_nop;
                        }
                        break;
                    }
                }
            }
            if (stack->max_size < stack->sp - stack->store) {
                Utf8String *ustr = utf8_create();
                getRuntimeStack(runtime, ustr);
                jvm_printf("Stack overflow :\n %s\n", utf8_cstr(ustr));
                exit(1);
            }
            localvar_init(runtime, ca->max_locals, method->para_slots);
            LocalVarItem *localvar = runtime->localvar;
            if (method_sync)_synchronized_lock_method(method, runtime);

            register u8 *opCode = ca->code;
            runtime->ca = ca;
            JavaThreadInfo *threadInfo = runtime->threadInfo;

            s32 exit_exec = 0;
            while (!exit_exec) {
                runtime->pc = opCode;
                u8 cur_inst = *opCode;
                if (JDWP_DEBUG) {
                    //breakpoint
                    if (method->breakpoint) {
                        jdwp_check_breakpoint(runtime);
                    }
                    //debug step
                    if (threadInfo->jdwp_step.active) {//单步状态
                        threadInfo->jdwp_step.bytecode_count++;
                        jdwp_check_debug_step(runtime);

                    }
                }
                //process thread suspend
                if (threadInfo->suspend_count) {
                    if (threadInfo->is_interrupt) {
                        ret = RUNTIME_STATUS_INTERRUPT;
                        break;
                    }
                    check_suspend_and_pause(runtime);
                }

#if _JVM_DEBUG_PROFILE
                s64 spent = 0;
                s64 start_at = nanoTime();
#endif


                /* ==================================opcode start =============================*/
#ifdef __JVM_DEBUG__
                s64 inst_pc = runtime->pc - ca->code;
#endif
                JUMP_TO_IP(cur_inst);
                switch (cur_inst) {
                    label_nop:
                    case op_nop: {
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("nop\n");
#endif
                        opCode++;

                        break;
                    }
                    label_aconst_null:
                    case op_aconst_null: {
                        push_ref(stack, NULL);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("aconst_null: push %d into stack\n", 0);
#endif
                        opCode++;

                        break;
                    }
                    label_iconst_m1:
                    case op_iconst_m1: {
                        _op_iconst_n(stack, runtime, -1);
                        opCode++;
                        break;
                    }


                    label_iconst_0:
                    case op_iconst_0: {
                        _op_iconst_n(stack, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_iconst_1:
                    case op_iconst_1: {
                        _op_iconst_n(stack, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_iconst_2:
                    case op_iconst_2: {
                        _op_iconst_n(stack, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_iconst_3:
                    case op_iconst_3: {
                        _op_iconst_n(stack, runtime, 3);
                        opCode++;
                        break;
                    }

                    label_iconst_4:
                    case op_iconst_4: {
                        _op_iconst_n(stack, runtime, 4);
                        opCode++;
                        break;
                    }

                    label_iconst_5:
                    case op_iconst_5: {
                        _op_iconst_n(stack, runtime, 5);
                        opCode++;
                        break;
                    }

                    label_lconst_0:
                    case op_lconst_0: {
                        _op_lconst_n(stack, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_lconst_1:
                    case op_lconst_1: {
                        _op_lconst_n(stack, runtime, 1);
                        opCode++;
                        break;
                    }


                    label_fconst_0:
                    case op_fconst_0: {
                        _op_fconst_n(stack, runtime, 0.0f);
                        opCode++;
                        break;
                    }

                    label_fconst_1:
                    case op_fconst_1: {
                        _op_fconst_n(stack, runtime, 1.0f);
                        opCode++;
                        break;
                    }

                    label_fconst_2:
                    case op_fconst_2: {
                        _op_fconst_n(stack, runtime, 2.0f);
                        opCode++;
                        break;
                    }


                    label_dconst_0:
                    case op_dconst_0: {
                        _op_dconst_n(stack, runtime, 0.0f);
                        opCode++;
                        break;
                    }

                    label_dconst_1:
                    case op_dconst_1: {
                        _op_dconst_n(stack, runtime, 1.0f);
                        opCode++;
                        break;
                    }

                    label_bipush:
                    case op_bipush: {

                        s32 value = (s8) opCode[1];
                        push_int(stack, value);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("bipush a byte %d onto the stack \n", value);
#endif
                        opCode += 2;

                        break;
                    }

                    label_sipush:
                    case op_sipush: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("sipush value %d\n", s2c.s);
#endif
                        push_int(stack, s2c.s);
                        opCode += 3;

                        break;
                    }


                    label_ldc:
                    case op_ldc: {
                        s32 index = opCode[1];
                        opCode += 2;
                        _op_ldc_impl(stack, clazz, runtime, index);
                        break;
                    }

                    label_ldc_w:
                    case op_ldc_w: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];
                        opCode += 3;
                        _op_ldc_impl(stack, clazz, runtime, s2c.us);
                        break;
                    }

                    label_ldc2_w:
                    case op_ldc2_w: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        s64 value = class_get_constant_long(clazz, s2c.us);//long or double

                        push_long(stack, value);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ldc2_w: push a constant(%d) [%llx] onto the stack \n", index, value);
#endif
                        opCode += 3;

                        break;
                    }


                    label_iload:
                    label_fload:
                    case op_iload:
                    case op_fload:
                    label_aload:
                    case op_aload: {
                        _op_load_1_slot(stack, localvar, runtime, (u8) opCode[1]);
                        opCode += 2;
                        break;
                    }
                    label_lload:
                    label_dload:
                    case op_lload:
                    case op_dload: {
                        _op_load_2_slot(stack, localvar, runtime, (u8) opCode[1]);
                        opCode += 2;
                        break;
                    }

                    label_iload_0:
                    case op_iload_0: {
                        _op_load_1_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_iload_1:
                    case op_iload_1: {
                        _op_load_1_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_iload_2:
                    case op_iload_2: {
                        _op_load_1_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_iload_3:
                    case op_iload_3: {
                        _op_load_1_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }

                    label_lload_0:
                    case op_lload_0: {
                        _op_load_2_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_lload_1:
                    case op_lload_1: {
                        _op_load_2_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_lload_2:
                    case op_lload_2: {
                        _op_load_2_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_lload_3:
                    case op_lload_3: {
                        _op_load_2_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }

                    label_fload_0:
                    case op_fload_0: {
                        _op_load_1_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_fload_1:
                    case op_fload_1: {
                        _op_load_1_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_fload_2:
                    case op_fload_2: {
                        _op_load_1_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_fload_3:
                    case op_fload_3: {
                        _op_load_1_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }


                    label_dload_0:
                    case op_dload_0: {
                        _op_load_2_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_dload_1:
                    case op_dload_1: {
                        _op_load_2_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_dload_2:
                    case op_dload_2: {
                        _op_load_2_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_dload_3:
                    case op_dload_3: {
                        _op_load_2_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }

                    label_aload_0:
                    case op_aload_0: {
                        _op_load_1_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_aload_1:
                    case op_aload_1: {
                        _op_load_1_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_aload_2:
                    case op_aload_2: {
                        _op_load_1_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_aload_3:
                    case op_aload_3: {
                        _op_load_1_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }

                    label_faload:
                    label_iaload:
                    case op_iaload:
                    case op_faload: {
                        s32 index = pop_int(stack);
                        Instance *arr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(arr, index, runtime);
                        if (!ret) {
                            s32 s = *((s32 *) (arr->arr_body) + index);
                            push_int(stack, s);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("if_aload push arr[%llx].(%d)=%x:%d:%lf into stack\n", (u64) (intptr_t) arr, index,
                                       s, s, *(f32 *) &s);
#endif
                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }

                        break;
                    }

                    label_laload:
                    label_daload:
                    case op_laload:
                    case op_daload: {
                        s32 index = pop_int(stack);
                        Instance *arr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(arr, index, runtime);
                        if (!ret) {
                            s64 s = *(((s64 *) arr->arr_body) + index);
                            push_long(stack, s);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("ld_aload push arr[%llx].(%d)=%llx:%lld:%lf into stack\n", (u64) (intptr_t) arr, index,
                                       s, s, *(f64 *) &s);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }

                    label_aaload:
                    case op_aaload: {
                        s32 index = pop_int(stack);
                        Instance *arr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(arr, index, runtime);
                        if (!ret) {
                            __refer s = *(((__refer *) arr->arr_body) + index);
                            push_ref(stack, s);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("aaload push arr[%llx].(%d)=%llx:%lld into stack\n", (u64) (intptr_t) arr, index,
                                       (s64) (intptr_t) s, (s64) (intptr_t) s);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }

                    label_baload:
                    case op_baload: {
                        s32 index = pop_int(stack);
                        Instance *arr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(arr, index, runtime);
                        if (!ret) {
                            s32 s = *(((s8 *) arr->arr_body) + index);
                            push_int(stack, s);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iaload push arr[%llx].(%d)=%x:%d:%lf into stack\n", (u64) (intptr_t) arr, index,
                                       s, s, *(f32 *) &s);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }

                    label_caload:
                    case op_caload: {
                        s32 index = pop_int(stack);
                        Instance *arr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(arr, index, runtime);
                        if (!ret) {
                            s32 s = *(((u16 *) arr->arr_body) + index);
                            push_int(stack, s);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iaload push arr[%llx].(%d)=%x:%d:%lf into stack\n", (u64) (intptr_t) arr, index,
                                       s, s, *(f32 *) &s);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }

                    label_saload:
                    case op_saload: {
                        s32 index = pop_int(stack);
                        Instance *arr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(arr, index, runtime);
                        if (!ret) {
                            s32 s = *(((s16 *) arr->arr_body) + index);
                            push_int(stack, s);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iaload push arr[%llx].(%d)=%x:%d:%lf into stack\n", (u64) (intptr_t) arr, index,
                                       s, s, *(f32 *) &s);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }

                    label_istore:
                    case op_istore:
                    label_fstore:
                    case op_fstore:
                    label_astore:
                    case op_astore: {
                        _op_store_1_slot(stack, localvar, runtime, (u8) opCode[1]);
                        opCode += 2;
                        break;
                    }


                    label_lstore:
                    case op_lstore:
                    label_dstore:
                    case op_dstore: {
                        _op_store_2_slot(stack, localvar, runtime, (u8) opCode[1]);
                        opCode += 2;

                        break;
                    }

                    label_istore_0:
                    case op_istore_0: {
                        _op_store_1_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_istore_1:
                    case op_istore_1: {
                        _op_store_1_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_istore_2:
                    case op_istore_2: {
                        _op_store_1_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_istore_3:
                    case op_istore_3: {
                        _op_store_1_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }

                    label_lstore_0:
                    case op_lstore_0: {
                        _op_store_2_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_lstore_1:
                    case op_lstore_1: {
                        _op_store_2_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_lstore_2:
                    case op_lstore_2: {
                        _op_store_2_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_lstore_3:
                    case op_lstore_3: {
                        _op_store_2_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }


                    label_fstore_0:
                    case op_fstore_0: {
                        _op_store_1_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_fstore_1:
                    case op_fstore_1: {
                        _op_store_1_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_fstore_2:
                    case op_fstore_2: {
                        _op_store_1_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_fstore_3:
                    case op_fstore_3: {
                        _op_store_1_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }


                    label_dstore_0:
                    case op_dstore_0: {
                        _op_store_2_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_dstore_1:
                    case op_dstore_1: {
                        _op_store_2_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    case op_dstore_2: {
                        label_dstore_2:
                        _op_store_2_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_dstore_3:
                    case op_dstore_3: {
                        _op_store_2_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }


                    label_astore_0:
                    case op_astore_0: {
                        _op_store_1_slot(stack, localvar, runtime, 0);
                        opCode++;
                        break;
                    }

                    label_astore_1:
                    case op_astore_1: {
                        _op_store_1_slot(stack, localvar, runtime, 1);
                        opCode++;
                        break;
                    }

                    label_astore_2:
                    case op_astore_2: {
                        _op_store_1_slot(stack, localvar, runtime, 2);
                        opCode++;
                        break;
                    }

                    label_astore_3:
                    case op_astore_3: {
                        _op_store_1_slot(stack, localvar, runtime, 3);
                        opCode++;
                        break;
                    }

                    label_fastore:
                    label_iastore:
                    case op_fastore:
                    case op_iastore: {
                        s32 i = pop_int(stack);
                        s32 index = pop_int(stack);
                        Instance *jarr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(jarr, index, runtime);
                        if (!ret) {
                            *(((s32 *) jarr->arr_body) + index) = i;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iastore: save array[%llx].(%d)=%d)\n",
                                       (s64) (intptr_t) jarr, index, i);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }

                    label_dastore:
                    label_lastore:
                    case op_dastore:
                    case op_lastore: {
                        s64 j = pop_long(stack);
                        s32 index = pop_int(stack);
                        Instance *jarr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(jarr, index, runtime);
                        if (!ret) {
                            *(((s64 *) jarr->arr_body) + index) = j;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iastore: save array[%llx].(%d)=%lld)\n",
                                       (s64) (intptr_t) jarr, index, j);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }


                    label_aastore:
                    case op_aastore: {
                        __refer r = pop_ref(stack);
                        s32 index = pop_int(stack);
                        Instance *jarr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(jarr, index, runtime);
                        if (!ret) {
                            *(((__refer *) jarr->arr_body) + index) = r;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iastore: save array[%llx].(%d)=%llx)\n",
                                       (s64) (intptr_t) jarr, index, (s64) (intptr_t) r);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }

                    label_bastore:
                    case op_bastore: {
                        s32 i = pop_int(stack);
                        s32 index = pop_int(stack);
                        Instance *jarr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(jarr, index, runtime);
                        if (!ret) {
                            *(((s8 *) jarr->arr_body) + index) = (s8) i;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iastore: save array[%llx].(%d)=%d)\n",
                                       (s64) (intptr_t) jarr, index, i);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }


                    label_castore:
                    case op_castore: {
                        s32 i = pop_int(stack);
                        s32 index = pop_int(stack);
                        Instance *jarr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(jarr, index, runtime);
                        if (!ret) {
                            *(((u16 *) jarr->arr_body) + index) = i;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iastore: save array[%llx].(%d)=%d)\n",
                                       (s64) (intptr_t) jarr, index, i);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }

                    label_sastore:
                    case op_sastore: {
                        s32 i = pop_int(stack);
                        s32 index = pop_int(stack);
                        Instance *jarr = (Instance *) pop_ref(stack);
                        ret = jarray_check_exception(jarr, index, runtime);
                        if (!ret) {
                            *(((s16 *) jarr->arr_body) + index) = i;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            jvm_printf("iastore: save array[%llx].(%d)=%d)\n",
                                       (s64) (intptr_t) jarr, index, i);
#endif

                            opCode++;
                        } else {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }


                    label_pop:
                    case op_pop: {
                        pop_empty(stack);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("pop\n");
#endif
                        opCode++;

                        break;
                    }


                    label_pop2:
                    case op_pop2: {
                        pop_empty(stack);
                        pop_empty(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("pop2\n");
#endif
                        opCode++;

                        break;
                    }


                    label_dup:
                    case op_dup: {

                        push_entry(stack, stack->sp - 1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dup\n");
#endif
                        opCode++;

                        break;
                    }

                    label_dup_x1:
                    case op_dup_x1: {
                        StackEntry entry1;
                        pop_entry(stack, &entry1);
                        StackEntry entry2;
                        pop_entry(stack, &entry2);

                        push_entry(stack, &entry1);
                        push_entry(stack, &entry2);
                        push_entry(stack, &entry1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dup_x1\n");
#endif
                        opCode++;

                        break;
                    }

                    label_dup_x2:
                    case op_dup_x2: {
                        StackEntry entry1;
                        pop_entry(stack, &entry1);
                        StackEntry entry2;
                        pop_entry(stack, &entry2);
                        StackEntry entry3;
                        pop_entry(stack, &entry3);

                        push_entry(stack, &entry1);
                        push_entry(stack, &entry3);
                        push_entry(stack, &entry2);
                        push_entry(stack, &entry1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dup_x2 \n");
#endif
                        opCode++;

                        break;
                    }

                    label_dup2:
                    case op_dup2: {
                        StackEntry entry1;
                        peek_entry(stack->sp - 1, &entry1);
                        StackEntry entry2;
                        peek_entry(stack->sp - 2, &entry2);

                        push_entry(stack, &entry2);
                        push_entry(stack, &entry1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_dup2\n");
#endif
                        opCode++;

                        break;
                    }


                    label_dup2_x1:
                    case op_dup2_x1: {
                        StackEntry entry1;
                        pop_entry(stack, &entry1);
                        StackEntry entry2;
                        pop_entry(stack, &entry2);
                        StackEntry entry3;
                        pop_entry(stack, &entry3);

                        push_entry(stack, &entry2);
                        push_entry(stack, &entry1);
                        push_entry(stack, &entry3);
                        push_entry(stack, &entry2);
                        push_entry(stack, &entry1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dup2_x1\n");
#endif
                        opCode++;

                        break;
                    }

                    label_dup2_x2:
                    case op_dup2_x2: {
                        StackEntry entry1;
                        pop_entry(stack, &entry1);
                        StackEntry entry2;
                        pop_entry(stack, &entry2);
                        StackEntry entry3;
                        pop_entry(stack, &entry3);
                        StackEntry entry4;
                        pop_entry(stack, &entry4);

                        push_entry(stack, &entry2);
                        push_entry(stack, &entry1);
                        push_entry(stack, &entry4);
                        push_entry(stack, &entry3);
                        push_entry(stack, &entry2);
                        push_entry(stack, &entry1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dup2_x2\n");
#endif
                        opCode++;

                        break;
                    }

                    label_swap:
                    case op_swap: {

                        StackEntry entry1;
                        pop_entry(stack, &entry1);
                        StackEntry entry2;
                        pop_entry(stack, &entry2);

                        push_entry(stack, &entry1);
                        push_entry(stack, &entry2);


#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("swap\n");
#endif
                        opCode++;

                        break;
                    }


                    label_iadd:
                    case op_iadd: {

                        s32 value1 = pop_int(stack);
                        s32 value2 = pop_int(stack);
                        s32 result = value1 + value2;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("iadd: %d + %d = %d\n", value1, value2, result);
#endif
                        push_int(stack, result);
                        opCode++;

                        break;
                    }


                    label_ladd:
                    case op_ladd: {
                        s64 value1 = pop_long(stack);
                        s64 value2 = pop_long(stack);
                        s64 result = value2 + value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ladd: %lld + %lld = %lld\n", value2, value1, result);
#endif
                        push_long(stack, result);
                        opCode++;

                        break;
                    }

                    label_fadd:
                    case op_fadd: {
                        f32 value1 = pop_float(stack);
                        f32 value2 = pop_float(stack);
                        f32 result = value2 + value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("fadd: %lf + %lf = %lf\n", value2, value1, result);
#endif
                        push_float(stack, result);
                        opCode++;

                        break;
                    }

                    label_dadd:
                    case op_dadd: {

                        f64 value1 = pop_double(stack);
                        f64 value2 = pop_double(stack);
                        f64 result = value1 + value2;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dadd: %lf + %lf = %lf\n", value1, value2, result);
#endif
                        push_double(stack, result);
                        opCode++;

                        break;
                    }

                    label_isub:
                    case op_isub: {
                        s32 value2 = pop_int(stack);
                        s32 value1 = pop_int(stack);
                        s32 result = value1 - value2;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("isub : %d - %d = %d\n", value1, value2, result);
#endif
                        push_int(stack, result);
                        opCode++;

                        break;
                    }

                    label_lsub:
                    case op_lsub: {
                        s64 value1 = pop_long(stack);
                        s64 value2 = pop_long(stack);
                        s64 result = value2 - value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lsub: %lld - %lld = %lld\n", value2, value1, result);
#endif
                        push_long(stack, result);
                        opCode++;

                        break;
                    }

                    label_fsub:
                    case op_fsub: {
                        f32 value1 = pop_float(stack);
                        f32 value2 = pop_float(stack);
                        f32 result = value2 - value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("fsub: %f - %f = %f\n", value2, value1, result);
#endif
                        push_float(stack, result);
                        opCode++;

                        break;
                    }


                    label_dsub:
                    case op_dsub: {
                        f64 value1 = pop_double(stack);
                        f64 value2 = pop_double(stack);
                        f64 result = value2 - value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dsub: %lf - %lf = %lf\n", value2, value1, result);
#endif
                        push_double(stack, result);
                        opCode++;

                        break;
                    }

                    label_imul:
                    case op_imul: {

                        s32 value1 = pop_int(stack);
                        s32 value2 = pop_int(stack);
                        s32 result = value1 * value2;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("imul: %d * %d = %d\n", value1, value2, result);
#endif
                        push_int(stack, result);
                        opCode++;

                        break;
                    }

                    label_lmul:
                    case op_lmul: {
                        s64 value1 = pop_long(stack);
                        s64 value2 = pop_long(stack);
                        s64 result = value2 * value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lmul: %lld * %lld = %lld\n", value2, value1, result);
#endif
                        push_long(stack, result);
                        opCode++;

                        break;
                    }

                    label_fmul:
                    case op_fmul: {
                        f32 value1 = pop_float(stack);
                        f32 value2 = pop_float(stack);
                        f32 result = value1 * value2;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("fmul: %f * %f = %f\n", value1, value2, result);
#endif
                        push_float(stack, result);
                        opCode++;

                        break;
                    }

                    label_dmul:
                    case op_dmul: {
                        f64 value1 = pop_double(stack);
                        f64 value2 = pop_double(stack);
                        f64 result = value1 * value2;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dmul: %lf * %lf = %lf\n", value1, value2, result);
#endif
                        push_double(stack, result);
                        opCode++;

                        break;
                    }


                    label_idiv:
                    case op_idiv: {

                        s32 value1 = pop_int(stack);
                        s32 value2 = pop_int(stack);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("idiv: %d / %d = %d\n", value1, value2, value2 / value1);
#endif
                        if (!value1) {
                            Instance *exception = exception_create(JVM_EXCEPTION_ARRITHMETIC, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            s32 result = value2 / value1;
                            push_int(stack, result);
                            opCode++;
                        }

                        break;
                    }

                    label_ldiv:
                    case op_ldiv: {
                        s64 value1 = pop_long(stack);
                        s64 value2 = pop_long(stack);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ldiv: %lld / %lld = %lld\n", value2, value1, value2 / value1);
#endif
                        if (!value1) {
                            Instance *exception = exception_create(JVM_EXCEPTION_ARRITHMETIC, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            s64 result = value2 / value1;
                            push_long(stack, result);
                            opCode++;
                        }

                        break;
                    }

                    label_fdiv:
                    case op_fdiv: {
                        f32 value1 = pop_float(stack);
                        f32 value2 = pop_float(stack);
                        f32 result = value2 / value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("fdiv: %f / %f = %f\n", value2, value1, result);
#endif
                        push_float(stack, result);
                        opCode++;

                        break;
                    }
                    label_ddiv:
                    case op_ddiv: {
                        f64 value1 = pop_double(stack);
                        f64 value2 = pop_double(stack);
                        f64 result = value2 / value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ddiv: %f / %f = %f\n", value2, value1, result);
#endif
                        push_double(stack, result);
                        opCode++;

                        break;
                    }


                    label_irem:
                    case op_irem: {
                        s32 value1 = pop_int(stack);
                        s32 value2 = pop_int(stack);
                        s32 result = value2 % value1;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("irem: %d % %d = %d\n", value2, value1, result);
#endif
                        push_int(stack, result);
                        opCode++;

                        break;
                    }

                    label_lrem:
                    case op_lrem: {
                        s64 value1 = pop_long(stack);
                        s64 value2 = pop_long(stack);
                        s64 result = value2 % value1;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lrem: %lld mod %lld = %lld\n", value2, value1, result);
#endif
                        push_long(stack, result);
                        opCode++;

                        break;
                    }

                    label_frem:
                    case op_frem: {
                        f32 value1 = pop_float(stack);
                        f32 value2 = pop_float(stack);
                        f32 result = value2 - ((int) (value2 / value1) * value1);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("frem: %f % %f = %f\n", value2, value1, result);
#endif
                        push_float(stack, result);
                        opCode++;

                        break;
                    }

                    label_drem:
                    case op_drem: {
                        f64 value1 = pop_double(stack);
                        f64 value2 = pop_double(stack);
                        f64 result = value2 - ((s64) (value2 / value1) * value1);;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("drem: %lf mod %lf = %lf\n", value2, value1, result);
#endif
                        push_double(stack, result);
                        opCode++;

                        break;
                    }

                    label_ineg:
                    case op_ineg: {
                        s32 value1 = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ineg: -(%d) = %d\n", value1, -value1);
#endif
                        push_int(stack, -value1);
                        opCode++;

                        break;
                    }

                    label_lneg:
                    case op_lneg: {
                        s64 value1 = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lneg: -(%lld) = %lld\n", value1, -value1);
#endif
                        push_long(stack, -value1);
                        opCode++;

                        break;
                    }


                    label_fneg:
                    case op_fneg: {
                        f32 value1 = pop_float(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("fneg: -(%f) = %f\n", value1, -value1);
#endif
                        push_float(stack, -value1);
                        opCode++;

                        break;
                    }


                    label_dneg:
                    case op_dneg: {
                        f64 value1 = pop_double(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dneg: -(%lf) = %lf\n", value1, -value1);
#endif
                        push_double(stack, -value1);
                        opCode++;

                        break;
                    }

                    label_ishl:
                    case op_ishl: {
                        s32 value1 = pop_int(stack);
                        s32 value2 = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ishl: %x << %x =%x \n", value2, value1, value2 << value1);
#endif
                        push_int(stack, value2 << value1);
                        opCode++;

                        break;
                    }

                    label_lshl:
                    case op_lshl: {
                        s32 value1 = pop_int(stack);
                        s64 value2 = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lshl: %llx << %x =%llx \n", value2, value1, (value2 << value1));
#endif
                        push_long(stack, value2 << value1);
                        opCode++;

                        break;
                    }

                    label_ishr:
                    case op_ishr: {
                        s32 value1 = pop_int(stack);
                        s32 value2 = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ishr: %x >> %x =%x \n", value2, value1, value2 >> value1);
#endif
                        push_int(stack, value2 >> value1);
                        opCode++;

                        break;
                    }

                    label_lshr:
                    case op_lshr: {
                        s32 value1 = pop_int(stack);
                        s64 value2 = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lshr: %llx >> %x =%llx \n", value2, value1, value2 >> value1);
#endif
                        push_long(stack, value2 >> value1);
                        opCode++;

                        break;
                    }

                    label_iushr:
                    case op_iushr: {
                        s32 value1 = pop_int(stack);
                        u32 value2 = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("iushr: %x >>> %x =%x \n", value2, value1, value2 >> value1);
#endif
                        push_int(stack, value2 >> value1);
                        opCode++;

                        break;
                    }

                    label_lushr:
                    case op_lushr: {
                        s32 value1 = pop_int(stack);
                        u64 value2 = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lushr: %llx >>> %x =%llx \n", value2, value1, value2 >> value1);
#endif
                        push_long(stack, value2 >> value1);
                        opCode++;

                        break;
                    }

                    label_iand:
                    case op_iand: {
                        s32 value1 = pop_int(stack);
                        s32 value2 = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("iand: %x & %x =%x \n", value2, value1, value2 & value1);
#endif
                        push_int(stack, value2 & value1);
                        opCode++;

                        break;
                    }

                    label_land:
                    case op_land: {
                        u64 value1 = pop_long(stack);
                        u64 value2 = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("land: %llx  &  %llx =%llx \n", value2, value1, value2 & value1);
#endif
                        push_long(stack, value2 & value1);
                        opCode++;

                        break;
                    }


                    label_ior:
                    case op_ior: {
                        u32 value1 = pop_int(stack);
                        u32 value2 = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ior: %x & %x =%x \n", value2, value1, value2 | value1);
#endif
                        push_int(stack, value2 | value1);
                        opCode++;

                        break;
                    }

                    label_lor:
                    case op_lor: {
                        u64 value1 = pop_long(stack);
                        u64 value2 = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lor: %llx  |  %llx =%llx \n", value2, value1, value2 | value1);
#endif
                        push_long(stack, value2 | value1);
                        opCode++;

                        break;
                    }


                    label_ixor:
                    case op_ixor: {
                        u32 value1 = pop_int(stack);
                        u32 value2 = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ixor: %x ^ %x =%x \n", value2, value1, value2 ^ value1);
#endif
                        push_int(stack, value2 ^ value1);
                        opCode++;

                        break;
                    }

                    label_lxor:
                    case op_lxor: {
                        u64 value1 = pop_long(stack);
                        u64 value2 = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lxor: %llx  ^  %llx =%llx \n", value2, value1, value2 ^ value1);
#endif
                        push_long(stack, value2 ^ value1);
                        opCode++;

                        break;
                    }

                    label_iinc:
                    case op_iinc: {
                        localvar[(u8) opCode[1]].ivalue += (s8) opCode[2];
                        opCode += 3;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("iinc: localvar(%d) = %d , inc %d\n", (u8) opCode[1], localvar[(u8) opCode[1]].ivalue, (s8) opCode[2]);
#endif

                        break;
                    }

                    label_i2l:
                    case op_i2l: {
                        s32 value = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("i2l: %d --> %lld\n", (s32) value, (s64) value);
#endif
                        push_long(stack, (s64) value);
                        opCode++;

                        break;
                    }

                    label_i2f:
                    case op_i2f: {
                        s32 value = pop_int(stack);
                        f32 result = (f32) value;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("i2f: %d --> %f\n", (s32) value, result);
#endif
                        push_float(stack, result);
                        opCode++;

                        break;
                    }

                    label_i2d:
                    case op_i2d: {
                        s32 value = pop_int(stack);
                        f64 result = value;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("i2d: %d --> %lf\n", (s32) value, result);
#endif
                        push_double(stack, result);
                        opCode++;

                        break;
                    }

                    label_l2i:
                    case op_l2i: {
                        s64 value = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("l2i: %d <-- %lld\n", (s32) value, value);
#endif
                        push_int(stack, (s32) value);
                        opCode++;

                        break;
                    }

                    label_l2f:
                    case op_l2f: {
                        s64 value = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("l2f: %f <-- %lld\n", (f32) value, value);
#endif
                        push_float(stack, (f32) value);
                        opCode++;

                        break;
                    }

                    label_l2d:
                    case op_l2d: {
                        s64 value = pop_long(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("l2d: %lf <-- %lld\n", (f64) value, value);
#endif
                        push_double(stack, (f64) value);
                        opCode++;

                        break;
                    }


                    label_f2i:
                    case op_f2i: {
                        f32 value1 = pop_float(stack);
                        s32 result = (s32) value1;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("f2i: %d <-- %f\n", result, value1);
#endif
                        push_int(stack, result);
                        opCode++;

                        break;
                    }

                    label_f2l:
                    case op_f2l: {
                        f32 value1 = pop_float(stack);
                        s64 result = (s64) value1;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("f2l: %lld <-- %f\n", result, value1);
#endif
                        push_long(stack, result);
                        opCode++;

                        break;
                    }

                    label_f2d:
                    case op_f2d: {
                        f32 value1 = pop_float(stack);
                        f64 result = value1;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("f2d: %f <-- %f\n", result, value1);
#endif
                        push_double(stack, result);
                        opCode++;

                        break;
                    }


                    label_d2i:
                    case op_d2i: {
                        f64 value1 = pop_double(stack);
                        s32 result = (s32) value1;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("d2i: %d <-- %lf\n", result, value1);
#endif
                        push_int(stack, result);
                        opCode++;

                        break;
                    }

                    label_d2l:
                    case op_d2l: {
                        f64 value1 = pop_double(stack);
                        s64 result = (s64) value1;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("d2l: %lld <-- %lf\n", result, value1);
#endif
                        push_long(stack, result);
                        opCode++;

                        break;
                    }

                    label_d2f:
                    case op_d2f: {
                        f64 value1 = pop_double(stack);
                        f32 result = (f32) value1;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("d2f: %f <-- %lf\n", result, value1);
#endif
                        push_float(stack, result);
                        opCode++;

                        break;
                    }

                    label_i2b:
                    case op_i2b: {
                        s32 value = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("i2b: %d --> %d\n", (s8) value, value);
#endif
                        push_int(stack, (s8) value);
                        opCode++;

                        break;
                    }


                    label_i2c:
                    case op_i2c: {
                        s32 value = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("i2s(c): %d --> %d\n", (s16) value, value);
#endif
                        push_int(stack, (u16) value);
                        opCode++;

                        break;
                    }
                    label_i2s:
                    case op_i2s: {
                        s32 value = pop_int(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("i2s(c): %d --> %d\n", (s16) value, value);
#endif
                        push_int(stack, (s16) value);
                        opCode++;

                        break;
                    }

                    label_lcmp:
                    case op_lcmp: {
                        s64 value1 = pop_long(stack);
                        s64 value2 = pop_long(stack);
                        s32 result = value2 == value1 ? 0 : (value2 > value1 ? 1 : -1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("lcmp: %llx cmp %llx = %d\n", value2, value1, result);
#endif
                        push_int(stack, result);

                        opCode++;

                        break;
                    }


                    label_fcmpl:
                    case op_fcmpl: {
                        f32 value1 = pop_float(stack);
                        f32 value2 = pop_float(stack);
                        s32 result = value2 == value1 ? 0 : (value2 > value1 ? 1 : -1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("fcmpl: %f < %f = %d\n", value2, value1, result);
#endif
                        push_int(stack, result);

                        opCode++;

                        break;
                    }

                    label_fcmpg:
                    case op_fcmpg: {
                        f32 value1 = pop_float(stack);
                        f32 value2 = pop_float(stack);
                        s32 result = value2 == value1 ? 0 : (value2 > value1 ? 1 : -1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("fcmpg: %f > %f = %d\n", value2, value1, result);
#endif
                        push_int(stack, result);

                        opCode++;

                        break;
                    }

                    label_dcmpl:
                    case op_dcmpl: {
                        f64 value1 = pop_double(stack);
                        f64 value2 = pop_double(stack);
                        s32 result = value2 == value1 ? 0 : (value2 > value1 ? 1 : -1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dcmpl: %lf < %lf = %d\n", value2, value1, result);
#endif
                        push_int(stack, result);

                        opCode++;

                        break;
                    }

                    label_dcmpg:
                    case op_dcmpg: {
                        f64 value1 = pop_double(stack);
                        f64 value2 = pop_double(stack);
                        s32 result = value2 == value1 ? 0 : (value2 > value1 ? 1 : -1);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("dcmpg: %lf > %lf = %d\n", value2, value1, result);
#endif
                        push_int(stack, result);

                        opCode++;

                        break;
                    }


                    label_ifeq:
                    case op_ifeq: {
                        s32 val = pop_int(stack);
                        if (val == 0) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_ifeq: %d != 0  then jump \n", val);
#endif


                        break;
                    }

                    label_ifne:
                    case op_ifne: {
                        s32 val = pop_int(stack);
                        if (val != 0) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_ifne: %d != 0  then jump\n", val);
#endif


                        break;
                    }


                    label_iflt:
                    case op_iflt: {
                        s32 val = pop_int(stack);
                        if (val < 0) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_iflt: %d < 0  then jump  \n", val);
#endif


                        break;
                    }

                    label_ifge:
                    case op_ifge: {
                        s32 val = pop_int(stack);
                        if (val >= 0) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_ifge: %d >= 0  then jump \n", val);
#endif


                        break;
                    }

                    label_ifgt:
                    case op_ifgt: {
                        s32 val = pop_int(stack);
                        if (val > 0) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_ifgt: %d > 0  then jump \n", val);
#endif


                        break;
                    }

                    label_ifle:
                    case op_ifle: {
                        s32 val = pop_int(stack);
                        if (val <= 0) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_ifle: %d <= 0  then jump \n", val);
#endif


                        break;
                    }

                    label_if_icmpeq:
                    case op_if_icmpeq: {
                        s32 v2 = pop_int(stack);
                        s32 v1 = pop_int(stack);
                        if (v1 == v2) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_if_icmpeq: %lld == %lld \n", (s64) (intptr_t) v1, (s64) (intptr_t) v2);
#endif

                        break;
                    }

                    label_if_icmpne:
                    case op_if_icmpne: {
                        s32 v2 = pop_int(stack);
                        s32 v1 = pop_int(stack);
                        if (v1 != v2) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_if_icmpne: %lld != %lld \n", (s64) (intptr_t) v1, (s64) (intptr_t) v2);
#endif

                        break;
                    }

                    label_if_icmplt:
                    case op_if_icmplt: {
                        s32 v2 = pop_int(stack);
                        s32 v1 = pop_int(stack);
                        if (v1 < v2) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_if_icmplt: %lld < %lld \n", (s64) (intptr_t) v1, (s64) (intptr_t) v2);
#endif

                        break;
                    }

                    label_if_icmpge:
                    case op_if_icmpge: {
                        s32 v2 = pop_int(stack);
                        s32 v1 = pop_int(stack);
                        if (v1 >= v2) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_if_icmpge: %lld >= %lld \n", (s64) (intptr_t) v1, (s64) (intptr_t) v2);
#endif

                        break;
                    }

                    label_if_icmpgt:
                    case op_if_icmpgt: {
                        s32 v2 = pop_int(stack);
                        s32 v1 = pop_int(stack);
                        if (v1 > v2) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_if_icmpgt: %lld > %lld \n", (s64) (intptr_t) v1, (s64) (intptr_t) v2);
#endif

                        break;
                    }


                    label_if_icmple:
                    case op_if_icmple: {
                        s32 v2 = pop_int(stack);
                        s32 v1 = pop_int(stack);
                        if (v1 <= v2) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_if_icmple: %lld <= %lld \n", (s64) (intptr_t) v1, (s64) (intptr_t) v2);
#endif

                        break;
                    }

                    label_if_acmpeq:
                    case op_if_acmpeq: {
                        __refer v2 = pop_ref(stack);
                        __refer v1 = pop_ref(stack);
                        if (v1 == v2) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_if_acmpeq: %lld == %lld \n", (s64) (intptr_t) v1, (s64) (intptr_t) v2);
#endif

                        break;
                    }

                    label_if_acmpne:
                    case op_if_acmpne: {
                        __refer v2 = pop_ref(stack);
                        __refer v1 = pop_ref(stack);
                        if (v1 != v2) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_if_acmpne: %lld != %lld \n", (s64) (intptr_t) v1, (s64) (intptr_t) v2);
#endif

                        break;
                    }

                    label_goto:
                    case op_goto: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        s32 branchoffset = s2c.s;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("goto: %d\n", branchoffset);
#endif
                        opCode += branchoffset;


                        break;
                    }

                    label_jsr:
                    case op_jsr: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        s32 branchoffset = s2c.s;
                        push_ref(stack, (__refer) (opCode + 3));
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("jsr: %d\n", branchoffset);
#endif
                        opCode += branchoffset;


                        break;
                    }

                    label_ret:
                    case op_ret: {
                        __refer addr = localvar_getRefer(localvar, (u8) opCode[1]);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ret: %x\n", (s64) (intptr_t) addr);
#endif
                        opCode = (u8 *) addr;


                        break;
                    }


                    label_tableswitch:
                    case op_tableswitch: {
                        s32 pos = 0;
                        pos = 4 - ((((u64) (intptr_t) opCode) - (u64) (intptr_t) (ca->code)) % 4);//4 byte对齐

                        Int2Float i2c;
                        i2c.c3 = opCode[pos++];
                        i2c.c2 = opCode[pos++];
                        i2c.c1 = opCode[pos++];
                        i2c.c0 = opCode[pos++];
                        s32 default_offset = i2c.i;
                        i2c.c3 = opCode[pos++];
                        i2c.c2 = opCode[pos++];
                        i2c.c1 = opCode[pos++];
                        i2c.c0 = opCode[pos++];
                        s32 low = i2c.i;
                        i2c.c3 = opCode[pos++];
                        i2c.c2 = opCode[pos++];
                        i2c.c1 = opCode[pos++];
                        i2c.c0 = opCode[pos++];
                        s32 high = i2c.i;

                        int val = pop_int(stack);// pop an int from the stack
                        int offset = 0;
                        if (val < low || val > high) {  // if its less than <low> or greater than <high>,
                            offset = default_offset;              // branch to default
                        } else {                        // otherwise
                            pos += (val - low) * 4;
                            i2c.c3 = opCode[pos++];
                            i2c.c2 = opCode[pos++];
                            i2c.c1 = opCode[pos++];
                            i2c.c0 = opCode[pos++];
                            offset = i2c.i;     // branch to entry in table
                        }

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("tableswitch: val=%d, offset=%d\n", val, offset);
#endif
                        opCode += offset;


                        break;
                    }

                    label_lookupswitch:
                    case op_lookupswitch: {
                        s32 pos = 0;
                        pos = 4 - ((((u64) (intptr_t) opCode) - (u64) (intptr_t) (ca->code)) % 4);//4 byte对齐
                        Int2Float i2c;
                        i2c.c3 = opCode[pos++];
                        i2c.c2 = opCode[pos++];
                        i2c.c1 = opCode[pos++];
                        i2c.c0 = opCode[pos++];
                        s32 default_offset = i2c.i;
                        i2c.c3 = opCode[pos++];
                        i2c.c2 = opCode[pos++];
                        i2c.c1 = opCode[pos++];
                        i2c.c0 = opCode[pos++];
                        s32 n = i2c.i;
                        s32 i, key;

                        int val = pop_int(stack);// pop an int from the stack
                        int offset = default_offset;
                        for (i = 0; i < n; i++) {
                            i2c.c3 = opCode[pos++];
                            i2c.c2 = opCode[pos++];
                            i2c.c1 = opCode[pos++];
                            i2c.c0 = opCode[pos++];
                            key = i2c.i;
                            if (key == val) {
                                i2c.c3 = opCode[pos++];
                                i2c.c2 = opCode[pos++];
                                i2c.c1 = opCode[pos++];
                                i2c.c0 = opCode[pos++];
                                offset = i2c.i;
                                break;
                            } else {
                                pos += 4;
                            }
                        }

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("tableswitch: val=%d, offset=%d\n", val, offset);
#endif
                        opCode += offset;

                        break;
                    }

                    label_lreturn:
                    label_dreturn:
                    case op_lreturn:
                    case op_dreturn: {
#if _JVM_DEBUG_BYTECODE_DETAIL > 5

                        StackEntry entry;
                        peek_entry(stack->sp - 1, &entry);
                        invoke_deepth(runtime);
                        jvm_printf("ld_return=[%x]/%d/[%llx]\n", entry_2_int(&entry), entry_2_int(&entry), entry_2_long(&entry));
#endif
                        s64 v = pop_long(stack);
                        localvar_dispose(runtime);
                        push_long(stack, v);
                        exit_exec = 1;
                        break;
                    }
                    label_ireturn:
                    label_freturn:
                    label_areturn:
                    case op_ireturn:
                    case op_freturn:
                    case op_areturn: {
                        StackEntry entry;
                        pop_entry(stack, &entry);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("ifa_return=[%x]/%d/[%llx]\n", entry_2_int(&entry), entry_2_int(&entry), entry_2_long(&entry));
#endif
                        localvar_dispose(runtime);
                        push_entry(stack, &entry);
                        exit_exec = 1;
                        break;
                    }

                    label_return:
                    case op_return: {
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("return: \n");
#endif
                        localvar_dispose(runtime);
                        exit_exec = 1;
                        break;
                    }

                    label_getstatic:
                    case op_getstatic: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        FieldInfo *fi = class_get_constant_fieldref(clazz, s2c.us)->fieldInfo;
                        if (!fi) {
                            ConstantFieldRef *cfr = class_get_constant_fieldref(clazz, s2c.us);
                            fi = find_fieldInfo_by_fieldref(clazz, cfr->item.index, runtime);
                            cfr->fieldInfo = fi;
                        }
                        c8 *ptr = getStaticFieldPtr(fi);

                        if (fi->isvolatile) {
                            barrier();
                        }
                        if (fi->isrefer) {
                            push_ref(stack, getFieldRefer(ptr));
                        } else {
                            // check variable type to determine s64/s32/f64/f32
                            s32 data_bytes = fi->datatype_bytes;
                            switch (data_bytes) {
                                case 4: {
                                    push_int(stack, getFieldInt(ptr));
                                    break;
                                }
                                case 1: {
                                    push_int(stack, getFieldByte(ptr));
                                    break;
                                }
                                case 8: {
                                    push_long(stack, getFieldLong(ptr));
                                    break;
                                }
                                case 2: {
                                    if (fi->datatype_idx == DATATYPE_JCHAR)push_int(stack, getFieldChar(ptr));
                                    else push_int(stack, getFieldShort(ptr));
                                    break;
                                }
                            }
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        StackEntry entry;
                        peek_entry(stack->sp - 1, &entry);
                        s64 v = entry_2_long(&entry);
                        jvm_printf("%s: push %s.%s[%llx]\n", "getstatic", utf8_cstr(clazz->name), utf8_cstr(fi->name), (s64) (intptr_t) ptr, v);
#endif
                        opCode += 3;
                        break;
                    }

                    label_putstatic:
                    case op_putstatic: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        FieldInfo *fi = class_get_constant_fieldref(clazz, s2c.us)->fieldInfo;
                        if (!fi) {
                            ConstantFieldRef *cfr = class_get_constant_fieldref(clazz, s2c.us);
                            fi = find_fieldInfo_by_fieldref(clazz, cfr->item.index, runtime);
                            cfr->fieldInfo = fi;
                        }

                        c8 *ptr = getStaticFieldPtr(fi);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        StackEntry entry;
                        peek_entry(stack->sp - 1, &entry);
                        invoke_deepth(runtime);
                        jvm_printf("%s  save:%s.%s[%llx]=[%llx]  \n", "putstatic", utf8_cstr(clazz->name), utf8_cstr(fi->name), (s64) (intptr_t) ptr, entry_2_long(&entry));
#endif

                        if (fi->isrefer) {//垃圾回收标识
                            setFieldRefer(ptr, pop_ref(stack));
                        } else {
                            // check variable type to determain long/s32/f64/f32
                            s32 data_bytes = fi->datatype_bytes;
                            //非引用类型
                            switch (data_bytes) {
                                case 4: {
                                    setFieldInt(ptr, pop_int(stack));
                                    break;
                                }
                                case 1: {
                                    setFieldByte(ptr, pop_int(stack));
                                    break;
                                }
                                case 8: {
                                    setFieldLong(ptr, pop_long(stack));
                                    break;
                                }
                                case 2: {
                                    setFieldShort(ptr, pop_int(stack));
                                    break;
                                }
                            }
                        }

                        opCode += 3;
                        break;
                    }

                    label_getfield:
                    case op_getfield: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];


                        Instance *ins = (Instance *) pop_ref(stack);
                        if (!ins) {
                            Instance *exception = exception_create(JVM_EXCEPTION_NULLPOINTER, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            FieldInfo *fi = class_get_constant_fieldref(clazz, s2c.us)->fieldInfo;
                            if (!fi) {
                                ConstantFieldRef *cfr = class_get_constant_fieldref(clazz, s2c.us);
                                fi = find_fieldInfo_by_fieldref(clazz, cfr->item.index, runtime);
                                cfr->fieldInfo = fi;
                            }
                            c8 *ptr = getInstanceFieldPtr(ins, fi);

                            if (fi->isvolatile) {
                                barrier();
                            }
                            if (fi->isrefer) {
                                push_ref(stack, getFieldRefer(ptr));
                            } else {
                                // check variable type to determine s64/s32/f64/f32
                                s32 data_bytes = fi->datatype_bytes;
                                switch (data_bytes) {
                                    case 4: {
                                        push_int(stack, getFieldInt(ptr));
                                        break;
                                    }
                                    case 1: {
                                        push_int(stack, getFieldByte(ptr));
                                        break;
                                    }
                                    case 8: {
                                        push_long(stack, getFieldLong(ptr));
                                        break;
                                    }
                                    case 2: {
                                        if (fi->datatype_idx == DATATYPE_JCHAR)push_int(stack, getFieldChar(ptr));
                                        else push_int(stack, getFieldShort(ptr));
                                        break;
                                    }
                                }
                            }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            invoke_deepth(runtime);
                            StackEntry entry;
                            peek_entry(stack->sp - 1, &entry);
                            s64 v = entry_2_long(&entry);
                            jvm_printf("%s: push %s.%s[%llx]\n", "getfield", utf8_cstr(clazz->name), utf8_cstr(fi->name), (s64) (intptr_t) ptr, v);
#endif

                            opCode += 3;
                        }
                        break;
                    }


                    label_putfield:
                    case op_putfield: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        StackEntry entry;
                        pop_entry(stack, &entry);
                        if (entry.type & (STACK_ENTRY_LONG | STACK_ENTRY_DOUBLE)) {
                            pop_entry(stack, &entry);
                        }

                        Instance *ins = (Instance *) pop_ref(stack);
                        if (!ins) {
                            Instance *exception = exception_create(JVM_EXCEPTION_NULLPOINTER, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            // check variable type to determain long/s32/f64/f32
                            FieldInfo *fi = class_get_constant_fieldref(clazz, s2c.us)->fieldInfo;
                            if (!fi) {
                                ConstantFieldRef *cfr = class_get_constant_fieldref(clazz, s2c.us);
                                fi = find_fieldInfo_by_fieldref(clazz, cfr->item.index, runtime);
                                cfr->fieldInfo = fi;
                            }
                            c8 *ptr = getInstanceFieldPtr(ins, fi);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                            if (utf8_equals_c(fi->name, "count") && utf8_equals_c(fi->_this_class->name, "java/lang/StringBuilder")) {
                                int debug = 1;
                            }
                            invoke_deepth(runtime);
                            jvm_printf("%s  save:%s.%s[%llx]=[%llx]  \n", "putfield", utf8_cstr(clazz->name), utf8_cstr(fi->name), (s64) (intptr_t) ptr, entry_2_long(&entry));
#endif

                            if (fi->isrefer) {//垃圾回收标识
                                setFieldRefer(ptr, entry_2_refer(&entry));
                            } else {
                                s32 data_bytes = fi->datatype_bytes;
                                //非引用类型
                                switch (data_bytes) {
                                    case 4: {
                                        setFieldInt(ptr, entry_2_int(&entry));
                                        break;
                                    }
                                    case 1: {
                                        setFieldByte(ptr, entry_2_int(&entry));
                                        break;
                                    }
                                    case 8: {
                                        setFieldLong(ptr, entry_2_long(&entry));
                                        break;
                                    }
                                    case 2: {
                                        setFieldShort(ptr, entry_2_int(&entry));
                                        break;
                                    }
                                }
                            }

                            opCode += 3;
                        }
                        break;
                    }

                    label_invokevirtual:
                    case op_invokevirtual: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        //此cmr所描述的方法，对于不同的实例，有不同的method
                        ConstantMethodRef *cmr = class_get_constant_method_ref(clazz, s2c.us);

                        Instance *ins = getInstanceInStack(cmr, stack);
                        if (ins == NULL) {
                            Instance *exception = exception_create(JVM_EXCEPTION_NULLPOINTER, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                        } else {
                            MethodInfo *m = (MethodInfo *) pairlist_get(cmr->virtual_methods, ins->mb.clazz);
                            if (m == NULL) {
                                m = find_instance_methodInfo_by_name(ins, cmr->name, cmr->descriptor, runtime);
                                pairlist_put(cmr->virtual_methods, ins->mb.clazz, m);//放入缓存，以便下次直接调用
                            }


#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                            if (utf8_equals_c(cmr->clsName, "java/io/FileInputStream")
                                && utf8_equals_c(cmr->name, "open")
                                //                                && utf8_equals_c(cmr->descriptor, "(Ljava/lang/String;)Ljava/lang/StringBuilder;")
                                    ) {
                                int debug = 1;
                            }
                            invoke_deepth(runtime);
                            jvm_printf("invokevirtual    %s.%s%s  {\n", utf8_cstr(m->_this_class->name), utf8_cstr(m->name), utf8_cstr(m->descriptor));
#endif

#if _JVM_DEBUG_PROFILE
                            spent = nanoTime() - start_at;
#endif
                            if (m) {
                                ret = execute_method_impl(m, runtime);
                            } else {
                                Instance *exception = exception_create_str(JVM_EXCEPTION_NOSUCHMETHOD, runtime,
                                                                           utf8_cstr(cmr->name));
                                push_ref(stack, (__refer) exception);
                                ret = RUNTIME_STATUS_EXCEPTION;
                            }
#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                            invoke_deepth(runtime);
                            jvm_printf("}\n");
#endif
                        }
                        if (ret == RUNTIME_STATUS_EXCEPTION) {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            opCode += 3;
                        }
                        break;
                    }


                    label_invokespecial:
                    case op_invokespecial: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        ConstantMethodRef *cmr = class_get_constant_method_ref(clazz, s2c.us);
                        MethodInfo *m = cmr->methodInfo;

#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                        if (utf8_equals_c(cmr->clsName, "org/mini/fs/InnerFile")
                            && utf8_equals_c(cmr->name, "<init>")
                            //                                && utf8_equals_c(cmr->descriptor, "(Ljava/lang/String;)Ljava/lang/StringBuilder;")
                                ) {
                            int debug = 1;
                        }
                        invoke_deepth(runtime);
                        jvm_printf("invokespecial    %s.%s%s {\n", utf8_cstr(m->_this_class->name), utf8_cstr(m->name), utf8_cstr(m->descriptor));
#endif
#if _JVM_DEBUG_PROFILE
                        spent = nanoTime() - start_at;
#endif
                        if (m) {
                            ret = execute_method_impl(m, runtime);
                        } else {
                            Instance *exception = exception_create_str(JVM_EXCEPTION_NOSUCHMETHOD, runtime, utf8_cstr(cmr->name));
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                        invoke_deepth(runtime);
                        jvm_printf("}\n");
#endif

                        if (ret == RUNTIME_STATUS_EXCEPTION) {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            opCode += 3;
                        }
                        break;
                    }


                    label_invokestatic:
                    case op_invokestatic: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];
                        ConstantMethodRef *cmr = class_get_constant_method_ref(clazz, s2c.us);

                        MethodInfo *m = cmr->methodInfo;
#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                        if (utf8_equals_c(cmr->name, "readbuf") && utf8_equals_c(cmr->clsName, "org/mini/fs/InnerFile")) {
                            int debug = 1;
                        }

                        invoke_deepth(runtime);
                        jvm_printf("invokestatic   | %s.%s%s {\n", utf8_cstr(m->_this_class->name), utf8_cstr(m->name), utf8_cstr(m->descriptor));
#endif
#if _JVM_DEBUG_PROFILE
                        spent = nanoTime() - start_at;
#endif
                        if (m) {
                            ret = execute_method_impl(m, runtime);
                        } else {
                            Instance *exception = exception_create_str(JVM_EXCEPTION_NOSUCHMETHOD, runtime, utf8_cstr(cmr->name));
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                        invoke_deepth(runtime);
                        jvm_printf("}\n");
#endif

                        if (ret == RUNTIME_STATUS_EXCEPTION) {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            opCode += 3;
                        }
                        break;
                    }


                    label_invokeinterface:
                    case op_invokeinterface: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        s32 paraCount = (u8) opCode[3];

                        ConstantMethodRef *cmr = class_get_constant_method_ref(clazz, s2c.us);
                        Instance *ins = getInstanceInStack(cmr, stack);
                        if (ins == NULL) {
                            Instance *exception = exception_create(JVM_EXCEPTION_NULLPOINTER, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                        } else {
                            //                            if (utf8_equals_c(cmr->name, "forEach") && utf8_equals_c(cmr->clsName, "java/util/List")) {
                            //                                int debug = 1;
                            //                            }
                            //                            if (utf8_equals_c(cmr->name, "hasNext") && utf8_equals_c(cmr->clsName, "java/util/Iterator")) {
                            //                                int debug = 1;
                            //                            }
                            MethodInfo *m = (MethodInfo *) pairlist_get(cmr->virtual_methods, ins->mb.clazz);
                            if (m == NULL) {
                                m = find_instance_methodInfo_by_name(ins, cmr->name, cmr->descriptor, runtime);
                                pairlist_put(cmr->virtual_methods, ins->mb.clazz, m);//放入缓存，以便下次直接调用
                            }
//                            }

#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                            invoke_deepth(runtime);
                            jvm_printf("invokeinterface   | %s.%s%s {\n", utf8_cstr(m->_this_class->name),
                                       utf8_cstr(m->name), utf8_cstr(m->descriptor));
#endif
#if _JVM_DEBUG_PROFILE
                            spent = nanoTime() - start_at;
#endif
                            if (m) {
                                ret = execute_method_impl(m, runtime);
                            } else {
                                Instance *exception = exception_create_str(JVM_EXCEPTION_NOSUCHMETHOD, runtime, utf8_cstr(cmr->name));
                                push_ref(stack, (__refer) exception);
                                ret = RUNTIME_STATUS_EXCEPTION;
                            }
#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                            invoke_deepth(runtime);
                            jvm_printf("}\n");
#endif

                        }
                        if (ret == RUNTIME_STATUS_EXCEPTION) {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            opCode += 5;
                        }
                        break;
                    }

                    label_invokedynamic:
                    case op_invokedynamic: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        //get bootMethod struct
                        ConstantInvokeDynamic *cid = class_get_invoke_dynamic(clazz, s2c.us);
                        BootstrapMethod *bootMethod = &clazz->bootstrapMethodAttr->bootstrap_methods[cid->bootstrap_method_attr_index];//Boot

                        if (bootMethod->make == NULL) {
                            /**
						* run bootstrap method java.lang.invoke.LambdaMetafactory
						*
						* public static CallSite metafactory(
                        *           MethodHandles.Lookup caller,
						*           String invokedName,
						*           MethodType invokedType,
						*           MethodType samMethodType,
						*           MethodHandle implMethod,
						*           MethodType instantiatedMethodType
                        *           )
						*
						*
						*  to generate Lambda Class implementation specify interface
						*  and new a callsite
						*/

                            //parper bootMethod parameter
                            Instance *lookup = method_handles_lookup_create(runtime, clazz);
                            push_ref(stack, lookup); //lookup

                            Utf8String *ustr_invokeName = class_get_constant_utf8(clazz, class_get_constant_name_and_type(clazz, cid->nameAndTypeIndex)->nameIndex)->utfstr;
                            Instance *jstr_invokeName = jstring_create(ustr_invokeName, runtime);
                            push_ref(stack, jstr_invokeName); //invokeName

                            Utf8String *ustr_invokeType = class_get_constant_utf8(clazz, class_get_constant_name_and_type(clazz, cid->nameAndTypeIndex)->typeIndex)->utfstr;
                            Instance *mt_invokeType = method_type_create(runtime, ustr_invokeType);
                            push_ref(stack, mt_invokeType); //invokeMethodType

                            //other bootMethod parameter

                            s32 i;
                            for (i = 0; i < bootMethod->num_bootstrap_arguments; i++) {
                                ConstantItem *item = class_get_constant_item(clazz, bootMethod->bootstrap_arguments[i]);
                                switch (item->tag) {
                                    case CONSTANT_METHOD_TYPE: {
                                        ConstantMethodType *cmt = (ConstantMethodType *) item;
                                        Utf8String *arg = class_get_constant_utf8(clazz, cmt->descriptor_index)->utfstr;
                                        Instance *mt = method_type_create(runtime, arg);
                                        push_ref(stack, mt);
                                        break;
                                    }
                                    case CONSTANT_STRING_REF: {
                                        ConstantStringRef *csr = (ConstantStringRef *) item;
                                        Utf8String *arg = class_get_constant_utf8(clazz, csr->stringIndex)->utfstr;
                                        Instance *spec = jstring_create(arg, runtime);
                                        push_ref(stack, spec);
                                        break;
                                    }
                                    case CONSTANT_METHOD_HANDLE: {
                                        ConstantMethodHandle *cmh = (ConstantMethodHandle *) item;
                                        MethodInfo *mip = find_methodInfo_by_methodref(clazz, cmh->reference_index, runtime);
                                        Instance *mh = method_handle_create(runtime, mip, cmh->reference_kind);
                                        push_ref(stack, mh);
                                        break;
                                    }
                                    default: {
                                        jvm_printf("invokedynamic para parse error.");
                                    }
                                }

                            }

                            //get bootmethod
                            MethodInfo *boot_m = find_methodInfo_by_methodref(clazz, class_get_method_handle(clazz, bootMethod->bootstrap_method_ref)->reference_index, runtime);

                            if (boot_m) {

                                ret = execute_method_impl(boot_m, runtime);
                                if (ret == RUNTIME_STATUS_NORMAL) {
                                    MethodInfo *finder = find_methodInfo_by_name_c("org/mini/reflect/vm/LambdaUtil", "getMethodInfoHandle", "(Ljava/lang/invoke/CallSite;)J", runtime);
                                    if (finder) {
                                        //run finder to convert calsite.target(MethodHandle) to MethodInfo * pointer
                                        ret = execute_method_impl(finder, runtime);
                                        if (ret == RUNTIME_STATUS_NORMAL) {
                                            MethodInfo *make = (MethodInfo *) (intptr_t) pop_long(stack);
                                            bootMethod->make = make;
                                        }
                                    } else {
                                        Instance *exception = exception_create(JVM_EXCEPTION_NOSUCHMETHOD, runtime);
                                        push_ref(stack, (__refer) exception);
                                        ret = RUNTIME_STATUS_EXCEPTION;
                                    }
                                }
                            } else {
                                Instance *exception = exception_create(JVM_EXCEPTION_NOSUCHMETHOD, runtime);
                                push_ref(stack, (__refer) exception);
                                ret = RUNTIME_STATUS_EXCEPTION;
                            }
                        }
                        MethodInfo *m = bootMethod->make;
#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                        invoke_deepth(runtime);
                        jvm_printf("invokedynamic   | %s.%s%s {\n", utf8_cstr(m->_this_class->name),
                                   utf8_cstr(m->name), utf8_cstr(m->descriptor));
#endif
#if _JVM_DEBUG_PROFILE
                        spent = nanoTime() - start_at;
#endif
                        if (ret == RUNTIME_STATUS_NORMAL) {
                            if (m) {
                                // run make to generate instance of Lambda Class
                                ret = execute_method_impl(m, runtime);
                            } else {
                                Instance *exception = exception_create(JVM_EXCEPTION_NOSUCHMETHOD, runtime);
                                push_ref(stack, (__refer) exception);
                                ret = RUNTIME_STATUS_EXCEPTION;
                            }
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 3
                        invoke_deepth(runtime);
                        jvm_printf("}\n");
#endif
                        if (ret == RUNTIME_STATUS_EXCEPTION) {
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            opCode += 5;
                        }
                        break;
                    }


                    label_new:
                    case op_new: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        ConstantClassRef *ccf = class_get_constant_classref(clazz, s2c.us);
                        if (!ccf->clazz) {
                            Utf8String *clsName = class_get_utf8_string(clazz, ccf->stringIndex);
                            ccf->clazz = classes_load_get(clsName, runtime);
                        }
                        JClass *other = ccf->clazz;
                        Instance *ins = NULL;
                        if (other) {
                            ins = instance_create(runtime, other);
                        }
                        push_ref(stack, (__refer) ins);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("new %s [%llx]\n", utf8_cstr(ccf->name), (s64) (intptr_t) ins);
#endif
                        opCode += 3;

                        break;
                    }


                    label_newarray:
                    case op_newarray: {
                        s32 typeIdx = opCode[1];

                        s32 count = pop_int(stack);

                        Instance *arr = jarray_create_by_type_index(runtime, count, typeIdx);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("(a)newarray  [%llx] type:%c , count:%d  \n", (s64) (intptr_t) arr, getDataTypeTag(typeIdx), count);
#endif
                        if (arr) {
                            push_ref(stack, (__refer) arr);
                            opCode += 2;
                        } else {
                            Instance *exception = exception_create(JVM_EXCEPTION_NULLPOINTER, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }

                        break;
                    }

                    label_anewarray:
                    case op_anewarray: {
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        s32 count = pop_int(stack);

                        JClass *arr_class = pairlist_get(clazz->arr_class_type, (__refer) (intptr_t) s2c.us);

                        Instance *arr = NULL;
                        if (!arr_class) {//cache to speed
                            arr_class = array_class_get_by_name(runtime, class_get_utf8_string(clazz, s2c.us));
                            pairlist_put(clazz->arr_class_type, (__refer) (intptr_t) s2c.us, arr_class);
                        }
                        arr = jarray_create_by_class(runtime, count, arr_class);


#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("(a)newarray  [%llx] type:%d , count:%d  \n", (s64) (intptr_t) arr, arr_class->arr_class_type, count);
#endif
                        if (arr) {
                            push_ref(stack, (__refer) arr);
                            opCode += 3;
                        } else {
                            Instance *exception = exception_create(JVM_EXCEPTION_NULLPOINTER, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }

                        break;
                    }

                    label_arraylength:
                    case op_arraylength: {
                        Instance *arr_ref = (Instance *) pop_ref(stack);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("arraylength  [%llx].arr_body[%llx] len:%d  \n",
                                   (s64) (intptr_t) arr_ref, (s64) (intptr_t) arr_ref->arr_body, arr_ref->arr_length);
#endif
                        if (arr_ref == NULL) {
                            Instance *exception = exception_create(JVM_EXCEPTION_NULLPOINTER, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            push_int(stack, arr_ref->arr_length);
                            opCode++;
                        }
                        break;
                    }


                    label_athrow:
                    case op_athrow: {
                        Instance *ins = (Instance *) pop_ref(stack);
                        push_ref(stack, (__refer) ins);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("athrow  [%llx].exception throws  \n", (s64) (intptr_t) ins);
#endif
                        //opCode +=  1;
                        ret = RUNTIME_STATUS_EXCEPTION;
                        if (exception_handle(stack, runtime)) {
                            exit_exec = 1;
                        } else {
                            opCode = runtime->pc;
                            ret = RUNTIME_STATUS_NORMAL;
                        }
                        break;
                    }

                    label_checkcast:
                    case op_checkcast: {
                        Instance *ins = (Instance *) pop_ref(stack);
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        s32 typeIdx = s2c.us;
                        s32 checkok = 0;
                        if (ins != NULL) {
                            if (ins->mb.type == MEM_TYPE_INS) {
                                JClass *cl = getClassByConstantClassRef(clazz, typeIdx, runtime);
                                //                                if (utf8_equals_c(ins->mb.clazz->name, "java/lang/String")&&utf8_equals_c(cl->name,"java/lang/Comparable")) {//
                                //                                    int debug = 1;
                                //                                }
                                if (instance_of(cl, ins, runtime)) {
                                    checkok = 1;
                                }
                            } else if (ins->mb.type == MEM_TYPE_ARR) {
                                Utf8String *utf = class_get_constant_classref(clazz, typeIdx)->name;
                                u8 ch = utf8_char_at(utf, 1);
                                if (getDataTypeIndex(ch) == ins->mb.clazz->mb.arr_type_index) {
                                    checkok = 1;
                                }
                            } else if (ins->mb.type == MEM_TYPE_CLASS) {
                                Utf8String *utf = class_get_constant_classref(clazz, typeIdx)->name;
                                if (utf8_equals_c(utf, STR_CLASS_JAVA_LANG_CLASS)) {
                                    checkok = 1;
                                }
                            }
                        } else {
                            checkok = 1;
                        }
                        if (!checkok) {
                            Instance *exception = exception_create(JVM_EXCEPTION_CLASSCAST, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        } else {
                            push_ref(stack, (__refer) ins);
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("checkcast  [%llx] instancof %s is:%d \n", (s64) (intptr_t) ins,
                                   utf8_cstr(class_get_constant_classref(clazz, typeIdx)->name),
                                   checkok);
#endif

                        break;
                    }


                    label_instanceof:
                    case op_instanceof: {
                        Instance *ins = (Instance *) pop_ref(stack);
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        s32 typeIdx = s2c.us;

                        s32 checkok = 0;
                        if (ins == NULL) {
                        } else if (ins->mb.type & (MEM_TYPE_INS | MEM_TYPE_ARR)) {
                            if (instance_of(getClassByConstantClassRef(clazz, typeIdx, runtime), ins, runtime)) {
                                checkok = 1;
                            }
                        }
                        //                        else {
                        //                            if (utf8_equals(ins->mb.clazz->name, getClassByConstantClassRef(clazz, typeIdx)->name)) {//
                        //                                checkok = 1;
                        //                            }
                        //                        }
                        push_int(stack, checkok);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("instanceof  [%llx] instancof %s  \n", (s64) (intptr_t) ins, utf8_cstr(class_get_constant_classref(clazz, typeIdx)->name));
#endif
                        opCode += 3;

                        break;
                    }

                    label_monitorenter:
                    case op_monitorenter: {
                        Instance *ins = (Instance *) pop_ref(stack);
                        jthread_lock(&ins->mb, runtime);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("monitorenter  [%llx] %s  \n", (s64) (intptr_t) ins, ins ? utf8_cstr(ins->mb.clazz->name) : "null");
#endif
                        opCode++;

                        break;
                    }

                    label_monitorexit:
                    case op_monitorexit: {
                        Instance *ins = (Instance *) pop_ref(stack);
                        jthread_unlock(&ins->mb, runtime);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("monitorexit  [%llx] %s  \n", (s64) (intptr_t) ins, ins ? utf8_cstr(ins->mb.clazz->name) : "null");
#endif
                        opCode++;

                        break;
                    }

                    label_wide:
                    case op_wide: {
#if _JVM_DEBUG_BYTECODE_DETAIL > 5

                        invoke_deepth(runtime);
                        jvm_printf("wide  \n");
#endif
                        opCode++;

                        runtime->pc = opCode;
                        cur_inst = *opCode;
                        switch (cur_inst) {
                            case op_iload:
                            case op_fload:
                            case op_aload: {
                                Short2Char s2c;
                                s2c.c1 = opCode[1];
                                s2c.c0 = opCode[2];
                                _op_load_1_slot(stack, localvar, runtime, s2c.us);
                                opCode += 3;
                                break;
                            }
                            case op_lload:
                            case op_dload: {
                                Short2Char s2c;
                                s2c.c1 = opCode[1];
                                s2c.c0 = opCode[2];
                                _op_load_2_slot(stack, localvar, runtime, s2c.us);
                                opCode += 3;
                                break;
                            }
                            case op_istore:
                            case op_fstore:
                            case op_astore: {
                                Short2Char s2c;
                                s2c.c1 = opCode[1];
                                s2c.c0 = opCode[2];
                                _op_store_1_slot(stack, localvar, runtime, s2c.us);
                                opCode += 3;
                                break;
                            }
                            case op_lstore:
                            case op_dstore: {
                                Short2Char s2c;
                                s2c.c1 = opCode[1];
                                s2c.c0 = opCode[2];
                                _op_store_2_slot(stack, localvar, runtime, s2c.us);
                                opCode += 3;
                                break;
                            }
                            case op_ret: {
                                Short2Char s2c;
                                s2c.c1 = opCode[1];
                                s2c.c0 = opCode[2];
                                __refer addr = localvar_getRefer(localvar, s2c.us);

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                                invoke_deepth(runtime);
                                jvm_printf("wide ret: %x\n", (s64) (intptr_t) addr);
#endif
                                opCode = (u8 *) addr;
                                break;
                            }
                            case op_iinc    : {
                                Short2Char s2c1, s2c2;

                                s2c1.c1 = opCode[1];
                                s2c1.c0 = opCode[2];
                                s2c2.c1 = opCode[3];
                                s2c2.c0 = opCode[4];

                                localvar[s2c1.us].ivalue += s2c2.s;
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                                invoke_deepth(runtime);
                                jvm_printf("wide iinc: localvar(%d) = %d , inc %d\n", s2c1.s, localvar[s2c1.us].ivalue, s2c2.s);
#endif
                                opCode += 5;
                                break;
                            }
                            default:
                                jvm_printf("instruct wide %x not found\n", cur_inst);
                        }
                        break;
                    }

                    label_multianewarray:
                    case op_multianewarray: {
                        //data type index
                        Short2Char s2c;
                        s2c.c1 = opCode[1];
                        s2c.c0 = opCode[2];

                        Utf8String *desc = class_get_utf8_string(clazz, s2c.us);
                        //array dim
                        s32 count = (u8) opCode[3];
#ifdef __JVM_OS_VS__
                        s32 dim[32];
#else
                        s32 dim[count];
#endif
                        int i;
                        for (i = 0; i < count; i++)
                            dim[i] = pop_int(stack);

                        Instance *arr = jarray_multi_create(runtime, dim, count, desc, 0);
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("multianewarray  [%llx] type:%s , count:%d  \n", (s64) (intptr_t) arr,
                                   utf8_cstr(desc), count);
#endif
                        if (arr) {
                            push_ref(stack, (__refer) arr);
                            opCode += 4;
                        } else {
                            Instance *exception = exception_create(JVM_EXCEPTION_NULLPOINTER, runtime);
                            push_ref(stack, (__refer) exception);
                            ret = RUNTIME_STATUS_EXCEPTION;
                            if (exception_handle(stack, runtime)) {
                                exit_exec = 1;
                            } else {
                                opCode = runtime->pc;
                                ret = RUNTIME_STATUS_NORMAL;
                            }
                        }
                        break;
                    }


                    label_ifnull:
                    case op_ifnull: {
                        __refer ref = pop_ref(stack);
                        if (!ref) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_ifnonnull: %d/%llx != 0  then jump %d \n", (s32) (intptr_t) ref,
                                   (s64) (intptr_t) ref);
#endif


                        break;
                    }

                    label_ifnonnull:
                    case op_ifnonnull: {
                        __refer ref = pop_ref(stack);
                        if (ref) {
                            Short2Char s2c;
                            s2c.c1 = opCode[1];
                            s2c.c0 = opCode[2];
                            opCode += s2c.s;
                        } else {
                            opCode += 3;
                        }
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("op_ifnonnull: %d/%llx != 0  then \n", (s32) (intptr_t) ref, (s64) (intptr_t) ref);
#endif

                        break;
                    }

                    label_breakpoint:
                    case op_breakpoint: {
#if _JVM_DEBUG_BYTECODE_DETAIL > 5

                        invoke_deepth(runtime);
                        jvm_printf("breakpoint \n");
#endif
                        //opCode +=  1;

                        break;
                    }


                    label_goto_w:
                    case op_goto_w: {
                        Int2Float i2f;
                        i2f.c3 = opCode[1];
                        i2f.c2 = opCode[2];
                        i2f.c1 = opCode[3];
                        i2f.c0 = opCode[4];

                        s32 branchoffset = i2f.i;

#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("goto: %d\n", branchoffset);
#endif
                        opCode += branchoffset;


                        break;
                    }

                    label_jsr_w:
                    case op_jsr_w: {
                        Int2Float i2f;
                        i2f.c3 = opCode[1];
                        i2f.c2 = opCode[2];
                        i2f.c1 = opCode[3];
                        i2f.c0 = opCode[4];

                        s32 branchoffset = i2f.i;
                        push_ref(stack, (__refer) (opCode + 3));
#if _JVM_DEBUG_BYTECODE_DETAIL > 5
                        invoke_deepth(runtime);
                        jvm_printf("jsr_w: %d\n", branchoffset);
#endif
                        opCode += branchoffset;


                        break;
                    }

                    default:
                        jvm_printf("instruct %x not found\n", cur_inst);
                }

                /* ================================== opcode end =============================*/


#if _JVM_DEBUG_PROFILE
                //time
                if (!spent) spent = nanoTime() - start_at;
                profile_put(cur_inst, spent, 1);
#endif

            }//end while
            if (method_sync)_synchronized_unlock_method(method, runtime);

        } else {
            jvm_printf("method code attribute is null.");
        }
    } else {//本地方法
        localvar_init(runtime, method->para_slots, method->para_slots);
        //缓存调用本地方法
        if (!method->native_func) { //把本地方法找出来缓存
            java_native_method *native = find_native_method(utf8_cstr(clazz->name), utf8_cstr(method->name),
                                                            utf8_cstr(method->descriptor));
            if (!native) {
                Instance *exception = exception_create_str(JVM_EXCEPTION_NOSUCHMETHOD, runtime,
                                                           utf8_cstr(method->name));
                push_ref(stack, (__refer) exception);
                ret = RUNTIME_STATUS_EXCEPTION;
            } else {
                method->native_func = native->func_pointer;
            }
        }

        if (method->native_func) {
            if (method_sync)_synchronized_lock_method(method, runtime);
            ret = method->native_func(runtime, clazz);
            if (method_sync)_synchronized_unlock_method(method, runtime);
            switch (method->return_slots) {
                case 0: {// V
                    localvar_dispose(runtime);
                    break;
                }
                case 1: { // F I R
                    StackEntry entry;
                    peek_entry(stack->sp - method->return_slots, &entry);
                    localvar_dispose(runtime);
                    push_entry(stack, &entry);
                    break;
                }
                case 2: {//J D return type , 2slots
                    s64 v = pop_long(stack);
                    localvar_dispose(runtime);
                    push_long(stack, v);
                    break;
                }
            }
        }
    }


#if _JVM_DEBUG_BYTECODE_DETAIL > 3
    Utf8String *ustr = method->descriptor;
    invoke_deepth(runtime);
    jvm_printf("stack size  %s.%s%s in:%d out:%d  \n", utf8_cstr(clazz->name), utf8_cstr(method->name), utf8_cstr(method->descriptor), (runtime->stack->sp - runtime->localvar), stack_size(stack));
    if (ret != RUNTIME_STATUS_EXCEPTION) {
        if (method->return_slots) {//无反回值
            if (stack->sp != runtime->localvar + method->return_slots) {
                exit(1);
            }
        }
    }
#endif
    runtime_destory_inl(runtime);
    pruntime->son = NULL;  //need for getLastSon()
    return ret;
}

