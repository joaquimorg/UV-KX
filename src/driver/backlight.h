// -----------------------------------------------------------------------------------------
// This file defines the Backlight class, a Hardware Abstraction Layer (HAL)
// for controlling the display backlight. It utilizes PWM for brightness adjustment
// and integrates with FreeRTOS timers for features like timed backlight deactivation
// or smooth fading effects.
// -----------------------------------------------------------------------------------------
#pragma once

#include "FreeRTOS.h" // Required for FreeRTOS timer functions (TimerHandle_t, xTimerCreateStatic, etc.)
#include "timers.h"   // FreeRTOS timer definitions
#include "pwmplus.h"  // PWM Plus peripheral register definitions
#include "portcon.h"  // Port controller for pin muxing

/**
 * @brief Provides an abstraction layer for controlling the display backlight.
 * This class manages backlight brightness using PWM and handles timed operations
 * like fading out the backlight using FreeRTOS timers.
 */
class Backlight {
public:
    /**
     * @brief Enumerates the possible states of the backlight.
     */
    enum class backLightState {
        OFF, ///< Backlight is completely off.
        ON   ///< Backlight is on (brightness level determined by `backlighLevel`).
    };

    /**
     * @brief Constructor for the Backlight class.
     * Initializes the PWM peripheral (PWM_PLUS0, Channel 0) for backlight control.
     * Configures the PWM frequency, period, pin muxing, and default states.
     * Also creates a FreeRTOS timer (`lightTimer`) for managing backlight fade-out.
     */
    Backlight() {
        // Configure PWM clock source and period for a target frequency (e.g., ~500Hz or 1kHz).
        // Calculation: PWM_CLKSRC prescaler = (SystemClock / Desired_PWM_Resolution / Target_PWM_Frequency)
        // Example: (48MHz / 1024 / 1000Hz) = 46.875. The prescaler is set in PWM_PLUS0_CLKSRC's upper bits.
        // The example uses a fixed prescaler value, resulting in a specific PWM frequency.
        // 48MHz / 94 / 1024 ~ 500Hz (Original comment)
        // Let's assume the prescaler part is (48000000 / 1024 / PWM_FREQUENCY_HZ)
        const uint32_t PWM_FREQUENCY_HZ = 1000; // Target PWM frequency of 1kHz
        PWM_PLUS0_CLKSRC |= ((48000000 / 1024 / PWM_FREQUENCY_HZ) << 16); // Set prescaler in upper 16 bits
        PWM_PLUS0_PERIOD = 1023; // Set PWM period (resolution of 1024 steps, 0-1023)

        // Configure Port B, Pin 6 (PB6) for PWM output (PWMP0_CH0).
        // Clear existing function selection for PB6.
        PORTCON_PORTB_SEL0 &= ~(0
            | PORTCON_PORTB_SEL0_B6_MASK // Mask for PB6 function selection
            );
        // Select PWMP0_CH0 as the function for PB6.
        PORTCON_PORTB_SEL0 |= 0
            | PORTCON_PORTB_SEL0_B6_BITS_PWMP0_CH0 // Assign PWM Plus 0 Channel 0 to PB6
            ;

        // Configure PWM_PLUS0 generator settings.
        PWM_PLUS0_GEN =
            PWMPLUS_GEN_CH0_OE_BITS_ENABLE |     // Enable output for Channel 0.
            PWMPLUS_GEN_CH0_OUTINV_BITS_ENABLE | // Invert the output of Channel 0 (common for LED control).
            0;

        // Configure PWM_PLUS0 general settings.
        PWM_PLUS0_CFG =
            PWMPLUS_CFG_CNT_REP_BITS_ENABLE |   // Enable repeat counter mode (continuous PWM).
            PWMPLUS_CFG_COUNTER_EN_BITS_ENABLE | // Enable the PWM counter.
            0;

        // Create a static FreeRTOS timer for backlight control (e.g., fade out).
        // Timer is named "light", period 70ms, not auto-reloading (pdFALSE),
        // passes `this` pointer as timer ID, and uses lightTimerCallback.
        lightTimer = xTimerCreateStatic("light", pdMS_TO_TICKS(70), pdFALSE, this, Backlight::lightTimerCallback, &lightTimerBuffer);
    }

    /**
     * @brief Sets the brightness level of the backlight.
     * @param level The desired brightness level. The interpretation of this value
     *              depends on the `setLevel` private method (likely 0-10 or similar range).
     */
    void setBrightness(uint8_t level) {
        setLevel(level);        // Apply the brightness level via PWM.
        backlighLevel = level;  // Store the current brightness level.
    }

    /**
     * @brief Sets the overall state of the backlight (ON or OFF).
     * If turning ON, it stops any active fade-out timer and sets brightness to `backlighLevel`.
     * If turning OFF, it initiates a fade-out sequence using `lightTimer`.
     * @param state The desired backlight state (backLightState::ON or backLightState::OFF).
     */
    void setBacklight(backLightState state) {
        if (this->state == state) return; // Do nothing if already in the desired state.
        this->state = state;

        if (state == backLightState::ON) {
            // If turning ON:
            if( xTimerIsTimerActive( lightTimer ) != pdFALSE ) {
                xTimerStop(lightTimer, 0); // Stop the fade-out timer if it's active.
            }
            setLevel(backlighLevel); // Set brightness to the stored level.
        }
        else {
            // If turning OFF:
            lightOffLevel = backlighLevel; // Start fade-out from the current brightness.
            xTimerStart(lightTimer, 0);    // Start the fade-out timer.
        }
    }

    /**
     * @brief Gets the current state of the backlight.
     * @return The current backLightState (ON or OFF).
     */
    backLightState getBacklightState() {
        return state;
    }

    /**
     * @brief Static callback function for the `lightTimer`.
     * This function is executed when the `lightTimer` expires. It implements a fade-out
     * effect by gradually decreasing `lightOffLevel` and updating the PWM duty cycle.
     * @param xTimer Handle to the timer that expired.
     */
    static void lightTimerCallback(TimerHandle_t xTimer) {
        // Retrieve the Backlight instance associated with this timer.
        Backlight* backlight = static_cast<Backlight*>(pvTimerGetTimerID(xTimer));
        if (backlight) {
            backlight->lightOffLevel--; // Decrement the fade-out level.
            backlight->setLevel(backlight->lightOffLevel); // Apply the new (dimmer) level.
            
            if (backlight->lightOffLevel == 0) {
                // If brightness is zero, fade-out is complete, do not restart timer.
                return;
            }
        }
        // If lightOffLevel > 0, restart the timer to continue fading.
        xTimerStart(xTimer, 0);
    }

private:
    /**
     * @brief Sets the raw PWM level for backlight brightness.
     * The input `level` is likely an abstract scale (e.g., 0-10).
     * This method translates it into a PWM compare value.
     * `(1 << level) - 1` suggests an exponential-like brightness curve,
     * or a way to map a small integer range to a wider PWM range.
     * For level 0, comp = 0. For level 1, comp = 1. For level 10, comp = 1023.
     * @param level The abstract brightness level.
     */
    void setLevel(uint8_t level) {
        // Set the PWM compare register. (1 << level) - 1 provides a non-linear mapping
        // from 'level' to the PWM duty cycle.
        // If level is 0, compare value is 0 (off).
        // If level is, e.g., 10 (assuming max level), compare is (1<<10)-1 = 1023 (max brightness for 1024 period).
        PWM_PLUS0_CH0_COMP = (uint32_t)(1 << level) - 1;
    }

    backLightState state = backLightState::OFF; ///< Current overall state of the backlight (ON/OFF).
    uint8_t backlighLevel = 10;                 ///< Desired brightness level when backlight is ON (e.g., 0-10 scale).

    TimerHandle_t lightTimer;                   ///< FreeRTOS timer handle for backlight effects.
    StaticTimer_t lightTimerBuffer;             ///< Static buffer for the lightTimer.

    int8_t lightOffLevel = 0;                   ///< Current level during the fade-out process.
};

