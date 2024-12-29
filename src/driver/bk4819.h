#pragma once

#include <array>

#include "sys.h"
#include "spi_sw_hal.h"
#include "bk4819-regs.h"

enum class ModType {
    MOD_FM,
    MOD_AM,
    MOD_LSB,
    MOD_USB,
    MOD_BYP,
    MOD_RAW,
    MOD_WFM,
    MOD_PRST
};

enum class BK4819_Filter_Bandwidth {
    BK4819_FILTER_BW_26k = 0, //  "W 26k",	//0
    BK4819_FILTER_BW_23k,     //  "W 23k",	//1
    BK4819_FILTER_BW_20k,     //  "W 20k",	//2
    BK4819_FILTER_BW_17k,     //  "W 17k",	//3
    BK4819_FILTER_BW_14k,     //  "W 14k",	//4
    BK4819_FILTER_BW_12k,     //  "W 12k",	//5
    BK4819_FILTER_BW_10k,     //  "N 10k",	//6
    BK4819_FILTER_BW_9k,      //  "N 9k",	//7
    BK4819_FILTER_BW_7k,      //  "U 7K",	//8
    BK4819_FILTER_BW_6k       //  "U 6K"	//9
};

// Enum class for BK4819 Audio Filter values
enum class BK4819_AF : uint16_t {
    MUTE = 0x0000, // BK4819_AF_MUTE
    FM = 0x0001, // BK4819_AF_FM
    ALAM = 0x0002, // BK4819_AF_ALAM (tone)
    BEEP = 0x0003, // BK4819_AF_BEEP (for tx)
    RAW = 0x0004, // BK4819_AF_RAW (SSB without IF filter)
    USB = 0x0005, // BK4819_AF_USB (LSB and USB at the same time)
    CTCO = 0x0006, // BK4819_AF_CTCO (CTCSS/DCS filter)
    AM = 0x0007, // BK4819_AF_AM
    FSKO = 0x0008, // BK4819_AF_FSKO (FSK output test)
    BYPASS = 0x0009  // BK4819_AF_BYPASS (FM without filter)
};

enum class SquelchType {
    SQUELCH_RSSI_NOISE_GLITCH,
    SQUELCH_RSSI_GLITCH,
    SQUELCH_RSSI_NOISE,
    SQUELCH_RSSI,
};

class SQObject {
public:
    static constexpr uint8_t SQ[2][6][11] = {
        {
            {0, 10, 62, 66, 74, 75, 92, 95, 98, 170, 252},
            {0, 5, 60, 64, 72, 70, 89, 92, 95, 166, 250},
            {255, 240, 56, 54, 48, 45, 32, 29, 20, 25, 20},
            {255, 250, 61, 58, 52, 48, 35, 32, 23, 30, 30},
            {255, 240, 135, 135, 116, 17, 3, 3, 2, 50, 50},
            {255, 250, 150, 140, 120, 20, 5, 5, 4, 45, 45},
        },
        {
            {0, 50, 78, 88, 94, 110, 114, 117, 119, 200, 252},
            {0, 40, 76, 86, 92, 106, 110, 113, 115, 195, 250},
            {255, 65, 49, 44, 42, 40, 33, 30, 22, 23, 22},
            {255, 70, 59, 54, 46, 45, 37, 34, 25, 27, 25},
            {255, 90, 135, 135, 116, 10, 8, 7, 6, 32, 32},
            {255, 100, 150, 140, 120, 15, 12, 11, 10, 30, 30},
        },
    };
};

class BK4819 {
public:

    BK4819() {
        initializeChip();
    }

    void initializeChip() {

        // Perform a soft reset
        softReset();

        // Clear any pending interrupts
        spi.writeRegister(BK4819_REG_02, 0x0000);
        spi.writeRegister(BK4819_REG_3F, 0x0000);

        // Disable unnecessary components
        spi.writeRegister(BK4819_REG_30,
            BK4819_REG_30_DISABLE_VCO_CALIB |
            BK4819_REG_30_DISABLE_RX_LINK |
            BK4819_REG_30_DISABLE_AF_DAC |
            BK4819_REG_30_DISABLE_DISC_MODE |
            BK4819_REG_30_DISABLE_PLL_VCO |
            BK4819_REG_30_DISABLE_PA_GAIN |
            BK4819_REG_30_DISABLE_MIC_ADC |
            BK4819_REG_30_DISABLE_TX_DSP |
            BK4819_REG_30_DISABLE_RX_DSP
        );

        /*  REG_37<14:12>   001 DSP voltage setting
            REG_37<11>      1   ANA LDO selection: 1: 2.7 V / 0: 2.4 V
            REG_37<10>      1   VCO LDO selection: 1: 2.7 V / 0: 2.4 V
            REG_37<9>       1   RF LDO selection: 1: 2.7 V / 0: 2.4 V
            REG_37<8>       1   PLL LDO selection: 1: 2.7 V / 0: 2.4 V
            REG_37<7>       0   ANA LDO Bypass: 1: Bypass / 0: Enable
            REG_37<6>       0   VCO LDO Bypass: 1: Bypass / 0: Enable
            REG_37<5>       0   RF LDO Bypass: 1: Bypass / 0: Enable
            REG_37<4>       0   PLL LDO Bypass: 1: Bypass / 0: Enable
            REG_37<3>       0   Reserved
            REG_37<2>       0   DSP enable: 1: Enable / 0: Disable
            REG_37<1>       0   XTAL enable: 1: Enable / 0: Disable
            REG_37<0>       0   Band Gap enable: 1: Enable / 0: Disable
        */
        spi.writeRegister(BK4819_REG_37, 0x1D0F); // 0001110100001111
        // PA
        spi.writeRegister(BK4819_REG_36, 0x0022);

        // Set GPIO state
        gpioOutState = 0x9000;
        spi.writeRegister(BK4819_REG_33, gpioOutState);
        spi.writeRegister(BK4819_REG_3F, 0);

        setAGC(true, 0);

        //Automatic MIC PGA Gain Controller
        spi.writeRegister(BK4819_REG_19, 0x1041);
        // MIC sensitivity
        spi.writeRegister(BK4819_REG_7D, 0xE94F);
        // AF
        spi.writeRegister(BK4819_REG_48, 0xB3A8);

        // DTMF_COEFFS ???

        // RF
        spi.writeRegister(BK4819_REG_1F, 0x5454);
        // Band selection threshold
        spi.writeRegister(BK4819_REG_3E, 0xA037);

        // Set GPIO5<1> to high - RED
        //toggleGpioOut(gPIO5_PIN1_RED, true);
    }

    // ------------------------------------------------------------------------

    void setupRegisters(void) {
        //GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
        toggleGreen(false);
        toggleRed(false);
        toggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, false);
        // setupPowerAmplifier(0, 0); // 0 is default

        // setFilterBandwidth(BK4819_FILTER_BW_WIDE);

        while (spi.readRegister(BK4819_REG_0C) & 1U) {
            spi.writeRegister(BK4819_REG_02, 0);
            delayMs(1);
        }
        spi.writeRegister(BK4819_REG_3F, 0);
        spi.writeRegister(BK4819_REG_7D, 0xE94F | 10); // mic
        // TX
        // spi.writeRegister(0x44, 38888);  // 300 resp TX
        spi.writeRegister(0x74, 0xAF1F); // 3k resp TX

        toggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, true);
        spi.writeRegister(
            BK4819_REG_48,
            (11u << 12) |    // ??? .. 0 ~ 15, doesn't seem to make any difference
            (1u << 10) | // AF Rx Gain-1
            (56 << 4) |  // AF Rx Gain-2
            (8 << 0));   // AF DAC Gain (after Gain-1 and Gain-2)

        // disableScramble(); // default is off
        // disableVox() // default is off;
        disableDTMF();

        // ToDo : Config interrupts
        uint16_t InterruptMask = BK4819_REG_3F_SQUELCH_FOUND | BK4819_REG_3F_SQUELCH_LOST;

        spi.writeRegister(BK4819_REG_3F, InterruptMask);

        spi.writeRegister(BK4819_REG_40, (spi.readRegister(BK4819_REG_40) & ~(0b11111111111)) |
            1000 | (1 << 12));
        // spi.writeRegister(BK4819_REG_40, (1 << 12) | (1450));
    }

    void setAGC(bool useDefault, uint8_t gainIndex) {
        const uint8_t GAIN_AUTO = 18;
        const bool enableAgc = gainIndex == GAIN_AUTO;
        uint16_t regVal = spi.readRegister(BK4819_REG_7E);

        spi.writeRegister(BK4819_REG_7E, (uint16_t)((regVal & ~(1 << 15) & ~(0b111 << 12)) |
            (!enableAgc << 15) // 0  AGC fix mode
            | (3u << 12))      // 3  AGC fix index
        );

        if (gainIndex == GAIN_AUTO) {
            spi.writeRegister(BK4819_REG_13, 0x03BE);
        }
        else {
            spi.writeRegister(BK4819_REG_13,
                gainTable[gainIndex].regValue | 6 | (3 << 3));
        }
        spi.writeRegister(BK4819_REG_12, 0x037B);
        spi.writeRegister(BK4819_REG_11, 0x027B);
        spi.writeRegister(BK4819_REG_10, 0x007A);

        uint8_t Lo = 0;    // 0-1 - auto, 2 - low, 3 high
        uint8_t low = 56;  // 1dB / LSB 56
        uint8_t high = 84; // 1dB / LSB 84

        if (useDefault) {
            spi.writeRegister(BK4819_REG_14, 0x0019);
        }
        else {
            spi.writeRegister(BK4819_REG_14, 0x0000);
            // slow 25 45
            // fast 15 50
            low = 20;
            high = 50;
        }
        spi.writeRegister(BK4819_REG_49, (Lo << 14) | (high << 7) | (low << 0));
        spi.writeRegister(BK4819_REG_7B, 0x8420);
    }

    void setFilterBandwidth(BK4819_Filter_Bandwidth bw) {

        uint8_t bandwidth = (uint8_t)bw;

        if (bandwidth > 9)
            return;

        // REG_43
        // <15>    0 ???
        //

        static const uint8_t rf[] = { 7, 5, 4, 3, 2, 1, 3, 1, 1, 0 };

        // <14:12> 4 RF filter bandwidth
        //         0 = 1.7  KHz
        //         1 = 2.0  KHz
        //         2 = 2.5  KHz
        //         3 = 3.0  KHz *W
        //         4 = 3.75 KHz *N
        //         5 = 4.0  KHz
        //         6 = 4.25 KHz
        //         7 = 4.5  KHz
        // if <5> == 1, RF filter bandwidth * 2

        static const uint8_t wb[] = { 6, 4, 3, 2, 2, 1, 2, 1, 0, 0 };

        // <11:9>  0 RF filter bandwidth when signal is weak
        //         0 = 1.7  KHz *WN
        //         1 = 2.0  KHz
        //         2 = 2.5  KHz
        //         3 = 3.0  KHz
        //         4 = 3.75 KHz
        //         5 = 4.0  KHz
        //         6 = 4.25 KHz
        //         7 = 4.5  KHz
        // if <5> == 1, RF filter bandwidth * 2

        static const uint8_t af[] = { 4, 5, 6, 7, 0, 0, 3, 0, 2, 1 };

        // <8:6>   1 AFTxLPF2 filter Band Width
        //         1 = 2.5  KHz (for 12.5k channel space) *N
        //         2 = 2.75 KHz
        //         0 = 3.0  KHz (for 25k   channel space) *W
        //         3 = 3.5  KHz
        //         4 = 4.5  KHz
        //         5 = 4.25 KHz
        //         6 = 4.0  KHz
        //         7 = 3.75 KHz

        static const uint8_t bs[] = { 2, 2, 2, 2, 2, 2, 0, 0, 1, 1 };

        // <5:4>   0 BW Mode Selection
        //         0 = 12.5k
        //         1 =  6.25k
        //         2 = 25k/20k
        //
        // <3>     1 ???
        //
        // <2>     0 Gain after FM Demodulation
        //         0 = 0dB
        //         1 = 6dB
        //
        // <1:0>   0 ???

        const uint16_t val =
            (0u << 15) |     //  0
            (rf[bandwidth] << 12) | // *3 RF filter bandwidth
            (wb[bandwidth] << 9) |  // *0 RF filter bandwidth when signal is weak
            (af[bandwidth] << 6) |  // *0 AFTxLPF2 filter Band Width
            (bs[bandwidth] << 4) |  //  2 BW Mode Selection 25K
            (1u << 3) |      //  1
            (0u << 2) |      //  0 Gain after FM Demodulation
            (0u << 0);       //  0

        spi.writeRegister(BK4819_REG_43, val);
    }

    void squelchType(SquelchType t) {
        setRegValue(RS_SQ_TYPE, squelchTypeValues[(uint8_t)t]);
    }

    void tuneTo(uint32_t frequency, bool precise) {
        selectFilter(frequency);
        setFrequency(frequency);
        uint16_t reg = spi.readRegister(BK4819_REG_30);
        if (precise) {
            spi.writeRegister(BK4819_REG_30, 0x0200);
        }
        else {
            spi.writeRegister(BK4819_REG_30, reg & ~BK4819_REG_30_ENABLE_VCO_CALIB);
        }
        spi.writeRegister(BK4819_REG_30, reg);
    }

    void rxTurnOn(void) {
        spi.writeRegister(BK4819_REG_37, 0x1F0F);
        spi.writeRegister(BK4819_REG_30, 0x0200);
        delayMs(10);
        spi.writeRegister(
            BK4819_REG_30,
            BK4819_REG_30_ENABLE_VCO_CALIB | BK4819_REG_30_DISABLE_UNKNOWN |
            BK4819_REG_30_ENABLE_RX_LINK | BK4819_REG_30_ENABLE_AF_DAC |
            BK4819_REG_30_ENABLE_DISC_MODE | BK4819_REG_30_ENABLE_PLL_VCO |
            BK4819_REG_30_DISABLE_PA_GAIN | BK4819_REG_30_DISABLE_MIC_ADC |
            BK4819_REG_30_DISABLE_TX_DSP | BK4819_REG_30_ENABLE_RX_DSP);
    }

    void setAF(BK4819_AF af) {
        spi.writeRegister(BK4819_REG_47, 0x6040 | ((int)af << 8));
    }

    void toggleAFBit(bool on) {
        uint16_t reg = spi.readRegister(BK4819_REG_47);
        reg &= ~(1 << 8);
        if (on)
            reg |= 1 << 8;
        spi.writeRegister(BK4819_REG_47, reg);
    }

    void toggleAFDAC(bool on) {
        uint16_t reg = spi.readRegister(BK4819_REG_30);
        reg &= ~BK4819_REG_30_ENABLE_AF_DAC;
        if (on)
            reg |= BK4819_REG_30_ENABLE_AF_DAC;
        spi.writeRegister(BK4819_REG_30, reg);
    }

    bool isSquelchOpen(void) {
        return (spi.readRegister(BK4819_REG_0C) >> 1) & 1;
    }

    // TODO: fix
    void squelch(uint8_t sql, __attribute__((unused))uint32_t f, uint8_t OpenDelay,
        uint8_t CloseDelay) {
        // TODO: fix
        const uint8_t band = 0;//f > SETTINGS_GetFilterBound() ? 1 : 0;
        setupSquelch(SQObject::SQ[band][0][sql], SQObject::SQ[band][1][sql], SQObject::SQ[band][2][sql],
            SQObject::SQ[band][3][sql], SQObject::SQ[band][4][sql], SQObject::SQ[band][5][sql],
            OpenDelay, CloseDelay);
    }

    void setIdle(void) {
        spi.writeRegister(BK4819_REG_30, 0x0000);
    }

    void setToneRegister(uint16_t toneConfig) {
        spi.writeRegister(BK4819_REG_71, toneConfig);
    }

    void setToneFrequency(uint16_t f) {
        setToneRegister(scaleFreq(f));
    }

    void setTone2Frequency(uint16_t f) {
        spi.writeRegister(BK4819_REG_72, scaleFreq(f));
    }

    void enterTxMute(void) {
        spi.writeRegister(BK4819_REG_50, 0xBB20);
    }

    void exitTxMute(void) {
        spi.writeRegister(BK4819_REG_50, 0x3B20);
    }

    uint16_t getToneRegister(void) {
        return spi.readRegister(BK4819_REG_71);
    }


    void playTone(uint16_t frequency, bool bTuningGainSwitch) {
        enterTxMute();
        setAF(BK4819_AF::BEEP);

        uint8_t gain = bTuningGainSwitch ? 28 : 96;

        uint16_t toneCfg = BK4819_REG_70_ENABLE_TONE1 |
            (gain << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN);
        spi.writeRegister(BK4819_REG_70, toneCfg);

        setIdle();
        spi.writeRegister(BK4819_REG_30, 0 | BK4819_REG_30_ENABLE_AF_DAC |
            BK4819_REG_30_ENABLE_DISC_MODE |
            BK4819_REG_30_ENABLE_TX_DSP);

        setToneFrequency(frequency);
    }

    void turnsOffTonesTurnsOnRX(void) {
        spi.writeRegister(BK4819_REG_70, 0);
        setAF(BK4819_AF::MUTE);
        exitTxMute();
        setIdle();
        spi.writeRegister(
            BK4819_REG_30,
            0 | BK4819_REG_30_ENABLE_VCO_CALIB | BK4819_REG_30_ENABLE_RX_LINK |
            BK4819_REG_30_ENABLE_AF_DAC | BK4819_REG_30_ENABLE_DISC_MODE |
            BK4819_REG_30_ENABLE_PLL_VCO | BK4819_REG_30_ENABLE_RX_DSP);
    }

    void setModulation(ModType type) {
        bool isSsb = type == ModType::MOD_LSB || type == ModType::MOD_USB;
        bool isFm = type == ModType::MOD_FM || type == ModType::MOD_WFM;
        setAF((BK4819_AF)modTypeRegValues[(uint8_t)type]);
        setRegValue(afDacGainRegSpec, 0x8);
        spi.writeRegister(0x3D, isSsb ? 0 : 0x2AAB);
        setRegValue(afcDisableRegSpec, !isFm);
        if (type == ModType::MOD_WFM) {
            setRegValue(RS_XTAL_MODE, 0);
            setRegValue(RS_IF_F, 14223);
            setRegValue(RS_RF_FILT_BW, 7);
            setRegValue(RS_RF_FILT_BW_WEAK, 7);
            setRegValue(RS_BW_MODE, 3);
        }
        else {
            setRegValue(RS_XTAL_MODE, 2);
            setRegValue(RS_IF_F, 10923);
        }
    }

    void resetRSSI(void) {
        uint16_t reg = spi.readRegister(BK4819_REG_30);
        reg &= ~1;
        spi.writeRegister(BK4819_REG_30, reg);
        reg |= 1;
        spi.writeRegister(BK4819_REG_30, reg);
    }

    uint16_t getRSSI(void) {
        return spi.readRegister(BK4819_REG_67) & 0x1FF;
    }

    uint8_t getNoise(void) {
        return (uint8_t)spi.readRegister(BK4819_REG_65) & 0xFF;
    }

    uint8_t getRSSIRelative(void) {
        return (uint8_t)(spi.readRegister(BK4819_REG_65) >> 8) & 0xFF;
    }

    uint8_t getGlitch(void) {
        return (uint8_t)spi.readRegister(BK4819_REG_63) & 0xFF;
    }

    uint8_t getSNR(void) {
        return (uint8_t)spi.readRegister(BK4819_REG_61) & 0xFF;
    }

    uint16_t getVoiceAmplitude(void) {
        return spi.readRegister(BK4819_REG_64);
    }


    void disableVox(void) {
        uint16_t v = spi.readRegister(BK4819_REG_31);
        spi.writeRegister(BK4819_REG_31, v & 0xFFFB);
    }

    void disableDTMF(void) {
        spi.writeRegister(BK4819_REG_24, 0);
    }

    uint16_t getInterruptRequest(void) {
        return spi.readRegister(BK4819_REG_0C);
    }

    void clearInterrupt(void) {
        spi.writeRegister(BK4819_REG_02, 0);
    }

    uint16_t readInterrupt(void) {
        return spi.readRegister(BK4819_REG_02);
    }

    void toggleGreen(bool on) {
        toggleGpioOut(BK4819_GPIO6_PIN2_GREEN, on);
    }

    void toggleRed(bool on) {
        toggleGpioOut(BK4819_GPIO5_PIN1_RED, on);
    }

private:

    static constexpr uint32_t frequencyMIN = 1600000;
    static constexpr uint32_t frequencyMAX = 134000000;

    static constexpr uint32_t VHF_UHF_BOUND1 = 24000000;
    static constexpr uint32_t VHF_UHF_BOUND2 = 28000000;

    // Structure to hold gain data
    struct Gain {
        uint16_t regValue;
        int8_t gainDb;
    };

    SPISoftwareInterface spi;
    uint16_t gpioOutState;

    static constexpr std::array<Gain, 19> gainTable = {
        Gain{0x000, -43},
        Gain{0x100, -40},
        Gain{0x020, -38},
        Gain{0x200, -35},
        Gain{0x040, -33},
        Gain{0x220, -30},
        Gain{0x060, -28},
        Gain{0x240, -25},
        Gain{0x0A0, -23},
        Gain{0x260, -20},
        Gain{0x1C0, -18},
        Gain{0x2A0, -15},
        Gain{0x2C0, -13},
        Gain{0x2E0, -11},
        Gain{0x360, -9},
        Gain{0x380, -6},
        Gain{0x3A0, -4},
        Gain{0x3C0, -2},
        Gain{0x3E0, 0}
    };

    static constexpr uint8_t squelchTypeValues[4] = { 0x88, 0xAA, 0xCC, 0xFF };

    static constexpr uint16_t modTypeRegValues[8] = {
        static_cast<uint16_t>(BK4819_AF::FM),      // [MOD_FM]
        static_cast<uint16_t>(BK4819_AF::AM),      // [MOD_AM]
        static_cast<uint16_t>(BK4819_AF::USB),     // [MOD_LSB]
        static_cast<uint16_t>(BK4819_AF::USB),     // [MOD_USB]
        static_cast<uint16_t>(BK4819_AF::BYPASS),  // [MOD_BYP]
        static_cast<uint16_t>(BK4819_AF::RAW),     // [MOD_RAW]
        static_cast<uint16_t>(BK4819_AF::FM),      // [MOD_WFM]
        static_cast<uint16_t>(BK4819_AF::RAW)      // [MOD_PRST]
    };


    // ------------------------------------------------------------------------

    // Internal methods

    // Method to perform a soft reset
    void softReset() {
        // Set REG_00<15> to 1 for soft reset
        spi.writeRegister(BK4819_REG_00, 0x8000);
        // Set back to normal mode
        spi.writeRegister(BK4819_REG_00, 0x0000);
    }

    // Method to toggle a GPIO output
    void toggleGpioOut(BK4819_GPIO_PIN_t pin, bool bSet) {

        if (bSet)
            gpioOutState |= (uint16_t)(0x40u >> pin);
        else
            gpioOutState &= (uint16_t)~(0x40u >> pin);

        spi.writeRegister(BK4819_REG_33, gpioOutState);
    }

    void setFrequency(uint32_t frequency) {
        spi.writeRegister(BK4819_REG_38, frequency & 0xFFFF);
        spi.writeRegister(BK4819_REG_39, (uint16_t)((frequency >> 16) & 0xFFFF));
    }

    void selectFilter(uint32_t frequency) {
        if (frequency < VHF_UHF_BOUND2)
        {	// VHF
            toggleGpioOut(BK4819_GPIO4_PIN32_VHF_LNA, true);
            toggleGpioOut(BK4819_GPIO3_PIN31_UHF_LNA, false);
        }
        else
            if (frequency == 0xFFFFFFFF)
            {	// OFF
                toggleGpioOut(BK4819_GPIO4_PIN32_VHF_LNA, false);
                toggleGpioOut(BK4819_GPIO3_PIN31_UHF_LNA, false);
            }
            else
            {	// UHF
                toggleGpioOut(BK4819_GPIO4_PIN32_VHF_LNA, false);
                toggleGpioOut(BK4819_GPIO3_PIN31_UHF_LNA, true);
            }
    }

    void setupSquelch(uint8_t ro, uint8_t rc, uint8_t no, uint8_t nc,
        uint8_t gc, uint8_t go, uint8_t delayO,
        uint8_t delayC) {
        spi.writeRegister(BK4819_REG_4D, 0xA000 | gc);
        spi.writeRegister(
            BK4819_REG_4E,
            (1u << 14) |                   //  1 ???
            (uint16_t)(delayO << 11) | // *5  squelch = open  delay .. 0 ~ 7
            (uint16_t)(delayC << 9) |  // *3  squelch = close delay .. 0 ~ 3
            go);
        spi.writeRegister(BK4819_REG_4F, (nc << 8) | no);
        spi.writeRegister(BK4819_REG_78, (ro << 8) | rc);
    }

    uint16_t getRegValue(RegisterSpec s) {
        return (spi.readRegister(s.num) >> s.offset) & s.mask;
    }

    void setRegValue(RegisterSpec s, uint16_t v) {
        uint16_t reg = spi.readRegister(s.num);
        reg &= (uint16_t)~(s.mask << s.offset);
        spi.writeRegister(s.num, reg | (v << s.offset));
    }

    uint16_t scaleFreq(const uint16_t freq) {
        return (uint16_t)((((uint32_t)freq * 1353245u) + (1u << 16)) >> 17); // with rounding
    }

};
