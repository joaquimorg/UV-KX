#pragma once

#include "FreeRTOS.h"
#include "timers.h"
#include "pwmplus.h"
#include "portcon.h"

// Backlight Hardware Abstraction Layer
class Backlight {
public:

    enum class backLightState {
        OFF,
        ON
    };

    Backlight() {
        // 48MHz / 94 / 1024 ~ 500Hz
        const uint32_t PWM_FREQUENCY_HZ = 1000;
        PWM_PLUS0_CLKSRC |= ((48000000 / 1024 / PWM_FREQUENCY_HZ) << 16);
        PWM_PLUS0_PERIOD = 1023;

        PORTCON_PORTB_SEL0 &= ~(0
            // Back light
            | PORTCON_PORTB_SEL0_B6_MASK
            );
        PORTCON_PORTB_SEL0 |= 0
            // Back light PWM
            | PORTCON_PORTB_SEL0_B6_BITS_PWMP0_CH0
            ;

        PWM_PLUS0_GEN =
            PWMPLUS_GEN_CH0_OE_BITS_ENABLE |
            PWMPLUS_GEN_CH0_OUTINV_BITS_ENABLE |
            0;

        PWM_PLUS0_CFG =
            PWMPLUS_CFG_CNT_REP_BITS_ENABLE |
            PWMPLUS_CFG_COUNTER_EN_BITS_ENABLE |
            0;

        lightTimer = xTimerCreateStatic("light", pdMS_TO_TICKS(70), pdFALSE, this, Backlight::lightTimerCallback, &lightTimerBuffer);

    }

    void setBrightness(uint8_t level) {
        setLevel(level);
        backlighLevel = level;
    }

    void setBacklight(backLightState state) {
        if (this->state == state) return;
        this->state = state;
        if (state == backLightState::ON) {
            if( xTimerIsTimerActive( lightTimer ) != pdFALSE ) {
                xTimerStop(lightTimer, 0);
            }
            setLevel(backlighLevel);
        }
        else {
            lightOffLevel = backlighLevel;
            xTimerStart(lightTimer, 0);
        }
    }

    backLightState getBacklightState() {
        return state;
    }

    static void lightTimerCallback(TimerHandle_t xTimer) {
        Backlight* backlight = static_cast<Backlight*>(pvTimerGetTimerID(xTimer));
        if (backlight) {
            backlight->lightOffLevel--;
            backlight->setLevel(backlight->lightOffLevel);
            //backlight->setLevel(1);
            if (backlight->lightOffLevel == 0) {
                return;
            }
        }
        xTimerStart(xTimer, 0);
    }

private:

    backLightState state = backLightState::OFF;
    uint8_t backlighLevel = 10;

    TimerHandle_t lightTimer;
    StaticTimer_t lightTimerBuffer;

    int8_t lightOffLevel = 0;

    void setLevel(uint8_t level) {
        PWM_PLUS0_CH0_COMP = (uint32_t)(1 << level) - 1;
    }
};

