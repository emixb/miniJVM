/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.gui;

import static org.mini.gui.GToolkit.nvgRGBA;
import static org.mini.nanovg.Nanovg.nvgBeginPath;
import static org.mini.nanovg.Nanovg.nvgBoxGradient;
import static org.mini.nanovg.Nanovg.nvgFill;
import static org.mini.nanovg.Nanovg.nvgFillPaint;
import static org.mini.nanovg.Nanovg.nvgRoundedRect;
import static org.mini.nanovg.Nanovg.nvgStroke;
import static org.mini.nanovg.Nanovg.nvgStrokeColor;

/**
 *
 * @author gust
 */
public class GDefaultStyle implements GStyle {

    @Override
    public float getTextFontSize() {
        return 18f;
    }

    @Override
    public float getTitleFontSize() {
        return 20f;
    }

    @Override
    public float getIconFontSize() {
        return 35f;
    }

    float[] textFontColor = nvgRGBA(255, 255, 255, 128);

    @Override
    public float[] getTextFontColor() {
        return textFontColor;
    }

    float[] frameBackground = nvgRGBA(28, 30, 34, 255);

    @Override
    public float[] getFrameBackground() {
        return frameBackground;
    }

    float[] frameTitleColor = nvgRGBA(220, 220, 220, 160);

    @Override
    public float[] getFrameTitleColor() {
        return frameTitleColor;
    }

    float[] hintFontColor = nvgRGBA(255, 255, 255, 64);

    @Override
    public float[] getHintFontColor() {
        return hintFontColor;
    }

    float[] editBackground = nvgRGBA(0, 0, 0, 32);

    @Override
    public float[] getEditBackground() {
        return editBackground;
    }

    @Override
    public float getIconFontWidth() {
        return 18;
    }

    public void drawEditBoxBase(long vg, float x, float y, float w, float h) {
        byte[] bg;
        // Edit
        bg = nvgBoxGradient(vg, x, y, w, h, 3, 4, getEditBackground(), nvgRGBA(32, 32, 32, 192));
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x + 1, y + 1, w - 2, h - 2, 4 - 1);
        nvgFillPaint(vg, bg);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1, h - 1, 4 - 0.5f);
        nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 16));
        nvgStroke(vg);
    }

    @Override
    public void drawFieldBoxBase(long vg, float x, float y, float w, float h) {
        byte[] bg;
        float cornerRadius = h / 2 - 1;
        bg = nvgBoxGradient(vg, x, y + 1.5f, w, h, h / 2, 5, nvgRGBA(0, 0, 0, 16), nvgRGBA(0, 0, 0, 92));
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, h, cornerRadius);
        nvgFillPaint(vg, bg);
        nvgFill(vg);
    }

    float[] selectedColor = nvgRGBA(128, 128, 255, 64);

    @Override
    public float[] getSelectedColor() {
        return selectedColor;
    }

    float[] unselectedColor = nvgRGBA(128, 128, 128, 16);

    @Override
    public float[] getUnselectedColor() {
        return unselectedColor;
    }

    float[] backgroundColor = nvgRGBA(96, 96, 100, 32);

    @Override
    public float[] getBackgroundColor() {
        return backgroundColor;
    }

    float[] popBackgroundColor = nvgRGBA(16, 16, 16, 255);

    @Override
    public float[] getPopBackgroundColor() {
        return popBackgroundColor;
    }
}
