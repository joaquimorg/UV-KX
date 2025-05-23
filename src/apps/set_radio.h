// This file defines the SetRadio application class, which is responsible for
// providing an interface to configure general radio settings.
// It displays a list of radio parameters that can be adjusted by the user.
#pragma once

#include "apps.h"
#include "radio.h" // May not be directly needed here, but often radio settings affect the Radio class
#include "ui.h"

namespace Applications
{
    /**
     * @brief Implements the application for setting general radio parameters.
     *        This class displays a menu of radio settings that the user can navigate
     *        and modify.
     */
    class SetRadio : public Application {
    public:
        /**
         * @brief Constructor for the SetRadio application.
         * @param systask Reference to the main SystemTask for system-level operations.
         * @param ui Reference to the UI object for display rendering.
         */
        SetRadio(System::SystemTask& systask, UI& ui)
            : Application(systask, ui), menulist(ui) { // Initialize base class and menulist member
        }

        /**
         * @brief Draws the radio settings screen, displaying the list of configurable parameters.
         */
        void drawScreen(void);

        /**
         * @brief Initializes the SetRadio application. Called when the application is loaded.
         *        Sets up the list of radio settings in the selection list.
         */
        void init(void);

        /**
         * @brief Updates the SetRadio application state. Called periodically.
         *        Currently, this function redraws the screen.
         */
        void update(void);

        /**
         * @brief Handles timeout events for the SetRadio application.
         *        Currently, this function does nothing specific beyond default Application behavior.
         */
        void timeout(void);

        /**
         * @brief Handles keyboard input for the SetRadio application.
         *        Processes up/down navigation, selection of settings, and modification of values.
         * @param keyCode The code of the key that was pressed or released.
         * @param keyState The state of the key (e.g., pressed, released, long press).
         */
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        SelectionList menulist; // UI element for displaying and managing the list of radio settings.
                                // Each item in this list will correspond to a radio parameter.
    };

} // namespace Applications
