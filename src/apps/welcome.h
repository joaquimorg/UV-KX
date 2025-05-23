// This file defines the Welcome application class, which is responsible for
// displaying the initial welcome screen of the radio firmware.
// This screen is typically shown on startup.
#pragma once

#include "apps.h" // Base Application class
#include "ui.h"     // UI rendering utilities

namespace Applications
{
    /**
     * @brief Implements the welcome screen application.
     *        This class handles the display of the initial splash screen, showing firmware
     *        information and potentially other relevant startup messages.
     */
    class Welcome : public Application {
    public:
        /**
         * @brief Constructor for the Welcome application.
         * @param systask Reference to the main SystemTask for system-level operations.
         * @param ui Reference to the UI object for display rendering.
         */
        Welcome(System::SystemTask& systask, UI& ui)
            : Application(systask, ui) { // Initialize the base Application class
        }

        /**
         * @brief Draws the welcome screen.
         *        Displays firmware version, author information, and any other welcome messages.
         */
        void drawScreen(void);

        /**
         * @brief Initializes the Welcome application. Called when the application is loaded.
         *        Currently, this function does not perform any specific initialization.
         */
        void init(void);

        /**
         * @brief Updates the Welcome application state. Called periodically.
         *        For the Welcome screen, this usually just involves redrawing the screen.
         */
        void update(void);

        /**
         * @brief Handles keyboard input for the Welcome application.
         *        Typically, any key press might dismiss the welcome screen.
         * @param keyCode The code of the key that was pressed or released.
         * @param keyState The state of the key (e.g., pressed, released).
         */
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

        /**
         * @brief Handles timeout events for the Welcome application.
         *        After a certain period, the welcome screen might automatically transition
         *        to the main application (e.g., MainVFO).
         */
        void timeout(void);

    private:
        // No private members specific to Welcome screen in this version.
        // Potential future members: display duration timer, specific assets to load, etc.
    };

} // namespace Applications

