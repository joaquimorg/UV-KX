// This file defines the base Application class and an enumeration of all available applications.
// It serves as a foundational header for creating and managing different application states within the system.
#pragma once

#include "ui.h"
#include "keyboard.h"

namespace System {
    class SystemTask; // Forward declaration of SystemTask to avoid circular dependency
}

namespace Applications
{
    /**
     * @brief Enumerates all possible applications that can run on the system.
     */
    enum class Applications
    {
        None = 0,       // Represents no application or an uninitialized state.
        Welcome,        // The initial welcome screen application.
        RESETINIT,      // Application for resetting and initializing the device.
        RESETEEPROM,    // Application for resetting the EEPROM.
        MainVFO,        // The main VFO (Variable Frequency Oscillator) screen.
        Menu,           // The main menu application.
        SETVFOA,        // Application for setting VFO A parameters.
        SETVFOB,        // Application for setting VFO B parameters.
        SETRADIO,       // Application for setting general radio parameters.
        MESSENGER,      // (Placeholder) Application for messaging functionality.
        SCANNER,        // (Placeholder) Application for scanning frequencies.
        ABOUT           // Application to display information about the firmware.
    };

    /**
     * @brief Abstract base class for all applications.
     *        Defines the common interface that all specific applications must implement.
     */
    class Application {
    public:
        /**
         * @brief Constructor for the Application class.
         * @param systask Reference to the main SystemTask, providing access to system-level functions.
         * @param ui Reference to the UI object, for interacting with the display.
         */
        explicit Application(System::SystemTask& systask, UI &ui) : systask{ systask }, ui{ui} {};

        /**
         * @brief Pure virtual function to initialize the application.
         *        This function is called when the application is first loaded.
         */
        virtual void init(void) = 0;

        /**
         * @brief Virtual function to update the application's state.
         *        This function is called periodically by the system.
         *        Default implementation does nothing.
         */
        virtual void update(void) {};

        /**
         * @brief Pure virtual function to handle keyboard input.
         * @param keyCode The code of the key that was pressed or released.
         * @param keyState The state of the key (e.g., pressed, released, long press).
         */
        virtual void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) = 0;

        /**
         * @brief Virtual function to handle timeout events.
         *        This function is called when a system-defined timeout occurs.
         *        Default implementation does nothing.
         */
        virtual void timeout(void) {};

    protected:
        System::SystemTask& systask; // Reference to the SystemTask for system interactions.
        UI& ui;                      // Reference to the UI object for display interactions.
    };

} // namespace Applications
