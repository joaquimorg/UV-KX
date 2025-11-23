#include "printf.h"
#include "apps.h"
#include "main_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void MainVFO::drawScreen(void) {

    Settings::VFOAB activeVFO1 = radio.getCurrentVFO();
    Settings::VFOAB activeVFO2 = activeVFO1 == Settings::VFOAB::VFOA ? Settings::VFOAB::VFOB : Settings::VFOAB::VFOA;
    auto& settings = systask.getSettings();

    Settings::VFO vfo1 = radio.getVFO(activeVFO1);
    Settings::VFO vfo2 = radio.getVFO(activeVFO2);
    bool rxVFO1 = (radio.getState() == Settings::RadioState::RX_ON && activeVFO1 == radio.getRXVFO());
    bool rxVFO2 = (radio.getState() == Settings::RadioState::RX_ON && activeVFO2 == radio.getRXVFO());

    bool activeMemoryMode = settings.radioSettings.showVFO[(uint8_t)activeVFO1] == Settings::ONOFF::OFF;

    ui.clearDisplay();

    ui.lcd()->setColorIndex(BLACK);

    ui.lcd()->drawBox(0, 0, 128, 7);

    ui.setFont(Font::FONT_8B_TR);
    const char* displayNameVFO1 = vfo1.name;
    if (!activeMemoryMode) {
        const char* bandName = radio.getBandName(vfo1.rx.frequency);
        if (bandName && bandName[0] != '\0') {
            displayNameVFO1 = bandName;
        }
    }
    ui.drawString(TextAlign::LEFT, 1, 0, 6, false, false, false, displayNameVFO1);

    ui.setFont(Font::FONT_5_TR);
    const char* powerA = ui.getStrValue(Settings::powerStr, (uint8_t)vfo1.power);
    const char* bandwidthA = ui.getStrValue(Settings::bandwidthStr, (uint8_t)vfo1.bw);
    const char* modulationA = ui.getStrValue(Settings::modulationStr, (uint8_t)vfo1.modulation);
    const char* rxCode;
    const char* txCode;
    uint8_t codeXend = 127;

    if (vfo1.rx.codeType == Settings::CodeType::CT) {
        rxCode = ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo1.rx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, radio.isRXToneDetected(), false, "%s %.*s%s", ui.RXStr, ui.stringLengthNL(rxCode), rxCode, ui.HZStr);
        codeXend -= 48;
    }
    else if (vfo1.rx.codeType == Settings::CodeType::DCS || vfo1.rx.codeType == Settings::CodeType::NDCS) {
        rxCode = ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo1.rx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, radio.isRXToneDetected(), false, "%s %.*s%s", ui.RXStr, ui.stringLengthNL(rxCode), rxCode, vfo1.rx.codeType == Settings::CodeType::NDCS ? "N" : "I");
        codeXend -= 48;
    }

    if (vfo1.tx.codeType == Settings::CodeType::CT) {
        txCode = ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo1.tx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, false, false, "%s %.*s%s", ui.TXStr, ui.stringLengthNL(txCode), txCode, ui.HZStr);
    }
    else if (vfo1.tx.codeType == Settings::CodeType::DCS || vfo1.tx.codeType == Settings::CodeType::NDCS) {
        txCode = ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo1.tx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, false, false, "%s %.*s%s", ui.TXStr, ui.stringLengthNL(txCode), txCode, vfo1.tx.codeType == Settings::CodeType::NDCS ? "N" : "I");
    }

    ui.drawStringf(TextAlign::RIGHT, 0, 127, 6, false, false, false, "%.*s %.*sK %.*s", ui.stringLengthNL(modulationA), modulationA, ui.stringLengthNL(bandwidthA), bandwidthA, ui.stringLengthNL(powerA), powerA);

    ui.setFont(Font::FONT_8_TR);
    ui.lcd()->setColorIndex(BLACK);
    
    char modeLabel[12] = {};
    const char* labelText = ui.VFOStr;

    if (activeMemoryMode && channelEntryActive && channelEntryValue > 0) {
        snprintf(modeLabel, sizeof(modeLabel), "CH-%03u*", channelEntryValue);
        labelText = modeLabel;
    } else if (activeMemoryMode) {
        uint16_t mem = settings.radioSettings.memory[(uint8_t)activeVFO1];
        if (mem >= 1 && mem <= Settings::MAX_CHANNELS) {
            snprintf(modeLabel, sizeof(modeLabel), "CH-%03u", mem);
            labelText = modeLabel;
        }
    }
    ui.drawString(TextAlign::LEFT, 0, 0, 22, true, channelEntryActive && channelEntryValue, false, labelText);

    //ui.lcd()->setColorIndex(BLACK);
    //ui.lcd()->drawLine(5, 9, 5, 25);

    ui.setFont(Font::FONT_8B_TR);
    bool showA = !(lastRXVFO == activeVFO1 && lastRXCounter > 0 && blinkState);
    if (showA)
        ui.drawStringf(TextAlign::LEFT, 2, 0, 14, true, true, false, "%s", activeVFO1 == Settings::VFOAB::VFOA ? "A" : "B");

    if (rxVFO1) {
        ui.drawString(TextAlign::LEFT, 12, 0, 14, true, true, false, ui.RXStr);
    }

    if (showFreqInput) {
        ui.drawFrequencyBig(true, freqInput, 111, 19);
    }
    else {
        ui.drawFrequencyBig(rxVFO1, vfo1.rx.frequency, 111, 19);
    }    

    uint8_t vfoBY = 28;

    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawBox(0, vfoBY, 128, 7);

    bool activeMemoryModeVFO2 = settings.radioSettings.showVFO[(uint8_t)activeVFO2] == Settings::ONOFF::OFF;

    ui.setFont(Font::FONT_5_TR);
    const char* displayNameVFO2 = vfo2.name;
    if (!activeMemoryModeVFO2) {
        const char* bandName = radio.getBandName(vfo2.rx.frequency);
        if (bandName && bandName[0] != '\0') {
            displayNameVFO2 = bandName;
        }
    }
    ui.drawStringf(TextAlign::LEFT, 1, 0, vfoBY + 6, false, false, false, "%S", displayNameVFO2);

    const char* powerB = ui.getStrValue(Settings::powerStr, (uint8_t)vfo2.power);
    const char* bandwidthB = ui.getStrValue(Settings::bandwidthStr, (uint8_t)vfo2.bw);
    const char* modulationB = ui.getStrValue(Settings::modulationStr, (uint8_t)vfo2.modulation);

    ui.drawStringf(TextAlign::RIGHT, 0, 127, vfoBY + 6, false, false, false, "%.*s %.*sK %.*s", ui.stringLengthNL(modulationB), modulationB, ui.stringLengthNL(bandwidthB), bandwidthB, ui.stringLengthNL(powerB), powerB);

    ui.drawFrequencySmall(rxVFO2, vfo2.rx.frequency, 126, vfoBY + 17);

    ui.setFont(Font::FONT_8B_TR);
    bool showB = !(lastRXVFO == activeVFO2 && lastRXCounter > 0 && blinkState);
    if (showB)
        ui.drawStringf(TextAlign::LEFT, 2, 0, vfoBY + 15, true, false, true, "%s", activeVFO2 == Settings::VFOAB::VFOB ? "B" : "A");

    if ((rxVFO2 && vfo2.rx.codeType != Settings::CodeType::NONE && radio.isRXToneDetected()) 
        || (rxVFO2 && vfo2.rx.codeType == Settings::CodeType::NONE)) {
        ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, true, false, ui.RXStr);
    }
    else {
        ui.setFont(Font::FONT_8_TR);
        labelText = ui.VFOStr;
        if (activeMemoryModeVFO2) {
            uint16_t mem = settings.radioSettings.memory[(uint8_t)activeVFO2];
            if (mem >= 1 && mem <= Settings::MAX_CHANNELS) {
                snprintf(modeLabel, sizeof(modeLabel), "CH-%03u", mem);
                labelText = modeLabel;
            }
        }
        ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, false, false, labelText);        
    }

    ui.lcd()->setColorIndex(BLACK);

    showRSSI(1, 52);

    //ui.draw_dotline(0, 47, BLACK);

    // Status information

    if (systask.getBattery().isCharging()) {
        ui.draw_ic8_charging(118, 52, BLACK);
    }
    else {
        ui.drawBattery(systask.getBattery().getBatteryPercentage(), 114, 52);
    }

    ui.setFont(Font::FONT_5_TR);
    ui.drawStringf(TextAlign::RIGHT, 0, 128, 64, true, false, false, "%i%%", systask.getBattery().getBatteryPercentage());

    if (systask.wasFKeyPressed()) {
        ui.drawString(TextAlign::RIGHT, 0, 97, 56, true, true, false, "F");
    }

    if (radio.getState() == Settings::RadioState::RX_ON) {
        ui.drawString(TextAlign::RIGHT, 0, 108, 64, true, false, false, radio.getRXVFO() == Settings::VFOAB::VFOA ? "A" : "B");
    }
    else {
        ui.drawString(TextAlign::RIGHT, 0, 108, 64, true, false, false, "A/B");
    }

    if (radio.isPowerSaveMode()) {
        //ui.drawString(TextAlign::RIGHT, 0, 80, 64, true, false, false, "PS");
        ui.draw_ps(78, 59, BLACK);
    }

    if (systask.getSettings().isRadioSavePending()) {
        ui.draw_save(68, 59, BLACK);
    }

    if (popupSelected != NONE) {
        popupList.drawPopup(ui);
    }

    ui.updateDisplay();
}

void MainVFO::showRSSI(uint8_t posX, uint8_t posY) {

    uint8_t sValue = 0;
    int16_t plusDB = 0;

    if (radio.getState() == Settings::RadioState::RX_ON) {
        int16_t rssi_dBm = radio.getRSSIdBm(); // Get the RSSI value in dBm
        sValue = radio.convertRSSIToSLevel(rssi_dBm); // Convert RSSI to S-level
        if (sValue == 10) {
            plusDB = radio.convertRSSIToPlusDB(rssi_dBm); // Convert to +dB value if greater than S9
        }
    }

    ui.drawRSSI(sValue, posX, posY + 1);

    ui.setFont(Font::FONT_8_TR);
    if (sValue > 0) {
        if (sValue == 10) {
            ui.drawString(TextAlign::LEFT, posX + 38, 0, posY + 5, true, false, false, "S9");
            ui.drawStringf(TextAlign::LEFT, posX + 38, 0, posY + 12, true, false, false, "+%idB", plusDB);
        }
        else {
            ui.drawStringf(TextAlign::LEFT, posX + 38, 0, posY + 5, true, false, false, "S%i", sValue);
        }
    }


}

void MainVFO::init(void) {
    prevRadioState = radio.getState();
    prevRXVFO = radio.getRXVFO();
    lastRXCounter = 0;
    blinkTimer = 0;
    blinkState = false;
    memoryChannelCount = 0;
    memoryChannelListValid = false;

    auto& settings = systask.getSettings();
    for (uint8_t index = 0; index < 2; ++index) {
        if (settings.radioSettings.showVFO[index] == Settings::ONOFF::OFF) {
            uint16_t channelNumber = settings.radioSettings.memory[index];
            Settings::VFO channelData;
            if (channelNumber >= 1 && channelNumber <= Settings::MAX_CHANNELS &&
                settings.getChannelData(channelNumber, channelData)) {
                radio.setVFO(static_cast<Settings::VFOAB>(index), channelData);
            } else {
                settings.radioSettings.showVFO[index] = Settings::ONOFF::ON;
            }
        }
    }

    // Preload the list of occupied memory channels so navigation is instant after power-on.
    refreshMemoryChannelList();
}

void MainVFO::update(void) {
    // Track radio state transitions for RX activity
    Settings::RadioState curState = radio.getState();
    if (curState == Settings::RadioState::RX_ON) {
        // remember the VFO that is currently receiving
        prevRXVFO = radio.getRXVFO();
    } else {
        if (prevRadioState == Settings::RadioState::RX_ON) {
            // RX just finished, start blink period
            lastRXVFO = prevRXVFO;
            lastRXCounter = LAST_RX_DURATION;
        } else if (lastRXCounter > 0) {
            lastRXCounter--;
        }
    }
    prevRadioState = curState;

    // Handle blink timing
    if (++blinkTimer >= BLINK_INTERVAL) {
        blinkTimer = 0;
        blinkState = !blinkState;
    }

    drawScreen();
}

void MainVFO::timeout(void) {
    if (popupSelected != NONE) {
        popupSelected = NONE;
    }
    if (showFreqInput) {
        showFreqInput = false;
    }
}

void MainVFO::savePopupValue(void) {
    Settings::VFO vfo = radio.getActiveVFO();

    if (popupSelected == BANDWIDTH) {
        vfo.bw = (BK4819_Filter_Bandwidth)popupList.getListPos();
    }
    else if (popupSelected == MODULATION) {
        vfo.modulation = (ModType)popupList.getListPos();
    }
    else if (popupSelected == POWER) {
        vfo.power = (Settings::TXOutputPower)popupList.getListPos();
    }

    applyActiveVFO(vfo);
}

void MainVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {

    Settings::VFO vfo = radio.getActiveVFO();

    if (keyState == Keyboard::KeyState::KEY_RELEASED) {

        if (popupSelected != NONE) {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                popupList.prev();
                savePopupValue();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                popupList.next();
                savePopupValue();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                popupSelected = NONE;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                popupSelected = NONE;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_4 || keyCode == Keyboard::KeyCode::KEY_5 || keyCode == Keyboard::KeyCode::KEY_6) {
                popupList.next();
                savePopupValue();
            }
        }
        else {

            auto& settings = systask.getSettings();
            uint8_t vfoIndex = (uint8_t)radio.getCurrentVFO();
            bool showingVFO = settings.radioSettings.showVFO[vfoIndex] == Settings::ONOFF::ON;

            if (showingVFO) {
                if (keyCode == Keyboard::KeyCode::KEY_UP) {
                    uint32_t newFrequency = vfo.rx.frequency + Settings::StepFrequencyTable[(uint8_t)vfo.step];
                    vfo.rx.frequency = (uint32_t)(newFrequency);

                    applyActiveVFO(vfo);
                }
                else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                    uint32_t newFrequency = vfo.rx.frequency - Settings::StepFrequencyTable[(uint8_t)vfo.step];
                    vfo.rx.frequency = (uint32_t)(newFrequency);

                    applyActiveVFO(vfo);
                }
                else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                    if (showFreqInput) {
                        showFreqInput = false;
                        vfo.rx.frequency = (uint32_t)(freqInput);
                        vfo.tx.frequency = (uint32_t)(freqInput);
                        applyActiveVFO(vfo);
                    }
                    else {
                        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Menu);
                    }
                }
                else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                    if (showFreqInput) {
                        showFreqInput = false;
                    }
                }
                else if (keyCode == Keyboard::KeyCode::KEY_STAR) {
                    if (showFreqInput && freqInput > 0) {
                        freqInput /= 10;
                    }
                }
                else if (keyCode >= Keyboard::KeyCode::KEY_0 && keyCode <= Keyboard::KeyCode::KEY_9) {
                    if (!showFreqInput) {
                        showFreqInput = true;
                        freqInput = 0;
                    }
                    uint8_t number = ui.keycodeToNumber(keyCode);
                    freqInput *= (uint32_t)(10);
                    freqInput += (uint32_t)(number);
                    if (freqInput >= 999999999) {
                        showFreqInput = false;
                    }
                }
            } else {
                showFreqInput = false;
                auto loadChannel = [&](uint16_t ch) {
                    Settings::VFO channelData;
                    if (settings.getChannelData(ch, channelData)) {
                        radio.setVFO(radio.getCurrentVFO(), channelData);
                        settings.radioSettings.memory[vfoIndex] = ch;
                        settings.scheduleSaveIfNeeded();
                        channelEntryActive = false;
                        channelEntryValue = 0;
                        return true;
                    }
                    radio.playBeep(Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                    return false;
                };

                if (keyCode == Keyboard::KeyCode::KEY_UP) {
                    if (!ensureMemoryChannelList()) {
                        radio.playBeep(Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                        return;
                    }
                    channelEntryActive = false;
                    channelEntryValue = 0;
                    uint16_t nextChannel;
                    uint16_t baseChannel = resolveActiveMemoryChannel(vfoIndex);
                    if (getNextMemoryChannel(baseChannel, 1, nextChannel)) {
                        loadChannel(nextChannel);
                    }
                }
                else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                    if (!ensureMemoryChannelList()) {
                        radio.playBeep(Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                        return;
                    }
                    channelEntryActive = false;
                    channelEntryValue = 0;
                    uint16_t nextChannel;
                    uint16_t baseChannel = resolveActiveMemoryChannel(vfoIndex);
                    if (getNextMemoryChannel(baseChannel, -1, nextChannel)) {
                        loadChannel(nextChannel);
                    }
                }
                else if (keyCode >= Keyboard::KeyCode::KEY_0 && keyCode <= Keyboard::KeyCode::KEY_9) {
                    uint8_t digit = ui.keycodeToNumber(keyCode);
                    if (!channelEntryActive) {
                        channelEntryValue = digit;
                    } else {
                        if (channelEntryValue >= 100) {
                            channelEntryValue = channelEntryValue % 100;
                        }
                        channelEntryValue = static_cast<uint16_t>(channelEntryValue * 10 + digit);
                    }
                    if (channelEntryValue > Settings::MAX_CHANNELS) {
                        channelEntryValue = digit;
                    }
                    channelEntryActive = true;
                    if (channelEntryValue == 0) {
                        channelEntryValue = 1;
                    }
                }
                else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                    if (channelEntryActive) {
                        if (channelEntryValue >= 1 && channelEntryValue <= Settings::MAX_CHANNELS) {
                            loadChannel(channelEntryValue);
                        } else {
                            radio.playBeep(Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                        }
                    } else {
                        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Menu);
                    }
                }
                else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                    if (channelEntryActive) {
                        channelEntryActive = false;
                        channelEntryValue = 0;
                    }
                }
                else if (keyCode == Keyboard::KeyCode::KEY_STAR) {
                    if (channelEntryActive) {
                        channelEntryValue /= 10;
                        if (channelEntryValue == 0) {
                            channelEntryActive = false;
                        }
                    }
                }
                else {
                    return;
                }
            }
        }
    }
    else if (keyState == Keyboard::KeyState::KEY_LONG_PRESSED || keyState == Keyboard::KeyState::KEY_PRESSED_WITH_F) {

        if (!showFreqInput && popupSelected == NONE) {
            if (keyCode == Keyboard::KeyCode::KEY_2) {
                radio.changeActiveVFO();
                auto& settings = systask.getSettings();
                settings.radioSettings.vfoSelected = radio.getCurrentVFO();
                settings.scheduleSaveIfNeeded();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_3) {
                auto& settings = systask.getSettings();
                uint8_t vfoIndex = (uint8_t)radio.getCurrentVFO();
                bool showingVFO = settings.radioSettings.showVFO[vfoIndex] == Settings::ONOFF::ON;

                if (showingVFO) {
                    channelEntryActive = false;
                    channelEntryValue = 0;
                    showFreqInput = false;
                    freqInput = 0;
                    if (!ensureMemoryChannelList()) {
                        radio.playBeep(Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                        return;
                    }
                    uint16_t channelNumber = resolveActiveMemoryChannel(vfoIndex);

                    Settings::VFO channelData;
                    if (!settings.getChannelData(channelNumber, channelData)) {
                        radio.playBeep(Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                        return;
                    }

                    vfoMemoryBackup[vfoIndex] = radio.getVFO(radio.getCurrentVFO());
                    vfoMemoryBackupValid[vfoIndex] = true;

                    radio.setVFO(radio.getCurrentVFO(), channelData);
                    settings.radioSettings.memory[vfoIndex] = channelNumber;
                    settings.radioSettings.showVFO[vfoIndex] = Settings::ONOFF::OFF;
                    settings.scheduleSaveIfNeeded();
                }
                else {
                    channelEntryActive = false;
                    channelEntryValue = 0;
                    Settings::VFO restoreVFO = settings.radioSettings.vfo[vfoIndex];
                    if (vfoMemoryBackupValid[vfoIndex]) {
                        restoreVFO = vfoMemoryBackup[vfoIndex];
                    }

                    applyActiveVFO(restoreVFO);
                    settings.radioSettings.showVFO[vfoIndex] = Settings::ONOFF::ON;
                    settings.scheduleSaveIfNeeded();
                    vfoMemoryBackupValid[vfoIndex] = false;
                }
            }
            else if (keyCode == Keyboard::KeyCode::KEY_4) {
                popupList.set((uint8_t)vfo.bw, 3, 0, Settings::bandwidthStr, ui.KHZStr);
                popupList.setPopupTitle("BANDWIDTH");
                popupSelected = BANDWIDTH;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_5) {
                popupList.set((uint8_t)vfo.modulation, 3, 0, Settings::modulationStr);
                popupList.setPopupTitle("MODULATION");
                popupSelected = MODULATION;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_6) {
                popupList.set((uint8_t)vfo.power, 3, 0, Settings::powerStr);
                popupList.setPopupTitle("TX POWER");
                popupSelected = POWER;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, radio.getCurrentVFO() == Settings::VFOAB::VFOA ? (uint32_t)Applications::SETVFOA : (uint32_t)Applications::SETVFOB);
            }
        }
    }
}

void MainVFO::refreshMemoryChannelList() {
    auto& settings = systask.getSettings();
    memoryChannelCount = 0;
    for (uint16_t ch = 1; ch <= Settings::MAX_CHANNELS; ++ch) {
        if (settings.isChannelInUse(ch)) {
            memoryChannelList[memoryChannelCount++] = ch;
        }
    }
    memoryChannelListValid = true;
}

bool MainVFO::ensureMemoryChannelList() {
    if (!memoryChannelListValid) {
        refreshMemoryChannelList();
    }
    return memoryChannelCount > 0;
}

bool MainVFO::getNextMemoryChannel(uint16_t currentChannel, int direction, uint16_t& result) {
    if (memoryChannelCount == 0) {
        return false;
    }

    int32_t index = -1;
    for (uint16_t i = 0; i < memoryChannelCount; ++i) {
        if (memoryChannelList[i] == currentChannel) {
            index = static_cast<int32_t>(i);
            break;
        }
    }

    if (index == -1) {
        index = (direction > 0) ? 0 : static_cast<int32_t>(memoryChannelCount) - 1;
    } else if (direction > 0) {
        index = (index + 1) % memoryChannelCount;
    } else {
        index = (index == 0) ? memoryChannelCount - 1 : index - 1;
    }

    result = memoryChannelList[index];
    return true;
}

uint16_t MainVFO::resolveActiveMemoryChannel(uint8_t vfoIndex) {
    auto& settings = systask.getSettings();
    uint16_t stored = settings.radioSettings.memory[vfoIndex];
    if (stored >= 1) {
        for (uint16_t i = 0; i < memoryChannelCount; ++i) {
            if (memoryChannelList[i] == stored) {
                return stored;
            }
        }
    }
    return memoryChannelCount > 0 ? memoryChannelList[0] : 0;
}

void MainVFO::applyActiveVFO(const Settings::VFO& vfo) {
    Settings::VFOAB current = radio.getCurrentVFO();
    radio.setVFO(current, vfo);
    radio.setupToVFO(current);
    auto& settings = systask.getSettings();
    settings.radioSettings.vfo[(uint8_t)current] = vfo;
    settings.scheduleSaveIfNeeded();
}
