/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.gui;

/**
 *
 * @author gust
 */
abstract public class GObject {

    public void init() {

    }

    public boolean update(long ctx) {
        return true;
    }
}