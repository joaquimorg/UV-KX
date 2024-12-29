#pragma once

#include <U8g2lib.h>
#include "printf.h"
#include "u8g2_hal.h"
#include "spi_hal.h"
#include "sys.h"

#include "icons.h"

#include "font_5_tr.h"
#include "font_8_tr.h"
#include "font_8b_tr.h"
#include "font_10_tr.h"
#include "font_bn_tn.h"

enum class TextAlign {
    LEFT,
    CENTER,
    RIGHT
};

enum class Font {
    FONT_5_TR,
    FONT_8_TR,
    FONT_8B_TR,
    FONT_10_TR,
    FONT_BN_TN,
    FONT_BATT_TN,
    //FONT_ICO_TN,
};

#define BLACK 1
#define WHITE 0

#define W 128
#define H 64

class UI {
public:
    UI(ST7565& st7565) : st7565{ st7565 } {};

    ST7565* lcd() { return &this->st7565; };

    uint8_t message_result = 0;

    uint8_t menu_pos = 1;

    void clearDisplay() {
        lcd()->setColorIndex(WHITE);
        lcd()->drawBox(0, 0, W, H);
        lcd()->sendBuffer();
    };

    void updateDisplay() {
        lcd()->sendBuffer();
    };

    void setFont(Font font) {
        switch (font) {
        case Font::FONT_5_TR:
            lcd()->setFont(u8g2_font_5_tr);
            break;
        case Font::FONT_8_TR:
            lcd()->setFont(u8g2_font_8_tr);
            break;
        case Font::FONT_8B_TR:
            lcd()->setFont(u8g2_font_8b_tr);
            break;
        case Font::FONT_10_TR:
            lcd()->setFont(u8g2_font_10_tr);
            break;
        case Font::FONT_BN_TN:
            lcd()->setFont(u8g2_font_bn_tn);
            break;
        case Font::FONT_BATT_TN:
            lcd()->setFont(u8g2_font_battery19_tn);
            break;
            /*case Font::FONT_32_NF:
                lcd()->setFont(u8g2_font_32_nf);
                break;
            case Font::FONT_56_NF:
                lcd()->setFont(u8g2_font_56_nf);
                break;*/
        }
    };

    void setBlackColor() {
        lcd()->setColorIndex(BLACK);
    };

    void setWhiteColor() {
        lcd()->setColorIndex(WHITE);
    };

    void drawStrf(u8g2_uint_t x, u8g2_uint_t y, const char* str, ...) {
        char text[52] = { 0 };

        va_list va;
        va_start(va, str);
        vsnprintf(text, sizeof(text), str, va);
        va_end(va);

        lcd()->drawStr(x, y, text);
    };

    void drawString(TextAlign tAlign, u8g2_uint_t xstart, u8g2_uint_t xend, u8g2_uint_t y, bool isBlack, bool isFill, bool isBox, const char* str) {

        u8g2_uint_t startX = xstart;
        u8g2_uint_t endX = xend;
        u8g2_uint_t stringWidth = lcd()->getStrWidth(str);

        u8g2_uint_t xx, yy, ww, hh;

        u8g2_uint_t border_width = 0;

        u8g2_uint_t padding_h = 2;
        u8g2_uint_t padding_v = 1;
        if (isBox) {
            padding_v++;
        }

        //int8_t a = lcd()->getAscent();
        //int8_t d = lcd()->getDescent();

        u8g2_uint_t h = (u8g2_uint_t)(lcd()->getAscent());//lcd()->getMaxCharHeight() + lcd()->getDescent();

        if (endX > startX) {
            if (tAlign == TextAlign::CENTER) {
                if (stringWidth < (endX - startX)) {
                    startX = (u8g2_uint_t)(((startX + endX) / 2) - (stringWidth / 2));
                    endX = stringWidth;
                }
            }
            else if (tAlign == TextAlign::RIGHT) {
                startX = endX - stringWidth;
            }
        }

        xx = startX;
        xx -= padding_h;
        xx -= border_width;
        ww = (u8g2_uint_t)((endX > startX ? (endX - startX) : stringWidth) + (2 * padding_h) + (2 * border_width));

        yy = y;
        //yy += u8g2->font_calc_vref(u8g2);
        //yy -= (u8g2_uint_t)a;
        yy -= h;
        yy -= padding_v;
        yy -= border_width;
        //hh = (u8g2_uint_t)(a - d + 2 * padding_v + 2 * border_width);
        hh = (u8g2_uint_t)(h + (2 * padding_v) + (2 * border_width));

        /*lcd()->setColorIndex(isBlack ? WHITE : BLACK);
        lcd()->drawBox(xx, yy, ww, hh);*/

        lcd()->setColorIndex(isBlack ? BLACK : WHITE);
        if (isFill) {
            //lcd()->drawRBox(xx, yy, ww, hh, 3);
            lcd()->drawBox(xx, yy, ww, hh);
            lcd()->setColorIndex(isBlack ? WHITE : BLACK);
        }
        else if (isBox) {
            //lcd()->drawRFrame(xx, yy, ww, hh, 3);
            lcd()->drawFrame(xx, yy, ww, hh);
        }

        lcd()->drawStr(startX, y, str);

    };

    void drawStringf(TextAlign tAlign, u8g2_uint_t xstart, u8g2_uint_t xend, u8g2_uint_t y, bool isBlack, bool isFill, bool isBox, const char* str, ...) {
        char text[52] = { 0 };

        va_list va;
        va_start(va, str);
        vsnprintf(text, sizeof(text), str, va);
        va_end(va);

        drawString(tAlign, xstart, xend, y, isBlack, isFill, isBox, text);
    };

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - */

    void draw_ic8_battery50(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, batt_50_width, batt_50_height, batt_50_bits); };

    void draw_ic8_charging(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, charging_width, charging_height, charging_bits); };

    void draw_smeter(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, smeter_width, smeter_height, smeter_bits); };

    void draw_mmeter(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, mmeter_width, mmeter_height, mmeter_bits); };

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - */

    void drawFrequencyBig(bool invert, uint32_t freq, u8g2_uint_t xend, u8g2_uint_t y) {

        setFont(Font::FONT_BN_TN);

        if (freq >= 100000000) {
            drawStringf(TextAlign::RIGHT, 0, xend, y, true, invert, false, "%1u.%03u.%03u", (freq / 100000000), (freq / 100000) % 1000, (freq % 100000) / 100);
        }
        else if (freq >= 10000000) {
            drawStringf(TextAlign::RIGHT, 0, xend, y, true, invert, false, "%3u.%03u", (freq / 100000), (freq % 100000) / 100);
        }
        else {
            drawStringf(TextAlign::RIGHT, 0, xend, y, true, invert, false, "%2u.%03u", (freq / 100000), (freq % 100000) / 100);
        }
        setBlackColor();

        setFont(Font::FONT_10_TR);
        drawStringf(TextAlign::LEFT, xend + 2, 0, y, true, invert, false, "%02u", (freq % 100));
    };

    void drawFrequencySmall(bool invert, uint32_t freq, u8g2_uint_t xend, u8g2_uint_t y) {

        setFont(Font::FONT_10_TR);
        if (freq >= 100000000) {
            drawStringf(TextAlign::RIGHT, 0, xend, y, true, invert, false, "%1u.%03u.%03u.%02u", (freq / 100000000), (freq / 100000) % 1000, (freq % 100000) / 100, (freq % 100));
        }
        else if (freq >= 10000000) {
            drawStringf(TextAlign::RIGHT, 0, xend, y, true, invert, false, "%3u.%03u.%02u", (freq / 100000), (freq % 100000) / 100, (freq % 100));
        }
        else {
            drawStringf(TextAlign::RIGHT, 0, xend, y, true, invert, false, "%2u.%03u.%02u", (freq / 100000), (freq % 100000) / 100, (freq % 100));
        }
    };

    int16_t convertRSSIToPixels(int16_t rssi_dBm) {
        int16_t pixels = 0;

        if (rssi_dBm <= -127) {
            pixels = 0; // Below S0
        }
        else if (rssi_dBm >= -73) {
            // Above S9
            int16_t extra_dB = rssi_dBm + 73;
            int16_t extraBlocks = extra_dB / 10;
            pixels = static_cast<int16_t>(34 + extraBlocks * 3); // Each block is 3 pixels wide
        }
        else {
            // S0 to S9
            int16_t sPoints = static_cast<int16_t>((rssi_dBm + 127) / 6); // Each S-point is 6 dB
            pixels = sPoints * 3; // Each block is 3 pixels wide
            int16_t remainder = static_cast<int16_t>((rssi_dBm + 127) % 6);
            pixels += static_cast<int16_t>((remainder * 3) / 6); // Add partial block width
        }

        // Cap the pixel value
        if (pixels > 51) {
            pixels = 51;
        }
        return pixels;
    }

    void drawRSSI(uint16_t rssipxl, u8g2_uint_t x, u8g2_uint_t y) {
        draw_smeter(x, y, BLACK);
        setBlackColor();

        // Draw blocks with 1 pixel spacing and each block 3 pixels wide
        uint16_t remainingPixels = rssipxl;
        u8g2_uint_t currentX = (u8g2_uint_t)(x + 7);

        while (remainingPixels > 0) {
            uint16_t blockWidth = (remainingPixels >= 3) ? 3 : remainingPixels;
            lcd()->drawBox(currentX, y + 6, blockWidth, 3);
            remainingPixels -= blockWidth;
            currentX += (u8g2_uint_t)(blockWidth + 1); // Move to the next block position with 1 pixel spacing
        }
    }

    void drawBattery(uint8_t level, u8g2_uint_t x, u8g2_uint_t y) {
        draw_ic8_battery50(x, y, BLACK);
        setBlackColor();
        // fill size 10
        uint8_t fill = (uint8_t)((level * 10) / 100);
        lcd()->drawBox(x + 1, y + 1, fill, 3);
    }

private:

    ST7565& st7565;

};

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
class SelectionList {

public:

    SelectionList(UI& ui) : ui{ ui } {};

    void next() {
        u8sl.current_pos++;
        if (u8sl.current_pos >= u8sl.total) {
            u8sl.current_pos = 0;
            u8sl.first_pos = 0;
        }
        else {
            uint8_t middle = u8sl.visible / 2;
            if (u8sl.current_pos >= middle && u8sl.current_pos < u8sl.total - middle) {
                u8sl.first_pos = u8sl.current_pos - middle;
            }
            else if (u8sl.current_pos >= u8sl.total - middle) {
                u8sl.first_pos = u8sl.total - u8sl.visible;
            }
        }
    }

    void prev() {
        if (u8sl.current_pos == 0) {
            u8sl.current_pos = u8sl.total - 1;
            u8sl.first_pos = (u8sl.total > u8sl.visible) ? (u8sl.total - u8sl.visible) : 0;
        }
        else {
            u8sl.current_pos--;
            uint8_t middle = u8sl.visible / 2;
            if (u8sl.current_pos >= middle && u8sl.current_pos < u8sl.total - middle) {
                u8sl.first_pos = u8sl.current_pos - middle;
            }
            else if (u8sl.current_pos < middle) {
                u8sl.first_pos = 0;
            }
        }
    }

    void set(uint8_t startPos, uint8_t displayLines, uint8_t maxw, const char* sl) {    

        if (startPos > 0)
            startPos--;

        u8sl.visible = displayLines;

        u8sl.total = u8x8_GetStringLineCnt(sl);
        if (u8sl.total <= u8sl.visible)
            u8sl.visible = u8sl.total;
        u8sl.first_pos = 0;
        u8sl.current_pos = startPos;

        if (u8sl.current_pos >= u8sl.total)
            u8sl.current_pos = u8sl.total - 1;
        if (u8sl.first_pos + u8sl.visible <= u8sl.current_pos)
            u8sl.first_pos = (uint8_t)(u8sl.current_pos - u8sl.visible + 1);

        slines = sl;
        maxWidth = maxw;
    }

    uint8_t getListPos() {
        return u8sl.current_pos;
    }

    uint8_t getTotal() {
        return u8sl.total;
    }

    void draw(uint8_t y) {
        ui.lcd()->setFontPosBaseline();
        drawSelectionList(y, slines);
    }

    void setStartXPos(uint8_t x) {
        startXPos = x;
    }

    void setMaxWidth(uint8_t w) {
        maxWidth = w;
    }

    void setShowLineNumbers(bool show) {
        showLineNumbers = show;
    }

private:
    u8sl_t u8sl;
    const char* slines;
    uint8_t maxWidth = 75;
    uint8_t startXPos = 2;
    bool showLineNumbers = true;

    u8g2_uint_t drawSelectionListLine(u8g2_uint_t y, uint8_t idx, const char* s) {
        //u8g2_uint_t yy;
        //uint8_t border_size = 0;
        uint8_t is_invert = 0;

        u8g2_uint_t line_height = (u8g2_uint_t)(ui.lcd()->getAscent() - ui.lcd()->getDescent() + 2);

        /* check whether this is the current cursor line */
        if (idx == u8sl.current_pos)
        {
            //border_size = 2;
            is_invert = 1;
        }

        /* get the line from the array */
        s = u8x8_GetStringLineStart(idx, s);

        /* draw the line */
        if (s == NULL)
            s = "";
        //u8g2_DrawUTF8Line(u8g2, MY_BORDER_SIZE, y, u8g2_GetDisplayWidth(u8g2) - 2 * MY_BORDER_SIZE, s, border_size, is_invert);

        if (showLineNumbers) {
            ui.setFont(Font::FONT_5_TR);
            ui.drawStringf(TextAlign::LEFT, startXPos, 0, y, true, false, false, "%02u", idx + 1);
        }

        if (is_invert) {
            ui.setFont(Font::FONT_8B_TR);
        }
        else {
            ui.setFont(Font::FONT_8_TR);
        }

        if (showLineNumbers) {
            ui.drawString(TextAlign::LEFT, startXPos + 14, maxWidth, y, is_invert, true, false, s);
        }
        else {
            ui.drawString(TextAlign::CENTER, startXPos, maxWidth, y, is_invert, true, false, s);
        }

        return line_height;
    }

    void drawSelectionList(u8g2_uint_t y, const char* s) {
        uint8_t i;
        for (i = 0; i < u8sl.visible; i++) {
            y += drawSelectionListLine(y, i + u8sl.first_pos, s);
        }
    }

protected:
    UI& ui;

};

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

class SelectionListPopup : public SelectionList {
public:
    SelectionListPopup(UI& ui) : SelectionList(ui) {}

    void drawPopup() {
        uint8_t popupWidth = 72; // Width of the popup
        uint8_t popupHeight = 34; // Height of the popup
        uint8_t x = (uint8_t)((W - popupWidth) / 2); // Center the popup horizontally
        uint8_t y = (uint8_t)((H - popupHeight) / 2); // Center the popup vertically

        // Draw the popup background
        ui.setWhiteColor();
        ui.lcd()->drawRBox(x - 1, y - 1, popupWidth + 3, popupHeight + 3, 5);
        ui.setBlackColor();
        ui.lcd()->drawRFrame(x, y, popupWidth, popupHeight, 5);
        ui.lcd()->drawRFrame(x, y, popupWidth + 1, popupHeight + 1, 5);

        ui.lcd()->drawBox(x + 1, y + 1, popupWidth - 1, 6);

        ui.setFont(Font::FONT_5_TR);
        ui.drawString(TextAlign::CENTER, x, x + popupWidth, y + 6, false, false, false, title);

        // Draw the selection list inside the popup
        setShowLineNumbers(false);
        setMaxWidth((uint8_t)(x + popupWidth - 4));
        setStartXPos(x + 4);
        draw(y + 14);
    }

    void setPopupTitle(const char* title) {
        this->title = title;
    }

private:
    const char* title;

};

