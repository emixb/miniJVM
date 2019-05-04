/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.gui;

import org.mini.guijni.GuiCallBack;
import java.util.Timer;
import java.util.TimerTask;
import static org.mini.gl.GL.GL_COLOR_BUFFER_BIT;
import static org.mini.gl.GL.GL_DEPTH_BUFFER_BIT;
import static org.mini.gl.GL.GL_STENCIL_BUFFER_BIT;
import static org.mini.gl.GL.glClear;
import org.mini.nanovg.StbFont;
import static org.mini.gl.GL.glClearColor;
import static org.mini.gl.GL.glViewport;
import static org.mini.gui.GObject.TYPE_FORM;
import static org.mini.gui.GToolkit.nvgRGBA;
import org.mini.gui.event.GAppActiveListener;
import org.mini.gui.event.GKeyboardShowListener;
import org.mini.gui.event.GNotifyListener;
import org.mini.gui.event.GPhotoPickedListener;
import org.mini.nanovg.Gutil;
import org.mini.nanovg.Nanovg;
import static org.mini.nanovg.Nanovg.NVG_ALIGN_MIDDLE;
import static org.mini.nanovg.Nanovg.nvgBeginFrame;
import static org.mini.nanovg.Nanovg.nvgEndFrame;
import static org.mini.nanovg.Nanovg.nvgFillColor;
import static org.mini.nanovg.Nanovg.nvgFontFace;
import static org.mini.nanovg.Nanovg.nvgFontSize;
import static org.mini.nanovg.Nanovg.nvgTextAlign;
import static org.mini.gui.GObject.flush;

/**
 *
 * @author gust
 */
public class GForm extends GViewPort {

    String title;
    long display; //glfw display
    long vg; //nk contex
    GuiCallBack callback;
    static StbFont gfont;
    float fps;
    float fpsExpect = 30;
    long last, count;
    //
    float pxRatio;

    int fbWidth, fbHeight;
    //
    boolean premult;

    GPhotoPickedListener pickListener;
    GKeyboardShowListener keyshowListener;
    GAppActiveListener activeListener;
    GNotifyListener notifyListener;

//    final static List<Integer> pendingDeleteImage = Collections.synchronizedList(new ArrayList());
    final static Timer timer = new Timer(true);//用于更新画面，UI系统采取按需刷新的原则

    static GCmdHandler cmdHandler = new GCmdHandler();

    public GForm() {
        this.title = title;
        callback = GuiCallBack.getInstance();
    }

    public int getType() {
        return TYPE_FORM;
    }

    public GuiCallBack getCallBack() {
        return this.callback;
    }

    static public void setGFont(StbFont pgfont) {
        gfont = pgfont;
    }

    static public StbFont getGFont() {
        return gfont;
    }

    public long getNvContext() {
        return callback.getNvContext();
    }

    public long getWinContext() {
        return display;
    }

    public int getDeviceWidth() {
        return (int) callback.getDeviceWidth();
    }

    public int getDeviceHeight() {
        return (int) callback.getDeviceHeight();
    }

    public void setTitle(String title) {
        callback.setDisplayTitle(title);
    }

    @Override
    public void init() {
        display = callback.getDisplay();
        vg = callback.getNvContext();
        if (vg == 0) {
            System.out.println("callback.getNvContext() is null.");
        }

        fbWidth = callback.getFrameBufferWidth();
        fbHeight = callback.getFrameBufferHeight();
        int winWidth, winHeight;
        winWidth = callback.getDeviceWidth();
        winHeight = callback.getDeviceHeight();

        pxRatio = callback.getDeviceRatio();

        setLocation(0, 0);
        setSize(winWidth, winHeight);
        //System.out.println("fbWidth=" + fbWidth + "  ,fbHeight=" + fbHeight);
        flush();
    }

    public void display(long vg) {

        long startAt, endAt, cost;
        try {
            startAt = System.currentTimeMillis();

            // Update and render
            glViewport(0, 0, fbWidth, fbHeight);
            if (premult) {
                glClearColor(0, 0, 0, 0);
            } else {
                glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            int winWidth, winHeight;
            winWidth = callback.getDeviceWidth();
            winHeight = callback.getDeviceHeight();

            nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
            //drawDebugInfo(vg);
            Nanovg.nvgReset(vg);
            Nanovg.nvgResetScissor(vg);
            Nanovg.nvgScissor(vg, 0, 0, winWidth, winHeight);
            update(vg);
            cmdHandler.update(this);
            nvgEndFrame(vg);

            //
            count++;
            endAt = System.currentTimeMillis();
            cost = endAt - startAt;
            if (cost > 1000) {
                //System.out.println("fps:" + count);
                fps = count;
                last = endAt;
                count = 0;
            }
//                if (cost < 1000 / fpsExpect) {
//                    Thread.sleep((long) (1000 / fpsExpect - cost));
//                }
            cmdHandler.process(this);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    void drawDebugInfo(long vg) {
        float font_size = 15;
        nvgFontSize(vg, font_size);
        nvgFontFace(vg, GToolkit.getFontWord());
        nvgTextAlign(vg, Nanovg.NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        GuiCallBack cb = (GuiCallBack) callback;
        float dx = 2, dy = 40;
        byte[] b;
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));

        dy += font_size;
        b = Gutil.toUtf8("form:" + getX() + "," + getY() + "," + getW() + "," + getH() + "  " + getInnerX() + "," + getInnerY() + "," + getInnerW() + "," + getInnerH());

        Nanovg.nvgTextJni(vg, dx, dy, b, 0, b.length);
        dy += font_size;
        if (focus != null) {
            b = Gutil.toUtf8("focus:" + focus.getX() + "," + focus.getY() + "," + focus.getW() + "," + focus.getH() + "  " + ((focus instanceof GContainer) ? ((GContainer) focus).getInnerX() + "," + ((GContainer) focus).getInnerY() + "," + ((GContainer) focus).getInnerW() + "," + ((GContainer) focus).getInnerH() : ""));
            Nanovg.nvgTextJni(vg, dx, dy, b, 0, b.length);
        }
    }

    TimerTask tt_OnTouch = new TimerTask() {
        public void run() {
            flush();
        }
    };

    void tt_setupOnTouch() {
        timer.schedule(tt_OnTouch, 0L);//, (long) (1000 / fpsExpect));
    }

    /**
     * @return the fps
     */
    public float getFps() {
        return fps;
    }

    public void setFps(float fps) {
        fpsExpect = fps;
    }

    public void onPhotoPicked(int uid, String url, byte[] data) {
        if (pickListener != null) {
            pickListener.onPicked(uid, url, data);
        }
    }

    /**
     * @return the pickListener
     */
    public GPhotoPickedListener getPickListener() {
        return pickListener;
    }

    /**
     * @param pickListener the pickListener to set
     */
    public void setPickListener(GPhotoPickedListener pickListener) {
        this.pickListener = pickListener;
    }

    public void KeyboardPopEvent(boolean visible, float x, float y, float w, float h) {
        if (keyshowListener != null) {
            keyshowListener.keyboardShow(visible, x, y, w, h);
        }
    }

    /**
     * @return the keyshowListener
     */
    public GKeyboardShowListener getKeyshowListener() {
        return keyshowListener;
    }

    /**
     * @param keyshowListener the keyshowListener to set
     */
    public void setKeyshowListener(GKeyboardShowListener keyshowListener) {
        this.keyshowListener = keyshowListener;
    }

    public static void deleteImage(int texture) {
        cmdHandler.addCmd(GCmd.GCMD_DESTORY_TEXTURE, texture);
    }

    public void onAppFocus(boolean focus) {
        //System.out.println("app focus:" + focus);
        if (activeListener != null) {
            activeListener.onAppActive(focus);
        }
        flush();
    }

    public void onNotify(String key, String val) {
        if (notifyListener != null) {
            notifyListener.onNotify(key, val);
        }
    }

    /**
     * @return the activeListener
     */
    public GAppActiveListener getActiveListener() {
        return activeListener;
    }

    /**
     * @param activeListener the activeListener to set
     */
    public void setActiveListener(GAppActiveListener activeListener) {
        this.activeListener = activeListener;
    }

    /**
     * @return the notifyListener
     */
    public GNotifyListener getNotifyListener() {
        return notifyListener;
    }

    /**
     * @param notifyListener the notifyListener to set
     */
    public void setNotifyListener(GNotifyListener notifyListener) {
        this.notifyListener = notifyListener;
    }

    /**
     *
     * @param s
     */
    public static void addMessage(String s) {
        cmdHandler.addCmd(GCmd.GCMD_SHOW_MESSAGE, s);
    }

    public static void clearMessage() {
        cmdHandler.addCmd(GCmd.GCMD_CLEAR_MESSAGE);
    }

    public static void showKeyboard() {
        cmdHandler.addCmd(GCmd.GCMD_SHOW_KEYBOARD);
    }

    public static void hideKeyboard() {
        cmdHandler.addCmd(GCmd.GCMD_HIDE_KEYBOARD);
    }

}
