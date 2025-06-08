#pragma once

#include "apps.h"
#include "radio.h"
#include "settings.h"
#include "ui.h"

namespace Applications
{

    class SetVFO : public Application {
    public:
        SetVFO(System::SystemTask& systask, UI& ui,Settings::VFOAB vfoab, Settings& settings, RadioNS::Radio& radio)
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

        Settings::VFOAB vfoab;
        Settings& settings;
        RadioNS::Radio& radio;

        Settings::VFO vfo;
        Settings::VFO initialVFO; // store original VFO settings for change detection

        uint8_t inputSelect = 0;
        uint8_t optionSelected = 0;
        uint8_t userOptionSelected = 0;
        uint32_t userOptionInput = 0;

        void loadOptions();
        void setOptions();
        const char* getCurrentOption();
        const char* codeValue(Settings::CodeType type, uint8_t code, SelectionList& list);
        bool configureCodeList(Settings::CodeType type, uint8_t code);

    };

} // namespace Applications
