package test;

import org.mini.apploader.AppManager;
import org.mini.guijni.GuiCallBack;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author gust
 */
public class AppManagerTest  {

    public static void main(String[] args) {
        GuiCallBack ccb = GuiCallBack.getInstance();
        ccb.init(800, 600);//window size

        AppManager.getInstance().active();

        ccb.mainLoop();
        ccb.destory();
    }
}
