#include "input_handler.h"
#include "globals.h"
#include "config.h"
#include "display_utils.h"
#include "nvs_utils.h"
#include "system_utils.h"
#include "audio_utils.h"
#include "ota_utils.h"


void handleModeSelectionInput() {
    const char* modeItems[] = {"Live Fire", "Dry Fire Par", "Noisy Range", "Settings"};
    int modeCount = sizeof(modeItems) / sizeof(modeItems[0]);
    int itemsPerScreen = MENU_ITEMS_PER_SCREEN_LANDSCAPE;

    if (currentMenuSelection < menuScrollOffset) {
        menuScrollOffset = currentMenuSelection; redrawMenu = true;
    } else if (currentMenuSelection >= menuScrollOffset + itemsPerScreen) {
        menuScrollOffset = currentMenuSelection - itemsPerScreen + 1; redrawMenu = true;
    }

    if (redrawMenu) {
        displayMenu("Select Mode", modeItems, modeCount, currentMenuSelection, menuScrollOffset);
        redrawMenu = false;
    }

    if (M5.BtnB.wasClicked()) {
        resetActivityTimer();
        currentMenuSelection = (currentMenuSelection + 1) % modeCount; redrawMenu = true;
    }

    if (M5.BtnA.wasClicked()) {
        resetActivityTimer();
        if (currentMenuSelection == 3) {
            // Settings
            setState(SETTINGS_MENU_MAIN);
            M5.Lcd.fillScreen(BLACK);
            settingsMenuLevel = 0;
            currentMenuSelection = 0;
            menuScrollOffset = 0;
        } else {
            currentMode = (OperatingMode)currentMenuSelection;
            switch (currentMode) {
                case MODE_LIVE_FIRE:   setState(LIVE_FIRE_READY); break;
                case MODE_DRY_FIRE:    setState(DRY_FIRE_READY); break;
                case MODE_NOISY_RANGE: setState(NOISY_RANGE_READY); break;
            }
            M5.Lcd.fillScreen(BLACK);
            menuScrollOffset = 0;
        }
    }
}

void handleSettingsInput() {
    resetActivityTimer();
    const char* title = "Settings";
    const char** items = nullptr;
    int itemCount = 0;
    int itemsPerScreen = MENU_ITEMS_PER_SCREEN_LANDSCAPE;

    static const char* mainItems[] = {"Live Fire", "Dry Fire", "Noisy Range", "Beep Settings", "Device", "Power Off Now", "Exit"};
    static const char* liveFireItems[] = {"Max Shots", "Shot Threshold", "Shot Timeout", "Min 1st Shot", "Start Delay Min", "Start Delay Max", "Calibrate Thresh.", "Back"};
    static const char* beepItems[] = {"Beep Duration", "Beep Tone", "Post Beep Delay", "UI Sounds", "Volume", "Tone Sweep", "Back"};
    static const char* noisyItems[] = {"Recoil Threshold", "Calibrate Recoil", "Back"};
    static const char* deviceItems[] = {"Orientation", "Auto Sleep", "Device Status", "WiFi Settings", "Back"};

    const int maxDryFireItems = 1 + MAX_PAR_BEEPS + 1;
    static const char* dryFireItemsBuffer[maxDryFireItems];
    static String dryFireItemStrings[MAX_PAR_BEEPS];

    switch (settingsMenuLevel) {
        case 0:
            items = mainItems;
            itemCount = sizeof(mainItems) / sizeof(mainItems[0]);
            title = "Settings";
            break;
        case 1:
            items = liveFireItems;
            itemCount = sizeof(liveFireItems) / sizeof(liveFireItems[0]);
            title = "Live Fire Settings";
            break;
        case 2:
            title = "Dry Fire Settings";
            itemCount = 0;
            dryFireItemsBuffer[itemCount++] = "Par Beep Count";
            for (int i = 0; i < dryFireParBeepCount && i < MAX_PAR_BEEPS; ++i) {
                dryFireItemStrings[i] = "Par Time " + String(i + 1) + ": " + String(dryFireParTimesSec[i], 1) + "s";
                dryFireItemsBuffer[itemCount++] = dryFireItemStrings[i].c_str();
            }
            dryFireItemsBuffer[itemCount++] = "Back";
            items = dryFireItemsBuffer;
            break;
        case 3:
            items = noisyItems;
            itemCount = sizeof(noisyItems) / sizeof(noisyItems[0]);
            title = "Noisy Range Settings";
            break;
        case 4:
            items = beepItems;
            itemCount = sizeof(beepItems) / sizeof(beepItems[0]);
            title = "Beep Settings";
            break;
        case 5:
            items = deviceItems;
            itemCount = sizeof(deviceItems) / sizeof(deviceItems[0]);
            title = "Device Settings";
            break;
    }

    if (currentMenuSelection >= itemCount) {
        currentMenuSelection = max(0, itemCount - 1);
        redrawMenu = true;
    }
    if (currentMenuSelection < menuScrollOffset) {
        menuScrollOffset = currentMenuSelection; redrawMenu = true;
    } else if (currentMenuSelection >= menuScrollOffset + itemsPerScreen) {
        menuScrollOffset = currentMenuSelection - itemsPerScreen + 1; redrawMenu = true;
    }

    if (redrawMenu) {
        if (items) { displayMenu(title, items, itemCount, currentMenuSelection, menuScrollOffset); }
        redrawMenu = false;
    }

    if (M5.BtnB.wasClicked()) {
        currentMenuSelection = (currentMenuSelection + 1) % itemCount; redrawMenu = true;
    }

    if (M5.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
         if (settingsMenuLevel == 0) {
             setState(MODE_SELECTION); currentMenuSelection = (int)currentMode; menuScrollOffset = 0;
             M5.Lcd.fillScreen(BLACK);
         } else {
             int oldMenuLevel = settingsMenuLevel;
             settingsMenuLevel = 0;
             switch (oldMenuLevel) {
                 case 1: currentMenuSelection = 0; break; // Live Fire
                 case 2: currentMenuSelection = 1; break; // Dry Fire
                 case 3: currentMenuSelection = 2; break; // Noisy Range
                 case 4: currentMenuSelection = 3; break; // Beep Settings
                 case 5: currentMenuSelection = 4; break; // Device
                 default: currentMenuSelection = 0; break;
             }
             menuScrollOffset = 0; redrawMenu = true;
         }
         return;
    }

    if (M5.BtnA.wasClicked()) {
        bool needsActionRedraw = true;

        if (settingsMenuLevel == 0) {
            if (strcmp(items[currentMenuSelection], "Live Fire") == 0) settingsMenuLevel = 1;
            else if (strcmp(items[currentMenuSelection], "Dry Fire") == 0) settingsMenuLevel = 2;
            else if (strcmp(items[currentMenuSelection], "Noisy Range") == 0) settingsMenuLevel = 3;
            else if (strcmp(items[currentMenuSelection], "Beep Settings") == 0) settingsMenuLevel = 4;
            else if (strcmp(items[currentMenuSelection], "Device") == 0) settingsMenuLevel = 5;
            else if (strcmp(items[currentMenuSelection], "Power Off Now") == 0) {
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setTextDatum(MC_DATUM);
                M5.Lcd.setTextFont(0);
                M5.Lcd.setTextSize(2);
                M5.Lcd.drawString("Power Off?", M5.Lcd.width()/2, M5.Lcd.height()/2 - 15);
                M5.Lcd.setTextSize(1);
                M5.Lcd.drawString("Press=Cancel / Hold=Off", M5.Lcd.width()/2, M5.Lcd.height()/2 + 15);
                while (true) {
                    M5.update();
                    if (M5.BtnA.wasClicked()) {
                        playUnsuccessBeeps();
                        redrawMenu = true;
                        break;
                    }
                    if (M5.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
                        M5.Lcd.fillScreen(BLACK);
                        M5.Lcd.drawString("Powering Off...", M5.Lcd.width()/2, M5.Lcd.height()/2);
                        delay(1500);
                        M5.Power.powerOff();
                    }
                    delay(10);
                }
            }
            else if (strcmp(items[currentMenuSelection], "Exit") == 0) {
                playSuccessBeeps(); setState(MODE_SELECTION);
                currentMenuSelection = (int)currentMode; menuScrollOffset = 0; needsActionRedraw = false;
                M5.Lcd.fillScreen(BLACK);
            }
            currentMenuSelection = 0; menuScrollOffset = 0;
        }
        else if (settingsMenuLevel == 1) {
            editingSettingName = items[currentMenuSelection];
            stateBeforeEdit = SETTINGS_MENU_GENERAL;
            if (strcmp(editingSettingName, "Max Shots") == 0) {
                settingBeingEdited = EDIT_MAX_SHOTS; editingIntValue = currentMaxShots; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Shot Threshold") == 0) {
                settingBeingEdited = EDIT_SHOT_THRESHOLD; editingIntValue = shotThresholdRms; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Shot Timeout") == 0) {
                settingBeingEdited = EDIT_SHOT_TIMEOUT; editingIntValue = shotTimeoutMs; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Min 1st Shot") == 0) {
                settingBeingEdited = EDIT_MIN_FIRST_SHOT; editingIntValue = minFirstShotTimeMs; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Start Delay Min") == 0) {
                settingBeingEdited = EDIT_START_DELAY_MIN; editingIntValue = startDelayMinMs; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Start Delay Max") == 0) {
                settingBeingEdited = EDIT_START_DELAY_MAX; editingIntValue = startDelayMaxMs; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Calibrate Thresh.") == 0) {
                setState(CALIBRATE_THRESHOLD); peakRMSOverall = 0; resetMicPeak(); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Back") == 0) {
                settingsMenuLevel = 0; currentMenuSelection = 0; menuScrollOffset = 0;
            }
        }
        else if (settingsMenuLevel == 2) { 
            stateBeforeEdit = SETTINGS_MENU_DRYFIRE;
            if (strcmp(items[currentMenuSelection], "Par Beep Count") == 0) {
                editingSettingName = items[currentMenuSelection];
                settingBeingEdited = EDIT_PAR_BEEP_COUNT; editingIntValue = dryFireParBeepCount; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strncmp(items[currentMenuSelection], "Par Time", 8) == 0) {
                int parTimeIndex = currentMenuSelection - 1; 
                if (parTimeIndex >= 0 && parTimeIndex < dryFireParBeepCount) {
                    String tempTitle = "Par Time " + String(parTimeIndex + 1);
                    static char parTimeEditTitle[20]; 
                    strncpy(parTimeEditTitle, tempTitle.c_str(), sizeof(parTimeEditTitle) - 1);
                    parTimeEditTitle[sizeof(parTimeEditTitle) - 1] = '\0';
                    editingSettingName = parTimeEditTitle;
                    settingBeingEdited = EDIT_PAR_TIME_ARRAY;
                    editingIntValue = parTimeIndex; 
                    editingFloatValue = dryFireParTimesSec[parTimeIndex];
                    setState(EDIT_SETTING);
                    needsActionRedraw = false;
                    M5.Lcd.fillScreen(BLACK);
                }
            } else if (strcmp(items[currentMenuSelection], "Back") == 0) {
                settingsMenuLevel = 0; currentMenuSelection = 1; menuScrollOffset = 0;
            }
        }
        else if (settingsMenuLevel == 3) {
            editingSettingName = items[currentMenuSelection];
            stateBeforeEdit = SETTINGS_MENU_NOISY;
            if (strcmp(editingSettingName, "Recoil Threshold") == 0) {
                settingBeingEdited = EDIT_RECOIL_THRESHOLD; editingFloatValue = recoilThreshold; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Calibrate Recoil") == 0) {
                setState(CALIBRATE_RECOIL); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK); peakRecoilValue = 0;
            } else if (strcmp(editingSettingName, "Back") == 0) {
                settingsMenuLevel = 0; currentMenuSelection = 2; menuScrollOffset = 0;
            }
        }
        else if (settingsMenuLevel == 4) {
            editingSettingName = items[currentMenuSelection];
            stateBeforeEdit = SETTINGS_MENU_BEEP;
            if (strcmp(editingSettingName, "Beep Duration") == 0) {
                settingBeingEdited = EDIT_BEEP_DURATION; editingULongValue = currentBeepDuration; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Beep Tone") == 0) {
                settingBeingEdited = EDIT_BEEP_TONE; editingIntValue = currentBeepToneHz; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Post Beep Delay") == 0) {
                settingBeingEdited = EDIT_POST_BEEP_DELAY; editingIntValue = postBeepDelayMs; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "UI Sounds") == 0) {
                settingBeingEdited = EDIT_UI_SOUNDS; editingBoolValue = enableUISounds; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Volume") == 0) {
                settingBeingEdited = EDIT_SPEAKER_VOLUME; editingIntValue = currentSpeakerVolume; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Tone Sweep") == 0) {
                settingBeingEdited = EDIT_TONE_SWEEP; editingIntValue = 2000; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Back") == 0) {
                settingsMenuLevel = 0; currentMenuSelection = 3;
                menuScrollOffset = 0;
            }
        }
        else if (settingsMenuLevel == 5) { // Device Settings
            editingSettingName = items[currentMenuSelection];
            stateBeforeEdit = SETTINGS_MENU_DEVICE;
            if (strcmp(editingSettingName, "Orientation") == 0) {
                settingBeingEdited = EDIT_ROTATION; editingIntValue = screenRotationSetting; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Auto Sleep") == 0) {
                settingBeingEdited = EDIT_AUTO_SLEEP; editingIntValue = autoSleepMinutes; setState(EDIT_SETTING); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "Device Status") == 0) {
                setState(DEVICE_STATUS); needsActionRedraw = false; M5.Lcd.fillScreen(BLACK);
            } else if (strcmp(editingSettingName, "WiFi Settings") == 0) {
                setState(OTA_UPDATE); startOtaUpdate(); needsActionRedraw = false;
            } else if (strcmp(editingSettingName, "Back") == 0) {
                settingsMenuLevel = 0; currentMenuSelection = 4; menuScrollOffset = 0;
            }
        }
        if (needsActionRedraw) redrawMenu = true;
    }
}

void handleEditSettingInput() {
    resetActivityTimer();
    bool valueChanged = false;

    bool upPressed   = M5.BtnA.wasClicked();
    bool downPressed = M5.BtnB.wasClicked();

    if (upPressed || downPressed) {
        valueChanged = true;
        int increment = upPressed ? 1 : -1;

        switch(settingBeingEdited) {
            case EDIT_MAX_SHOTS: {
                int newVal = editingIntValue + increment;
                if (newVal > MAX_SHOTS_LIMIT) newVal = 0;
                else if (newVal < 0) newVal = MAX_SHOTS_LIMIT;
                editingIntValue = newVal;
                break;
            }
            case EDIT_BEEP_DURATION: editingULongValue = min(max(editingULongValue + (unsigned long)(increment * 50), 50UL), 2000UL); break;
            case EDIT_BEEP_TONE: editingIntValue = min(max(editingIntValue + (increment * 100), 500), 8000); break;
            case EDIT_SHOT_THRESHOLD: editingIntValue = min(max(editingIntValue + (increment * 500), 100), 32000); break;
            case EDIT_MIN_FIRST_SHOT: editingIntValue = min(max(editingIntValue + (increment * 10), 0), 500); break;
            case EDIT_POST_BEEP_DELAY: editingIntValue = min(max(editingIntValue + (increment * 25), 50), 1000); break;
            case EDIT_PAR_BEEP_COUNT: editingIntValue = min(max(editingIntValue + increment, 1), MAX_PAR_BEEPS); break;
            case EDIT_PAR_TIME_ARRAY: editingFloatValue = min(max(editingFloatValue + (increment * 0.1f), 0.1f), 10.0f); break;
            case EDIT_RECOIL_THRESHOLD: editingFloatValue = min(max(editingFloatValue + (increment * 0.1f), 0.5f), 5.0f); break;
            case EDIT_ROTATION: editingIntValue = (editingIntValue == 1) ? 3 : 1; break;
            case EDIT_AUTO_SLEEP: {
                int idx = 0;
                for (int i = 0; i < AUTO_SLEEP_OPTIONS_COUNT; i++) {
                    if (AUTO_SLEEP_OPTIONS[i] == editingIntValue) { idx = i; break; }
                }
                idx = (idx + increment + AUTO_SLEEP_OPTIONS_COUNT) % AUTO_SLEEP_OPTIONS_COUNT;
                editingIntValue = AUTO_SLEEP_OPTIONS[idx];
                break;
            }
            case EDIT_UI_SOUNDS: editingBoolValue = !editingBoolValue; break;
            case EDIT_SPEAKER_VOLUME:
                editingIntValue = min(max(editingIntValue + (increment * 10), 0), 255);
                M5.Speaker.setVolume(editingIntValue);
                break;
            case EDIT_TONE_SWEEP:
                editingIntValue = min(max(editingIntValue + (increment * 50), 2000), 4000);
                playTone(editingIntValue, 100);
                break;
            case EDIT_START_DELAY_MIN:
                editingIntValue = min(max(editingIntValue + (increment * 250), 0), min(startDelayMaxMs, MAX_START_DELAY_MS));
                break;
            case EDIT_START_DELAY_MAX:
                editingIntValue = min(max(editingIntValue + (increment * 250), startDelayMinMs), MAX_START_DELAY_MS);
                break;
            case EDIT_SHOT_TIMEOUT: {
                if (editingIntValue == 0) {
                    editingIntValue = (increment > 0) ? 5000 : 60000;
                } else {
                    int newVal = editingIntValue + (increment * 1000);
                    if (newVal > 60000) newVal = 0;
                    else if (newVal < 5000) newVal = 0;
                    editingIntValue = newVal;
                }
                break;
            }
            default: valueChanged = false; break;
        }
        if (settingBeingEdited == EDIT_ROTATION) {
            M5.Lcd.setRotation(editingIntValue);
            redrawMenu = true;
        }
        if (valueChanged &&
            settingBeingEdited != EDIT_UI_SOUNDS &&
            settingBeingEdited != EDIT_TONE_SWEEP &&
            settingBeingEdited != EDIT_SPEAKER_VOLUME) {
            playFeedbackTone(2500, 20);
        }
    }

    if (M5.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
        // cancel — restore original rotation if it was changed
        if (settingBeingEdited == EDIT_ROTATION) {
             M5.Lcd.setRotation(screenRotationSetting);
        }
        setState(stateBeforeEdit);
        M5.Lcd.fillScreen(BLACK);
        settingBeingEdited = EDIT_NONE;
        playUnsuccessBeeps();
        return;
    }

    if (M5.BtnB.pressedFor(LONG_PRESS_DURATION_MS)) {
        // save
        switch(settingBeingEdited) {
            case EDIT_MAX_SHOTS: currentMaxShots = editingIntValue; break;
            case EDIT_BEEP_DURATION: currentBeepDuration = editingULongValue; break;
            case EDIT_BEEP_TONE: currentBeepToneHz = editingIntValue; break;
            case EDIT_TONE_SWEEP: currentBeepToneHz = editingIntValue; break;
            case EDIT_SHOT_THRESHOLD: shotThresholdRms = editingIntValue; break;
            case EDIT_MIN_FIRST_SHOT: minFirstShotTimeMs = editingIntValue; break;
            case EDIT_POST_BEEP_DELAY: postBeepDelayMs = editingIntValue; break;
            case EDIT_PAR_BEEP_COUNT: dryFireParBeepCount = editingIntValue; break;
            case EDIT_PAR_TIME_ARRAY:
                if (editingIntValue >= 0 && editingIntValue < MAX_PAR_BEEPS) { 
                    dryFireParTimesSec[editingIntValue] = editingFloatValue;
                }
                break;
            case EDIT_RECOIL_THRESHOLD: recoilThreshold = editingFloatValue; break;
            case EDIT_ROTATION: screenRotationSetting = editingIntValue; break;
            case EDIT_AUTO_SLEEP: autoSleepMinutes = editingIntValue; break;
            case EDIT_UI_SOUNDS: enableUISounds = editingBoolValue; break;
            case EDIT_SPEAKER_VOLUME:
                currentSpeakerVolume = editingIntValue;
                M5.Speaker.setVolume(currentSpeakerVolume);
                break;
            case EDIT_START_DELAY_MIN: startDelayMinMs = editingIntValue; break;
            case EDIT_START_DELAY_MAX: startDelayMaxMs = editingIntValue; break;
            case EDIT_SHOT_TIMEOUT: shotTimeoutMs = editingIntValue; break;
            default: break;
        }
        saveSettings();
        setState(stateBeforeEdit);
        M5.Lcd.fillScreen(BLACK);
        settingBeingEdited = EDIT_NONE;
        playSuccessBeeps();
        return;
    }

    if (redrawMenu || valueChanged) {
        displayEditScreen();
        redrawMenu = false;
    }
}

void handleDeviceStatusInput() {
    resetActivityTimer();
    if (redrawMenu) {
        displayDeviceStatusScreen();
        redrawMenu = false;
    }
    if (M5.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
        setState(SETTINGS_MENU_DEVICE);
        settingsMenuLevel = 5;
        currentMenuSelection = 3;
        menuScrollOffset = 0;
        M5.Lcd.fillScreen(BLACK);
    }
}


void handleCalibrationInput(TimerState calibrationType) {
    resetActivityTimer();
    float currentValue = 0.0f;
    float accX, accY, accZ;
    const char* title = "Calibrating...";
    const char* unit = "";
    bool valueChanged = false;
    int rotation = M5.Lcd.getRotation();
    int itemsPerScreen = MENU_ITEMS_PER_SCREEN_LANDSCAPE;

    if (calibrationType == CALIBRATE_THRESHOLD) {
        title = "Calibrate Threshold";
        unit = "RMS";
        currentValue = getMicPeak();
        if (currentValue > peakRMSOverall) {
            peakRMSOverall = currentValue;
            valueChanged = true;
        }
        resetMicPeak();
    } else if (calibrationType == CALIBRATE_RECOIL) {
        title = "Calibrate Recoil";
        unit = "G";
        M5.Imu.getAccelData(&accX, &accY, &accZ);
        currentValue = abs(accZ);
        if (currentValue > peakRecoilValue) {
            peakRecoilValue = currentValue;
            valueChanged = true;
        }
    }

    if (redrawMenu || valueChanged) {
        if (!redrawMenu && valueChanged) {
             M5.Lcd.fillRect(0, M5.Lcd.height() / 2 - 25, M5.Lcd.width(), 50, BLACK);
        } else {
            M5.Lcd.fillScreen(BLACK);
            displayCalibrationScreen(title, (calibrationType == CALIBRATE_RECOIL ? peakRecoilValue : peakRMSOverall), unit);
        }
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextFont(1);
        M5.Lcd.setTextSize(3);
        String peakStr = "PEAK: " + String((calibrationType == CALIBRATE_RECOIL ? peakRecoilValue : peakRMSOverall), (calibrationType == CALIBRATE_RECOIL ? 2 : 0));
        M5.Lcd.drawString(peakStr, M5.Lcd.width() / 2, (M5.Lcd.height()/2));
        drawLowBatteryIndicator(); 
        redrawMenu = false;
    }

    if (M5.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
        stateBeforeEdit = (calibrationType == CALIBRATE_THRESHOLD) ? SETTINGS_MENU_GENERAL : SETTINGS_MENU_NOISY;
        settingsMenuLevel = (calibrationType == CALIBRATE_THRESHOLD) ? 1 : 3;
        setState(stateBeforeEdit);
        currentMenuSelection = (calibrationType == CALIBRATE_THRESHOLD) ? 5 : 1;
        menuScrollOffset = max(0, currentMenuSelection - itemsPerScreen + 1);
        M5.Lcd.fillScreen(BLACK);
        playUnsuccessBeeps();
    } else if (M5.BtnA.wasClicked()) {
        if (calibrationType == CALIBRATE_THRESHOLD) {
            shotThresholdRms = (int)(peakRMSOverall * 0.75f);
            stateBeforeEdit = SETTINGS_MENU_GENERAL;
            settingsMenuLevel = 1;
            setState(stateBeforeEdit);
            currentMenuSelection = 5;
            menuScrollOffset = max(0, currentMenuSelection - itemsPerScreen + 1);
        } else if (calibrationType == CALIBRATE_RECOIL) {
            recoilThreshold = peakRecoilValue;
            stateBeforeEdit = SETTINGS_MENU_NOISY;
            settingsMenuLevel = 3;
            setState(stateBeforeEdit);
            currentMenuSelection = 1;
            menuScrollOffset = max(0, currentMenuSelection - itemsPerScreen + 1);
        }
        M5.Lcd.fillScreen(BLACK);
        playSuccessBeeps();
    }
}


bool checkTimerExitButtons() {
    return false; 
}
