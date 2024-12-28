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
    ui.setFont(Font::FONT_8_TR);
    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawStr(12, 14, "M195");

    ui.drawString(TextAlign::RIGHT, 0, 126, 6, false, false, false, "FM 26k HIGH");
    ui.setFont(Font::FONT_5_TR);
    ui.drawString(TextAlign::RIGHT, 0, 126, 28, true, false, false, "12.5K  TX 131.8  RX D023N");

    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawLine(5, 9, 5, 27);

    ui.setFont(Font::FONT_8B_TR);
    //ui.lcd()->drawStr(2, 14, "A");
    ui.drawString(TextAlign::LEFT, 2, 0, 21, true, true, false, "A");

    if (radio.getState() == RadioNS::Radio::RadioState::RX_ON) {
        //ui.lcd()->drawStr(2, 20, "RX");
        ui.drawString(TextAlign::LEFT, 12, 0, 21, true, true, false, "RX");
    }
    ui.drawFrequencyBig((radio.getState() == RadioNS::Radio::RadioState::RX_ON), vfo.rx.frequency, 113, 20);



    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawLine(5, 33, 5, 35);
    ui.lcd()->drawLine(5, 44, 5, 46);
    ui.setFont(Font::FONT_8B_TR);
    //ui.lcd()->drawStr(2, 38, "B");
    ui.drawString(TextAlign::LEFT, 2, 0, 42, true, false, true, "B");
    ui.drawFrequencySmall(false, 43932500, 126, 40);
    //ui.drawFrequencyBig(143932500, 110, 40);

    ui.setFont(Font::FONT_8_TR);
    ui.drawString(TextAlign::LEFT, 12, 0, 38, true, false, false, "VIALON1234");
    ui.drawString(TextAlign::LEFT, 12, 0, 46, true, false, false, "M103");
    ui.setFont(Font::FONT_5_TR);
    ui.drawString(TextAlign::RIGHT, 0, 126, 46, true, false, false, "FM 26k LOW");

    /*if (radio.getState() == RadioNS::RadioState::RX_ON) {
        //ui.lcd()->drawStr(2, 20, "RX");
        ui.setFont(Font::FONT_8B_TR);
        ui.drawString(TextAlign::LEFT, 2, 0, 48, true, true, false, "RX");
    } else {

    }*/



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
    }
    else {
        ui.drawBattery(systask.getBattery().getBatteryPercentage(), 96, 59);
    }

    ui.setFont(Font::FONT_5_TR);
    ui.drawStringf(TextAlign::RIGHT, 0, 128, 64, true, false, false, "%i%%", systask.getBattery().getBatteryPercentage());

    ui.drawStringf(TextAlign::RIGHT, 0, 128, 58, true, false, false, "PS  A/B");

    if (showPower) {
        powerList.drawPopup();
    }

    ui.updateDisplay();
}

void MainVFO::showRSSI(void) {

    int16_t rssiPixels = 0;
    if (radio.getState() == RadioNS::Radio::RadioState::RX_ON) {
        int16_t rssi_dBm = radio.getRSSIdBm(); // Get the RSSI value in dBm
        rssiPixels = ui.convertRSSIToPixels(rssi_dBm); // Convert RSSI to pixel scale
    }

    ui.drawRSSI(rssiPixels, 2, 52);

}

void MainVFO::init(void) {
    powerList.set(0, 3, 40, "HIGH\nMID\nLOW");
}

void MainVFO::update(void) {
    drawScreen();
}

void MainVFO::timeout(void) {
   if (showPower) {
        showPower = false;
    }
}

void MainVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {

    RadioNS::Radio::VFO vfo = radio.getVFO(activeVFO);

    if (keyState == Keyboard::KeyState::KEY_RELEASED) {

        if (keyCode == Keyboard::KeyCode::KEY_UP) {
            if (showPower) {
                powerList.prev();
            }
            else {
                vfo.rx.frequency += 1250;
                radio.setVFO(activeVFO, vfo.rx.frequency, vfo.rx.frequency, 0, ModType::MOD_FM);
                radio.setupToVFO(activeVFO);
            }
        }
        else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
            if (showPower) {
                powerList.next();
            }
            else {
                vfo.rx.frequency -= 1250;
                radio.setVFO(activeVFO, vfo.rx.frequency, vfo.rx.frequency, 0, ModType::MOD_FM);
                radio.setupToVFO(activeVFO);
            }

        }
        else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
            if (showPower) {

            } else {
                systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Menu);
            }
        }
        else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
            if (showPower) {
                showPower = false;
            }
        }
    }

    if (keyState == Keyboard::KeyState::KEY_LONG_PRESSED) {
        if (keyCode == Keyboard::KeyCode::KEY_6) {
            showPower = !showPower;
        }
    }
}
