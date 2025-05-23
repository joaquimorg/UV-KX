// -----------------------------------------------------------------------------------------
// This file defines the Battery class, which encapsulates functionality
// for monitoring battery status, including voltage, current, percentage,
// display level, and charging state. It uses ADC readings and lookup tables
// for voltage-to-percentage conversion based on battery type.
// -----------------------------------------------------------------------------------------
#pragma once

#include <cstdint>  // For standard integer types like uint16_t, uint8_t
#include <array>      // For std::array
#include <cassert>    // For assert (not directly used but good for robustness)
#include <algorithm>  // For std::min
#include "sys.h"      // For boardADCGetBatteryInfo (hardware-specific ADC reading)
#include "misc.h"     // Miscellaneous utilities (potentially used by sys.h or for other types)

/**
 * @brief Manages battery status, including voltage, current, percentage, and charging state.
 * This class periodically reads battery ADC values, calculates average voltage,
 * converts voltage to percentage using lookup tables for different battery types,
 * and determines display levels and low battery warnings.
 */
class Battery {
public:
    /**
     * @brief Enumerates known battery types, typically differing by capacity.
     * This affects the voltage-to-percentage conversion.
     */
    enum class Type {
        TYPE_1600_MAH, ///< Represents a 1600 mAh battery type.
        TYPE_2200_MAH, ///< Represents a 2200 mAh battery type.
        TYPE_UNKNOWN   ///< Represents an unknown or default battery type.
    };

    /**
     * @brief Constructor for the Battery class.
     * Initializes member variables to default states (e.g., zero voltage/current,
     * not charging, 1600mAh type by default).
     */
    Battery() :
        batteryCalibration({ 0 }),      // Initialize calibration data (placeholder)
        batteryCurrent(0),              // Initialize current reading
        batteryVoltages({ 0 }),         // Initialize raw voltage readings
        batteryVoltageAverage(0),       // Initialize average voltage
        batteryPercentage(0),           // Initialize calculated percentage
        batteryDisplayLevel(0),         // Initialize display level (e.g., bars)
        chargingWithTypeC(false),       // Initialize charging status
        lowBattery(false),              // Initialize low battery flag
        lowBatteryConfirmed(false),     // Initialize confirmed low battery flag (unused in provided code)
        batteryType(Type::TYPE_1600_MAH) { // Default battery type
    }

    /**
     * @brief Gets the calculated average battery voltage.
     * The voltage is typically represented in 10mV units (e.g., 740 means 7.40V).
     * @return The average battery voltage as a uint16_t.
     */
    uint16_t getBatteryVoltageAverage(void) {
        return batteryVoltageAverage;
    }

    /**
     * @brief Gets the battery display level.
     * This is a simplified representation of battery charge, often for icons (e.g., 0-7 bars).
     * @return The battery display level as a uint8_t.
     */
    uint8_t getBatteryDisplayLevel(void) {
        return batteryDisplayLevel;
    }

    /**
     * @brief Gets the calculated battery percentage (0-100%).
     * @return The battery percentage as a uint8_t.
     */
    uint8_t getBatteryPercentage(void) {
        return batteryPercentage;
    }

    /**
     * @brief Checks if the battery is currently charging (assumed via Type-C).
     * @return True if charging, false otherwise.
     */
    bool isCharging(void) {
        return chargingWithTypeC;
    }

    /**
     * @brief Reads raw battery voltage and current from ADC, calculates average voltage,
     * percentage, display level, and updates charging and low battery status.
     * This method should be called periodically to update battery information.
     */
    void getReadings() {
        // uint8_t previousBatteryLevel = batteryDisplayLevel; // Store previous level if needed for change detection

        // Read multiple ADC samples for voltage and current.
        // `boardADCGetBatteryInfo` is a hardware-specific function.
        for (unsigned int i = 0; i < sizeof(batteryVoltages); i++) // sizeof(batteryVoltages) is likely incorrect for number of readings, should be batteryVoltages.size()
            boardADCGetBatteryInfo(&batteryVoltages[i], &batteryCurrent); // TODO: Review loop condition, should be .size() for std::array

        // Calculate average of the first 4 voltage readings.
        uint16_t voltage = static_cast<uint16_t>((batteryVoltages[0] + batteryVoltages[1] + batteryVoltages[2] + batteryVoltages[3]) / 4);

        // Calibrate the raw ADC voltage reading.
        // The commented-out line suggests `batteryCalibration[3]` would be a calibration value from EEPROM.
        // A fixed divisor (1875) is used here as a placeholder or default.
        // batteryVoltageAverage = static_cast<uint16_t>((voltage * 760) / batteryCalibration[3]);
        batteryVoltageAverage = static_cast<uint16_t>((voltage * 760) / 1875); // Scale factor 760, divisor 1875

        // Convert the calibrated average voltage to a percentage.
        batteryPercentage = voltsToPercent(batteryVoltageAverage);

        // Determine the display level (e.g., bars on an icon) based on average voltage.
        // This provides a coarser representation than percentage.
        if (batteryVoltageAverage > 890) { // Max voltage for full display
            batteryDisplayLevel = 7;
        }
        else if (batteryVoltageAverage < 630) { // Min voltage for empty display
            batteryDisplayLevel = 0;
        }
        else {
            // Intermediate levels based on percentage thresholds.
            batteryDisplayLevel = 1; // Default to 1 bar if not empty.
            const uint8_t levels[] = { 5, 17, 41, 65, 88 }; // Percentage thresholds for display levels 2-6
            uint8_t perc = static_cast<uint8_t>(batteryPercentage);
            for (uint8_t i = 6; i >= 1; --i) { // Iterate from highest level down
                if (perc > levels[i - 2]) { // levels array is 0-indexed, for levels 2-6, indices are 0-4
                    batteryDisplayLevel = i;
                    break;
                }
            }
        }

        // Determine charging status based on measured current.
        // A current reading >= 501 (units depend on ADC/shunt resistor) indicates charging.
        chargingWithTypeC = (batteryCurrent >= 501);

        // Update low battery status based on display level.
        if (batteryDisplayLevel > 2) { // If more than 2 bars
            lowBattery = false;
        }
        else if (batteryDisplayLevel < 2) { // If less than 2 bars
            lowBattery = true;
        }
        else { // If exactly 2 bars
            lowBattery = false; // Consider 2 bars not critically low for this flag.
        }
    }

    /**
     * @brief Checks if the battery is in a low state, with a debounce mechanism.
     * @return True if the battery is low and not charging, and the low battery period has expired. False otherwise.
     */
    bool isLowBattery(void) {
        if (chargingWithTypeC) {         
            return false; // Not low if charging.
        }
        if (lowBattery) { // If the `lowBattery` flag (from getReadings) is true
            if (lowBatteryPeriod == 0) {
                return true; // Confirmed low battery after debounce period.
            }
            else {
                lowBatteryPeriod--; // Decrement debounce counter.
                return false;     // Still in debounce period.
            }
        }
        else {
            lowBatteryPeriod = 15; // Reset debounce period when battery is not low.
            return false;
        }
    }

private:
    // Member variables storing battery state and configuration.
    std::array<uint16_t, 6> batteryCalibration; ///< ADC calibration values (e.g., from EEPROM). (Currently unused in calculations shown)
    uint16_t batteryCurrent;                    ///< Last read battery current (raw ADC value or mA).
    std::array<uint16_t, 4> batteryVoltages;    ///< Array to store multiple raw ADC voltage readings for averaging.
    uint16_t batteryVoltageAverage;             ///< Calculated average battery voltage (calibrated, typically 10mV units).
    uint8_t batteryPercentage;                  ///< Calculated battery charge percentage (0-100%).
    uint8_t batteryDisplayLevel;                ///< Battery level for display purposes (e.g., 0-7 bars).
    bool chargingWithTypeC;                     ///< True if charging via Type-C is detected.
    bool lowBattery;                            ///< Flag indicating a potential low battery state (based on display level).
    bool lowBatteryConfirmed;                   ///< Flag for confirmed low battery (unused in provided code, could be for hysteresis).

    Type batteryType;                           ///< Currently selected battery type (e.g., 1600mAh, 2200mAh).

    uint8_t lowBatteryPeriod = 30;              ///< Debounce counter for low battery warnings.

    // Static constant lookup tables for voltage-to-percentage conversion.
    // Each inner array is {voltage_in_10mV, percentage}.
    // These define points on the battery discharge curve.
    static constexpr std::array<std::array<uint16_t, 2>, 7> Voltage2Percentage_1600 = 
        { {{828, 100}, {814, 97}, {760, 25}, {729, 6}, {630, 0}, {0, 0}, {0, 0}} };
    static constexpr std::array<std::array<uint16_t, 2>, 7> Voltage2Percentage_2200 = 
        { {{832, 100}, {813, 95}, {740, 60}, {707, 21}, {682, 5}, {630, 0}, {0, 0}} };
    // Table to select between the 1600mAh and 2200mAh curves based on `batteryType`.
    static constexpr std::array<std::array<std::array<uint16_t, 2>, 7>, 2> Voltage2PercentageTable = 
        { {Voltage2Percentage_1600, Voltage2Percentage_2200} };

    /**
     * @brief Converts a given battery voltage (in 10mV units) to a percentage.
     * Uses linear interpolation between points defined in the `Voltage2PercentageTable`
     * corresponding to the current `batteryType`.
     * @param voltage_10mV The battery voltage in 10mV units (e.g., 740 for 7.40V).
     * @return The calculated battery percentage (0-100) as a uint8_t.
     */
    uint8_t voltsToPercent(unsigned int voltage_10mV) const {
        // Select the correct discharge curve based on batteryType.
        const auto& crv = Voltage2PercentageTable[static_cast<int>(batteryType)];
        const int multiplier = 1000; // Multiplier for fixed-point arithmetic to avoid floats.

        // Iterate through the curve points to find the segment where the current voltage falls.
        for (unsigned int i = 1; i < crv.size(); ++i) {
            if (voltage_10mV > crv[i][0]) { // Current voltage is between crv[i-1][0] and crv[i][0]
                // Linear interpolation: P = P1 + (V - V1) * (P0 - P1) / (V0 - V1)
                // Or, y = mx + c form: P = a*V + b
                // a = (P0 - P1) / (V0 - V1)
                // b = P1 - a*V1
                // Here, P0=crv[i-1][1], V0=crv[i-1][0], P1=crv[i][1], V1=crv[i][0]
                int a = (crv[i - 1][1] - crv[i][1]) * multiplier / (crv[i - 1][0] - crv[i][0]);
                int b = crv[i][1] - a * crv[i][0] / multiplier;
                int p = a * static_cast<int>(voltage_10mV) / multiplier + b;
                return static_cast<uint8_t>(std::min(p, 100)); // Cap percentage at 100.
            }
        }
        return 0; // Voltage is below the lowest point in the curve.
    }
};

