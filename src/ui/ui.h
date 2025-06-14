#pragma once

#include <U8g2lib.h>
#include "printf.h"
#include "u8g2_hal.h"
#include "spi_hal.h"
#include "sys.h"
#include "uart_hal.h"
#include "keyboard.h"

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
    FONT_BN_TN
};

#define BLACK 1
#define WHITE 0

#define W 128
#define H 64

static constexpr uint16_t CHAR_BUFFER_SIZE = 600;
static char uiBuffer[CHAR_BUFFER_SIZE];

class UI {
public:
    UI(ST7565& st7565, UART& uart) : st7565{ st7565 }, uart{ uart } {};

    ST7565* lcd() { return &this->st7565; };

    static constexpr char TXStr[] = "TX";
    static constexpr char RXStr[] = "RX";
    static constexpr char HZStr[] = "Hz";
    static constexpr char KHZStr[] = "KHz";
    static constexpr char VFOStr[] = "VFO";
    static constexpr char DBStr[] = "Db";

    uint8_t message_result = 0;

    uint8_t menu_pos = 1;

    static constexpr const char* InfoMessageStr = "BATTERY LOW\nTX DISABLED";

    enum class InfoMessageType : uint8_t {
        INFO_NONE = 0,
        LOW_BATTERY = 1,
        TX_DISABLED = 2
    };

    void clearDisplay() {
        //lcd()->setColorIndex(WHITE);
        //lcd()->drawBox(0, 0, W, H);
        //lcd()->sendBuffer();
        lcd()->clearBuffer();
    }

    void updateDisplay() {
        // show popup info message
        if (infoMessage != InfoMessageType::INFO_NONE) {
            drawPopupWindow(20, 20, 88, 24, "Info");
            setFont(Font::FONT_8B_TR);
            drawString(TextAlign::CENTER, 22, 106, 38, true, false, false, getStrValue(InfoMessageStr, (uint8_t)infoMessage - 1));            
        }
        lcd()->sendBuffer();
        uart.sendScreenBuffer(lcd()->getBufferPtr(), 1024);
    }

    void timeOut() {
        if (infoMessage != InfoMessageType::INFO_NONE) {
            infoMessage = InfoMessageType::INFO_NONE;         
        }
    }

    void setInfoMessage(InfoMessageType message) {
        infoMessage = message;
    }

    void setFont(Font font) {
        switch (font) {
        case Font::FONT_5_TR:
            lcd()->setFont(u8g2_font_5_tr);
            onlyUpperCase = true;
            break;
        case Font::FONT_8_TR:
            lcd()->setFont(u8g2_font_8_tr);
            onlyUpperCase = false;
            break;
        case Font::FONT_8B_TR:
            lcd()->setFont(u8g2_font_8b_tr);
            onlyUpperCase = false;
            break;
        case Font::FONT_10_TR:
            lcd()->setFont(u8g2_font_10_tr);
            onlyUpperCase = true;
            break;
        case Font::FONT_BN_TN:
            lcd()->setFont(u8g2_font_bn_tn);
            onlyUpperCase = true;
            break;
        }
    }

    void setBlackColor() {
        lcd()->setColorIndex(BLACK);
    }

    void setWhiteColor() {
        lcd()->setColorIndex(WHITE);
    }

    void drawStrf(u8g2_uint_t x, u8g2_uint_t y, const char* str, ...) {
        char text[40] = { 0 };

        va_list va;
        va_start(va, str);
        vsnprintf(text, sizeof(text), str, va);
        va_end(va);

        lcd()->drawStr(x, y, text);
    }

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
            if (tAlign == TextAlign::CENTER) {
                lcd()->drawBox(xstart, yy, xend - xstart, hh);
            }
            else {
                lcd()->drawBox(xx, yy, ww, hh);
            }
            lcd()->setColorIndex(isBlack ? WHITE : BLACK);
        }
        else if (isBox) {
            //lcd()->drawRFrame(xx, yy, ww, hh, 3);
            lcd()->drawFrame(xx, yy, ww, hh);
        }

        lcd()->drawStr(startX, y, str);

    }

    // display a string on multiple text lines, keeping words intact where possible,
    // and accepting \n to force a new line
    void drawWords(u8g2_uint_t xloc, u8g2_uint_t yloc, const char* msg) {
        u8g2_uint_t dspwidth = lcd()->getDisplayWidth(); // display width in pixels
        int strwidth = 0;  // string width in pixels
        char glyph[2]; glyph[1] = 0;
        for (const char* ptr = msg, *lastblank = NULL; *ptr; ++ptr) {
            while (xloc == 0 && (*msg == ' ' || *msg == '\n'))
                if (ptr == msg++) ++ptr; // skip blanks and newlines at the left edge
            glyph[0] = *ptr;
            strwidth += lcd()->getStrWidth(glyph); // accumulate the pixel width
            if (*ptr == ' ')  lastblank = ptr; // remember where the last blank was
            else ++strwidth; // non-blanks will be separated by one additional pixel
            if (*ptr == '\n' ||   // if we found a newline character,
                xloc + strwidth > dspwidth) { // or if we ran past the right edge of the display
                u8g2_int_t starting_xloc = xloc;
                // print to just before the last blank, or to just before where we got to
                while (msg < (lastblank ? lastblank : ptr)) {
                    glyph[0] = *msg++;
                    xloc += lcd()->drawStr(xloc, yloc, glyph);
                }
                strwidth -= xloc - starting_xloc; // account for what we printed
                yloc += (u8g2_uint_t)lcd()->getMaxCharHeight(); // advance to the next line
                xloc = 0; lastblank = NULL;
            }
        }
        while (*msg) { // print any characters left over
            glyph[0] = *msg++;
            xloc += lcd()->drawStr(xloc, yloc, glyph);
        }
    }

    void drawStringf(TextAlign tAlign, u8g2_uint_t xstart, u8g2_uint_t xend, u8g2_uint_t y, bool isBlack, bool isFill, bool isBox, const char* str, ...) {
        char text[60] = { 0 };

        va_list va;
        va_start(va, str);
        vsnprintf(text, sizeof(text), str, va);
        va_end(va);

        drawString(tAlign, xstart, xend, y, isBlack, isFill, isBox, text);
    }

    const char* getStrValue(const char* str, uint8_t index) {
        return u8x8_GetStringLineStart(index, str);
    }

    int stringLengthNL(const char* str) {
        int length = 0;
        while (str[length] != '\n' && str[length] != '\0') {
            ++length;
        }
        return length;
    }

    void drawPopupWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* title) {
        // Draw the popup background
        setWhiteColor();
        lcd()->drawRBox(x - 1, y - 1, w + 3, h + 4, 5);
        setBlackColor();
        lcd()->drawRFrame(x, y, w, h + 1, 5);
        lcd()->drawRFrame(x, y, w + 1, h + 2, 5);

        lcd()->drawBox(x + 1, y + 1, w - 1, 6);

        setFont(Font::FONT_8B_TR);
        drawString(TextAlign::CENTER, x, x + w, y + 6, false, false, false, title);
    }

    uint8_t keycodeToNumber(Keyboard::KeyCode keyCode) {
        if (keyCode >= Keyboard::KeyCode::KEY_0 && keyCode <= Keyboard::KeyCode::KEY_9) {
            return static_cast<uint8_t>(keyCode);
        }
        return 0;
    }

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - */

    void draw_ic8_battery50(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, batt_50_width, batt_50_height, batt_50_bits); }

    void draw_ic8_charging(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, charging_width, charging_height, charging_bits); }

    void draw_smeter(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, smeter_width, smeter_height, smeter_bits); }

    void draw_mmeter(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, mmeter_width, mmeter_height, mmeter_bits); }

    void draw_dotline(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, dotline_width, dotline_height, dotline_bits); }

    void draw_ps(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, batt_ps_width, batt_ps_height, batt_ps_bits); }

    void draw_save(u8g2_uint_t x, u8g2_uint_t y, bool color) { lcd()->setColorIndex(color);  lcd()->drawXBM(x, y, memory_width, memory_height, memory_bits); }

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
    }

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
    }

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

    void drawRSSI(uint8_t sLevel, /*uint16_t plusDB, */u8g2_uint_t x, u8g2_uint_t y) {
        draw_smeter(x, y, BLACK);
        setBlackColor();

        // Draw S1 to S9 blocks
        u8g2_uint_t currentX = x;
        for (uint8_t i = 0; i < sLevel && i < 9; ++i) {
            lcd()->drawBox(currentX, y + 6, 3, 3);
            currentX += 4; // Move to the next block position with 1 pixel spacing
        }

        // Draw dB bar if S level is greater than S9
        if (sLevel == 10) {
            /*u8g2_uint_t dbStartX = x + 38;
            u8g2_uint_t dbEndX = (u8g2_uint_t)(plusDB * 27 / 65); // Scale dB to fit within 27 pixels (38 to 65)
            if (dbEndX > 27) {
                dbEndX = 27;
            }
            lcd()->drawBox(dbStartX, y + 6, dbEndX, 4);*/
        }
    }

    void drawBattery(uint8_t level, u8g2_uint_t x, u8g2_uint_t y) {
        draw_ic8_battery50(x, y, BLACK);
        setBlackColor();
        // fill size 10
        uint8_t fill = static_cast<uint8_t>((level * 10) / 100);
        lcd()->drawBox(x + 1, y + 1, fill, 3);
    }

    const char* generateCTDCList(const uint16_t* options, size_t count, bool isCTCSS = true) {
        char* ptr = uiBuffer;
        size_t  remaining = CHAR_BUFFER_SIZE;
        int written;

        uiBuffer[0] = '\0';

        for (size_t i = 0; i < count; i++) {

            if (isCTCSS) {
                written = snprintf(ptr, remaining, "%u.%u%s", options[i] / 10, options[i] % 10, (i == count - 1) ? "" : "\n");
            }
            else {
                written = snprintf(ptr, remaining, "D%03o%s", options[i], (i == count - 1) ? "" : "\n");
            }

            if (written < 0 || (size_t)written >= remaining) {
                break;
            }

            ptr += written;
            remaining -= written;
        }

        return uiBuffer;
    }

    const char* getFrequencyString(uint32_t frequency, uint8_t precision = 0, bool isKHz = false) {
       // Format the frequency string based on the precision and whether it's in kHz
        if (isKHz) {
            if (precision == 0) {
                snprintf(uiBuffer, CHAR_BUFFER_SIZE, "%u.%03u KHz", frequency / 1000, (frequency % 1000) / 10);
            } else {
                snprintf(uiBuffer, CHAR_BUFFER_SIZE, "%u.%03u KHz", frequency / 1000, frequency % 1000);
            }
        } else {
            if (precision == 0) {
                snprintf(uiBuffer, CHAR_BUFFER_SIZE, "%u.%03u Hz", frequency / 1000, (frequency % 1000) / 10);
            } else {
                snprintf(uiBuffer, CHAR_BUFFER_SIZE, "%u.%03u Hz", frequency / 1000, frequency % 1000);
            }
        }
        return uiBuffer;
    }

private:

    ST7565& st7565;
    UART& uart;

    bool onlyUpperCase = false;
    InfoMessageType infoMessage = InfoMessageType::INFO_NONE;

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

    void set(uint8_t startPos, uint8_t displayLines, uint8_t maxw, const char* sl, const char* sf = NULL) {

        u8sl.visible = displayLines;

        u8sl.total = u8x8_GetStringLineCnt(sl);
        if (u8sl.total <= u8sl.visible)
            u8sl.visible = u8sl.total;

        // Calculate the middle position
        uint8_t middlePos = u8sl.visible / 2;

        // Set the current position
        u8sl.current_pos = startPos;

        // Adjust first_pos to center the current_pos if possible
        if (u8sl.current_pos >= middlePos) {
            u8sl.first_pos = u8sl.current_pos - middlePos;
        }
        else {
            u8sl.first_pos = 0;
        }

        // Ensure first_pos does not exceed the total lines
        if (u8sl.first_pos + u8sl.visible > u8sl.total) {
            u8sl.first_pos = u8sl.total - u8sl.visible;
        }

        // Ensure current_pos is within the valid range
        if (u8sl.current_pos >= u8sl.total) {
            u8sl.current_pos = u8sl.total - 1;
        }

        slines = sl;
        suffix = sf;
        maxWidth = maxw;
    }

    void setCurrentPos(uint8_t pos) {
        u8sl.current_pos = pos;
        uint8_t middlePos = u8sl.visible / 2;

        // Adjust first_pos to center the current_pos if possible
        if (u8sl.current_pos >= middlePos) {
            u8sl.first_pos = u8sl.current_pos - middlePos;
        } else {
            u8sl.first_pos = 0;
        }

        // Ensure first_pos does not exceed the total lines
        if (u8sl.first_pos + u8sl.visible > u8sl.total) {
            u8sl.first_pos = u8sl.total - u8sl.visible;
        }

        // Ensure current_pos is within the valid range
        if (u8sl.current_pos >= u8sl.total) {
            u8sl.current_pos = u8sl.total - 1;
        }
    }

    uint8_t getListPos() {
        return u8sl.current_pos;
    }

    uint8_t getTotal() {
        return u8sl.total;
    }

    void draw(uint8_t y, const char* info = NULL) {
        ui.lcd()->setFontPosBaseline();
        drawSelectionList(y, slines, info);
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

    const char* getStringLine() {
        return u8x8_GetStringLineStart(u8sl.current_pos, slines);
    }

    void setSuffix(const char* sf) {
        suffix = sf;
    }

private:
    u8sl_t u8sl;
    const char* slines;
    const char* suffix;
    uint8_t maxWidth = 75;
    uint8_t startXPos = 2;
    bool showLineNumbers = true;

    u8g2_uint_t drawSelectionListLine(u8g2_uint_t y, uint8_t idx, const char* s, const char* info = NULL) {

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

        if (s == NULL) {
            return line_height;
        }

        /* draw the line */
        /*if (s == NULL)
            s = "";*/

        if (showLineNumbers) {
            ui.setFont(Font::FONT_5_TR);
            ui.drawStringf(TextAlign::LEFT, startXPos, 0, y, is_invert, true, false, "%02u", idx + 1);
        }

        if (is_invert) {
            ui.setFont(Font::FONT_8B_TR);
        }
        else {
            ui.setFont(Font::FONT_8_TR);
        }

        if (showLineNumbers) {
            ui.drawString(TextAlign::LEFT, startXPos + 14, maxWidth, y, is_invert, true, false, s);
            if (info != NULL && is_invert) {
                ui.setFont(Font::FONT_8B_TR);
                if (suffix == NULL) {
                    ui.drawString(TextAlign::RIGHT, 0, maxWidth - 2, y, !is_invert, true, false, info);
                }
                else {
                    ui.drawStringf(TextAlign::RIGHT, 0, maxWidth - 2, y, !is_invert, true, false, "%.*s %s", ui.stringLengthNL(info), info, suffix);
                }
            }
        }
        else {

            if (suffix == NULL) {
                ui.drawString(TextAlign::CENTER, startXPos, maxWidth, y, is_invert, true, false, s);
            }
            else {
                ui.drawStringf(TextAlign::CENTER, startXPos, maxWidth, y, is_invert, true, false, "%.*s %s", ui.stringLengthNL(s), s, suffix);
            }
        }

        return line_height;
    }

    void drawSelectionList(u8g2_uint_t y, const char* s, const char* info = NULL) {
        uint8_t i;
        for (i = 0; i < u8sl.visible; i++) {
            y += drawSelectionListLine(y, i + u8sl.first_pos, s, info);
        }
    }

protected:
    UI& ui;

};

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

class SelectionListPopup : public SelectionList {
public:
    SelectionListPopup(UI& ui) : SelectionList(ui) {
        setShowLineNumbers(false);
    }

    void drawPopup(UI& ui, bool isSetttings = false) {
        uint8_t popupWidth, popupHeight, x, y;
        if (isSetttings) {
            popupWidth = 90; // Width of the popup
            popupHeight = 52; // Height of the popup
            x = 36;
            y = static_cast<uint8_t>((H - popupHeight) / 2); // Center the popup vertically
        }
        else {
            popupWidth = 72; // Width of the popup
            popupHeight = 34; // Height of the popup
            x = static_cast<uint8_t>((W - popupWidth) / 2); // Center the popup horizontally
            y = static_cast<uint8_t>((H - popupHeight) / 2); // Center the popup vertically
        }

        ui.drawPopupWindow(x, y, popupWidth, popupHeight, title);

        // Draw the selection list inside the popup        
        setMaxWidth(static_cast<uint8_t>(x + popupWidth - 4));
        setStartXPos(x + 4);
        draw(y + 14);
    }

    void setPopupTitle(const char* title) {
        this->title = title;
    }

private:
    const char* title;

};

