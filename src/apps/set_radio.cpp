#include "printf.h"
#include "apps.h"
#include "set_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void SetRadio::drawScreen(void) {

    ui.lcd()->clearBuffer();

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
    menulist.set(0, 6, 80, "FREQUENCY\nMODE\nFILTER\nATTENUATOR\nAGC\nSTEP\nRIT\nXIT\nVOLUME\nSQUELCH\nSCAN\nLOCK\nRESET");
}

void SetRadio::update(void) {
    drawScreen();
}

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
