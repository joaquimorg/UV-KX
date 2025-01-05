#pragma once

#include "apps.h"
#include "radio.h"
#include "settings.h"
#include "ui.h"

namespace Applications
{

    class SetVFO : public Application {
    public:
        SetVFO(System::SystemTask& systask, UI& ui, RadioNS::Radio::VFOAB vfoab, Settings& settings, RadioNS::Radio& radio)
            : Application(systask, ui), menulist(ui), optionlist(ui), vfoab{ vfoab }, settings{ settings }, radio{ radio } {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void timeout(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        SelectionList menulist;
        SelectionListPopup optionlist;

        RadioNS::Radio::VFOAB vfoab;
        Settings& settings;
        RadioNS::Radio& radio;

        RadioNS::Radio::VFO vfo;

        uint8_t optionSelected = 0;

        void loadOptions();
        void setOptions();

    };

} // namespace Applications
