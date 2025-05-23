// This file implements the MainVFO application class, which is responsible for
// the main Variable Frequency Oscillator screen of the radio.
// It handles drawing the screen, processing user input, managing popups for settings,
// and interacting with the radio and system tasks.

#include "printf.h"
#include "apps.h"
#include "main_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

/**
 * @brief Draws the main VFO screen.
 * This function is responsible for rendering all visual elements on the main VFO screen,
 * including VFO A and VFO B information, frequencies, signal strength (RSSI),
 * status icons (battery, charging, F key, power save), and any active popups.
 */
void MainVFO::drawScreen(void) {

    // Determine active VFOs for display
    Settings::VFOAB activeVFO1 = radio.getCurrentVFO(); // Currently selected VFO
    Settings::VFOAB activeVFO2 = activeVFO1 == Settings::VFOAB::VFOA ? Settings::VFOAB::VFOB : Settings::VFOAB::VFOA; // The other VFO

    // Get VFO data
    Settings::VFO vfo1 = radio.getVFO(activeVFO1);
    Settings::VFO vfo2 = radio.getVFO(activeVFO2);

    // Determine if VFOs are currently receiving
    bool rxVFO1 = (radio.getState() == Settings::RadioState::RX_ON && activeVFO1 == radio.getRXVFO());
    bool rxVFO2 = (radio.getState() == Settings::RadioState::RX_ON && activeVFO2 == radio.getRXVFO());

    ui.clearDisplay(); // Clear the display buffer

    ui.lcd()->setColorIndex(BLACK); // Set drawing color to black

    // --- Draw VFO 1 (Top VFO) ---
    ui.lcd()->drawBox(0, 0, 128, 7); // Top banner for VFO1 name

    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 1, 0, 6, false, false, false, vfo1.name); // VFO1 name

    // VFO1 settings (Power, Bandwidth, Modulation)
    ui.setFont(Font::FONT_5_TR);
    const char* powerA = ui.getStrValue(Settings::powerStr, (uint8_t)vfo1.power);
    const char* bandwidthA = ui.getStrValue(Settings::bandwidthStr, (uint8_t)vfo1.bw);
    const char* modulationA = ui.getStrValue(Settings::modulationStr, (uint8_t)vfo1.modulation);
    ui.drawStringf(TextAlign::RIGHT, 0, 127, 6, false, false, false, "%.*s %.*sK %.*s", ui.stringLengthNL(modulationA), modulationA, ui.stringLengthNL(bandwidthA), bandwidthA, ui.stringLengthNL(powerA), powerA);

    // VFO1 RX/TX Codes (CTCSS/DCS)
    const char* rxCode;
    const char* txCode;
    uint8_t codeXend = 127; // X-coordinate for right alignment of codes

    if (vfo1.rx.codeType == Settings::CodeType::CT) {
        rxCode = ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo1.rx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, radio.isRXToneDetected(), false, "%s %.*s%s", ui.RXStr, ui.stringLengthNL(rxCode), rxCode, ui.HZStr);
        codeXend -= 48; // Adjust X for next code if displayed
    }
    else if (vfo1.rx.codeType == Settings::CodeType::DCS || vfo1.rx.codeType == Settings::CodeType::NDCS) {
        rxCode = ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo1.rx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, radio.isRXToneDetected(), false, "%s %.*s%s", ui.RXStr, ui.stringLengthNL(rxCode), rxCode, vfo1.rx.codeType == Settings::CodeType::NDCS ? "N" : "I");
        codeXend -= 48; // Adjust X for next code if displayed
    }

    if (vfo1.tx.codeType == Settings::CodeType::CT) {
        txCode = ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo1.tx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, false, false, "%s %.*s%s", ui.TXStr, ui.stringLengthNL(txCode), txCode, ui.HZStr);
    }
    else if (vfo1.tx.codeType == Settings::CodeType::DCS || vfo1.tx.codeType == Settings::CodeType::NDCS) {
        txCode = ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo1.tx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, false, false, "%s %.*s%s", ui.TXStr, ui.stringLengthNL(txCode), txCode, vfo1.tx.codeType == Settings::CodeType::NDCS ? "N" : "I");
    }

    // VFO1 Indicator ("VFO") and Active VFO Letter (A/B)
    ui.setFont(Font::FONT_8_TR);
    ui.lcd()->setColorIndex(BLACK);
    ui.drawString(TextAlign::LEFT, 0, 0, 22, true, false, false, ui.VFOStr); // "VFO" label
    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, 14, true, true, false, "%s", activeVFO1 == Settings::VFOAB::VFOA ? "A" : "B"); // "A" or "B"

    // RX indicator for VFO1
    if (rxVFO1) {
        ui.drawString(TextAlign::LEFT, 12, 0, 14, true, true, false, ui.RXStr); // "RX"
    }

    // VFO1 Frequency Display (either direct input or current frequency)
    if (showFreqInput) {
        ui.drawFrequencyBig(true, freqInput, 111, 19); // Show frequency being input
    }
    else {
        ui.drawFrequencyBig(rxVFO1, vfo1.rx.frequency, 111, 19); // Show current VFO1 frequency
    }

    // --- Draw VFO 2 (Bottom VFO) ---
    uint8_t vfoBY = 28; // Y-coordinate for VFO2 elements

    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawBox(0, vfoBY, 128, 7); // Bottom banner for VFO2 name

    ui.setFont(Font::FONT_5_TR);
    ui.drawStringf(TextAlign::LEFT, 1, 0, vfoBY + 6, false, false, false, "%S", vfo2.name); // VFO2 name

    // VFO2 settings (Power, Bandwidth, Modulation)
    const char* powerB = ui.getStrValue(Settings::powerStr, (uint8_t)vfo2.power);
    const char* bandwidthB = ui.getStrValue(Settings::bandwidthStr, (uint8_t)vfo2.bw);
    const char* modulationB = ui.getStrValue(Settings::modulationStr, (uint8_t)vfo2.modulation);
    ui.drawStringf(TextAlign::RIGHT, 0, 127, vfoBY + 6, false, false, false, "%.*s %.*sK %.*s", ui.stringLengthNL(modulationB), modulationB, ui.stringLengthNL(bandwidthB), bandwidthB, ui.stringLengthNL(powerB), powerB);

    // VFO2 Frequency Display
    ui.drawFrequencySmall(rxVFO2, vfo2.rx.frequency, 126, vfoBY + 17);

    // VFO2 Indicator and Active VFO Letter (A/B)
    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, vfoBY + 15, true, false, true, "%s", activeVFO2 == Settings::VFOAB::VFOB ? "B" : "A"); // "A" or "B"

    // RX/VFO indicator for VFO2
    if ((rxVFO2 && vfo2.rx.codeType != Settings::CodeType::NONE && radio.isRXToneDetected()) 
        || (rxVFO2 && vfo2.rx.codeType == Settings::CodeType::NONE)) {
        ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, true, false, ui.RXStr); // "RX"
    }
    else {
        ui.setFont(Font::FONT_8_TR);
        ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, false, false, ui.VFOStr); // "VFO"
    }

    ui.lcd()->setColorIndex(BLACK);

    // --- RSSI S-Meter Display ---
    showRSSI(1, 52);

    // --- Status Information Area (Bottom part of the screen) ---
    // Battery status and charging indicator
    if (systask.getBattery().isCharging()) {
        ui.draw_ic8_charging(118, 52, BLACK); // Charging icon
    }
    else {
        ui.drawBattery(systask.getBattery().getBatteryPercentage(), 114, 52); // Battery level icon
    }
    ui.setFont(Font::FONT_5_TR);
    ui.drawStringf(TextAlign::RIGHT, 0, 128, 64, true, false, false, "%i%%", systask.getBattery().getBatteryPercentage()); // Battery percentage text

    // "F" key indicator
    if (systask.wasFKeyPressed()) {
        ui.drawString(TextAlign::RIGHT, 0, 97, 56, true, true, false, "F");
    }

    // Active RX VFO indicator (A or B)
    if (radio.getState() == Settings::RadioState::RX_ON) {
        ui.drawString(TextAlign::RIGHT, 0, 108, 64, true, false, false, radio.getRXVFO() == Settings::VFOAB::VFOA ? "A" : "B");
    }
    else {
        ui.drawString(TextAlign::RIGHT, 0, 108, 64, true, false, false, "A/B"); // Default when not actively RXing
    }

    // Power Save mode indicator
    if (radio.isPowerSaveMode()) {
        ui.draw_ps(78, 59, BLACK); // "PS" icon
    }

    // --- Draw Popup (if active) ---
    if (popupSelected != NONE) {
        popupList.drawPopup(ui);
    }

    ui.updateDisplay(); // Send the buffer to the physical display
}

/**
 * @brief Displays the RSSI (Received Signal Strength Indicator) S-meter.
 * Calculates S-level and +dB value from RSSI and draws the S-meter bar graph and text.
 * @param posX The X coordinate for the top-left of the S-meter.
 * @param posY The Y coordinate for the top-left of the S-meter.
 */
void MainVFO::showRSSI(uint8_t posX, uint8_t posY) {

    uint8_t sValue = 0; // S-meter value (0-9, or 10 for S9+)
    int16_t plusDB = 0; // dB value over S9 (e.g., S9+10dB)

    // Get RSSI only when radio is actively receiving
    if (radio.getState() == Settings::RadioState::RX_ON) {
        int16_t rssi_dBm = radio.getRSSIdBm();          // Get the RSSI value in dBm
        sValue = radio.convertRSSIToSLevel(rssi_dBm); // Convert RSSI to S-level (0-9)
        if (sValue == 10) { // S-level 10 indicates S9 or stronger
            plusDB = radio.convertRSSIToPlusDB(rssi_dBm); // Convert to +dB value if S9 or stronger
        }
    }

    // Draw the graphical S-meter
    ui.drawRSSI(sValue, posX, posY + 1);

    // Draw the textual representation of S-level
    ui.setFont(Font::FONT_8_TR);
    if (sValue > 0) { // Only display text if there's a signal
        if (sValue == 10) { // Signal is S9 or stronger
            ui.drawString(TextAlign::LEFT, posX + 38, 0, posY + 5, true, false, false, "S9");
            ui.drawStringf(TextAlign::LEFT, posX + 38, 0, posY + 12, true, false, false, "+%idB", plusDB);
        }
        else { // Signal is S1 to S8
            ui.drawStringf(TextAlign::LEFT, posX + 38, 0, posY + 5, true, false, false, "S%i", sValue);
        }
    }
}

/**
 * @brief Initializes the MainVFO application.
 * This function is called when the MainVFO application is loaded.
 * Currently, it does not perform any specific initialization.
 */
void MainVFO::init(void) {
    // No specific initialization required for MainVFO at the moment.
}

/**
 * @brief Updates the MainVFO application state.
 * This function is called periodically by the system task to refresh the display.
 */
void MainVFO::update(void) {
    drawScreen(); // Redraw the screen on every update cycle
}

/**
 * @brief Handles timeout events for the MainVFO application.
 * If a popup or frequency input is active, this function hides them.
 */
void MainVFO::timeout(void) {
    if (popupSelected != NONE) {
        popupSelected = NONE; // Hide any active popup
    }
    if (showFreqInput) {
        showFreqInput = false; // Exit frequency input mode
    }
}

/**
 * @brief Saves the value selected in the currently active popup.
 * Reads the selected value from the popupList and applies it to the
 * corresponding setting (Bandwidth, Modulation, or Power) of the active VFO.
 */
void MainVFO::savePopupValue(void) {
    Settings::VFO vfo = radio.getActiveVFO(); // Get current active VFO settings

    // Update the VFO setting based on which popup is active
    if (popupSelected == BANDWIDTH) {
        vfo.bw = (BK4819_Filter_Bandwidth)popupList.getListPos();
    }
    else if (popupSelected == MODULATION) {
        vfo.modulation = (ModType)popupList.getListPos();
    }
    else if (popupSelected == POWER) {
        vfo.power = (Settings::TXOutputPower)popupList.getListPos();
    }

    // Apply the modified VFO settings back to the radio
    radio.setVFO(radio.getCurrentVFO(), vfo);
    radio.setupToVFO(radio.getCurrentVFO()); // Reconfigure radio with new settings
}

/**
 * @brief Handles keyboard input for the MainVFO application.
 * This function processes key presses and releases to navigate popups,
 * change frequencies, enter direct frequency input mode, and access menus.
 * @param keyCode The code of the key that was pressed or released.
 * @param keyState The state of the key (e.g., KEY_RELEASED, KEY_LONG_PRESSED).
 */
void MainVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {

    Settings::VFO vfo = radio.getActiveVFO(); // Get current active VFO settings

    if (keyState == Keyboard::KeyState::KEY_RELEASED) {
        // --- Handle key releases when a popup is active ---
        if (popupSelected != NONE) {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                popupList.prev(); // Navigate up in the popup list
                savePopupValue(); // Save the new selection
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                popupList.next(); // Navigate down in the popup list
                savePopupValue(); // Save the new selection
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                popupSelected = NONE; // Close popup on MENU press
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                popupSelected = NONE; // Close popup on EXIT press
            }
            // Allow quick selection using number keys that might correspond to popup items
            else if (keyCode == Keyboard::KeyCode::KEY_4 || keyCode == Keyboard::KeyCode::KEY_5 || keyCode == Keyboard::KeyCode::KEY_6) {
                popupList.next(); // Example: treat as next item, specific logic may vary
                savePopupValue();
            }
        }
        // --- Handle key releases when no popup is active (main VFO screen) ---
        else {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                // Increase frequency by step
                uint32_t newFrequency = vfo.rx.frequency + Settings::StepFrequencyTable[(uint8_t)vfo.step];
                vfo.rx.frequency = (uint32_t)(newFrequency & 0x07FFFFFF); // Apply mask if necessary for frequency range
                radio.setVFO(radio.getCurrentVFO(), vfo);
                radio.setupToVFO(radio.getCurrentVFO());
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                // Decrease frequency by step
                uint32_t newFrequency = vfo.rx.frequency - Settings::StepFrequencyTable[(uint8_t)vfo.step];
                vfo.rx.frequency = (uint32_t)(newFrequency & 0x7FFFFFF); // Apply mask
                radio.setVFO(radio.getCurrentVFO(), vfo);
                radio.setupToVFO(radio.getCurrentVFO());
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                if (showFreqInput) {
                    // If in frequency input mode, confirm entered frequency
                    showFreqInput = false;
                    vfo.rx.frequency = (uint32_t)(freqInput & 0x7FFFFFF);
                    vfo.tx.frequency = (uint32_t)(freqInput & 0x7FFFFFF); // Typically RX and TX frequencies are linked in VFO mode
                    radio.setVFO(radio.getCurrentVFO(), vfo);
                    radio.setupToVFO(radio.getCurrentVFO());
                }
                else {
                    // Otherwise, load the main menu application
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Menu);
                }
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                if (showFreqInput) {
                    showFreqInput = false; // Cancel frequency input mode
                }
            }
            else if (keyCode == Keyboard::KeyCode::KEY_STAR) {
                // Backspace functionality during frequency input
                if (showFreqInput && freqInput > 0) {
                    freqInput /= 10;
                }
            }
            // Handle numeric key presses for direct frequency input
            else if (keyCode >= Keyboard::KeyCode::KEY_0 && keyCode <= Keyboard::KeyCode::KEY_9) {
                if (!showFreqInput) {
                    showFreqInput = true; // Enter frequency input mode
                    freqInput = 0;        // Reset current input
                }
                uint8_t number = ui.keycodeToNumber(keyCode); // Convert keycode to number

                freqInput *= (uint32_t)(10 & 0x7FFFFFF); // Shift existing digits left
                freqInput += (uint32_t)(number & 0x7FFFFFF); // Add new digit
                if (freqInput >= 999999999) { // Limit frequency input length (e.g., 9 digits)
                    showFreqInput = false; // Auto-confirm or handle overflow
                }
            }
        }
    }
    // --- Handle long presses or F-key combinations ---
    else if (keyState == Keyboard::KeyState::KEY_LONG_PRESSED || keyState == Keyboard::KeyState::KEY_PRESSED_WITH_F) {
        if (!showFreqInput && popupSelected == NONE) { // Ensure not in freq input or popup mode
            if (keyCode == Keyboard::KeyCode::KEY_2) {
                radio.changeActiveVFO(); // Toggle between VFO A and VFO B
            }
            // Open popups for specific settings
            else if (keyCode == Keyboard::KeyCode::KEY_4) { // Bandwidth selection
                popupList.set((uint8_t)vfo.bw, 3, 0, Settings::bandwidthStr, ui.KHZStr); // Configure popup
                popupList.setPopupTitle("BANDWIDTH");
                popupSelected = BANDWIDTH; // Set active popup
            }
            else if (keyCode == Keyboard::KeyCode::KEY_5) { // Modulation selection
                popupList.set((uint8_t)vfo.modulation, 3, 0, Settings::modulationStr);
                popupList.setPopupTitle("MODULATION");
                popupSelected = MODULATION;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_6) { // TX Power selection
                popupList.set((uint8_t)vfo.power, 3, 0, Settings::powerStr);
                popupList.setPopupTitle("TX POWER");
                popupSelected = POWER;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                // Long press MENU loads the settings for the current VFO (A or B)
                systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, radio.getCurrentVFO() == Settings::VFOAB::VFOA ? (uint32_t)Applications::SETVFOA : (uint32_t)Applications::SETVFOB);
            }
        }
    }
}
