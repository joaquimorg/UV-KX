#include "settings.h"
#include "system.h"

void Settings::applyRadioSettings()
{
    // Apply backlight timeout based on stored setting
    uint16_t timeout = 0;
    switch (radioSettings.backlightTime) {
    default:
    case BacklightTime::BACKLIGHT_OFF:
        timeout = 0;
        break;
    case BacklightTime::BACKLIGHT_ON:
        timeout = 0xFFFF; // effectively disable timeout
        break;
    case BacklightTime::BACKLIGHT_5S:
        timeout = 5;
        break;
    case BacklightTime::BACKLIGHT_10S:
        timeout = 10;
        break;
    case BacklightTime::BACKLIGHT_15S:
        timeout = 15;
        break;
    case BacklightTime::BACKLIGHT_20S:
        timeout = 20;
        break;
    case BacklightTime::BACKLIGHT_30S:
        timeout = 30;
        break;
    case BacklightTime::BACKLIGHT_60S:
        timeout = 60;
        break;
    case BacklightTime::BACKLIGHT_120S:
        timeout = 120;
        break;
    case BacklightTime::BACKLIGHT_240S:
        timeout = 240;
        break;
    }
    systask.setBacklightTimeout(timeout);
    systask.setLCDContrast((uint8_t)(100 + (radioSettings.lcdContrast * 10)));
    systask.setBacklightLevel(radioSettings.backlightLevel);
}


void Settings::scheduleSaveIfNeeded() {
    if (memcmp(&radioSettings, &lastSavedRadioSettings, sizeof(SETTINGS)) != 0) {
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_SAVESETTINGS, 0);
    }
}
