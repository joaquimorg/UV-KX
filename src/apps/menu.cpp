#include "printf.h"
#include "apps.h"
#include "menu.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void Menu::drawScreen(void) {

    ui.lcd()->clearBuffer();

    ui.setBlackColor();

    ui.lcd()->drawBox(0, 0, 128, 7);
    
    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, "MENU");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", ui.getSelectionListPos() + 1, ui.getSelectionListTotal());

    ui.setBlackColor();
    ui.setFont(Font::FONT_8_TR);
    //ui.drawStrf(10, 30, "BATT : %u.%02uV - %3i%%", systask.getBattery().getBatteryVoltageAverage() / 100, systask.getBattery().getBatteryVoltageAverage() % 100, systask.getBattery().getBatteryPercentage());    
    ui.drawMenu();

    ui.lcd()->sendBuffer();
}


void Menu::init(void) {
    ui.setMenu();
}

void Menu::update(void) {
    drawScreen();
}

void Menu::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {    

    if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
        if (keyCode == Keyboard::KeyCode::KEY_UP) {
             ui.u8slPrev();
        } else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
            ui.u8slNext();
        } else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
        }     
    }
}
