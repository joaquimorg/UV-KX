#pragma once

#include "apps.h"
#include "ui.h"
#include "radio.h"

namespace Applications
{

    class MainVFO : public Application {
    public:
        MainVFO(System::SystemTask& systask, UI& ui, RadioNS::Radio& radio)
            : Application(systask, ui), radio{ radio }, popupList(ui) {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void timeout(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        RadioNS::Radio& radio;

        SelectionListPopup popupList;

        bool showPopup = false;

        uint8_t convertRSSIToSLevel(int16_t rssi_dBm);
        int16_t convertRSSIToPlusDB(int16_t rssi_dBm);
        void showRSSI(void);

    };

} // namespace Applications
