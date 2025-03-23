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
    menulist.set(0, 6, 127, "MIC DB\nBATT SAVE\nBUSY LOCKOUT\nBCKLIGHT LEVEL\nBCKLIGHT TIME\nBCKLIGHT MODE\nLCD CONTRAST\nTX TOT\nBEEP\nRESET");
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
                
                case 0: // MIC DB
                    break;
                case 1: // BATT SAVE
                    break;
                case 2: // BUSY LOCKOUT
                    break;
                case 3: // BCKLIGHT LEVEL
                    break;
                case 4: // BCKLIGHT TIME
                    break;
                case 5: // BCKLIGHT MODE
                    break;
                case 6: // LCD CONTRAST
                    break;
                case 7: // TX TOT
                    break;
                case 8: // BEEP
                    break;
                case 9: // RESET
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::RESETEEPROM);
                    break;

                default:
                    break;
            }
        } 
    }
}
