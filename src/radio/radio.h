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

        enum class BatteryType : uint8_t {
            BAT_1600 = 0,
            BAT_2200 = 1,
            BAT_3500 = 2,
        };

        enum class TXOutputPower : uint8_t {
            TX_POWER_LOW = 0,
            TX_POWER_MID = 1,
            TX_POWER_HIGH = 2
        };

        enum class BEEPType : uint8_t {
            BEEP_NONE = 0,
            BEEP_1KHZ_60MS_OPTIONAL = 1,
            BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL = 2,
            BEEP_440HZ_500MS = 3,
            BEEP_880HZ_200MS = 4,
            BEEP_880HZ_500MS = 5,
            BEEP_500HZ_60MS_DOUBLE_BEEP = 6,
            BEEP_440HZ_40MS_OPTIONAL = 7,
            BEEP_880HZ_40MS_OPTIONAL = 8,
            BEEP_880HZ_60MS_TRIPLE_BEEP = 9
        };

        enum class RadioState : uint8_t {
            IDLE = 0,
            RX_ON = 1,
            TX_ON = 2,
        };

        enum class VFOAB : uint8_t {
            VFOA = 0,
            VFOB = 1,
            NONE = 2
        };

        enum class Step : uint8_t {
            STEP_0_5kHz = 0,
            STEP_1_0kHz = 1,
            STEP_2_5kHz = 2,
            STEP_5_0kHz = 3,
            STEP_6_25kHz = 4,
            STEP_10_0kHz = 5,
            STEP_12_5kHz = 6,
            STEP_15_0kHz = 7,
            STEP_20_0kHz = 8,
            STEP_25_0kHz = 9,
            STEP_50_0kHz = 10,
            STEP_100_0kHz = 11,
            STEP_500_0kHz = 12,
        };

        enum class OffsetDirection : uint8_t {
            OFFSET_NONE = 0,
            OFFSET_PLUS = 1,
            OFFSET_MINUS = 2,
        };

        enum class TXRX : uint8_t {
            OFF = 0,
            RX = 1,
            TX = 2,
            RX_TX = 3
        };

        enum class ONOFF : uint8_t {
            OFF = 0,
            ON = 1
        };

        enum class CodeType : uint8_t {
            NONE = 0,
            CT = 1,
            DCS = 2,
            NDCS = 3
        };

        /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

        static constexpr const char* codetypeStr = "NONE\nCT\nDCS\n-DCS";

        static constexpr const char* txrxStr = "OFF\nRX\nTX\nRX/TX";

        static constexpr const char* onoffStr = "OFF\nON";

        static constexpr const char* powerStr = "LOW\nMID\nHIGH";

        static constexpr const char* offsetStr = "OFF\n+\n-";

        static constexpr const char* modulationStr = "FM\nAM\nLSB\nUSB\nBYP\nRAW\nWFM\nPRST";

        static constexpr const char* bandwidthStr = "W 26\nW 23\nW 20\nW 17\nW 14\nW 12\nN 10\nN 9\nU 7\nU 6";

        static constexpr const char* stepStr = "0.5\n1.0\n2.5\n5.0\n6.25\n10.0\n12.5\n15.0\n20.0\n25.0\n30.0\n50.0\n100.0\n500.0";

        struct FREQ {
            uint32_t frequency : 27;
            CodeType codeType : 4;
            uint8_t code;
        } __attribute__((packed)); // 5 Bytes

        struct VFO {
            FREQ rx;
            FREQ tx;
            char name[11];
            int16_t channel;
            uint8_t squelch : 4;
            Step step : 4;
            ModType modulation : 4;
            BK4819_Filter_Bandwidth bw : 4;
            TXOutputPower power : 2;
            OffsetDirection shift : 2;
            ONOFF repeaterSte : 1;
            ONOFF ste : 1;
            TXRX compander : 2;
            uint8_t pttid : 4;
            uint8_t afc : 4;
            uint8_t rxagc : 4;
        } __attribute__((packed));

        VFO radioVFO[2];

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
        void setVFO(VFOAB vfo, uint32_t rx, uint32_t tx, int16_t channel, ModType modulation);
        void setupToVFO(VFOAB vfo);

        void playBeep(BEEPType beep);

        // get VFO
        VFO getActiveVFO() { return radioVFO[(uint8_t)activeVFO]; };
        VFO getVFO(VFOAB vfo) { return radioVFO[(uint8_t)vfo]; };
        void setVFO(VFOAB vfoab, VFO vfo) { 
            radioVFO[(uint8_t)vfoab] = vfo;
            setupToVFO(vfoab);
        };
        // get active VFO
        VFOAB getCurrentVFO(void) { return activeVFO; };

        // Change Active VFO
        void changeActiveVFO(void) {
            activeVFO = (activeVFO == VFOAB::VFOA) ? VFOAB::VFOB : VFOAB::VFOA;
            rxVFO = activeVFO;
            setupToVFO(activeVFO);
        }

        VFOAB getRXVFO(void) { return rxVFO; };


        void setRXVFO(VFOAB vfo) {
            rxVFO = vfo;
            setupToVFO(vfo);
        }

        void toggleRX(bool on);
        void checkRadioInterrupts(void);

        RadioState getState() { return state; }

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

    private:
        System::SystemTask& systask;
        UART& uart;
        BK4819& bk4819;
        Settings& settings;

        bool dualWatch = true;
        uint8_t dualWatchTimer = 0;

        static constexpr uint8_t dualWatchTime = 20;

        bool speakerOn = false; // speaker on/off
        RadioState state = RadioState::IDLE;
        VFOAB activeVFO = VFOAB::VFOA;
        VFOAB rxVFO = VFOAB::VFOA;
        VFOAB lastRXVFO = VFOAB::NONE;

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
            {"HAM 17m", 1806800, 1816800, 1},
            {"HAM 15m", 2100000, 2145000, 1},
            {"HAM 12m", 2489000, 2499000, 1},
            {"HAM 10m", 2800000, 2970000, 1},
            {"HAM 6m", 5000000, 5400000, 1},
            {"HAM 4m EU", 7000000, 7100000, 1},
            {"HAM 2m", 14400000, 14800000, 1},
            {"HAM 1.25m", 21900000, 22500000, 1},
            {"HAM 70cm", 42000000, 44600625, 1},
            {"PMR 446", 44600625, 44619375, 1},
            {"HAM 33cm", 90200000, 92800000, 1},
            {"HAM 23cm", 124000000, 130000000, 1},
            // Other Bands
            {"CB RADIO", 2696500, 2740500, 1}, // 11m Citizens Band
            {"FM", 8800000, 10800000, 0},
            {"AIRCRAFT", 10800000, 13700000, 0},
            {"MARINE VHF", 15600000, 17400000, 0},
        };

        void toggleBK4819(bool on);

    };

} // namespace Radio