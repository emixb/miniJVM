/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.gui.impl;

import org.mini.apploader.GlfmMain;
import static org.mini.gl.GL.GL_TRUE;
import static org.mini.gl.GL.glClearColor;
import org.mini.glfw.Glfw;
import static org.mini.glfw.Glfw.GLFW_CONTEXT_VERSION_MAJOR;
import static org.mini.glfw.Glfw.GLFW_CONTEXT_VERSION_MINOR;
import static org.mini.glfw.Glfw.GLFW_DEPTH_BITS;
import static org.mini.glfw.Glfw.GLFW_KEY_ESCAPE;
import static org.mini.glfw.Glfw.GLFW_OPENGL_CORE_PROFILE;
import static org.mini.glfw.Glfw.GLFW_OPENGL_FORWARD_COMPAT;
import static org.mini.glfw.Glfw.GLFW_OPENGL_PROFILE;
import static org.mini.glfw.Glfw.GLFW_PRESS;
import static org.mini.glfw.Glfw.GLFW_TRANSPARENT_FRAMEBUFFER;
import static org.mini.glfw.Glfw.GLFW_TRUE;
import static org.mini.glfw.Glfw.glfwGetFramebufferHeight;
import static org.mini.glfw.Glfw.glfwGetFramebufferWidth;
import static org.mini.glfw.Glfw.glfwPollEvents;
import static org.mini.glfw.Glfw.glfwSetWindowShouldClose;
import static org.mini.glfw.Glfw.glfwSwapBuffers;
import static org.mini.glfw.Glfw.glfwSwapInterval;
import static org.mini.glfw.Glfw.glfwTerminate;
import static org.mini.glfw.Glfw.glfwWindowHint;
import static org.mini.glfw.Glfw.glfwWindowShouldClose;
import org.mini.glfw.GlfwCallbackAdapter;
import org.mini.gui.GForm;
import org.mini.gui.GObject;
import org.mini.gui.GToolkit;
import static org.mini.gui.GObject.HEIGHT;
import static org.mini.gui.GObject.WIDTH;
import org.mini.nanovg.Gutil;
import static org.mini.nanovg.Nanovg.NVG_ANTIALIAS;
import static org.mini.nanovg.Nanovg.NVG_DEBUG;
import static org.mini.nanovg.Nanovg.NVG_STENCIL_STROKES;
import static org.mini.nanovg.Nanovg.nvgCreateGL3;
import static org.mini.nanovg.Nanovg.nvgDeleteGL3;

/**
 *
 * @author Gust
 */
public class GuiCallBack extends GlfwCallbackAdapter {

    GForm gform;
    long display;

    int winWidth, winHeight;
    int fbWidth, fbHeight;
    float pxRatio;

    public int mouseX, mouseY, button;
    long mouseLastPressed;
    int CLICK_PERIOD = 200;

    boolean drag;
    int hoverX, hoverY;//mouse 

    long vg;

    //not in mobile
    int fps;
    int fpsExpect = 60;

    static GuiCallBack instance = new GuiCallBack();

    public static GuiCallBack getInstance() {
        return instance;
    }

    private GuiCallBack() {
    }

    public void setDisplay(long display) {
        this.display = display;
    }

    public long getDisplay() {
        return display;
    }

    public GForm getForm() {
        return gform;
    }

    public void setForm(GForm form) {
        gform = form;
    }

    public long getNvContext() {
        return vg;
    }

    public int getDeviceWidth() {
        return (int) winWidth;
    }

    public int getDeviceHeight() {
        return (int) winHeight;
    }

    public int getFrameBufferWidth() {
        return (int) fbWidth;
    }

    public int getFrameBufferHeight() {
        return (int) fbHeight;
    }

    public float getDeviceRatio() {
        return pxRatio;
    }

    public String getResRoot() {
        return "./";
    }

    public void setDisplayTitle(String title) {
        Glfw.glfwSetWindowTitle(display, title);
    }

    public void init(int width, int height) {
        this.winWidth = width;
        this.winHeight = height;

        if (!Glfw.glfwInit()) {
            System.out.println("glfw init error.");
            System.exit(1);
        }
        String osname = System.getProperty("os.name");
        if (osname != null && osname.contains("Mac")) {
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(Glfw.GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);
        }
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        glfwWindowHint(GLFW_DEPTH_BITS, 16);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        display = Glfw.glfwCreateWindow(winWidth, winHeight, Gutil.toUtf8(""), 0, 0);
        if (display == 0) {
            glfwTerminate();
            System.exit(1);
        }
        Glfw.glfwSetCallback(display, this);
        Glfw.glfwMakeContextCurrent(display);
        glfwSwapInterval(1);
        winWidth = Glfw.glfwGetWindowWidth(display);
        winHeight = Glfw.glfwGetWindowHeight(display);
        fbWidth = glfwGetFramebufferWidth(display);
        fbHeight = glfwGetFramebufferHeight(display);
        // Calculate pixel ration for hi-dpi devices.
        pxRatio = (float) fbWidth / (float) winWidth;
        System.out.println("fbWidth=" + fbWidth + "  ,fbHeight=" + fbHeight);

        vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
        if (vg == 0) {
            System.out.println("Could not init nanovg.\n");
            System.out.println("callback.getNvContext() is null.");
        }
        GToolkit.FontHolder.loadFont(vg);
        GlfmMain.onSurfaceCreated();
    }

    @Override
    public void mainLoop() {
        //
        long last = System.currentTimeMillis(), now;
        int count = 0;

        long startAt, endAt, cost;
        while (!glfwWindowShouldClose(display)) {
            try {
                startAt = System.currentTimeMillis();
                if (gform != null) {
                    if (gform.getWinContext() == 0) {
                        gform.init();
                    }
                }
                //user define contents
                if (GObject.flushReq()) {
                    gform.display(vg);
                    glfwSwapBuffers(display);
                }
                glfwPollEvents();
                count++;
                now = System.currentTimeMillis();
                if (now - last > 1000) {
                    //System.out.println("fps:" + count);
                    fps = count;
                    last = now;
                    count = 0;
                }

                endAt = System.currentTimeMillis();
                cost = endAt - startAt;
                if (cost < 1000 / fpsExpect) {
                    Thread.sleep((long) (1000 / fpsExpect - cost));
                }
            } catch (Exception ex) {
                ex.printStackTrace();

            }
        }
    }

    public void destory() {
        nvgDeleteGL3(vg);
        glfwTerminate();
        GToolkit.removeForm(vg);
        vg = 0;
    }

    @Override
    public void key(long window, int key, int scancode, int action, int mods) {
        if (gform == null) {
            return;
        }
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        gform.keyEvent(key, scancode, action, mods);
        gform.flush();
    }

    @Override
    public void character(long window, char character) {
        if (gform == null) {
            return;
        }
        gform.characterEvent(character);
        gform.flush();
    }

    @Override
    public void mouseButton(long window, int button, boolean pressed) {
        if (gform == null) {
            return;
        }
        if (window == display) {
            switch (button) {
                case Glfw.GLFW_MOUSE_BUTTON_1: {//left
                    if (pressed) {
                        drag = true;
                        hoverX = mouseX;
                        hoverY = mouseY;
                    } else {
                        drag = false;
                    }
                    break;
                }
                case Glfw.GLFW_MOUSE_BUTTON_2: {//right
                    if (pressed) {
                        drag = true;
                        hoverX = mouseX;
                        hoverY = mouseY;
                    } else {
                        drag = false;
                    }
                    break;
                }
                case Glfw.GLFW_MOUSE_BUTTON_3: {//middle
                    break;
                }
            }
            //click event
            long cur = System.currentTimeMillis();
            if (pressed && cur - mouseLastPressed < CLICK_PERIOD && this.button == button) {
                gform.clickEvent(button, mouseX, mouseY);
            } else //press event
            {
                gform.mouseButtonEvent(button, pressed, mouseX, mouseY);
            }
            this.button = button;
            mouseLastPressed = cur;
        }
        gform.flush();
        //System.out.println("flushed");
    }

    @Override
    public void scroll(long window, double scrollX, double scrollY) {
        if (gform == null) {
            return;
        }
        gform.scrollEvent((float) scrollX, (float) scrollY, mouseX, mouseY);
        gform.flush();
    }

    @Override
    public void cursorPos(long window, int x, int y) {
        if (gform == null) {
            return;
        }
        if (display == window) {
            mouseX = x;
            mouseY = y;
            gform.cursorPosEvent(x, y);
            if (drag) {
                gform.dragEvent(x - hoverX, y - hoverY, x, y);
                hoverX = mouseX;
                hoverY = mouseY;
            }
            gform.flush();
        }
    }

    @Override
    public boolean windowClose(long window) {
        if (gform == null) {
            return true;
        }
        gform.flush();
        return true;
    }

    @Override
    public void windowSize(long window, int width, int height) {
        if (gform == null) {
            return;
        }
        gform.flush();
    }

    @Override
    public void framebufferSize(long window, int x, int y) {
        if (gform == null) {
            return;
        }
        gform.getBoundle()[WIDTH] = x;
        gform.getBoundle()[HEIGHT] = y;
        gform.flush();
    }

    @Override
    public void drop(long window, int count, String[] paths) {
        if (gform == null) {
            return;
        }
        gform.dropEvent(count, paths);
        gform.flush();
    }

    public void error(int error, String description) {
        System.out.println("error: " + error + " message: " + description);
    }
}
