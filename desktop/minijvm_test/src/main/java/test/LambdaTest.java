/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 *
 * @author Gust
 */
public class LambdaTest {

    volatile int a;

    void t1() {
        a = 3;
        String[] ss = new String[]{"one", "two", "three"};
        List sslist = Arrays.asList(ss);
        String[] c = new String[1];
        List<String> list = new ArrayList();
        int a = 4;
        for (int i = 0; i < 100000; i++) {
            sslist.forEach(s -> {
                c[0] = (this + ":" + a + ":" + s);
            });

            list.clear();
            list.addAll(sslist);
            list.removeIf(s -> s.length() < a);
            list.forEach(s -> c[0] = (":" + s));
        }
    }

    public static void main(String args[]) {
        LambdaTest obj = new LambdaTest();
        obj.t1();
    }
}
