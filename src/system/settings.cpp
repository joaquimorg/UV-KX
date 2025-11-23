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

    // Apply brightness/contrast immediately so the user sees the change in the menu
    systask.setBacklightLevel(radioSettings.backlightLevel);
    systask.setLCDContrast(static_cast<uint8_t>(100 + (radioSettings.lcdContrast * 10)));

    // Battery save controls the power-save timer (disable when OFF)
    systask.setPowerSaveEnabled(radioSettings.batterySave == ONOFF::ON);
}


void Settings::scheduleSaveIfNeeded() {
    // Check if radio settings have changed
    if (memcmp(&radioSettings, &lastSavedRadioSettings, sizeof(SETTINGS)) != 0) {
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_SAVESETTINGS, 0);
    }
}


void Settings::scheduleMemorySaveIfNeeded(uint16_t channelNumber, uint8_t vfoIndex) {
    if (channelNumber < 1 || channelNumber > MAX_CHANNELS || vfoIndex > 1) {
        return;
    }
    
    // Read current channel data from EEPROM
    VFO currentChannelData;
    bool channelExists = readChannel(channelNumber, currentChannelData);
    
    // If channel doesn't exist or VFO data has changed, schedule save
    if (!channelExists || memcmp(&radioSettings.vfo[vfoIndex], &currentChannelData, sizeof(VFO)) != 0) {
        // Store the channel and VFO to save
        pendingMemoryChannel = channelNumber;
        pendingMemoryVFO = vfoIndex;
        memorySavePending = true;
        memorySaveDelay = saveDelayTicks;
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_SAVESETTINGS, 0);
    }
}
