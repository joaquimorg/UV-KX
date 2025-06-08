#pragma once

#include "apps.h"
#include "radio.h" // May not be directly needed here, but often radio settings affect the Radio class
#include "settings.h"
#include "ui.h"

namespace Applications
{

    class SetRadio : public Application {
    public:
        /**
         * @brief Constructor for the SetRadio application.
         * @param systask Reference to the main SystemTask for system-level operations.
         * @param ui Reference to the UI object for display rendering.
         */
        SetRadio(System::SystemTask& systask, UI& ui, Settings& settings)
            : Application(systask, ui), menulist(ui), optionlist(ui), settings{ settings } {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void timeout(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        SelectionList menulist;        // Main list of radio settings
        SelectionListPopup optionlist; // Popup for selecting option values

        Settings& settings;            // Reference to global settings

        uint8_t inputSelect = 0;       // Accumulates digits for direct menu selection
        uint8_t optionSelected = 0;    // Currently active option index (0 when none)

        void loadOptions();            // Populate optionlist for selected item
        void setOptions();             // Save selected option back to settings
        const char* getCurrentOption();// Return string for current value
    };

} // namespace Applications
