/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.glfm;

import java.io.File;
import org.mini.guijni.GuiCallBack;

/**
 *
 * @author gust
 */
public class Glfm {

    public static final int //
            GLFMRenderingAPIOpenGLES2 = 0,
            GLFMRenderingAPIOpenGLES3 = 1,
            GLFMRenderingAPIOpenGLES31 = 2,
            GLFMRenderingAPIOpenGLES32 = 3;
    public static final int //
            GLFMColorFormatRGBA8888 = 0,
            GLFMColorFormatRGB565 = 1;

    public static final int //
            GLFMDepthFormatNone = 0,
            GLFMDepthFormat16 = 1,
            GLFMDepthFormat24 = 2;

    public static final int //
            GLFMStencilFormatNone = 0,
            GLFMStencilFormat8 = 1;

    public static final int //
            GLFMMultisampleNone = 0,
            GLFMMultisample4X = 1;

    public static final int //
            GLFMUserInterfaceChromeNavigation = 0,
            GLFMUserInterfaceChromeNavigationAndStatusBar = 1,
            GLFMUserInterfaceChromeFullscreen = 2;

    public static final int //
            GLFMUserInterfaceOrientationAny = 0,
            GLFMUserInterfaceOrientationPortrait = 1,
            GLFMUserInterfaceOrientationLandscape = 2;

    public static final int //
            GLFMTouchPhaseHover = 0,
            GLFMTouchPhaseBegan = 1,
            GLFMTouchPhaseMoved = 2,
            GLFMTouchPhaseEnded = 3,
            GLFMTouchPhaseCancelled = 4;

    public static final int //
            GLFMMouseCursorAuto = 0,
            GLFMMouseCursorNone = 1,
            GLFMMouseCursorDefault = 2,
            GLFMMouseCursorPointer = 3,
            GLFMMouseCursorCrosshair = 4,
            GLFMMouseCursorText = 5;

    public static final int //
            GLFMKeyBackspace = 0x08,
            GLFMKeyTab = 0x09,
            GLFMKeyEnter = 0x0d,
            GLFMKeyEscape = 0x1b,
            GLFMKeySpace = 0x20,
            GLFMKeyLeft = 0x25,
            GLFMKeyUp = 0x26,
            GLFMKeyRight = 0x27,
            GLFMKeyDown = 0x28,
            GLFMKeyNavBack = 0x1000,
            GLFMKeyNavMenu = 0x1001,
            GLFMKeyNavSelect = 0x1002,
            GLFMKeyPlayPause = 0x2000;

    public static final int //
            GLFMKeyModifierShift = (1 << 0),
            GLFMKeyModifierCtrl = (1 << 1),
            GLFMKeyModifierAlt = (1 << 2),
            GLFMKeyModifierMeta = (1 << 3);

    public static int //
            GLFMKeyActionPressed = 0,
            GLFMKeyActionRepeated = 1,
            GLFMKeyActionReleased = 2;

    public static String glfmGetClipBoardContent() {
        return null;
    }

    public static void glfmSetClipBoardContent(String str) {

    }

    public static void glfmSetKeyboardVisible(long display, boolean visible) {

    }

    public static boolean glfmGetKeyboardVisible(long display) {
        return true;
    }

    public static void glfmPickPhotoAlbum(long display, int uid, int type) {

    }

    public static void glfmPickPhotoCamera(long display, int uid, int type) {

    }

    public static void glfmImageCrop(long display, int uid, String uris, int x, int y, int width, int height) {

    }

    public static void glfmSetDisplayConfig(long display,
            int preferredAPI,
            int colorFormat,
            int depthFormat,
            int stencilFormat,
            int multisample) {

    }

    public static void glfmSetCallBack(long display, GuiCallBack app) {

    }

    public static String glfmGetSaveRoot() {
        return null;
    }

    public static String glfmGetResRoot() {
        return null;
    }
}
