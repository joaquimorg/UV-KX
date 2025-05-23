// This file defines the MainVFO application class, which is responsible for
// the main Variable Frequency Oscillator screen of the radio.
// It handles displaying VFO information, RSSI, and manages popups for settings like
// bandwidth, modulation, and power.
#pragma once

#include "apps.h"
#include "ui.h"
#include "radio.h"
#include "settings.h"

namespace Applications
{
    /**
     * @brief Implements the main VFO (Variable Frequency Oscillator) application.
     *        This class handles the display and interaction for the primary radio screen,
     *        showing frequency, signal strength, and allowing modification of radio parameters
     *        through pop-up menus.
     */
    class MainVFO : public Application {
    public:
        /**
         * @brief Constructor for the MainVFO application.
         * @param systask Reference to the main SystemTask for system-level operations.
         * @param ui Reference to the UI object for display rendering.
         * @param radio Reference to the Radio object for radio-specific functions.
         */
        MainVFO(System::SystemTask& systask, UI& ui, RadioNS::Radio& radio)
            : Application(systask, ui), radio{ radio }, popupList(ui) {
        }

        /**
         * @brief Draws the main VFO screen, including frequency, RSSI, and other status indicators.
         */
        void drawScreen(void);

        /**
         * @brief Initializes the MainVFO application. Called when the application is loaded.
         */
        void init(void);

        /**
         * @brief Updates the MainVFO application state. Called periodically.
         */
        void update(void);

        /**
         * @brief Handles timeout events for the MainVFO application.
         */
        void timeout(void);

        /**
         * @brief Handles keyboard input for the MainVFO application.
         * @param keyCode The code of the key that was pressed or released.
         * @param keyState The state of the key (e.g., pressed, released, long press).
         */
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        RadioNS::Radio& radio; // Reference to the radio control object.

        SelectionListPopup popupList; // UI element for displaying selection popups.

        /**
         * @brief Enumeration for the different types of popups that can be displayed on the MainVFO screen.
         */
        enum PopupList {
            BANDWIDTH,  // Popup for selecting bandwidth.
            MODULATION, // Popup for selecting modulation type.
            POWER,      // Popup for selecting transmission power.
            NONE        // Represents no popup being active.
        };

        bool showPopup = false;         // Flag indicating whether a popup is currently visible.
        bool showFreqInput = false;     // Flag indicating whether the frequency input mode is active.
        uint32_t freqInput = 0;         // Stores the currently entered frequency digits.
        PopupList popupSelected = NONE; // Tracks which popup is currently selected or active.
        
        /**
         * @brief Converts RSSI (Received Signal Strength Indicator) in dBm to an S-level.
         * @param rssi_dBm The RSSI value in dBm.
         * @return The corresponding S-level (0-9).
         */
        uint8_t convertRSSIToSLevel(int16_t rssi_dBm);

        /**
         * @brief Converts RSSI in dBm to a "plus dB" value over S9.
         * @param rssi_dBm The RSSI value in dBm.
         * @return The dB value over S9 (e.g., 10 for S9+10dB). Returns 0 if below S9.
         */
        int16_t convertRSSIToPlusDB(int16_t rssi_dBm);

        /**
         * @brief Displays the RSSI S-meter on the screen.
         * @param posX The X coordinate for the top-left of the S-meter.
         * @param posY The Y coordinate for the top-left of the S-meter.
         */
        void showRSSI(uint8_t posX, uint8_t posY);

        /**
         * @brief Saves the value selected in the currently active popup to the radio settings.
         */
        void savePopupValue(void);

    };

} // namespace Applications
