#include "printf.h"

#include "apps.h"
#include "welcome.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void Welcome::drawScreen(void) {

    ui.lcd()->clearBuffer();

    ui.lcd()->setColorIndex(BLACK);

    ui.setFont(Font::FONT_8B_TR);

    ui.lcd()->drawStr(5, 10, "Hello !");

    ui.lcd()->drawStr(5, 20, "UV-Kx Open Firmware");

    ui.drawBattery(systask.getBattery().getBatteryPercentage(), 20, 30);

    ui.setFont(Font::FONT_8_TR);
    ui.drawStrf(8, 42, "%i%% %u.%02uV", systask.getBattery().getBatteryPercentage(), systask.getBattery().getBatteryVoltageAverage() / 100, systask.getBattery().getBatteryVoltageAverage() % 100);

    ui.lcd()->drawStr(64, 33, "SI4732");
    ui.lcd()->drawStr(64, 42, "EEPROM+");

    ui.lcd()->drawStr(115, 33, "-");
    ui.lcd()->drawStr(115, 42, "-");

    ui.setFont(Font::FONT_5_TR);

    //ui.drawString(TextAlign::CENTER, 0, 128, 55, true, false, false, "Any key to continue...");

    ui.lcd()->drawBox(0, 57, 128, 7);
    ui.drawString(TextAlign::CENTER, 0, 128, 63, false, false, false, AUTHOR_STRING " - " VERSION_STRING);

    ui.lcd()->sendBuffer();
}


void Welcome::init(void) {
    drawScreen();
}

void Welcome::timeout(void) {
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
};

void Welcome::action(__attribute__((unused)) Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {
    if (keyState == Keyboard::KeyState::KEY_PRESSED) {
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
    }
}
