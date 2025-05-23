// This file implements the Menu application class, which is responsible for
// displaying and managing the main menu of the radio firmware.
// It handles drawing the menu, processing user input for navigation and selection,
// and launching other applications based on user choices.

#include "printf.h"
#include "apps.h"
#include "menu.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

/**
 * @brief Draws the menu screen.
 * This function clears the display, draws the menu title bar with the current
 * item number and total items, and then draws the scrollable list of menu items.
 */
void Menu::drawScreen(void) {
    ui.clearDisplay(); // Clear the display buffer

    ui.setBlackColor(); // Set drawing color to black for the title bar

    // Draw the title bar background
    ui.lcd()->drawBox(0, 0, 128, 7); 
    
    // Draw the "MENU" title and item count
    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, "MENU");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor(); // Ensure color is set for the menu list items

    // Draw the menu list items starting at Y position 15
    menulist.draw(15);

    ui.updateDisplay(); // Send the buffer to the physical display
}

/**
 * @brief Initializes the Menu application.
 * This function is called when the Menu application is loaded.
 * It sets up the menu items that will be displayed in the `menulist`.
 * The items are provided as a newline-separated string.
 */
void Menu::init(void) {
    // Define the menu items. Each item is separated by a newline character.
    menulist.set(0, 6, 127, "VFO A SETTINGS\nVFO B SETTINGS\nRADIO SETTINGS\nMESSENGER\nSCANNER\nABOUT");
    // drawScreen(); // Initial draw can be handled by the first update() call
}

/**
 * @brief Updates the Menu application state.
 * This function is called periodically by the system task.
 * For the Menu application, it simply redraws the screen to reflect any changes.
 */
void Menu::update(void) {
    drawScreen(); // Redraw the screen on every update cycle
}

/**
 * @brief Handles timeout events for the Menu application.
 * If a timeout occurs while in the menu, it loads the MainVFO application.
 */
void Menu::timeout(void) {
    // On timeout, exit the menu and return to the main VFO screen
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
}

/**
 * @brief Handles keyboard input for the Menu application.
 * This function processes key presses for navigating up and down the menu,
 * selecting a menu item (MENU key), or exiting the menu (EXIT key).
 * @param keyCode The code of the key that was pressed or released.
 * @param keyState The state of the key (e.g., KEY_PRESSED, KEY_LONG_PRESSED_CONT).
 */
void Menu::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {    
    // Process key presses or continuous long presses
    if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
        if (keyCode == Keyboard::KeyCode::KEY_UP) {
             menulist.prev(); // Navigate to the previous menu item
        } else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
            menulist.next(); // Navigate to the next menu item
        } else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
            // Exit the menu and return to the main VFO screen
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
        }

        // Handle selection of a menu item
        if (keyCode == Keyboard::KeyCode::KEY_MENU) {
            // Load the corresponding application based on the selected menu item's position
            switch (menulist.getListPos()) {
                case 0: // VFO A SETTINGS
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::SETVFOA);
                    break;
                case 1: // VFO B SETTINGS
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::SETVFOB);
                    break;
                case 2: // RADIO SETTINGS
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::SETRADIO);
                    break;
                case 3: // MESSENGER
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MESSENGER);
                    break;
                case 4: // SCANNER
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::SCANNER);
                    break;
                case 5: // ABOUT
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::ABOUT);
                    break;
                default:
                    // Should not happen if menu items and cases are synchronized
                    break;
            }
        } 
    }
}
