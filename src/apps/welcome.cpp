// This file implements the Welcome application class, which is responsible for
// displaying the initial welcome screen of the radio firmware.
// It handles drawing the screen content, including firmware information,
// battery status, and potentially other checks, and transitions to the
// main application upon user input or timeout.

#include "printf.h"

#include "apps.h"
#include "welcome.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

/**
 * @brief Draws the welcome screen.
 * This function clears the display and renders various pieces of information,
 * including a greeting, firmware name, battery status (icon and text),
 * and indicators for certain hardware components/features (SI4732, EEPROM, FM).
 * It also displays the firmware author and version in a footer.
 */
void Welcome::drawScreen(void) {
    ui.clearDisplay(); // Clear the display buffer

    ui.lcd()->setColorIndex(BLACK); // Set default drawing color

    // Display greeting and firmware name
    ui.setFont(Font::FONT_8B_TR);
    ui.lcd()->drawStr(5, 10, "Hello !");
    ui.lcd()->drawStr(5, 20, "UV-Kx Open Firmware");

    // Display battery information
    ui.drawBattery(systask.getBattery().getBatteryPercentage(), 20, 30); // Battery icon
    ui.setFont(Font::FONT_8_TR);
    // Battery percentage and voltage text
    ui.drawStrf(8, 42, "%i%% %u.%02uV", systask.getBattery().getBatteryPercentage(), systask.getBattery().getBatteryVoltageAverage() / 100, systask.getBattery().getBatteryVoltageAverage() % 100);

    // Display status of various components/features (placeholders or actual checks)
    // These appear to be labels on the left and their status/value on the right.
    ui.lcd()->drawStr(64, 33, "SI4732"); // Label for SI4732 chip
    ui.lcd()->drawStr(64, 42, "EEPROM"); // Label for EEPROM
    ui.lcd()->drawStr(64, 51, "FM");     // Label for FM radio capability/module

    ui.setFont(Font::FONT_5_TR); // Smaller font for status values
    ui.lcd()->drawStr(110, 33, "NO");   // Status for SI4732 (e.g., detected: YES/NO)
    ui.lcd()->drawStr(110, 42, "64");   // Status for EEPROM (e.g., size or type: 64KB)
    ui.lcd()->drawStr(110, 51, "YES");  // Status for FM (e.g., enabled: YES/NO)

    // Footer: Author and Version string
    ui.setFont(Font::FONT_5_TR);
    // ui.drawString(TextAlign::CENTER, 0, 128, 55, true, false, false, "Any key to continue..."); // Optional "press any key" message
    ui.lcd()->drawBox(0, 57, 128, 7); // Footer background bar
    ui.drawString(TextAlign::CENTER, 0, 128, 63, false, false, false, AUTHOR_STRING " - " VERSION_STRING);

    ui.updateDisplay(); // Send the buffer to the physical display
}

/**
 * @brief Initializes the Welcome application.
 * This function is called when the Welcome application is loaded.
 * Currently, it does not perform any specific initialization actions.
 * The welcome screen's content is static or derived from system calls within drawScreen.
 */
void Welcome::init(void) {    
    // No specific initialization needed for the Welcome screen at this time.
    // If, for example, a specific image needed to be loaded or a timer started
    // for automatic dismissal, it would go here.
}

/**
 * @brief Updates the Welcome application state.
 * This function is called periodically by the system task.
 * For the Welcome application, it simply redraws the screen. This ensures that
 * dynamic information like battery voltage is kept up-to-date if the screen
 * is displayed for an extended period.
 */
void Welcome::update(void) {
    drawScreen(); // Redraw the screen on every update cycle
}

/**
 * @brief Handles timeout events for the Welcome application.
 * If a timeout occurs (e.g., the welcome screen has been displayed for a set duration),
 * this function loads the MainVFO application, transitioning the user to the main
 * operational screen of the radio.
 */
void Welcome::timeout(void) {
    // On timeout, automatically transition to the main VFO application
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
};

/**
 * @brief Handles keyboard input for the Welcome application.
 * @param keyCode The code of the key that was pressed (unused in this function).
 * @param keyState The state of the key (e.g., KEY_PRESSED).
 *
 * If any key is pressed while the welcome screen is active, this function
 * loads the MainVFO application, effectively dismissing the welcome screen and
 * proceeding to the main operational screen of the radio.
 */
void Welcome::action(__attribute__((unused)) Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {
    // If any key is pressed, dismiss the welcome screen and load the MainVFO application
    if (keyState == Keyboard::KeyState::KEY_PRESSED) {
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
    }
}
