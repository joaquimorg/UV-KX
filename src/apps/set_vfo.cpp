#include "printf.h"
#include "apps.h"
#include "set_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void SetVFO::drawScreen(void) {

    ui.lcd()->clearBuffer();

    ui.setBlackColor();

    ui.lcd()->drawBox(0, 0, 128, 7);
    
    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, 6, false, false, false, "VFO %s", vfoab == RadioNS::Radio::VFOAB::VFOA ? "A" : "B");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor();

    menulist.draw(15);

    ui.updateDisplay();
}

void SetVFO::init(void) {
    menulist.set(0, 6, 80, "SQUELCH\nSTEP\nMODE\nBANDWIDTH\nTX POWER\nSHIFT\nOFFSET\nRX CTCS\nTX CTCS\nRX DTCS\nTX DTCS\nTX STE\nRX STE\nCOMPANDER\nPTT ID\nAFC\nRX ACG");
}

void SetVFO::update(void) {
    drawScreen();
}

void SetVFO::timeout(void) {
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
};

void SetVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {    

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
