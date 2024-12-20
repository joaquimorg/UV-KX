#pragma once

#include "bk4819.h"
#include "uart_hal.h"
#include "misc.h"

namespace System {
    class SystemTask;
}

namespace RadioNS
{

    class Radio {
    public:

        enum class RadioState {
            IDLE,
            RX_ON,
            TX_ON,
        };

        struct FREQ {
            uint32_t frequency;
            uint8_t codeType;
            uint8_t code;
        };

        struct VFO {
            FREQ rx;
            FREQ tx;
            int16_t channel;
            ModType modulation;
        };

        VFO radioVFO[2];        

        Radio(System::SystemTask& systask, UART& uart, BK4819& bk4819) : systask{ systask }, uart{ uart }, bk4819{ bk4819 } {};

        void toggleSpeaker(bool on);

        void setSquelch(uint32_t f, uint8_t sql);
        void setFilterBandwidth(BK4819_Filter_Bandwidth bw) { bk4819.setFilterBandwidth(bw); }
        void setVFO(uint8_t vfo, uint32_t rx, uint32_t tx, int16_t channel, ModType modulation);
        void setupToVFO(uint8_t vfo);

        // get VFO
        VFO getVFO(uint8_t vfo) { return radioVFO[vfo]; };

        void toggleRX(bool on);
        void checkRadioInterrupts(void);

        RadioState getState() { return state; };

        uint16_t getRSSI() { return bk4819.getRSSI(); };

        int16_t getRSSIdBm(void) {
            uint16_t rssi = bk4819.getRSSI();
            int16_t rssidbm = (int16_t)((rssi / 2) - 160);
            // TODO: RSSI gRxVfo->Band
            return rssidbm + dBmCorrTable[6];
        }

    private:
        System::SystemTask& systask;
        UART& uart;
        BK4819& bk4819;

        bool speakerOn = false; // speaker on/off
        RadioState state = RadioState::IDLE;

        static constexpr int8_t dBmCorrTable[7] = {
                -15, // band 1
                -25, // band 2
                -20, // band 3
                -4, // band 4
                -7, // band 5
                -6, // band 6
                 -1  // band 7
        };

        void toggleBK4819(bool on);

    };

} // namespace Radio