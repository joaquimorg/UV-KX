#pragma once

#include "bk4819.h"
#include "uart_hal.h"
#include "misc.h"
#include "settings.h"

namespace System {
    class SystemTask;
}

namespace RadioNS
{

    class Radio {
    public:

        /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

        Settings::VFO radioVFO[2];

        struct FrequencyBand {
            char name[11];
            uint32_t lower_freq;
            uint32_t upper_freq;
            bool txEnable;
        };

        Radio(System::SystemTask& systask, UART& uart, BK4819& bk4819, Settings& settings) : systask{ systask }, uart{ uart }, bk4819{ bk4819 }, settings{ settings } {};

        void toggleSpeaker(bool on);

        void setSquelch(uint32_t f, uint8_t sql);
        void setFilterBandwidth(BK4819_Filter_Bandwidth bw) { bk4819.setFilterBandwidth(bw); }
        void setVFO(Settings::VFOAB vfo, uint32_t rx, uint32_t tx, int16_t channel, ModType modulation);
        void setupToVFO(Settings::VFOAB vfo);

        void playBeep(Settings::BEEPType beep);

        // get VFO
        Settings::VFO getActiveVFO() { return radioVFO[(uint8_t)activeVFO]; };
        Settings::VFO getVFO(Settings::VFOAB vfo) { return radioVFO[(uint8_t)vfo]; };
        void setVFO(Settings::VFOAB vfoab, Settings::VFO vfo) {
            uint8_t vfoIndex = (uint8_t)vfoab;
            radioVFO[vfoIndex] = vfo;

            if (radioVFO[vfoIndex].channel > 0) {
                snprintf(radioVFO[vfoIndex].name, sizeof(radioVFO[vfoIndex].name), "CH-%03d", radioVFO[vfoIndex].channel);
            }
            else {
                strncpy(radioVFO[vfoIndex].name, getBandName(radioVFO[vfoIndex].rx.frequency), sizeof(radioVFO[vfoIndex].name) - 1);
                radioVFO[vfoIndex].name[sizeof(radioVFO[vfoIndex].name) - 1] = '\0'; // Ensure null termination
            }
            setupToVFO(vfoab);
        };
        // get active VFO
        Settings::VFOAB getCurrentVFO(void) { return activeVFO; };

        // Change Active VFO
        void changeActiveVFO(void) {
            activeVFO = (activeVFO == Settings::VFOAB::VFOA) ? Settings::VFOAB::VFOB : Settings::VFOAB::VFOA;
            setupToVFO(activeVFO);
        }

        Settings::VFOAB getRXVFO(void) { return rxVFO; };


        void setRXVFO(Settings::VFOAB vfo) {
            rxVFO = vfo;
            setupToVFO(vfo);
        }

        void toggleRX(bool on, Settings::CodeType codeType);
        void checkRadioInterrupts(void);

        Settings::RadioState getState() { return state; }

        uint16_t getRSSI() { return bk4819.getRSSI(); }

        int16_t getRSSIdBm(void) {
            uint16_t rssi = bk4819.getRSSI();
            int16_t rssidbm = (int16_t)((rssi / 2) - 160);
            // TODO: RSSI gRxVfo->Band
            return rssidbm + dBmCorrTable[6];
        }

        uint8_t convertRSSIToSLevel(int16_t rssi_dBm);
        int16_t convertRSSIToPlusDB(int16_t rssi_dBm);

        void runDualWatch(void);

        const char* getBandName(uint32_t frequency) {
            int num_bands = sizeof(radioBands) / sizeof(radioBands[0]);

            for (int i = 0; i < num_bands; ++i) {
                if (frequency >= radioBands[i].lower_freq && frequency <= radioBands[i].upper_freq) {
                    return radioBands[i].name;
                }
            }

            return "";
        }

        void setupToneDetection(Settings::VFOAB vfo);

        bool isRXToneDetected(void) { return rxToneDetected; }

        bool isRadioReady(void) { return radioReady; }
        void setRadioReady(bool ready) { radioReady = ready; }
        
        void setPowerSaveMode() {
            inPowerSaveMode = true;            
            bk4819.setSleepMode();            
        }

        bool isPowerSaveMode(void) { return inPowerSaveMode; }

        void setNormalPowerMode() {
            if (!inPowerSaveMode) {
                return;
            }
            inPowerSaveMode = false;
            bk4819.setNormalMode();
        }

    private:
        System::SystemTask& systask;
        UART& uart;
        BK4819& bk4819;
        Settings& settings;

        bool inPowerSaveMode = false;

        bool dualWatch = true;
        uint8_t dualWatchTimer = 0;
        uint8_t timeoutPSDualWatch = 10;

        static constexpr uint8_t dualWatchTime = 50;

        bool rxToneDetected = false;

        bool radioReady = false;

        bool speakerOn = false; // speaker on/off
        Settings::RadioState state = Settings::RadioState::IDLE;
        Settings::VFOAB activeVFO = Settings::VFOAB::VFOA;
        Settings::VFOAB rxVFO = Settings::VFOAB::VFOA;
        Settings::VFOAB lastRXVFO = Settings::VFOAB::NONE;

        static constexpr int8_t dBmCorrTable[7] = {
                -15, // band 1
                -25, // band 2
                -20, // band 3
                -4, // band 4
                -7, // band 5
                -6, // band 6
                 -1  // band 7
        };

        static constexpr FrequencyBand radioBands[] = {
            {"HAM 17m", 1806800, 1816800, 0},
            {"HAM 15m", 2100000, 2145000, 0},
            {"HAM 12m", 2489000, 2499000, 0},
            {"HAM 10m", 2800000, 2970000, 0},
            {"HAM 6m", 5000000, 5400000, 0},
            {"HAM 4m EU", 7000000, 7100000, 0},
            {"HAM 2m", 14400000, 14800000, 1},
            {"HAM 1.25m", 21900000, 22500000, 0},
            {"HAM 70cm", 42000000, 44600625, 1},
            {"PMR 446", 44600625, 44619375, 1},
            {"HAM 33cm", 90200000, 92800000, 0},
            {"HAM 23cm", 124000000, 130000000, 0},
            // Other Bands
            {"CB RADIO", 2696500, 2740500, 0}, // 11m Citizens Band
            {"FM", 8800000, 10800000, 0},
            {"AIRCRAFT", 10800000, 13700000, 0},
            {"MARINE VHF", 15600000, 17400000, 0},
        };

        void toggleBK4819(bool on);

        uint32_t DCSCalculateGolay(uint32_t codeWord) {
            unsigned int i;
            uint32_t word = codeWord;
            for (i = 0; i < 12; i++) {
                word <<= 1;
                if (word & 0x1000)
                    word ^= 0x08EA;
            }
            return codeWord | ((word & 0x0FFE) << 11);
        }

        uint32_t DCSGetGolayCodeWord(Settings::CodeType codeType, uint8_t option) {
            uint32_t code = DCSCalculateGolay(Settings::DCSOptions[option] + 0x800U);
            if (codeType == Settings::CodeType::NDCS)
                code ^= 0x7FFFFF;
            return code;
        }

    };

} // namespace Radio