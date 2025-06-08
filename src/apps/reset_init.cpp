#include "printf.h"

#include "apps.h"
#include "reset_init.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

//const uint8_t text1[] = { 0x4C, 0x71, 0x1A, 0x10, 0x43, 0xD1, 0x38, 0xC6, 0x82, 0x38, 0xD4, 0xC4, 0x35, 0x36, 0x88, 0x49, 0xA2, 0x0D, 0x08, 0xE3, 0x0F, 0x01, 0x32, 0x01, 0x2C, 0x46, 0xDA, 0x4C, 0xE6, 0x94, 0x48, 0x46, 0x80, 0x2C, 0xB6, 0x85, 0x10, 0x04, 0xD4, 0x44, 0x44, 0x9C, 0x68, 0x84, 0xDA, 0x31, 0x44, 0x93, 0x68, 0x11, 0x1A, 0x20, 0xD2, 0x13, 0x20, 0x02, 0xC8, 0x64, 0x40, 0xDB, 0x69, 0x31, 0xC8, 0x49, 0xA0, 0x02, 0x4C, 0x83, 0x8D, 0x69, 0x62, 0x0B, 0x2D, 0xA1, 0x11, 0x01, 0x21, 0x1A, 0x00, 0xB2, 0xDA, 0x09, 0x44, 0x51, 0x10, 0xD4, 0xDA, 0x0C, 0x04, 0xC0, 0x6D, 0xA3, 0x00, 0x28, 0x46, 0x80, 0x68, 0x10, 0x02, 0x29, 0x43, 0xDA, 0x04, 0x41, 0x4E, 0x44, 0x46, 0x82, 0x38, 0xD4, 0xC8, 0x35, 0x42, 0x0D, 0x19, 0xB0 } /* ("THE EEPROM CONTENT IS INCOMPATIBLE. TO USE ALL FEATURES, IT MUST BE INITIALIZED. THIS ACTION WILL ERASE ALL CURRENT DATA. MAKE A BACKUP BEFORE CONTINUING.") */;

void ResetInit::drawScreen(void) {

    ui.clearDisplay();

    ui.lcd()->setColorIndex(BLACK);

    ui.setFont(Font::FONT_8B_TR);

    if (isToInitialize) {
        ui.drawString(TextAlign::CENTER, 0, 128, 8, true, false, false, "EEPROM INITIALIZATION");
        // show progress bar and percentage
        u8g2_uint_t barWidth = (u8g2_uint_t)((initProgress * 120) / 100);
        ui.lcd()->drawFrame(4, 20, 120, 10);
        ui.lcd()->drawBox(4, 20, barWidth, 10);
        ui.drawStringf(TextAlign::CENTER, 0, 128, 46, true, false, false, "%d%%", initProgress);
        if (isReady) {
            ui.drawString(TextAlign::CENTER, 0, 128, 36, true, false, false, "DONE");
        }
    }
    else {

        ui.drawString(TextAlign::CENTER, 0, 128, 8, true, false, false, "WARNING !");

        ui.setFont(Font::FONT_5_TR);

        ui.drawWords(0, 16, "THE EEPROM CONTENT IS INCOMPATIBLE. TO USE ALL FEATURES, IT MUST BE INITIALIZED. THIS ACTION WILL ERASE ALL CURRENT DATA.");

        ui.setFont(Font::FONT_8B_TR);
        ui.drawWords(0, 46, "MAKE A BACKUP BEFORE CONTINUING...");
        //ui.drawWords(0, 16, ui.decompressText(text1, sizeof(text1)));        

        if (showQuestion) {
            ui.drawPopupWindow(15, 20, 96, 32, "Init. EEPROM ?");
            ui.setFont(Font::FONT_8_TR);
            ui.drawString(TextAlign::CENTER, 17, 111, 36, true, false, false, "Press 1 to accept.");
            ui.drawStringf(TextAlign::CENTER, 17, 111, 46, true, false, false, "%s to cancel.", isInit ? "Other key" : "EXIT");            
        }
    }

    ui.setFont(Font::FONT_5_TR);

    ui.lcd()->drawBox(0, 57, 128, 7);
    ui.drawString(TextAlign::CENTER, 0, 128, 63, false, false, false, AUTHOR_STRING " - " VERSION_STRING);

    ui.updateDisplay();
}


void ResetInit::init(void) {
}

void ResetInit::update(void) {
    if (isToInitialize) {
        if (initProgress < 100) {            
            initProgress = settings.initEEPROM();
        }
        else {
            isReady = true;
        }
    }
    drawScreen();
}

void ResetInit::timeout(void) {
    if (isToInitialize) {
        if (isReady) {
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Welcome);
        }
    }
};

void ResetInit::action(__attribute__((unused)) Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {
    if (keyState == Keyboard::KeyState::KEY_PRESSED) {

        if (!isToInitialize) {
            if (showQuestion) {
                if (keyCode == Keyboard::KeyCode::KEY_1) {
                    //systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Welcome);
                    initProgress = 0;
                    isToInitialize = true;
                } else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                    if (!isInit) {
                        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
                    }
                }
                showQuestion = false;                
            }
            else {
                showQuestion = true;                
            }

        }
    }
}
