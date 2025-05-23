// This file implements the ResetInit application class, which is responsible for
// handling the device initialization and EEPROM reset functionalities.
// It manages the user interface for confirming the action, displays progress,
// and interacts with the Settings module to perform the actual EEPROM operations.

#include "printf.h"

#include "apps.h"
#include "reset_init.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

// Decompressed warning text (currently commented out in drawScreen, but kept for reference)
//const uint8_t text1[] = { 0x4C, 0x71, 0x1A, 0x10, 0x43, 0xD1, 0x38, 0xC6, 0x82, 0x38, 0xD4, 0xC4, 0x35, 0x36, 0x88, 0x49, 0xA2, 0x0D, 0x08, 0xE3, 0x0F, 0x01, 0x32, 0x01, 0x2C, 0x46, 0xDA, 0x4C, 0xE6, 0x94, 0x48, 0x46, 0x80, 0x2C, 0xB6, 0x85, 0x10, 0x04, 0xD4, 0x44, 0x44, 0x9C, 0x68, 0x84, 0xDA, 0x31, 0x44, 0x93, 0x68, 0x11, 0x1A, 0x20, 0xD2, 0x13, 0x20, 0x02, 0xC8, 0x64, 0x40, 0xDB, 0x69, 0x31, 0xC8, 0x49, 0xA0, 0x02, 0x4C, 0x83, 0x8D, 0x69, 0x62, 0x0B, 0x2D, 0xA1, 0x11, 0x01, 0x21, 0x1A, 0x00, 0xB2, 0xDA, 0x09, 0x44, 0x51, 0x10, 0xD4, 0xDA, 0x0C, 0x04, 0xC0, 0x6D, 0xA3, 0x00, 0x28, 0x46, 0x80, 0x68, 0x10, 0x02, 0x29, 0x43, 0xDA, 0x04, 0x41, 0x4E, 0x44, 0x46, 0x82, 0x38, 0xD4, 0xC8, 0x35, 0x42, 0x0D, 0x19, 0xB0 } /* ("THE EEPROM CONTENT IS INCOMPATIBLE. TO USE ALL FEATURES, IT MUST BE INITIALIZED. THIS ACTION WILL ERASE ALL CURRENT DATA. MAKE A BACKUP BEFORE CONTINUING.") */;

/**
 * @brief Draws the screen for the ResetInit application.
 * Depending on the state, it shows a warning message, a confirmation question,
 * or a progress bar during EEPROM initialization.
 * It also displays the firmware author and version at the bottom.
 */
void ResetInit::drawScreen(void) {
    ui.clearDisplay(); // Clear the display buffer

    ui.lcd()->setColorIndex(BLACK); // Set drawing color

    ui.setFont(Font::FONT_8B_TR); // Set font for titles

    if (isToInitialize) {
        // --- Displaying EEPROM Initialization Progress ---
        ui.drawString(TextAlign::CENTER, 0, 128, 8, true, false, false, "EEPROM INITIALIZATION");
        // Calculate and draw the progress bar
        u8g2_uint_t barWidth = (u8g2_uint_t)((initProgress * 120) / 100); // Scale progress to bar width
        ui.lcd()->drawFrame(4, 20, 120, 10); // Draw progress bar frame
        ui.lcd()->drawBox(4, 20, barWidth, 10); // Draw filled part of progress bar
        ui.drawStringf(TextAlign::CENTER, 0, 128, 46, true, false, false, "%d%%", initProgress); // Display progress percentage
    }
    else {
        // --- Displaying Warning and Confirmation Question ---
        ui.drawString(TextAlign::CENTER, 0, 128, 8, true, false, false, "WARNING !");

        ui.setFont(Font::FONT_5_TR); // Smaller font for detailed text
        // Warning message about incompatible EEPROM and data erasure
        ui.drawWords(0, 16, "THE EEPROM CONTENT IS INCOMPATIBLE. TO USE ALL FEATURES, IT MUST BE INITIALIZED. THIS ACTION WILL ERASE ALL CURRENT DATA.");

        ui.setFont(Font::FONT_8B_TR); // Larger font for emphasis
        ui.drawWords(0, 46, "MAKE A BACKUP BEFORE CONTINUING...");
        // ui.drawWords(0, 16, ui.decompressText(text1, sizeof(text1))); // Option to use decompressed text

        // If showQuestion is true, display the confirmation popup
        if (showQuestion) {
            ui.drawPopupWindow(15, 20, 96, 32, "Init. EEPROM ?"); // Popup title
            ui.setFont(Font::FONT_8_TR); // Font for instructions within popup
            ui.drawString(TextAlign::CENTER, 17, 111, 36, true, false, false, "Press 1 to accept.");
            // Instruction for cancelling depends on whether it's full init or just EEPROM reset
            ui.drawStringf(TextAlign::CENTER, 17, 111, 46, true, false, false, "%s to cancel.", isInit ? "Other key" : "EXIT");            
        }
    }

    // --- Footer with Author and Version ---
    ui.setFont(Font::FONT_5_TR);
    ui.lcd()->drawBox(0, 57, 128, 7); // Footer background
    ui.drawString(TextAlign::CENTER, 0, 128, 63, false, false, false, AUTHOR_STRING " - " VERSION_STRING);

    ui.updateDisplay(); // Send buffer to the physical display
}

/**
 * @brief Initializes the ResetInit application.
 * This function is called when the ResetInit application is loaded.
 * Currently, it does not perform any specific initialization actions beyond what the constructor does.
 */
void ResetInit::init(void) {
    // No specific initialization required for ResetInit at the moment,
    // initial state is set by constructor and first update/draw.
}

/**
 * @brief Updates the ResetInit application state.
 * This function is called periodically. If initialization is in progress (`isToInitialize` is true),
 * it calls `settings.initEEPROM()` to advance the initialization process until it reaches 100%.
 * After updating the state, it redraws the screen.
 */
void ResetInit::update(void) {
    if (isToInitialize) {
        if (initProgress < 100) {            
            // Perform a step of EEPROM initialization and get the current progress
            initProgress = settings.initEEPROM(); 
        }
        else {
            // Initialization is complete
            isReady = true; 
        }
    }
    drawScreen(); // Redraw the screen to reflect current state or progress
}

/**
 * @brief Handles timeout events for the ResetInit application.
 * If the initialization process was started and is now ready (complete),
 * this function loads the Welcome application.
 * Otherwise, it does nothing on timeout if not in `isToInitialize` and `isReady` state.
 */
void ResetInit::timeout(void) {
    if (isToInitialize) {
        if (isReady) {
            // If initialization is complete, proceed to the Welcome screen
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Welcome);
        }
    }
    // If not initializing or not ready, timeout might lead to exiting or staying (current behavior implies staying or handled by SystemTask)
};

/**
 * @brief Handles keyboard input for the ResetInit application.
 * @param keyCode The code of the key that was pressed.
 * @param keyState The state of the key (e.g., KEY_PRESSED).
 *
 * If not yet in initialization mode (`!isToInitialize`):
 *   - If the confirmation question is shown (`showQuestion` is true):
 *     - KEY_1: Confirms initialization, sets `isToInitialize` to true.
 *     - KEY_EXIT: Cancels (if `!isInit` mode, loads MainVFO).
 *     - Hides the question popup.
 *   - If the confirmation question is not shown, pressing any key will show it.
 * Input is ignored if initialization is already in progress (`isToInitialize` is true).
 */
void ResetInit::action(__attribute__((unused)) Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {
    if (keyState == Keyboard::KeyState::KEY_PRESSED) {
        // Handle actions only if EEPROM initialization process has not started
        if (!isToInitialize) {
            if (showQuestion) {
                // User is responding to the "Init. EEPROM ?" question
                if (keyCode == Keyboard::KeyCode::KEY_1) {
                    // User pressed '1' to accept initialization
                    initProgress = 0;      // Reset progress
                    isToInitialize = true; // Start the initialization process in the update() loop
                } else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                    // User pressed 'EXIT' to cancel
                    if (!isInit) { 
                        // If this was an EEPROM reset triggered from menu (not initial mandatory init)
                        // then exit back to the main VFO screen.
                        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
                    }
                    // If isInit is true (mandatory init on first boot with bad EEPROM),
                    // pressing EXIT here might be intended to do nothing or handled by a global timeout.
                }
                showQuestion = false; // Hide the confirmation question popup
            }
            else {
                // No question shown yet, so display it on first key press in this app
                showQuestion = true;                
            }
        }
        // If isToInitialize is true, key presses are generally ignored during the progress bar display.
        // Timeout or completion in update() will handle moving to the next state.
    }
}
