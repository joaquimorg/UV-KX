// This file defines the SetVFO application class, which is responsible for
// providing an interface to configure settings for a specific VFO (VFO A or VFO B).
// It displays a list of VFO parameters that can be adjusted by the user,
// and manages sub-menus or input methods for changing these parameters.
#pragma once

#include "apps.h"
#include "radio.h"
#include "settings.h"
#include "ui.h"

namespace Applications
{
    /**
     * @brief Implements the application for setting parameters of a specific VFO (VFO A or VFO B).
     *        This class displays a menu of VFO-specific settings (e.g., frequency, CTCSS/DCS codes,
     *        bandwidth, power) and allows the user to modify them. It uses a main list for parameters
     *        and a pop-up list for selecting options for each parameter.
     */
    class SetVFO : public Application {
    public:
        /**
         * @brief Constructor for the SetVFO application.
         * @param systask Reference to the main SystemTask for system-level operations.
         * @param ui Reference to the UI object for display rendering.
         * @param vfoab Specifies which VFO (VFOA or VFOB) this instance will configure.
         * @param settings Reference to the global Settings object to read/write VFO parameters.
         * @param radio Reference to the Radio object to apply settings and get radio state.
         */
        SetVFO(System::SystemTask& systask, UI& ui,Settings::VFOAB vfoab, Settings& settings, RadioNS::Radio& radio)
            : Application(systask, ui), menulist(ui), optionlist(ui), vfoab{ vfoab }, settings{ settings }, radio{ radio } {
        }

        /**
         * @brief Draws the VFO settings screen.
         *        Displays the list of VFO parameters and the current value of the selected parameter.
         *        If an option list is active, it draws the popup for option selection.
         */
        void drawScreen(void);

        /**
         * @brief Initializes the SetVFO application. Called when the application is loaded.
         *        Loads the settings for the specified VFO and populates the main menu list.
         */
        void init(void);

        /**
         * @brief Updates the SetVFO application state. Called periodically.
         *        Currently, this function redraws the screen.
         */
        void update(void);

        /**
         * @brief Handles timeout events for the SetVFO application.
         *        If a timeout occurs, it typically exits to the main VFO screen.
         */
        void timeout(void);

        /**
         * @brief Handles keyboard input for the SetVFO application.
         *        Processes navigation through the settings list, selection of parameters,
         *        and interaction with the option selection popup.
         * @param keyCode The code of the key that was pressed or released.
         * @param keyState The state of the key (e.g., pressed, released).
         */
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        SelectionList menulist;         // UI element for the main list of VFO parameters.
        SelectionListPopup optionlist;  // UI element for the popup list used to select options for a parameter.

        Settings::VFOAB vfoab;          // Identifies which VFO (A or B) is being configured.
        Settings& settings;             // Reference to the global settings object.
        RadioNS::Radio& radio;          // Reference to the radio control object.

        Settings::VFO vfo;              // Local copy of the VFO settings being edited.

        uint8_t inputSelect = 0;        // Index of the currently selected item in the main `menulist`.
        uint8_t optionSelected = 0;     // Flag indicating if the `optionlist` popup is currently active (1) or not (0).
                                        // Also used to store the index of the item in `menulist` that `optionlist` refers to.
        uint8_t userOptionSelected = 0; // Index of the currently selected item within the `optionlist` popup.
        uint32_t userOptionInput = 0;   // Stores user input for numerical values (e.g., frequency, CTCSS code).

        /**
         * @brief Loads the appropriate options into the `optionlist` based on the currently selected VFO parameter.
         *        For example, if "RX CTCSS" is selected, this loads the list of CTCSS tones.
         */
        void loadOptions();

        /**
         * @brief Applies the selected option from `optionlist` or `userOptionInput` to the local `vfo` settings.
         *        After applying, it also saves the VFO settings back to the global settings and reconfigures the radio.
         */
        void setOptions();

        /**
         * @brief Gets a string representation of the current value for the VFO parameter selected in `menulist`.
         *        Used for displaying the current value next to the parameter name.
         * @return A const char* pointing to the string representation of the current value.
         */
        const char* getCurrentOption();
    };

} // namespace Applications
