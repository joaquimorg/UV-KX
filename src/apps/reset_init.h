// This file defines the ResetInit application class, which is responsible for
// handling the device initialization and EEPROM reset functionalities.
// It presents the user with options to confirm or cancel these actions
// and displays progress during the operation.
#pragma once

#include "apps.h"
#include "settings.h"
#include "ui.h"

namespace Applications
{
    /**
     * @brief Implements the application for device reset and initialization.
     *        This class handles user interaction for confirming reset/initialization,
     *        performing the operation, and displaying progress. It can operate in
     *        two modes: full device initialization or EEPROM reset.
     */
    class ResetInit : public Application {
    public:
        /**
         * @brief Constructor for the ResetInit application.
         * @param systask Reference to the main SystemTask for system-level operations.
         * @param ui Reference to the UI object for display rendering.
         * @param settings Reference to the Settings object for EEPROM operations.
         * @param isInit True if the application should perform full initialization, 
         *               false if it should perform EEPROM reset. Defaults to true.
         */
        ResetInit(System::SystemTask& systask, UI& ui, Settings& settings, bool isInit = true) 
            : Application(systask, ui), settings{ settings }, isInit{ isInit } {};

        /**
         * @brief Draws the reset/initialization screen.
         *        Displays confirmation questions, progress bars, or completion messages.
         */
        void drawScreen(void);

        /**
         * @brief Initializes the ResetInit application. Called when the application is loaded.
         *        Sets the initial state for the reset/initialization process.
         */
        void init(void);

        /**
         * @brief Updates the ResetInit application state. Called periodically.
         *        Manages the progression of the reset/initialization process.
         */
        void update(void);

        /**
         * @brief Handles keyboard input for the ResetInit application.
         *        Processes user confirmation (MENU) or cancellation (EXIT).
         * @param keyCode The code of the key that was pressed or released.
         * @param keyState The state of the key (e.g., pressed, released).
         */
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

        /**
         * @brief Handles timeout events for the ResetInit application.
         *        If a timeout occurs, it typically exits to the main VFO screen.
         */
        void timeout(void);

    private:
        Settings& settings;     // Reference to the settings object for EEPROM operations.
        bool isInit;            // True for full initialization, false for EEPROM reset.
        bool showQuestion;      // Flag to indicate if the confirmation question should be shown.
        bool isToInitialize;    // Flag to indicate if the user has confirmed the operation.
        bool isReady;           // Flag to indicate if the operation is complete and ready to exit.
        uint8_t initProgress;   // Variable to store the progress of the initialization/reset operation (0-100).
    };

} // namespace Applications

