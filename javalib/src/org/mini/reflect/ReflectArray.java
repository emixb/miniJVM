/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.reflect;

/**
 * 反射一个数组实例，如
 *
 * int[] a=new int[3]; Array rarr = new Array(RefNative.obj2id(a)); int
 * v=(int)rarr.getVal(2);
 *
 * 即可通过rarr 访问内数组成员
 *
 * @author gust
 */
public class ReflectArray {

    public long arrayId;
    //不可随意改动字段类型及名字，要和native一起改
    //native field name ,dont change name
    public byte typeTag;
    long body_addr;
    public int length;

    //
    DirectMemObj dmo;

    /**
     *
     * @param array
     */
    public ReflectArray(long array) {
        arrayId = array;
        mapArray(arrayId);
        dmo = new DirectMemObj(body_addr, length, typeTag);
    }

    /**
     * return data start memory address
     *
     * @return
     */
    public long getDataPtr() {
        return body_addr;
    }

    public void setValObj(int index, Object val) {
        dmo.setValObj(index, val);
    }

    public Object getValObj(int index) {
        return dmo.getValObj(index);
    }

    final native void mapArray(long classId);

    /*
     * Private
     */
    public static native Object newArray(Class componentType, int length);

    public static native Object multiNewArray(Class componentType, int[] dimensions) throws IllegalArgumentException;
}
