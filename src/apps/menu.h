// This file defines the Menu application class, which is responsible for
// displaying and managing the main menu of the radio firmware.
#pragma once

#include "apps.h"
#include "ui.h"

namespace Applications
{
    /**
     * @brief Implements the main menu application.
     *        This class handles the display of menu items and navigation within the menu.
     *        It allows users to select different settings screens or functionalities.
     */
    class Menu : public Application {
    public:
        /**
         * @brief Constructor for the Menu application.
         * @param systask Reference to the main SystemTask for system-level operations.
         * @param ui Reference to the UI object for display rendering.
         */
        Menu(System::SystemTask& systask, UI& ui)
            : Application(systask, ui), menulist(ui) { // Initialize base class and menulist member
        }

        /**
         * @brief Draws the menu screen, displaying the list of menu items.
         */
        void drawScreen(void);

        /**
         * @brief Initializes the Menu application. Called when the application is loaded.
         *        Sets up the menu items in the selection list.
         */
        void init(void);

        /**
         * @brief Updates the Menu application state. Called periodically.
         *        Currently, this function redraws the screen.
         */
        void update(void);

        /**
         * @brief Handles timeout events for the Menu application.
         *        Currently, this function does nothing.
         */
        void timeout(void);

        /**
         * @brief Handles keyboard input for the Menu application.
         *        Processes up/down navigation and selection of menu items.
         * @param keyCode The code of the key that was pressed or released.
         * @param keyState The state of the key (e.g., pressed, released, long press).
         */
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        SelectionList menulist; // UI element for displaying and managing the list of menu items.
    };

} // namespace Applications
