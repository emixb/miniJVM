/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.reflect;

import org.mini.reflect.vm.RConst;
import org.mini.reflect.vm.RefNative;

/**
 *
 * @author Gust
 */
public class DirectMemObj {

    /**
     * on DirectMemObj finalize call
     */
    public interface DMOFinalizer {

        void onFinalize();
    }

    //dont change field name
    long memAddr;
    int length;
    char typeDesc; //'1','2','4','8','R'
    //
    public byte typeTag;
    DMOFinalizer finalizer;

    public DirectMemObj(long mem_addr, int len) {
        this(mem_addr, len, RConst.TAG_BYTE);
    }

    /**
     * 
     * typeTag is RConst.xxxtag
     *
     * @param mem_addr
     * @param len
     * @param type
     */
    public DirectMemObj(long mem_addr, int len, byte type) {
        this.memAddr = mem_addr;
        this.typeTag = type;
        this.length = len;
        typeDesc = RConst.getBytes(type);
    }

    public void setFinalizer(DMOFinalizer finalizer) {
        this.finalizer = finalizer;
    }

    /**
     *
     */
    @Override
    public void finalize() {
        try {
            super.finalize();
        } catch (Throwable ex) {
        }
        if (finalizer != null) {
            finalizer.onFinalize();
            System.out.println("DMO finalized");
        }
    }

    /**
     * return data start memory address
     *
     * @return
     */
    public long getDataPtr() {
        return memAddr;
    }

    public int getLength() {
        return length;
    }

    public byte getTypeTag() {
        return typeTag;
    }

    public char getDesc() {
        return typeDesc;
    }

    public void setValObj(int index, Object val) {
        switch (typeDesc) {
            case '1': {
                if (typeTag == RConst.TAG_BOOLEAN) {
                    setVal(index, ((Boolean) val) ? 1 : 0);

                } else {
                    setVal(index, (Byte) val);
                }
                break;
            }
            case '2': {
                if (typeTag == RConst.TAG_CHAR) {
                    setVal(index, (Character) val);

                } else {
                    setVal(index, (Byte) val);
                }
                break;
            }
            case '4': {
                setVal(index, (Integer) val);
                break;
            }
            case '8': {
                setVal(index, (Long) val);
                break;
            }
            case 'R': {
                setVal(index, RefNative.obj2id(val));
                break;
            }
            default: {

                throw new IllegalArgumentException();
            }
        }
    }

    public Object getValObj(int index) {

        switch (typeDesc) {
            case '1':
                if (typeTag == RConst.TAG_BOOLEAN) {
                    return getVal(index) != 0;
                }
                return ((byte) getVal(index));

            case '2':
                if (typeTag == RConst.TAG_CHAR) {
                    return ((char) getVal(index));
                }
                return ((short) getVal(index));
            case '4':
                return ((int) getVal(index));
            case '8':
                return getVal(index);
            case 'R': {
                long objptr = getVal(index);
                if (objptr != 0) {
                    return RefNative.id2obj(objptr);
                }
                return null;
            }
        }
        throw new IllegalArgumentException();
    }

    public Object getObject(int index) {
        return RefNative.id2obj(getVal(index));
    }

    public byte getByte(int index) {
        return (byte) getVal(index);
    }

    public short getShort(int index) {
        return (short) getVal(index);
    }

    public char getChar(int index) {
        return (char) getVal(index);
    }

    public int getInt(int index) {
        return (int) getVal(index);
    }

    public long getLong(int index) {
        return (long) getVal(index);
    }

    public float getFloat(int index) {
        return Float.intBitsToFloat((int) getVal(index));
    }

    public double getDouble(int index) {
        return Double.longBitsToDouble(getVal(index));
    }

    public void setByte(int index, byte val) {
        setVal(index, val);
    }

    public void setShort(int index, short val) {
        setVal(index, val);
    }

    public void setChar(int index, char val) {
        setVal(index, val);
    }

    public void setInt(int index, int val) {
        setVal(index, val);
    }

    public void setLong(int index, long val) {
        setVal(index, val);
    }

    public void setFloat(int index, float val) {
        setVal(index, Float.floatToIntBits(val));
    }

    public void setDouble(int index, double val) {
        setVal(index, Double.doubleToLongBits(val));
    }

    public void copyTo(Object targetArray) {
        copyTo(0, targetArray, 0, length);
    }

    public void copyTo(int src_offset, Object targetArray, int tgt_offset, int len) {
        Class cls = targetArray.getClass();
        System.out.println("arrtype:"+cls.getName());
        if (cls.isArray() && cls.getName().charAt(1) == typeTag) {
            copyTo0(src_offset, targetArray, tgt_offset, len);
        } else {
            throw new IllegalArgumentException("espected array and same datatype: " + typeTag);
        }
    }

    native long getVal(int index);

    native void setVal(int index, long val);

    native void copyTo0(int src_offset, Object tgt, int tgt_offset, int len);
}
