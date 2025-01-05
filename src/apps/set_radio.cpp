#include "printf.h"
#include "apps.h"
#include "set_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void SetRadio::drawScreen(void) {

    ui.clearDisplay();

    ui.setBlackColor();

    ui.lcd()->drawBox(0, 0, 128, 7);
    
    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, "RADIO");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor();

    menulist.draw(15);

    ui.updateDisplay();
}


void SetRadio::init(void) {
    menulist.set(0, 6, 120, "MIC DB\nBATTERY SAVE\nBUSY LOCKOUT\nBACKLIGHT LEVEL\nBACKLIGHT TIME\nBACKLIGHT MODE\nLCD CONTRAST\nTX TOT\nBEEP\nRESET");
}

void SetRadio::update(void) {
    drawScreen();
}

void SetRadio::timeout(void) {
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
};

void SetRadio::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {    

    if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
        if (keyCode == Keyboard::KeyCode::KEY_UP) {
             menulist.prev();
        } else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
            menulist.next();
        } else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
        }

        if (keyCode == Keyboard::KeyCode::KEY_MENU) {
            switch (menulist.getListPos()) {
                
                default:
                    break;
            }
        } 
    }
}
