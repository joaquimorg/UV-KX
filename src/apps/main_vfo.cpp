#include "printf.h"
#include "apps.h"
#include "main_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void MainVFO::drawScreen(void) {

    RadioNS::Radio::VFO vfo = radio.getVFO(activeVFO);

    //char key = '-';

    ui.lcd()->clearBuffer();

    ui.lcd()->setColorIndex(BLACK);

    ui.lcd()->drawBox(0, 0, 128, 7);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, "PMR-14-FRN");
    //ui.drawString(TextAlign::LEFT, 2, 0, 8, false, false, false, "PMR446");
    ui.setFont(Font::FONT_5_TR);
    ui.drawString(TextAlign::RIGHT, 0, 126, 6, false, false, false, "FM 26k HIGH");
    ui.setFont(Font::FONT_8_TR);
    ui.drawString(TextAlign::RIGHT, 0, 126, 28, true, false, false, "12.5K  TX 131.8  RX D023N");

    ui.lcd()->setColorIndex(BLACK);
    //ui.setFont(Font::FONT_8_TR);
    ui.lcd()->drawStr(12, 14, "M195");
    ui.setFont(Font::FONT_8B_TR);
    //ui.lcd()->drawStr(2, 14, "A");
    ui.drawString(TextAlign::LEFT, 2, 0, 14, true, true, false, "A");

    if (radio.getState() == RadioNS::Radio::RadioState::RX_ON) {
        //ui.lcd()->drawStr(2, 20, "RX");
        ui.drawString(TextAlign::LEFT, 2, 0, 21, true, true, false, "RX");
    }
    ui.drawFrequencyBig((radio.getState() == RadioNS::Radio::RadioState::RX_ON), vfo.rx.frequency, 113, 20);



    ui.lcd()->setColorIndex(BLACK);
    ui.setFont(Font::FONT_8_TR);
    //ui.lcd()->drawStr(2, 38, "B");
    ui.drawString(TextAlign::LEFT, 2, 0, 38, true, false, true, "B");
    ui.drawFrequencySmall(false, 43932500, 126, 40);
    //ui.drawFrequencyBig(143932500, 110, 40);

    ui.setFont(Font::FONT_8_TR);
    ui.drawString(TextAlign::LEFT, 12, 0, 38, true, false, false, "VIALON1234");

    ui.drawString(TextAlign::RIGHT, 0, 126, 48, true, false, false, "FM 26k LOW");

    /*if (radio.getState() == RadioNS::RadioState::RX_ON) {
        //ui.lcd()->drawStr(2, 20, "RX");
        ui.setFont(Font::FONT_8B_TR);
        ui.drawString(TextAlign::LEFT, 2, 0, 48, true, true, false, "RX");
    } else {*/
    ui.drawString(TextAlign::LEFT, 12, 0, 48, true, false, false, "M103");
    //}



    /*ui.lcd()->setColorIndex(BLACK);
    ui.setFont(Font::FONT_8_TR);
    ui.lcd()->drawStr(2, 50, "C");
    ui.drawFrequencySmall(143865000, 126, 50);*/

    //ui.setFont(Font::FONT_8B_TR);
    //ui.drawString(TextAlign::LEFT, 10, 0, 62, true, false, false, "M049 LISBOA1234");

    //ui.lcd()->setColorIndex(BLACK);

    //ui.drawStrf(10, 30, "BATT : %u.%02uV - %3i%%", systask.getBattery().getBatteryVoltageAverage() / 100, systask.getBattery().getBatteryVoltageAverage() % 100, systask.getBattery().getBatteryPercentage());

    /*ui.setFont(Font::FONT_5_TR);
    ui.drawStrf(5, 63, "%i", getElapsedMilliseconds());*/


    showRSSI();

    if (systask.getBattery().isCharging()) {
        ui.draw_ic8_charging(100, 59, BLACK);
    } else {
        ui.drawBattery(systask.getBattery().getBatteryPercentage(), 96, 59);
    }

    ui.setFont(Font::FONT_5_TR);
    ui.drawStringf(TextAlign::RIGHT, 0, 128, 64, true, false, false, "%i%%", systask.getBattery().getBatteryPercentage());

    ui.lcd()->sendBuffer();
}

void MainVFO::showRSSI(void) {

    int16_t rssiPixels = 0;
    if (radio.getState() == RadioNS::Radio::RadioState::RX_ON) {
        int16_t rssi_dBm = radio.getRSSIdBm(); // Get the RSSI value in dBm
        rssiPixels = ui.convertRSSIToPixels(rssi_dBm); // Convert RSSI to pixel scale
    }

    ui.drawRSSI(rssiPixels, 2, 54);

}

void MainVFO::init(void) {
}

void MainVFO::update(void) {
    drawScreen();
}

void MainVFO::action(void) {

    RadioNS::Radio::VFO vfo = radio.getVFO(activeVFO);

    if (keypad.isPressed()) {
        // Key is currently pressed        

        if (keypad.isLongPressed()) {
            return;
        }
    }

    if (keypad.isReleased()) {
        //systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, 0);
        //systask.loadApplication(Applications::MainVFO);
        key = keypad.getKey();

        if (key == 'U') {
            vfo.rx.frequency += 1250;
            radio.setVFO(activeVFO, vfo.rx.frequency, vfo.rx.frequency, 0, ModType::MOD_FM);
            radio.setupToVFO(activeVFO);
        }
        else if (key == 'D') {
            vfo.rx.frequency -= 1250;
            radio.setVFO(activeVFO, vfo.rx.frequency, vfo.rx.frequency, 0, ModType::MOD_FM);
            radio.setupToVFO(activeVFO);
        }
        else if (key == 'M') {
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Menu);
        }
        key = '-';
    }
}
