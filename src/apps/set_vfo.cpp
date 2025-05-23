// This file implements the SetVFO application class, which is responsible for
// providing an interface to configure settings for a specific VFO (VFO A or VFO B).
// It handles drawing the settings screen, processing user input for navigation
// and selection of parameters, managing popups for option selection, and applying
// the changes to the radio's VFO settings.

#include "printf.h"
#include "apps.h"
#include "set_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

/**
 * @brief Draws the VFO settings screen.
 * This function clears the display and draws the title bar indicating which VFO is being set (A or B),
 * the current item number, and total items. It then draws the main list of VFO parameters
 * and the current value of the selected parameter. If an option selection popup or user input
 * window is active, it's drawn on top.
 */
void SetVFO::drawScreen(void) {
    ui.clearDisplay(); // Clear the display buffer

    ui.setBlackColor(); // Set drawing color for the title bar

    // Draw the title bar background
    ui.lcd()->drawBox(0, 0, 128, 7);

    // Display VFO identifier (A or B) and item count
    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, 6, false, false, false, "%s %s", ui.VFOStr, vfoab == Settings::VFOAB::VFOA ? "A" : "B");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor(); // Ensure color is set for the list items

    // Draw the main list of VFO settings, showing the current value of the selected item
    menulist.draw(15, getCurrentOption());

    // If an option selection popup is active, draw it
    if (optionSelected != 0) {
        optionlist.drawPopup(ui, true);
    }
    // If user input mode for a specific option (like frequency offset) is active, draw a popup for it
    if (userOptionSelected != 0) {
        // This part seems to intend to show a user input window.
        // The current implementation uses drawPopupWindow with the menu item's name as title.
        // Specific input handling (like drawing digits) would be in action() or a separate input handler.
        ui.drawPopupWindow(36, 15, 90, 34, menulist.getStringLine());
        // TODO: Add rendering for userOptionInput digits here if needed.
    }

    ui.updateDisplay(); // Send the buffer to the physical display
}

/**
 * @brief Initializes the SetVFO application.
 * This function is called when the SetVFO application is loaded.
 * It populates the `menulist` with the VFO parameters and loads the
 * current settings for the specified VFO (A or B) into the local `vfo` variable.
 */
void SetVFO::init(void) {
    // Define the VFO settings menu items
    menulist.set(0, 6, 127, "SQUELCH\nSTEP\nMODE\nBANDWIDTH\nTX POWER\nSHIFT\nOFFSET\nRX CODE TYPE\nRX CODE\nTX CODE TYPE\nTX CODE\nTX STE\nRX STE\nCOMPANDER\nRX ACG\nPTT ID\nROGER");
    // Load the settings for the VFO (A or B) that this instance is configured to edit
    vfo = radio.getVFO(vfoab);
}

/**
 * @brief Updates the SetVFO application state.
 * This function is called periodically by the system task.
 * For the SetVFO application, it simply redraws the screen.
 */
void SetVFO::update(void) {
    drawScreen(); // Redraw the screen on every update cycle
}

/**
 * @brief Handles timeout events for the SetVFO application.
 * If no option selection or user input is active, it loads the MainVFO application.
 * Otherwise, it resets the input selection state.
 */
void SetVFO::timeout(void) {
    if (optionSelected == 0 && userOptionSelected == 0) {
        // If no popup or user input is active, exit to the main VFO screen
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
    }
    else {
        // If a popup or user input was active, clear these states
        // optionSelected = 0; // Clearing these here might be too abrupt, consider if user expects to resume.
        // userOptionSelected = 0;
        inputSelect = 0; // Reset direct selection input index
    }
};

/**
 * @brief Retrieves the string representation of the current value for the selected VFO parameter.
 * This function is used by `drawScreen` to display the current setting next to the parameter name.
 * It checks the currently selected item in `menulist` and formats its value from the local `vfo` copy.
 * @return A const char* to the string representation of the value, or NULL if not applicable.
 */
const char* SetVFO::getCurrentOption() {
    menulist.setSuffix(NULL); // Reset suffix, specific cases below will set it
    switch (menulist.getListPos() + 1) { // Get 1-based index of the current menu item
    case 1: // SQUELCH
        return ui.getStrValue(Settings::squelchStr, (uint8_t)vfo.squelch);
    case 2: // STEP
        menulist.setSuffix(ui.KHZStr); // Add "kHz" suffix for step value
        return ui.getStrValue(Settings::stepStr, (uint8_t)vfo.step);
    case 3: // MODE (Modulation)
        return ui.getStrValue(Settings::modulationStr, (uint8_t)vfo.modulation);
    case 4: // BANDWIDTH
        menulist.setSuffix(ui.KHZStr); // Add "kHz" suffix
        return ui.getStrValue(Settings::bandwidthStr, (uint8_t)vfo.bw);
    case 5: // TX POWER
        return ui.getStrValue(Settings::powerStr, (uint8_t)vfo.power);
    case 6: // SHIFT (Offset Direction)
        return ui.getStrValue(Settings::offsetStr, (uint8_t)vfo.shift);
    case 7: // OFFSET (Frequency Value)
        menulist.setSuffix(ui.KHZStr); // Add "kHz" suffix
        // TODO: Display the actual offset value (vfo.rx.frequency - vfo.tx.frequency)
        // The current "0.00" is a placeholder. This needs calculation and formatting.
        return "0.00"; // Placeholder for actual offset frequency
    case 8: // RX CODE TYPE (CTCSS/DCS)
        return ui.getStrValue(Settings::codetypeStr, (uint8_t)vfo.rx.codeType);
    case 10: // TX CODE TYPE (CTCSS/DCS)
        return ui.getStrValue(Settings::codetypeStr, (uint8_t)vfo.tx.codeType);
    case 9: // RX CODE Value
        if (vfo.rx.codeType == Settings::CodeType::CT) {
            menulist.setSuffix(ui.HZStr); // "Hz" for CTCSS
            return ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo.rx.code);
        }
        else if (vfo.rx.codeType == Settings::CodeType::DCS) {
            menulist.setSuffix("I"); // "I" for DCS Inverted
            return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo.rx.code);
        }
        else if (vfo.rx.codeType == Settings::CodeType::NDCS) {
            menulist.setSuffix("N"); // "N" for DCS Normal
            return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo.rx.code);
        }
        else {
            return NULL; // No code set
        }
    case 11: // TX CODE Value
        if (vfo.tx.codeType == Settings::CodeType::CT) {
            menulist.setSuffix(ui.HZStr);
            return ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo.tx.code);
        }
        else if (vfo.tx.codeType == Settings::CodeType::DCS) {
            menulist.setSuffix("I");
            return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo.tx.code);
        }
        else if (vfo.tx.codeType == Settings::CodeType::NDCS) {
            menulist.setSuffix("N");
            return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo.tx.code);
        }
        else {
            return NULL;
        }
    case 12: // TX STE (Squelch Tail Elimination for repeater)
        return ui.getStrValue(Settings::onoffStr, (uint8_t)vfo.repeaterSte);
    case 13: // RX STE (Squelch Tail Elimination for direct)
        return ui.getStrValue(Settings::onoffStr, (uint8_t)vfo.ste);
    case 14: // COMPANDER
        return ui.getStrValue(Settings::txrxStr, (uint8_t)vfo.compander);    
    case 15: // RX ACG (Automatic Gain Control)
        if (vfo.rxagc < ui.stringLengthNL(Settings::AGCStr) - 1) { // Check if not "OFF"
            menulist.setSuffix(ui.DBStr); // Add "dB" suffix if a gain value
        }
        return ui.getStrValue(Settings::AGCStr, (uint8_t)vfo.rxagc);
    case 16: // PTT ID
        return ui.getStrValue(Settings::pttIDStr, (uint8_t)vfo.pttid);
    case 17: // ROGER Beep
        return ui.getStrValue(Settings::rogerStr, (uint8_t)vfo.roger);
    default:
        return NULL; // Should not happen
    }
}

/**
 * @brief Loads the options for the currently selected VFO parameter into the `optionlist` popup.
 * This function is called when the user presses MENU on a VFO parameter in the main list.
 * It configures the `optionlist` with the appropriate items, current selection, and title.
 * For parameters requiring direct user input (like OFFSET frequency), it sets `userOptionSelected`.
 */
void SetVFO::loadOptions() {
    // `optionSelected` here is the 1-based index from `menulist.getListPos() + 1`
    switch (optionSelected) {
    case 1: // SQUELCH
        optionlist.set((uint8_t)vfo.squelch, 5, 0, Settings::squelchStr);
        break;
    case 2: // STEP
        optionlist.set((uint8_t)vfo.step, 5, 0, Settings::stepStr, ui.KHZStr);
        break;
    case 3: // MODE (Modulation)
        optionlist.set((uint8_t)vfo.modulation, 5, 0, Settings::modulationStr);
        break;
    case 4: // BANDWIDTH
        optionlist.set((uint8_t)vfo.bw, 5, 0, Settings::bandwidthStr, ui.KHZStr);
        break;
    case 5: // TX POWER
        optionlist.set((uint8_t)vfo.power, 5, 0, Settings::powerStr);
        break;
    case 6: // SHIFT (Offset Direction)
        optionlist.set((uint8_t)vfo.shift, 3, 0, Settings::offsetStr);
        break;
    case 7: // OFFSET (Frequency value) - This requires user input, not a list selection.
        // Store the current offset for potential editing or display.
        userOptionInput = uint32_t(vfo.rx.frequency - vfo.tx.frequency); // This might need abs() or specific logic for direction
        // `userOptionSelected` will be set to 7 in `action()` to signify direct input mode for OFFSET.
        // `optionSelected` should be cleared if this path is taken directly without list.
        break;
    case 8: // RX CODE TYPE
        optionlist.set((uint8_t)vfo.rx.codeType, 5, 0, Settings::codetypeStr);
        break;
    case 10: // TX CODE TYPE
        optionlist.set((uint8_t)vfo.tx.codeType, 5, 0, Settings::codetypeStr);
        break;
    case 9: // RX CODE Value
        if (vfo.rx.codeType == Settings::CodeType::CT) { // CTCSS
            optionlist.set((uint8_t)vfo.rx.code, 5, 0, ui.generateCTDCList(Settings::CTCSSOptions, 50), ui.HZStr);
        }
        else if (vfo.rx.codeType == Settings::CodeType::DCS) { // DCS Inverted
            optionlist.set((uint8_t)vfo.rx.code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "I");
        }
        else if (vfo.rx.codeType == Settings::CodeType::NDCS) { // DCS Normal
            optionlist.set((uint8_t)vfo.rx.code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "N");
        }
        else { // CodeType is NONE
            optionSelected = 0; // No options to show, close the popup.
        }
        break;
    case 11: // TX CODE Value
        if (vfo.tx.codeType == Settings::CodeType::CT) {
            optionlist.set((uint8_t)vfo.tx.code, 5, 0, ui.generateCTDCList(Settings::CTCSSOptions, 50), ui.HZStr);
        }
        else if (vfo.tx.codeType == Settings::CodeType::DCS) {
            optionlist.set((uint8_t)vfo.tx.code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "I");
        }
        else if (vfo.tx.codeType == Settings::CodeType::NDCS) {
            optionlist.set((uint8_t)vfo.tx.code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "N");
        }
        else { // CodeType is NONE
            optionSelected = 0;
        }
        break;
    case 12: // TX STE
        optionlist.set((uint8_t)vfo.repeaterSte, 5, 0, Settings::onoffStr);
        break;
    case 13: // RX STE
        optionlist.set((uint8_t)vfo.ste, 5, 0, Settings::onoffStr);
        break;
    case 14: // COMPANDER
        optionlist.set((uint8_t)vfo.compander, 5, 0, Settings::txrxStr);
        break;    
    case 15: // RX ACG
        optionlist.set((uint8_t)vfo.rxagc, 5, 0, Settings::AGCStr, ui.DBStr);
        break;
    case 16: // PTT ID
        optionlist.set((uint8_t)vfo.pttid, 5, 0, Settings::pttIDStr);
        break;
    case 17: // ROGER Beep
        optionlist.set((uint8_t)vfo.roger, 5, 0, Settings::rogerStr);
        break;
    default: // Should not happen if optionSelected is valid
        optionSelected = 0; // Close popup if unknown item
        break;
    }
}

/**
 * @brief Applies the selected option from `optionlist` or `userOptionInput` to the local `vfo` settings.
 * This function is called when the user confirms a selection in a popup or finishes user input.
 * It updates the corresponding field in the local `vfo` copy.
 * Note: Saving to global settings and radio hardware is done in `action()` after calling this.
 */
void SetVFO::setOptions() {
    uint8_t currentOptionListSelection = optionlist.getListPos(); // Get selected index from popup
    // `optionSelected` holds the 1-based index of the parameter from the main `menulist`
    switch (optionSelected) {
    case 1: // SQUELCH
        vfo.squelch = currentOptionListSelection & 0xF; // Apply selected squelch level
        break;
    case 2: // STEP
        vfo.step = (Settings::Step)currentOptionListSelection;
        break;
    case 3: // MODE (Modulation)
        vfo.modulation = (ModType)currentOptionListSelection;
        break;
    case 4: // BANDWIDTH
        vfo.bw = (BK4819_Filter_Bandwidth)currentOptionListSelection;
        break;
    case 5: // TX POWER
        vfo.power = (Settings::TXOutputPower)currentOptionListSelection;
        break;
    case 6: // SHIFT (Offset Direction)
        vfo.shift = (Settings::OffsetDirection)currentOptionListSelection;
        // If shift is set to NONE, ensure TX frequency equals RX frequency.
        if (vfo.shift == Settings::OffsetDirection::OFFSET_NONE) {
            vfo.tx.frequency = vfo.rx.frequency;
        }
        break;
    case 7: // OFFSET (Frequency value) - This is handled by userOptionInput
        // Apply the user-entered offset to TX frequency based on shift direction
        if (vfo.shift == Settings::OffsetDirection::OFFSET_PLUS) {
            vfo.tx.frequency = static_cast<unsigned int>(vfo.rx.frequency + userOptionInput) & 0x7FFFFFF; // Mask if necessary
        }
        else if (vfo.shift == Settings::OffsetDirection::OFFSET_MINUS) {
            vfo.tx.frequency = static_cast<unsigned int>(vfo.rx.frequency - userOptionInput) & 0x7FFFFFF; // Mask if necessary
        }
        // If shift is NONE, tx.frequency should remain same as rx.frequency (handled in SHIFT case or should be ensured)
        break;
    case 8: // RX CODE TYPE
        vfo.rx.codeType = (Settings::CodeType)currentOptionListSelection;
        if (vfo.rx.codeType == Settings::CodeType::NONE) vfo.rx.code = 0; // Reset code if type is NONE
        break;
    case 10: // TX CODE TYPE
        vfo.tx.codeType = (Settings::CodeType)currentOptionListSelection;
        if (vfo.tx.codeType == Settings::CodeType::NONE) vfo.tx.code = 0; // Reset code if type is NONE
        break;
    case 9: // RX CODE Value
        vfo.rx.code = currentOptionListSelection;
        break;
    case 11: // TX CODE Value
        vfo.tx.code = currentOptionListSelection;
        break;
    case 12: // TX STE
        vfo.repeaterSte = (Settings::ONOFF)currentOptionListSelection;
        break;
    case 13: // RX STE
        vfo.ste = (Settings::ONOFF)currentOptionListSelection;
        break;
    case 14: // COMPANDER
        vfo.compander = (Settings::TXRX)currentOptionListSelection;
        break;    
    case 15: // RX ACG
        vfo.rxagc = currentOptionListSelection & 0x1F; // Mask to ensure valid range
        break;
    case 16: // PTT ID
        vfo.pttid = currentOptionListSelection & 0xF; // Mask for PTT ID options
        break;
    case 17: // ROGER Beep
        vfo.roger = currentOptionListSelection & 0xF; // Mask for Roger beep options
        break;
    default: // Should not happen
        break;
    }
}

/**
 * @brief Handles keyboard input for the SetVFO application.
 * Manages navigation in the main settings list and interaction with option popups or user input fields.
 * @param keyCode The code of the key that was pressed/released.
 * @param keyState The state of the key (pressed, released, long press).
 */
void SetVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {
    uint8_t currentMainMenuSelection = 0; // Used to store 1-based index from menulist

    // --- Handle input when no popup/user input is active (main settings list) ---
    if (optionSelected == 0 && userOptionSelected == 0) {
        switch (keyCode) {
            case Keyboard::KeyCode::KEY_UP:
                if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
                    menulist.prev(); // Navigate up in the main list
                }
                break;
            case Keyboard::KeyCode::KEY_DOWN:
                if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
                    menulist.next(); // Navigate down in the main list
                }
                break;
            case Keyboard::KeyCode::KEY_EXIT:
                if (keyState == Keyboard::KeyState::KEY_PRESSED) {
                    // TODO: Consider prompting to save changes if any were made to `vfo` but not yet applied.
                    // Currently, changes are applied immediately when MENU is pressed in popup.
                    // Exiting here discards unconfirmed changes in `vfo` if MENU wasn't pressed in a popup.
                    // However, `vfo` is reloaded in `init()`, so this effectively cancels.
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
                }
                break;
            case Keyboard::KeyCode::KEY_MENU:
                if (keyState == Keyboard::KeyState::KEY_PRESSED) {
                    inputSelect = 0; // Reset direct number input for list selection
                    currentMainMenuSelection = menulist.getListPos() + 1; // Get 1-based index of selected item

                    if (currentMainMenuSelection == 7) { // OFFSET parameter selected
                        // OFFSET requires direct user input, not a list popup.
                        if (vfo.shift == Settings::OffsetDirection::OFFSET_NONE) {
                            // Beep to indicate OFFSET cannot be set if SHIFT is OFF.
                            systask.pushMessage(System::SystemTask::SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                        } else {
                            // Activate user input mode for OFFSET.
                            userOptionSelected = currentMainMenuSelection; 
                            userOptionInput = 0; // Reset previous input if any for OFFSET.
                            // `loadOptions()` will be called, but for OFFSET it might just prepare `userOptionInput`.
                        }
                    } else {
                        // For other parameters, activate the option selection popup.
                        optionSelected = currentMainMenuSelection; 
                        optionlist.setPopupTitle(menulist.getStringLine()); // Set popup title to parameter name.
                    }
                    loadOptions(); // Load options for the selected parameter into the popup or prepare user input.
                }
                break;
            default: // Handle numeric keys for direct list item selection
                if ((keyCode >= Keyboard::KeyCode::KEY_0 && keyCode <= Keyboard::KeyCode::KEY_9) && keyState == Keyboard::KeyState::KEY_PRESSED) {
                    uint8_t num = ui.keycodeToNumber(keyCode);
                    inputSelect = (inputSelect == 0) ? num : static_cast<uint8_t>((inputSelect * 10) + num);
                    
                    if (inputSelect > menulist.getTotal() || inputSelect == 0) { // Check if input is out of bounds
                        inputSelect = 0; // Reset if invalid or too large
                    } else {
                        menulist.setCurrentPos(inputSelect - 1); // Set current position (0-based)
                        if (inputSelect * 10 > menulist.getTotal() && inputSelect >=10 ) { // If input makes it impossible to form a larger valid number
                             // if inputSelect is already 2 digits, reset it.
                            inputSelect = 0;
                        }
                    }
                }
                break;
        }
    } 
    // --- Handle input when an option popup or user input field is active ---
    else { 
        // This block handles KEY_UP, KEY_DOWN, KEY_EXIT, KEY_MENU for popups/user input.
        // TODO: Add specific handling for numeric input if `userOptionSelected` is active (e.g., for OFFSET).
        switch (keyCode) {
            case Keyboard::KeyCode::KEY_UP:
                if (keyState == Keyboard::KeyState::KEY_PRESSED && optionSelected != 0) { // Check if it's a list popup
                    optionlist.prev(); // Navigate up in the popup list
                }
                // TODO: Handle KEY_UP for userOptionSelected (e.g., increment digit or value)
                break;
            case Keyboard::KeyCode::KEY_DOWN:
                if (keyState == Keyboard::KeyState::KEY_PRESSED && optionSelected != 0) { // Check if it's a list popup
                    optionlist.next(); // Navigate down in the popup list
                }
                // TODO: Handle KEY_DOWN for userOptionSelected (e.g., decrement digit or value)
                break;
            case Keyboard::KeyCode::KEY_EXIT:
                if (keyState == Keyboard::KeyState::KEY_PRESSED) {
                    // Cancel popup/user input and return to the main settings list
                    optionSelected = 0;
                    userOptionSelected = 0;
                    userOptionInput = 0; // Reset any partial user input
                }
                break;
            case Keyboard::KeyCode::KEY_MENU:
                if (keyState == Keyboard::KeyState::KEY_PRESSED) {
                    // Confirm selection/input
                    setOptions(); // Apply the selected option or entered value to local `vfo`
                    radio.setVFO(vfoab, vfo); // Save the updated `vfo` to global settings and hardware
                    // Exit popup/user input mode
                    optionSelected = 0;
                    userOptionSelected = 0;
                    userOptionInput = 0;
                }
                break;
            default: // Potentially handle numeric input for userOptionSelected fields here
               // Example: if (userOptionSelected == 7 && keyCode >= KEY_0 && keyCode <= KEY_9) { ... }
               break;
        }
    }
}
