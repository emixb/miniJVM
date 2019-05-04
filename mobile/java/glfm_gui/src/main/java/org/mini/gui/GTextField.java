/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.gui;

import org.mini.glfm.Glfm;
import org.mini.glfw.Glfw;
import static org.mini.gui.GObject.HEIGHT;
import static org.mini.gui.GObject.isInBoundle;
import static org.mini.gui.GToolkit.nvgRGBA;
import static org.mini.nanovg.Gutil.toUtf8;
import org.mini.nanovg.Nanovg;
import static org.mini.nanovg.Nanovg.NVG_ALIGN_CENTER;
import static org.mini.nanovg.Nanovg.NVG_ALIGN_LEFT;
import static org.mini.nanovg.Nanovg.NVG_ALIGN_MIDDLE;
import static org.mini.nanovg.Nanovg.nvgCreateNVGglyphPosition;
import static org.mini.nanovg.Nanovg.nvgFillColor;
import static org.mini.nanovg.Nanovg.nvgFontFace;
import static org.mini.nanovg.Nanovg.nvgFontSize;
import static org.mini.nanovg.Nanovg.nvgNVGglyphPosition_x;
import static org.mini.nanovg.Nanovg.nvgTextAlign;
import static org.mini.nanovg.Nanovg.nvgTextGlyphPositionsJni;
import static org.mini.nanovg.Nanovg.nvgTextJni;
import static org.mini.nanovg.Nanovg.nvgTextMetrics;

/**
 *
 * @author gust
 */
public class GTextField extends GTextObject {

    static public final int BOX_STYLE_EDIT = 0;
    static public final int BOX_STYLE_SEARCH = 1;

    float[] reset_boundle;
    int text_max = 256;
    int boxStyle = BOX_STYLE_EDIT;
    //
    byte[] search_arr = {(byte) 0xe2, (byte) 0x8c, (byte) 0xa8, 0};
    byte[] reset_arr = toUtf8("" + ICON_CIRCLED_CROSS);
    //
    float[] lineh = {0};
    short[] text_pos;
    //
    int caretIndex;
    int selectStart = -1;//选取开始
    int selectEnd = -1;//选取结束

    boolean password = false;//是否密码字段

    public GTextField() {

    }

    public GTextField(String text, String hint, int left, int top, int width, int height) {
        this(text, hint, (float) left, top, width, height);
    }

    public GTextField(String text, String hint, float left, float top, float width, float height) {
        setText(text);
        setHint(hint);
        setLocation(left, top);
        setSize(width, height);
        reset_boundle = new float[]{left + width - height, top, height, height};
        setFocusListener(this);
    }

    public int getType() {
        return TYPE_TEXTFIELD;
    }

    @Override
    void onSetText(String text) {
        if (text != null) {
            caretIndex = text.length();
        } else {
            caretIndex = 0;
        }
        resetSelect();
    }

    public void setBoxStyle(int boxStyle) {
        this.boxStyle = boxStyle;
    }

    public void setMaxTextLength(int len) {
        text_max = len;
    }

    public void setPasswordMode(boolean pwd) {
        password = pwd;
    }

    public boolean isPasswordMode() {
        return password;
    }

    @Override
    public void mouseButtonEvent(int button, boolean pressed, int x, int y) {
        int rx = (int) (x - parent.getInnerX());
        int ry = (int) (y - parent.getInnerY());
        if (isInArea(x, y)) {
            if (button == Glfw.GLFW_MOUSE_BUTTON_1) {
                if (pressed) {
                } else if (isInBoundle(reset_boundle, rx, ry)) {
                    deleteAll();
                    resetSelect();
                    disposeEditMenu();
                } else {
                    if (selectMode) {
                        resetSelect();
                        disposeEditMenu();
                    }
                    setCaretIndex(getCaretIndex(x, y));
                }
            } else if (button == Glfw.GLFW_MOUSE_BUTTON_2) {
                if (pressed) {

                } else {
                    callEditMenu(this, x, y);
                }
            }
        }
    }

    @Override
    public void keyEvent(int key, int scanCode, int action, int mods) {
        if (parent.getFocus() != this) {
            return;
        }
        if (action == Glfw.GLFW_PRESS || action == Glfw.GLFW_REPEAT) {
            if (key == Glfw.GLFW_KEY_BACKSPACE) {
                if (editable) {
                    if (textsb.length() > 0 && caretIndex > 0) {
                        int[] selectFromTo = getSelected();
                        if (selectFromTo != null) {
                            deleteSelectedText();
                        } else {
                            setCaretIndex(caretIndex - 1);
                            deleteTextByIndex(caretIndex);
                        }
                    }
                }
            }
        }
    }

    @Override
    public void clickEvent(int button, int x, int y) {
        if (isInArea(x, y)) {

            doSelectAll();
        }
    }

    @Override
    public void touchEvent(int phase, int x, int y) {
        int rx = (int) (x - parent.getInnerX());
        int ry = (int) (y - parent.getInnerY());
        if (isInBoundle(boundle, rx, ry)) {
            if (phase == Glfm.GLFMTouchPhaseEnded) {
                if (isInBoundle(reset_boundle, rx, ry)) {
                    deleteAll();
                    resetSelect();
                    disposeEditMenu();
                } else {
                    if (selectMode) {
                        resetSelect();
                        disposeEditMenu();
                    }
                    setCaretIndex(getCaretIndex(x, y));
                }
            }
        }
        super.touchEvent(phase, x, y);
    }

    @Override
    public void characterEvent(String str, int mods) {

        for (int i = 0, imax = str.length(); i < imax; i++) {
            char character = str.charAt(i);
            if (character != '\n' && character != '\r' && textsb.length() < text_max) {
                insertTextByIndex(caretIndex, character);
                setCaretIndex(caretIndex + 1);
            }
        }
    }

    /**
     *
     * @param character
     */
    @Override
    public void characterEvent(char character) {
        if (editable) {
            if (character != '\n' && character != '\r' && textsb.length() < text_max) {
                insertTextByIndex(caretIndex, character);
                setCaretIndex(caretIndex + 1);
            }
        }
    }

    @Override
    public void keyEvent(int key, int action, int mods) {

        if (action == Glfm.GLFMKeyActionPressed || action == Glfm.GLFMKeyActionRepeated) {
            if (key == Glfm.GLFMKeyBackspace) {
                if (editable) {
                    if (textsb.length() > 0 && caretIndex > 0) {
                        int[] selectFromTo = getSelected();
                        if (selectFromTo != null) {
                            deleteSelectedText();
                            text_arr = null;
                        } else {
                            setCaretIndex(caretIndex - 1);
                            deleteTextByIndex(caretIndex);
                        }
                    }
                }
            }
        }
    }

    public int getCaretIndex(int x, int y) {
        if (text_pos == null) {
            return textsb.length();
        }
        for (int j = 0; j < text_pos.length; j++) {
            //取第 j 个字符的X座标
            float x0 = text_pos[j];
            if (x < x0) {
                return j;
            }
        }
        return text_pos.length;
    }

    /**
     * @param caretIndex the caretIndex to set
     */
    private void setCaretIndex(int caretIndex) {
        if (caretIndex < 0) {
            caretIndex = 0;
        } else if (caretIndex > textsb.length()) {
            caretIndex = textsb.length();
        }
        this.caretIndex = caretIndex;
    }

    int[] getSelected() {
        if (selectStart != -1 && selectEnd != -1) {
            int select1, select2;
            select1 = selectStart > selectEnd ? selectEnd : selectStart;
            select2 = selectStart < selectEnd ? selectEnd : selectStart;
            return new int[]{select1, select2};
        }
        return null;
    }

    @Override
    public void deleteSelectedText() {
        int[] sarr = getSelected();
        if (sarr == null || sarr[0] == -1 || sarr[1] == -1) {
            return;
        }
        setCaretIndex(sarr[0]);
        textsb.delete(sarr[0], sarr[1]);
        text_arr = null;
        resetSelect();
        doStateChange();
    }

    @Override
    void resetSelect() {
        selectStart = selectEnd = -1;
        selectMode = false;
    }

    @Override
    public String getSelectedText() {
        int[] sarr = getSelected();
        if (sarr == null || sarr[0] == -1 || sarr[1] == -1) {
            return null;
        }
        return textsb.substring(sarr[0], sarr[1]);
    }

    @Override
    public void insertTextAtCaret(String str) {
        for (int i = 0, imax = str.length(); i < imax; i++) {
            char character = str.charAt(i);
            textsb.insert(caretIndex, character);
            setCaretIndex(caretIndex + 1);
            doStateChange();
        }
        text_arr = null;
    }

    @Override
    public void doSelectText() {
        doSelectAll();
        selectMode = true;
    }

    @Override
    public void doSelectAll() {
        selectStart = 0;
        selectEnd = textsb.length();
        selectMode = true;
    }

    /**
     *
     * @param vg
     * @return
     */
    @Override
    public boolean update(long vg) {
        float x = getX();
        float y = getY();
        float w = getW();
        float h = getH();

        byte[] bg;
        float FONT_WIDTH = GToolkit.getStyle().getIconFontWidth();
        float leftIcons = 0.5f;//图标占位宽度
        // Edit
        if (boxStyle == BOX_STYLE_SEARCH) {
            GToolkit.getStyle().drawFieldBoxBase(vg, x, y, w, h);
            nvgFontSize(vg, GToolkit.getStyle().getIconFontSize());
            nvgFontFace(vg, GToolkit.getFontIcon());
            nvgFillColor(vg, GToolkit.getStyle().getHintFontColor());
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

            nvgTextJni(vg, x + FONT_WIDTH * 1.5f, y + h * 0.55f, search_arr, 0, search_arr.length);
            leftIcons = 2;
        } else {
            GToolkit.getStyle().drawEditBoxBase(vg, x, y, w, h);
        }

        nvgFontSize(vg, GToolkit.getStyle().getIconFontSize());
        nvgFontFace(vg, GToolkit.getFontIcon());
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 32));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgTextJni(vg, x + w - h * 0.55f, y + h * 0.55f, reset_arr, 0, reset_arr.length);

        nvgFontSize(vg, GToolkit.getStyle().getTextFontSize());
        nvgFontFace(vg, GToolkit.getFontWord());

        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        if (text_arr == null) {//文字被修改过
            if (password) {
                int len = textsb.length();
                text_arr = new byte[len + 1];
                for (int i = 0; i < len; i++) {
                    text_arr[i] = '*';
                }
            } else {
                text_arr = toUtf8(textsb.toString());
            }
        }
        float wordx = x + FONT_WIDTH * leftIcons;
        float wordy = y + boundle[HEIGHT] * 0.5f;
        float text_show_area_x = wordx;
        float text_show_area_w = boundle[WIDTH] - FONT_WIDTH * (leftIcons + 2.5f);
        float text_width = Nanovg.nvgTextBoundsJni(vg, 0, 0, text_arr, 0, text_arr.length, null);
        if (parent.getFocus() != this && (textsb == null || textsb.length() == 0)) {
            if (hint_arr != null) {
                nvgFillColor(vg, GToolkit.getStyle().getHintFontColor());
                nvgTextJni(vg, wordx, wordy, hint_arr, 0, hint_arr.length);
            }
        } else {

            long glyphsHandle = nvgCreateNVGglyphPosition(text_max);
            if (parent.getFocus() == this) {
                nvgTextMetrics(vg, null, null, lineh);
            }

            try {
                if (text_width > text_show_area_w) {
                    wordx -= text_width - text_show_area_w;
                }
                int char_count = nvgTextGlyphPositionsJni(vg, wordx, wordy, text_arr, 0, text_arr.length, glyphsHandle, text_max);

                text_pos = new short[char_count];
                float caretx = 0;
                //确定每个char的位置
                for (int j = 0; j < char_count; j++) {
                    //取第 j 个字符的X座标
                    float x0 = nvgNVGglyphPosition_x(glyphsHandle, j);
                    text_pos[j] = (short) x0;
                    if (caretIndex == j) {
                        caretx = x0;
                    }
                }
                if (caretx == 0) {
                    caretx = wordx + text_width;
                }
                if (parent.getFocus() == this) {
                    GToolkit.drawCaret(vg, caretx, wordy - 0.5f * lineh[0], 1, lineh[0], false);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            Nanovg.nvgDeleteNVGglyphPosition(glyphsHandle);

            if (selectStart != -1 && selectEnd != -1) {

                GToolkit.drawRect(vg, text_show_area_x, wordy - lineh[0] * .5f, text_show_area_w, lineh[0], GToolkit.getStyle().getSelectedColor());

            }
            nvgFillColor(vg, GToolkit.getStyle().getTextFontColor());
            Nanovg.nvgScissor(vg, text_show_area_x, y, text_show_area_w, h);
            Nanovg.nvgIntersectScissor(vg, parent.getX(), parent.getY(), parent.getW(), parent.getH());
            nvgTextJni(vg, wordx, wordy, text_arr, 0, text_arr.length);
        }
        return true;
    }

}
