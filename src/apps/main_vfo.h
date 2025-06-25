#pragma once

#include "apps.h"
#include "ui.h"
#include "radio.h"
#include "settings.h"

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
        enum PopupList {
            BANDWIDTH,
            MODULATION,
            POWER,
            NONE
        };

        bool showPopup = false;
        bool showFreqInput = false;
        uint32_t freqInput = 0;
        PopupList popupSelected = NONE;

        // Timings
        static constexpr uint8_t BLINK_INTERVAL = 10;    // 1s with 100ms updates
        static constexpr uint16_t LAST_RX_DURATION = 1200; // 2 minutes

        // Track which VFO had the last reception
        Settings::VFOAB lastRXVFO = Settings::VFOAB::NONE;
        uint16_t lastRXCounter = 0;    // counter in 100ms units

        // Previous radio state for detecting RX termination
        Settings::RadioState prevRadioState = Settings::RadioState::IDLE;
        Settings::VFOAB prevRXVFO = Settings::VFOAB::NONE;

        // Blink handling
        uint8_t blinkTimer = 0;

        bool blinkState = false;
        

        uint8_t convertRSSIToSLevel(int16_t rssi_dBm);
        int16_t convertRSSIToPlusDB(int16_t rssi_dBm);
        void showRSSI(uint8_t posX, uint8_t posY);
        void savePopupValue(void);

    };

} // namespace Applications
