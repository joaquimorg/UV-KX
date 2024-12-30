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
    ui.drawString(TextAlign::RIGHT, 0, 126, 26, true, false, false, "12.5K  TX 131.8  RX D023N");

    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawLine(5, 9, 5, 25);

    ui.setFont(Font::FONT_8B_TR);
    //ui.lcd()->drawStr(2, 14, "A");
    ui.drawString(TextAlign::LEFT, 2, 0, 20, true, true, false, "A");

    if (radio.getState() == RadioNS::Radio::RadioState::RX_ON) {
        //ui.lcd()->drawStr(2, 20, "RX");
        ui.drawString(TextAlign::LEFT, 12, 0, 20, true, true, false, "RX");
    }
    ui.drawFrequencyBig((radio.getState() == RadioNS::Radio::RadioState::RX_ON), vfo.rx.frequency, 113, 19);



    uint8_t vfoBY = 29;
    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawLine(5, vfoBY, 5, vfoBY + 3);
    ui.lcd()->drawLine(5, vfoBY + 12, 5, vfoBY + 14);
    ui.setFont(Font::FONT_8B_TR);
    //ui.lcd()->drawStr(2, 38, "B");
    ui.drawString(TextAlign::LEFT, 2, 0, vfoBY + 10, true, false, true, "B");
    ui.drawFrequencySmall(false, 43932500, 126, vfoBY + 8);
    //ui.drawFrequencyBig(143932500, 110, 40);

    ui.setFont(Font::FONT_8_TR);
    ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 5, true, false, false, "VIALON1234");
    ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, false, false, "M103");
    ui.setFont(Font::FONT_5_TR);
    ui.drawString(TextAlign::RIGHT, 0, 126, vfoBY + 15, true, false, false, "FM 26k LOW");

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

    if (systask.wasFKeyPressed()) {
        ui.drawStringf(TextAlign::RIGHT, 0, 120, 57, true, true, false, "F");
    }
    else {
        ui.drawStringf(TextAlign::RIGHT, 0, 128, 58, true, false, false, "PS  A/B");
    }

    if (showPopup) {
        popupList.drawPopup();
    }

    ui.updateDisplay();
}

void MainVFO::showRSSI(void) {

    int16_t rssiPixels = 0;
    if (radio.getState() == RadioNS::Radio::RadioState::RX_ON) {
        int16_t rssi_dBm = radio.getRSSIdBm(); // Get the RSSI value in dBm
        rssiPixels = ui.convertRSSIToPixels(rssi_dBm); // Convert RSSI to pixel scale
    }

    ui.drawRSSI(rssiPixels, 0, 52);

}

void MainVFO::init(void) {
}

void MainVFO::update(void) {
    drawScreen();
}

void MainVFO::timeout(void) {
    if (showPopup) {
        showPopup = false;
    }
}

void MainVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {

    RadioNS::Radio::VFO vfo = radio.getVFO(activeVFO);

    if (keyState == Keyboard::KeyState::KEY_RELEASED) {

        if (showPopup) {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                popupList.prev();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                popupList.next();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                showPopup = false;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_4 || keyCode == Keyboard::KeyCode::KEY_5 || keyCode == Keyboard::KeyCode::KEY_6) {
                popupList.next();
            }
        }

        if (keyCode == Keyboard::KeyCode::KEY_UP) {
            vfo.rx.frequency += 1250;
            radio.setVFO(activeVFO, vfo.rx.frequency, vfo.rx.frequency, 0, ModType::MOD_FM);
            radio.setupToVFO(activeVFO);
        }
        else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
            vfo.rx.frequency -= 1250;
            radio.setVFO(activeVFO, vfo.rx.frequency, vfo.rx.frequency, 0, ModType::MOD_FM);
            radio.setupToVFO(activeVFO);
        }
        else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Menu);
        }
        else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {

        }
    }

    if (keyState == Keyboard::KeyState::KEY_LONG_PRESSED || keyState == Keyboard::KeyState::KEY_PRESSED_WITH_F) {
        if (keyCode == Keyboard::KeyCode::KEY_4) {
            popupList.set(0, 3, 0, "W 26k\nW 23k\nW 20k\nW 17k\nW 14k\nW 12k\nN 10k\nN 9k\nU 7k\nU 6k");
            popupList.setPopupTitle("BANDWIDTH");
            showPopup = true;
        }

        if (keyCode == Keyboard::KeyCode::KEY_5) {
            popupList.set(0, 3, 0, "FM\nAM\nUSB");
            popupList.setPopupTitle("MODULATION");
            showPopup = true;
        }

        if (keyCode == Keyboard::KeyCode::KEY_6) {
            popupList.set(0, 3, 0, "HIGH\nMID\nLOW");
            popupList.setPopupTitle("TX POWER");
            showPopup = true;
        }
    }
}
