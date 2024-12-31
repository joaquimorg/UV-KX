#pragma once

#include <cstdint>
#include "sys.h"
#include "eeprom.h"


/*

    SQUELCH
    STEP
    MODE
    BANDWIDTH
    TX POWER
    SHIFT
    OFFSET
    RX CTCS
    TX CTCS
    RX DTCS
    TX DTCS
    TX STE
    RX STE
    COMPANDER
    PTT ID
    AFC
    RX ACG

    MIC DB
    BATTERY SAVE
    BUSY LOCKOUT
    BACKLIGHT LEVEL
    BACKLIGHT TIME
    BACKLIGHT MODE
    LCD CONTRAST
    TX TOT
    BEEP

*/


class Settings {
public:
    Settings(System::SystemTask& systask) : systask{ systask }, eeprom() {}


    void factoryReset() {};

private:

    System::SystemTask& systask;
    EEPROM eeprom;

};
