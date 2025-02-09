#pragma once

#include <cstdint>
#include <array>
#include <cassert>
#include <algorithm>
#include "sys.h"
#include "misc.h"

class Battery {
public:
    enum class Type {
        TYPE_1600_MAH,
        TYPE_2200_MAH,
        TYPE_UNKNOWN
    };

    Battery() :
        batteryCalibration({ 0 }),
        batteryCurrent(0),
        batteryVoltages({ 0 }),
        batteryVoltageAverage(0),
        batteryPercentage(0),
        batteryDisplayLevel(0),
        chargingWithTypeC(false),
        lowBattery(false),
        lowBatteryConfirmed(false),
        batteryType(Type::TYPE_1600_MAH) {
    }

    uint16_t getBatteryVoltageAverage(void) {
        return batteryVoltageAverage;
    }

    uint8_t getBatteryDisplayLevel(void) {
        return batteryDisplayLevel;
    }

    uint8_t getBatteryPercentage(void) {
        return batteryPercentage;
    }

    bool isCharging(void) {
        return chargingWithTypeC;
    }

    void getReadings() {
        //uint8_t previousBatteryLevel = batteryDisplayLevel;

        for (unsigned int i = 0; i < sizeof(batteryVoltages); i++)
            boardADCGetBatteryInfo(&batteryVoltages[i], &batteryCurrent);

        uint16_t voltage = static_cast<uint16_t>((batteryVoltages[0] + batteryVoltages[1] + batteryVoltages[2] + batteryVoltages[3]) / 4);

        // missing EEPROM reading
        //batteryVoltageAverage = static_cast<uint16_t>((voltage * 760) / batteryCalibration[3]);
        //
        batteryVoltageAverage = static_cast<uint16_t>((voltage * 760) / 1875);

        batteryPercentage = voltsToPercent(batteryVoltageAverage);

        if (batteryVoltageAverage > 890) {
            batteryDisplayLevel = 7;
        }
        else if (batteryVoltageAverage < 630) {
            batteryDisplayLevel = 0;
        }
        else {
            batteryDisplayLevel = 1;
            const uint8_t levels[] = { 5, 17, 41, 65, 88 };
            uint8_t perc = static_cast<uint8_t>(batteryPercentage);
            for (uint8_t i = 6; i >= 1; --i) {
                if (perc > levels[i - 2]) {
                    batteryDisplayLevel = i;
                    break;
                }
            }
        }

        chargingWithTypeC = (batteryCurrent >= 501);

        if (batteryDisplayLevel > 2) {
            lowBattery = false;
        }
        else if (batteryDisplayLevel < 2) {
            lowBattery = true;
        }
        else {
            lowBattery = false;
        }

    }

    bool isLowBattery(void) {

        if (chargingWithTypeC) {         
            return false;
        }
        if (lowBattery) {
            if (lowBatteryPeriod == 0) {
                return true;
            }
            else {
                lowBatteryPeriod--;
                return false;
            }
        }
        else {
            lowBatteryPeriod = 15;
            return false;
        }
    }

private:
    std::array<uint16_t, 6> batteryCalibration;
    uint16_t batteryCurrent;
    std::array<uint16_t, 4> batteryVoltages;
    uint16_t batteryVoltageAverage;
    uint8_t batteryPercentage;
    uint8_t batteryDisplayLevel;
    bool chargingWithTypeC;
    bool lowBattery;
    bool lowBatteryConfirmed;

    Type batteryType;

    uint8_t lowBatteryPeriod = 30;

    static constexpr std::array<std::array<uint16_t, 2>, 7> Voltage2Percentage_1600 = { {{828, 100}, {814, 97}, {760, 25}, {729, 6}, {630, 0}, {0, 0}, {0, 0}} };
    static constexpr std::array<std::array<uint16_t, 2>, 7> Voltage2Percentage_2200 = { {{832, 100}, {813, 95}, {740, 60}, {707, 21}, {682, 5}, {630, 0}, {0, 0}} };
    static constexpr std::array<std::array<std::array<uint16_t, 2>, 7>, 2> Voltage2PercentageTable = { {Voltage2Percentage_1600, Voltage2Percentage_2200} };

    uint8_t voltsToPercent(unsigned int voltage_10mV) const {
        const auto& crv = Voltage2PercentageTable[static_cast<int>(batteryType)];
        const int multiplier = 1000;
        for (unsigned int i = 1; i < crv.size(); ++i) {
            if (voltage_10mV > crv[i][0]) {
                int a = (crv[i - 1][1] - crv[i][1]) * multiplier / (crv[i - 1][0] - crv[i][0]);
                int b = crv[i][1] - a * crv[i][0] / multiplier;
                int p = a * static_cast<int>(voltage_10mV) / multiplier + b;
                return static_cast<uint8_t>(std::min(p, 100));
            }
        }
        return 0;
    }

};

