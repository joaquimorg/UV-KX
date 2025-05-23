// This file implements the SetRadio application class, which is responsible for
// providing an interface to configure general radio settings.
// It handles drawing the settings menu, processing user input for navigation
// and selection, and potentially launching other applications or sub-menus
// for specific settings.

#include "printf.h"
#include "apps.h"
#include "set_vfo.h" // Included for potential navigation, though set_radio.h might be more direct
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

/**
 * @brief Draws the radio settings screen.
 * This function clears the display, draws the title bar ("RADIO") with the
 * current item number and total items, and then draws the scrollable list of radio settings.
 */
void SetRadio::drawScreen(void) {
    ui.clearDisplay(); // Clear the display buffer

    ui.setBlackColor(); // Set drawing color for the title bar

    // Draw the title bar background
    ui.lcd()->drawBox(0, 0, 128, 7); 
    
    // Draw the "RADIO" title and item count
    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, "RADIO");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor(); // Ensure color is set for the menu list items

    // Draw the radio settings list items starting at Y position 15
    menulist.draw(15);

    ui.updateDisplay(); // Send the buffer to the physical display
}

/**
 * @brief Initializes the SetRadio application.
 * This function is called when the SetRadio application is loaded.
 * It populates the `menulist` with the available radio settings.
 * Each setting is separated by a newline character.
 */
void SetRadio::init(void) {
    // Define the radio settings menu items
    menulist.set(0, 6, 127, "MIC DB\nBATT SAVE\nBUSY LOCKOUT\nBCKLIGHT LEVEL\nBCKLIGHT TIME\nBCKLIGHT MODE\nLCD CONTRAST\nTX TOT\nBEEP\nRESET");
}

/**
 * @brief Updates the SetRadio application state.
 * This function is called periodically by the system task.
 * For the SetRadio application, it simply redraws the screen.
 */
void SetRadio::update(void) {
    drawScreen(); // Redraw the screen on every update cycle
}

/**
 * @brief Handles timeout events for the SetRadio application.
 * If a timeout occurs while in the radio settings menu, it loads the MainVFO application.
 */
void SetRadio::timeout(void) {
    // On timeout, exit the radio settings menu and return to the main VFO screen
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
};

/**
 * @brief Handles keyboard input for the SetRadio application.
 * This function processes key presses for navigating the settings list,
 * selecting a setting (MENU key), or exiting the menu (EXIT key).
 * @param keyCode The code of the key that was pressed or released.
 * @param keyState The state of the key (e.g., KEY_PRESSED, KEY_LONG_PRESSED_CONT).
 */
void SetRadio::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {    
    // Process key presses or continuous long presses
    if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
        if (keyCode == Keyboard::KeyCode::KEY_UP) {
            menulist.prev(); // Navigate to the previous setting
        } else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
            menulist.next(); // Navigate to the next setting
        } else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
            // Exit the radio settings menu and return to the main VFO screen
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
        }

        // Handle selection of a setting item
        if (keyCode == Keyboard::KeyCode::KEY_MENU) {
            switch (menulist.getListPos()) {
                // TODO: Implement actions for each radio setting.
                // This will likely involve:
                // 1. Storing the current setting's index or a unique ID.
                // 2. Loading a sub-screen or a value editor for the selected setting.
                // 3. For settings with predefined options (e.g., BATT SAVE), it might involve
                //    cycling through options directly or using a popup.
                // 4. For numerical values (e.g., MIC DB, LCD CONTRAST), it might involve
                //    incrementing/decrementing values or direct number input.
                
                case 0: // MIC DB
                    // Example: Load a value editor for MIC DB
                    break;
                case 1: // BATT SAVE
                    // Example: Cycle through BATT SAVE options or load a selection list
                    break;
                case 2: // BUSY LOCKOUT
                    break;
                case 3: // BCKLIGHT LEVEL
                    break;
                case 4: // BCKLIGHT TIME
                    break;
                case 5: // BCKLIGHT MODE
                    break;
                case 6: // LCD CONTRAST
                    break;
                case 7: // TX TOT (Transmit Time-Out Timer)
                    break;
                case 8: // BEEP
                    break;
                case 9: // RESET
                    // Load the EEPROM reset application
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::RESETEEPROM);
                    break;

                default:
                    // Should not happen if menu items and cases are synchronized
                    break;
            }
        } 
    }
}
