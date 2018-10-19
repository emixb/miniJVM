/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.crypt;

import java.util.Random;

/**
 *
 * @author Gust
 */
public class XorCrypt {

    static Random rand = new Random();

    /**
     *
     * generate a key with length byte
     *
     * @param len
     * @return
     */
    public static byte[] genKey(int len) {
        if (len < 0) {
            return null;
        }
        byte[] key = new byte[len];
        for (int i = 0; i < len; i++) {
            key[i] = (byte) rand.nextInt(0xff);
        }
        return key;
    }

    /**
     *
     * encrypt or decrypt msg with key
     *
     * when msg encrypted , must decrypt it with same key
     *
     * @param msg
     * @param key
     * @return
     */
    public static byte[] xor_crypt(byte[] msg, byte[] key) {
        byte[] r = new byte[msg.length];

        for (int i = 0, imax = msg.length; i < imax; i++) {
            byte v = msg[i];
            for (int j = 0, jmax = key.length; j < jmax; j++) {
                v = (byte) (v ^ key[j]);
            }
            r[i] = v;
        }
        return r;
    }
}
