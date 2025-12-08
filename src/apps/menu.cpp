#include "printf.h"
#include "apps.h"

#include "menu.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void Menu::drawScreen(void) {

    ui.clearDisplay();

    ui.setBlackColor();

    ui.lcd()->drawBox(0, 0, 128, 7);
    
    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, "MENU");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor();

    menulist.draw(15);

    ui.updateDisplay();
}


void Menu::init(void) {
    firstVFOIsA = radio.getCurrentVFO() != Settings::VFOAB::VFOB;
    const char* first = firstVFOIsA ? "VFO A SETTINGS" : "VFO B SETTINGS";
    const char* second = firstVFOIsA ? "VFO B SETTINGS" : "VFO A SETTINGS";

    snprintf(menuText, sizeof(menuText), "%s\n%s\nRADIO SETTINGS\nMESSENGER\nSCANNER\nABOUT", first, second);
    menulist.set(0, 6, 127, menuText);
    //drawScreen();
}

void Menu::update(void) {
    drawScreen();
}

void Menu::timeout(void) {
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
}

void Menu::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {    

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
                case 0:
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD,
                        (uint32_t)(firstVFOIsA ? Applications::SETVFOA : Applications::SETVFOB));
                    break;
                case 1:
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD,
                        (uint32_t)(firstVFOIsA ? Applications::SETVFOB : Applications::SETVFOA));
                    break;
                case 2:
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::SETRADIO);
                    break;
                case 3:
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MESSENGER);
                    break;
                case 4:
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::SCANNER);
                    break;
                case 5:
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::ABOUT);
                    break;
                default:
                    break;
            }
        } 
    }
}
