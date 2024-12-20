#include "printf.h"
#include "apps.h"
#include "menu.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void Menu::drawScreen(void) {

    //char key = '-';

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

void Menu::action(void) {
    
    if (keypad.isPressed()) {
        // Key is currently pressed
        key = keypad.getKey();

        if (keypad.isLongPressed()) {
            return;
        }        
    }

    if (keypad.isReleased()) {
        //systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, 0);
        //systask.loadApplication(Applications::Menu);
        if (key == 'U') {
             ui.u8slPrev();
        } else if (key == 'D') {
            ui.u8slNext();
        } else if (key == 'E') {
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
        }
        key = '-';
    }
}
