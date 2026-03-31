#include "display_utils.h"
#include "globals.h" // Access to global variables
#include "config.h"  // Access to constants and enums
#include <float.h>   // Added for FLT_MAX
#include <LittleFS.h> // Added for LittleFS

void displayBootScreen(const char* line1a, const char* line1b, const char* line2) {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextDatum(MC_DATUM);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextSize(2);
    StickCP2.Lcd.drawString(line1a, StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 - 25);
    StickCP2.Lcd.drawString(line1b, StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 - 5);
    StickCP2.Lcd.setTextSize(1);
    StickCP2.Lcd.drawString(line2, StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 + 25);
}

String getUpButtonLabel() {
    int rotation = StickCP2.Lcd.getRotation();
    switch (rotation) {
        case 0: return "Right";
        case 1: return "Top";
        case 2: return "Left";
        case 3: return "Bottom";
        default: return "Top";
    }
}

String getDownButtonLabel() {
    int rotation = StickCP2.Lcd.getRotation();
    switch (rotation) {
        case 0: return "Left";
        case 1: return "Bottom";
        case 2: return "Right";
        case 3: return "Top";
        default: return "Bottom";
    }
}

void displayMenu(const char* title, const char* items[], int count, int selection, int scrollOffset) {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextDatum(TC_DATUM);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextSize(2);
    StickCP2.Lcd.drawString(title, StickCP2.Lcd.width() / 2, 10);

    int y_pos = 45;

    if (strcmp(title, "Select Mode") == 0) {
        StickCP2.Lcd.setTextDatum(TR_DATUM);
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(1);
        int batt_pct = StickCP2.Power.getBatteryLevel();
        String statusText = String(batt_pct) + "%";

        if (a2dp_source.is_connected()){
            statusText = "[B] " + statusText;
        }
        StickCP2.Lcd.setTextColor(WHITE, BLACK);
        StickCP2.Lcd.drawString(statusText, StickCP2.Lcd.width() - 5, 5);
    }
    else if (strcmp(title, "Bluetooth Settings") == 0) {
        StickCP2.Lcd.setTextDatum(TC_DATUM);
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(1);
        String btStatus = "Status: ";
        if (a2dp_source.is_connected()) {
            btStatus += "Connected";
            StickCP2.Lcd.setTextColor(GREEN, BLACK);
        } else {
            btStatus += "Disconnected";
            StickCP2.Lcd.setTextColor(YELLOW, BLACK);
        }
        StickCP2.Lcd.drawString(btStatus, StickCP2.Lcd.width() / 2, 30);
        StickCP2.Lcd.setTextColor(WHITE, BLACK);
        y_pos = 55;
    }

    StickCP2.Lcd.setTextDatum(TL_DATUM);
    int rotation = StickCP2.Lcd.getRotation();
    int itemsPerScreen = (rotation % 2 == 0) ? MENU_ITEMS_PER_SCREEN_PORTRAIT : MENU_ITEMS_PER_SCREEN_LANDSCAPE;
    int itemHeight = (rotation % 2 == 0) ? MENU_ITEM_HEIGHT_PORTRAIT : MENU_ITEM_HEIGHT_LANDSCAPE;
    int textSize = (rotation % 2 == 0) ? 1 : 2;

    StickCP2.Lcd.setTextSize(textSize);

    int startIdx = scrollOffset;
    int endIdx = min(scrollOffset + itemsPerScreen, count);

    for (int i = startIdx; i < endIdx; ++i) {
        int display_y = y_pos + (i - startIdx) * itemHeight;
        String itemText = items[i];

        bool isNavOrAction = (strcmp(items[i], "Back") == 0 ||
                              strcmp(items[i], "Calibrate Thresh.") == 0 ||
                              strcmp(items[i], "Calibrate Recoil") == 0 ||
                              strcmp(items[i], "Device Status") == 0 ||
                              strcmp(items[i], "List Files") == 0 ||
                              strcmp(items[i], "Power Off Now") == 0 ||
                              strcmp(items[i], "Beep Settings") == 0 ||
                              strcmp(items[i], "Bluetooth Settings") == 0 ||
                              strcmp(items[i], "Tone Sweep") == 0 ||
                              strcmp(items[i], "Save & Exit") == 0 ||
                              strcmp(items[i], "Update Firmware") == 0);

        bool isParTimeSetting = (settingsMenuLevel == 2 && strncmp(items[i], "Par Time", 8) == 0);
        bool isBluetoothConnectItem = (settingsMenuLevel == 5 && strcmp(items[i], "Connect") == 0 );
        bool isBluetoothScanItem = (settingsMenuLevel == 5 && strcmp(items[i], "Scan for Devices") == 0);
        bool isBluetoothDisconnectItem = (settingsMenuLevel == 5 && strcmp(items[i], "Disconnect") == 0);

        if (settingsMenuLevel > 0 && !isNavOrAction && !isParTimeSetting && !isBluetoothConnectItem && !isBluetoothScanItem && !isBluetoothDisconnectItem) {
            itemText += ": ";
            if (strcmp(items[i], "Max Shots") == 0) { if (currentMaxShots == 0) itemText += "Off"; else itemText += currentMaxShots; }
            else if (strcmp(items[i], "Beep Duration") == 0) itemText += currentBeepDuration;
            else if (strcmp(items[i], "Beep Tone") == 0) itemText += currentBeepToneHz;
            else if (strcmp(items[i], "Shot Threshold") == 0) itemText += shotThresholdRms;
            else if (strcmp(items[i], "Par Beep Count") == 0) itemText += dryFireParBeepCount;
            else if (strcmp(items[i], "Recoil Threshold") == 0) itemText += String(recoilThreshold, 1);
            else if (strcmp(items[i], "Screen Rotation") == 0) itemText += screenRotationSetting;
            else if (strcmp(items[i], "Boot Animation") == 0) itemText += (playBootAnimation ? "On" : "Off");
            else if (strcmp(items[i], "Auto Sleep") == 0) itemText += (enableAutoSleep ? "On" : "Off");

            else if (strcmp(items[i], "Min 1st Shot") == 0) { itemText += minFirstShotTimeMs; itemText += "ms"; }
            else if (strcmp(items[i], "Post Beep Delay") == 0) { itemText += postBeepDelayMs; itemText += "ms"; }
            else if (strcmp(items[i], "Start Delay Min") == 0) { itemText += startDelayMinMs; itemText += "ms"; }
            else if (strcmp(items[i], "Start Delay Max") == 0) { itemText += startDelayMaxMs; itemText += "ms"; }
            else if (settingsMenuLevel == 5 && strcmp(items[i], "Auto Reconnect") == 0) {
                itemText += (currentBluetoothAutoReconnect ? "On" : "Off");
            }
            else if (settingsMenuLevel == 5 && strcmp(items[i], "Volume") == 0) {
                itemText += currentBluetoothVolume;
            }
            else if (settingsMenuLevel == 5 && strcmp(items[i], "BT Audio Offset") == 0) { 
                itemText += currentBluetoothAudioOffsetMs;
                itemText += "ms";
            }
        }

        if (i == selection) {
            StickCP2.Lcd.setTextColor(BLACK, WHITE);
            StickCP2.Lcd.fillRect(5, display_y - 2, StickCP2.Lcd.width() - 10, (textSize == 1 ? 14 : 20), WHITE);
            StickCP2.Lcd.drawString(itemText, 15, display_y);
            StickCP2.Lcd.setTextColor(WHITE, BLACK);
        } else {
            StickCP2.Lcd.drawString(itemText, 15, display_y);
        }
    }

    if (scrollOffset > 0) {
        StickCP2.Lcd.fillTriangle(StickCP2.Lcd.width() / 2, y_pos - itemHeight/2 - ( (strcmp(title, "Bluetooth Settings") == 0) ? 5 : 10 ), StickCP2.Lcd.width() / 2 - 5, y_pos - itemHeight/2 - ( (strcmp(title, "Bluetooth Settings") == 0) ? 0 : 5 ), StickCP2.Lcd.width() / 2 + 5, y_pos - itemHeight/2 - ( (strcmp(title, "Bluetooth Settings") == 0) ? 0 : 5 ), WHITE);
    }
    if (endIdx < count) {
        StickCP2.Lcd.fillTriangle(StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 5, StickCP2.Lcd.width() / 2 - 5, StickCP2.Lcd.height() - 10, StickCP2.Lcd.width() / 2 + 5, StickCP2.Lcd.height() - 10, WHITE);
    }

    drawLowBatteryIndicator();
    StickCP2.Lcd.setTextDatum(TL_DATUM);
}

void displayTimingScreen(float elapsedTime, int count, float lastSplit) {
    static float prevElapsedTime = -1.0f;
    static int prevCount = -1;
    static float prevLastSplit = -1.0f;
    static bool prevLowBattery = false;
    int rotation = StickCP2.Lcd.getRotation();

    bool updateNeeded = redrawMenu ||
                        abs(elapsedTime - prevElapsedTime) > 0.01f ||
                        count != prevCount ||
                        abs(lastSplit - prevLastSplit) > 0.01f ||
                        lowBatteryWarning != prevLowBattery;

    if (redrawMenu) {
        StickCP2.Lcd.fillScreen(BLACK);
    }
    if (!updateNeeded && !redrawMenu) {
        return;
    }

    StickCP2.Lcd.setTextColor(WHITE, BLACK);
    StickCP2.Lcd.setTextDatum(TL_DATUM);

    if (redrawMenu || abs(elapsedTime - prevElapsedTime) > 0.01f) {
        StickCP2.Lcd.setTextFont(7);
        StickCP2.Lcd.setTextSize(1);
        int time_y = (rotation % 2 == 0) ? 20 : 15;
        StickCP2.Lcd.fillRect(5, time_y, StickCP2.Lcd.width() - 10 , StickCP2.Lcd.fontHeight(7) + 4, BLACK);
        StickCP2.Lcd.setCursor(10, time_y);
        StickCP2.Lcd.printf("%.2f", elapsedTime);
        prevElapsedTime = elapsedTime;
    }

    int shots_y = (rotation % 2 == 0) ? 80 : 75;
    int split_y = shots_y + ((rotation % 2 == 0) ? 20 : 25);
    int text_size = (rotation % 2 == 0) ? 1 : 2;
    int line_h = (text_size == 1) ? 14 : 20;

    if (redrawMenu || count != prevCount) {
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(text_size);
        StickCP2.Lcd.fillRect(10, shots_y, StickCP2.Lcd.width() - 20, line_h, BLACK);
        StickCP2.Lcd.setCursor(10, shots_y);
        if (currentMaxShots == 0) StickCP2.Lcd.printf("Shots: %d", count);
        else StickCP2.Lcd.printf("Shots: %d/%d", count, currentMaxShots);
        prevCount = count;
    }

    if (redrawMenu || abs(lastSplit - prevLastSplit) > 0.01f || count != prevCount) {
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(text_size);
        StickCP2.Lcd.fillRect(10, split_y, StickCP2.Lcd.width() - 20, line_h, BLACK);
        StickCP2.Lcd.setCursor(10, split_y);
        if (count > 0) {
            StickCP2.Lcd.printf("Split: %.2fs", lastSplit);
        } else {
            StickCP2.Lcd.print("Split: ---");
        }
         prevLastSplit = lastSplit;
    }

    if (redrawMenu || lowBatteryWarning != prevLowBattery) {
        StickCP2.Lcd.fillRect(StickCP2.Lcd.width() - 40, 5, 35, 10, BLACK);
        drawLowBatteryIndicator();
        prevLowBattery = lowBatteryWarning;
    }
    redrawMenu = false;
}

void displayStoppedScreen() {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextColor(WHITE, BLACK);
    StickCP2.Lcd.setTextDatum(TL_DATUM);
    int rotation = StickCP2.Lcd.getRotation();
    int text_size = (rotation % 2 == 0) ? 1 : 2;
    int line_h = (text_size == 1) ? 16 : 22;
    int y_pos = 15;

    StickCP2.Lcd.setTextSize(text_size);

    if (shotCount > 0) {
        float totalTime = (shotTimestamps[shotCount - 1] - startTime) / 1000.0f;
        StickCP2.Lcd.setCursor(10, y_pos);
        StickCP2.Lcd.printf("Total: %.2fs (%d)", totalTime, shotCount);
    } else {
        StickCP2.Lcd.setCursor(10, y_pos);
        StickCP2.Lcd.printf("Total Shots: %d", shotCount);
    }
    y_pos += line_h;

    StickCP2.Lcd.setCursor(10, y_pos);
    if (shotCount > 0) { StickCP2.Lcd.printf("First: %.2fs", splitTimes[0]); }
    else { StickCP2.Lcd.print("First: ---s"); }
    y_pos += line_h;

    StickCP2.Lcd.setCursor(10, y_pos);
     if (shotCount > 1) { StickCP2.Lcd.printf("Last Split: %.2fs", splitTimes[shotCount - 1]); }
     else if (shotCount == 1) { StickCP2.Lcd.print("Last Split: N/A"); }
     else { StickCP2.Lcd.print("Last Split: ---s"); }
    y_pos += line_h;

    if (shotCount > 1) {
        float fastestSplit = FLT_MAX; 
        int fastestSplitIndex = -1;
        for (int i = 1; i < shotCount; ++i) { 
            if (splitTimes[i] < fastestSplit && splitTimes[i] > 0.0f) { 
                fastestSplit = splitTimes[i];
                fastestSplitIndex = i;
            }
        }
        StickCP2.Lcd.setCursor(10, y_pos);
        if(fastestSplitIndex != -1) {
             StickCP2.Lcd.printf("Fastest: %.2fs (S%d)", fastestSplit, fastestSplitIndex + 1);
        } else {
             StickCP2.Lcd.print("Fastest: N/A");
        }
        y_pos += line_h;
    } else {
        StickCP2.Lcd.setCursor(10, y_pos);
        StickCP2.Lcd.print("Fastest: N/A");
        y_pos += line_h;
    }

    StickCP2.Lcd.setTextDatum(BC_DATUM);
    StickCP2.Lcd.setTextSize(1);
    if (shotCount > 0) {
        StickCP2.Lcd.drawString("Scroll=Shots / Front=Reset", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 5);
    } else {
        StickCP2.Lcd.drawString("Press Front to Reset", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 5);
    }
    drawLowBatteryIndicator();
}

void displayShotReviewScreen(int index) {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextColor(WHITE, BLACK);
    int rotation = StickCP2.Lcd.getRotation();
    int text_size = (rotation % 2 == 0) ? 1 : 2;
    int line_h = (text_size == 1) ? 16 : 22;

    // Title
    StickCP2.Lcd.setTextDatum(TC_DATUM);
    StickCP2.Lcd.setTextSize(text_size);
    char title[20];
    snprintf(title, sizeof(title), "Shot %d / %d", index + 1, shotCount);
    StickCP2.Lcd.drawString(title, StickCP2.Lcd.width() / 2, 10);

    // Shot time from buzzer
    int y_pos = 10 + line_h + 8;
    StickCP2.Lcd.setTextDatum(TL_DATUM);
    StickCP2.Lcd.setTextSize(text_size);

    float timeFromStart = (shotTimestamps[index] - startTime) / 1000.0f;
    StickCP2.Lcd.setCursor(10, y_pos);
    StickCP2.Lcd.printf("Time: %.2fs", timeFromStart);
    y_pos += line_h;

    // Split time
    StickCP2.Lcd.setCursor(10, y_pos);
    if (index == 0) {
        StickCP2.Lcd.printf("Draw: %.2fs", splitTimes[0]);
    } else {
        StickCP2.Lcd.printf("Split: %.2fs", splitTimes[index]);
    }

    // Navigation hint
    StickCP2.Lcd.setTextDatum(BC_DATUM);
    StickCP2.Lcd.setTextSize(1);
    StickCP2.Lcd.drawString("Scroll=Shots / Front=Reset", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 5);
    drawLowBatteryIndicator();
}

void displayEditScreen() {
    if (!redrawMenu) {
         StickCP2.Lcd.fillRect(0, StickCP2.Lcd.height()/2 - 25, StickCP2.Lcd.width(), 50, BLACK);
    } else {
        StickCP2.Lcd.fillScreen(BLACK);
        StickCP2.Lcd.setTextDatum(TC_DATUM);
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(2);
        if (settingBeingEdited == EDIT_PAR_TIME_ARRAY) {
             String titleStr = "Par Time " + String(editingIntValue + 1);
             StickCP2.Lcd.drawString(titleStr, StickCP2.Lcd.width() / 2, 15);
        } else {
            StickCP2.Lcd.drawString(editingSettingName, StickCP2.Lcd.width() / 2, 15);
        }
        StickCP2.Lcd.setTextDatum(BC_DATUM);
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(1);
        if (settingBeingEdited == EDIT_BOOT_ANIM || settingBeingEdited == EDIT_AUTO_SLEEP || settingBeingEdited == EDIT_BT_AUTO_RECONNECT) {
            StickCP2.Lcd.drawString(getUpButtonLabel() + " or " + getDownButtonLabel() + " = Toggle", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 25);
        } else {
            StickCP2.Lcd.drawString(getUpButtonLabel() + "=Up / " + getDownButtonLabel() + "=Down", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 25);
        }
        StickCP2.Lcd.drawString("Press=OK / Hold=Cancel", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 10);
    }

    StickCP2.Lcd.setTextDatum(MC_DATUM);

    switch(settingBeingEdited) {
        case EDIT_MAX_SHOTS:
             if (editingIntValue == 0) {
                 StickCP2.Lcd.setTextFont(4); StickCP2.Lcd.setTextSize(1);
                 StickCP2.Lcd.drawString("Off", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
             } else {
                 StickCP2.Lcd.setTextFont(7); StickCP2.Lcd.setTextSize(1);
                 StickCP2.Lcd.drawNumber(editingIntValue, StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
             }
             break;
        case EDIT_BEEP_TONE:
        case EDIT_SHOT_THRESHOLD:
        case EDIT_PAR_BEEP_COUNT:
        case EDIT_ROTATION:
        case EDIT_BT_VOLUME:
        case EDIT_BT_AUDIO_OFFSET: 
        case EDIT_MIN_FIRST_SHOT:
        case EDIT_POST_BEEP_DELAY:
        case EDIT_START_DELAY_MIN:
        case EDIT_START_DELAY_MAX:
        case EDIT_TONE_SWEEP:
             StickCP2.Lcd.setTextFont(7); StickCP2.Lcd.setTextSize(1);
             StickCP2.Lcd.drawNumber(editingIntValue, StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
             if (settingBeingEdited == EDIT_BT_AUDIO_OFFSET || settingBeingEdited == EDIT_MIN_FIRST_SHOT || settingBeingEdited == EDIT_POST_BEEP_DELAY ||
                 settingBeingEdited == EDIT_START_DELAY_MIN || settingBeingEdited == EDIT_START_DELAY_MAX) {
                StickCP2.Lcd.setTextFont(0); StickCP2.Lcd.setTextSize(1); 
                StickCP2.Lcd.drawString("ms", StickCP2.Lcd.width() / 2 + StickCP2.Lcd.textWidth(String(editingIntValue))/2 + 15, StickCP2.Lcd.height() / 2);
             }
             if (settingBeingEdited == EDIT_TONE_SWEEP) { 
                StickCP2.Lcd.setTextFont(0); StickCP2.Lcd.setTextSize(1); 
                StickCP2.Lcd.drawString("Hz", StickCP2.Lcd.width() / 2 + StickCP2.Lcd.textWidth(String(editingIntValue))/2 + 15, StickCP2.Lcd.height() / 2);
             }
             break;
        case EDIT_BEEP_DURATION:
             StickCP2.Lcd.setTextFont(7); StickCP2.Lcd.setTextSize(1);
             StickCP2.Lcd.drawNumber(editingULongValue, StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
             break;
        case EDIT_PAR_TIME_ARRAY:
        case EDIT_RECOIL_THRESHOLD:
             StickCP2.Lcd.setTextFont(7); StickCP2.Lcd.setTextSize(1);
             StickCP2.Lcd.drawFloat(editingFloatValue, 1, StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
             break;
        case EDIT_BOOT_ANIM:
        case EDIT_AUTO_SLEEP:
        case EDIT_BT_AUTO_RECONNECT:
             StickCP2.Lcd.setTextFont(4); StickCP2.Lcd.setTextSize(1);
             StickCP2.Lcd.drawString(editingBoolValue ? "On" : "Off", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
             break;
        default:
             StickCP2.Lcd.setTextFont(7); StickCP2.Lcd.setTextSize(1);
             StickCP2.Lcd.drawString("ERROR", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
             break;
    }
    drawLowBatteryIndicator();
}

void displayCalibrationScreen(const char* title, float peakValue, const char* unit) {
    if (redrawMenu) { 
        StickCP2.Lcd.fillScreen(BLACK);
        StickCP2.Lcd.setTextDatum(TC_DATUM); StickCP2.Lcd.setTextFont(0); StickCP2.Lcd.setTextSize(2);
        StickCP2.Lcd.drawString(title, StickCP2.Lcd.width() / 2, 10);
        StickCP2.Lcd.setTextDatum(BC_DATUM); StickCP2.Lcd.setTextFont(0); StickCP2.Lcd.setTextSize(1);
        StickCP2.Lcd.drawString("Press Front=Save Peak", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 25);
        StickCP2.Lcd.drawString("Hold Front=Cancel", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 10);
        if (currentState == CALIBRATE_RECOIL) {
             StickCP2.Lcd.drawString("Trigger Recoil", StickCP2.Lcd.width()/2, StickCP2.Lcd.height()-45);
        }
    } else { 
        StickCP2.Lcd.fillRect(0, StickCP2.Lcd.height() / 2 - 25, StickCP2.Lcd.width(), 50, BLACK);
    }

    StickCP2.Lcd.setTextDatum(MC_DATUM);
    StickCP2.Lcd.setTextFont(1);
    StickCP2.Lcd.setTextSize(3);
    String peakStr = "PEAK: " + String(peakValue, (currentState == CALIBRATE_RECOIL ? 2 : 0)); 
    StickCP2.Lcd.drawString(peakStr, StickCP2.Lcd.width() / 2, (StickCP2.Lcd.height()/2));
    drawLowBatteryIndicator(); 
}


void displayDeviceStatusScreen() {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextDatum(TC_DATUM);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextSize(2);
    StickCP2.Lcd.drawString("Device Status", StickCP2.Lcd.width() / 2, 10);

    StickCP2.Lcd.setTextDatum(TL_DATUM);
    StickCP2.Lcd.setTextSize(1);
    int y_pos = 35;
    int line_h = 12;

    float batt_v = StickCP2.Power.getBatteryVoltage() / 1000.0f;
    int batt_pct = StickCP2.Power.getBatteryLevel();
    bool charging = StickCP2.Power.isCharging();

    StickCP2.Lcd.setCursor(10, y_pos);
    StickCP2.Lcd.printf("Batt: %.2fV (%d%%) %s", batt_v, batt_pct, charging ? "Chg" : "");
    y_pos += line_h;
    StickCP2.Lcd.setCursor(10, y_pos);
    StickCP2.Lcd.printf("Peak V: %.2fV", peakBatteryVoltage);
    y_pos += line_h + 5;

    float accX, accY, accZ, gyroX, gyroY, gyroZ, temp;
    StickCP2.Imu.getAccelData(&accX, &accY, &accZ);

    StickCP2.Lcd.setCursor(10, y_pos);
    StickCP2.Lcd.print("IMU Acc (G):");
    y_pos += line_h;
    StickCP2.Lcd.setCursor(15, y_pos);
    StickCP2.Lcd.printf("X:%.2f, Y:%.2f, Z:%.2f", accX, accY, accZ);
    y_pos += line_h + 5;

    StickCP2.Lcd.setCursor(10, y_pos);
    if (filesystem_ok_for_boot) { 
        size_t totalBytes = LittleFS.totalBytes();
        size_t usedBytes = LittleFS.usedBytes();
        StickCP2.Lcd.printf("LittleFS: %u/%u B used", usedBytes, totalBytes);
    } else {
        StickCP2.Lcd.print("LittleFS: Not Mounted!");
    }
    y_pos += line_h;

    StickCP2.Lcd.setTextDatum(BC_DATUM);
    StickCP2.Lcd.setTextSize(1);
    StickCP2.Lcd.drawString("Hold Front to Return", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 10);
    drawLowBatteryIndicator();
}

void displayListFilesScreen() {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextDatum(TC_DATUM);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextSize(2);
    StickCP2.Lcd.drawString("LittleFS Files", StickCP2.Lcd.width() / 2, 10);

    StickCP2.Lcd.setTextDatum(TL_DATUM);
    StickCP2.Lcd.setTextSize(1);
    int y_pos = 35;
    int line_h = 12;
    int rotation = StickCP2.Lcd.getRotation();
    int itemsPerScreen = (rotation % 2 == 0) ? MENU_ITEMS_PER_SCREEN_PORTRAIT + 2 : MENU_ITEMS_PER_SCREEN_LANDSCAPE + 1;

    if (fileListCount == 0) {
        StickCP2.Lcd.setCursor(10, y_pos);
        StickCP2.Lcd.print("No files found or");
        y_pos += line_h;
        StickCP2.Lcd.setCursor(10, y_pos);
        StickCP2.Lcd.print("LittleFS error.");
    } else {
        int startIdx = fileListScrollOffset;
        int endIdx = min(fileListScrollOffset + itemsPerScreen, fileListCount);

        for (int i = startIdx; i < endIdx; ++i) {
            int display_y = y_pos + (i - startIdx) * line_h;
            StickCP2.Lcd.setCursor(5, display_y);
            String displayName = fileListNames[i];
            if (displayName.length() > 20) {
                displayName = displayName.substring(0, 17) + "...";
            }
            StickCP2.Lcd.printf("%-20s %6d B", displayName.c_str(), (int)fileListSizes[i]);
        }

        if (fileListScrollOffset > 0) {
             StickCP2.Lcd.fillTriangle(StickCP2.Lcd.width() / 2, 28, StickCP2.Lcd.width() / 2 - 4, 33, StickCP2.Lcd.width() / 2 + 4, 33, WHITE);
        }
        if (endIdx < fileListCount) {
             StickCP2.Lcd.fillTriangle(StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 15, StickCP2.Lcd.width() / 2 - 4, StickCP2.Lcd.height() - 20, StickCP2.Lcd.width() / 2 + 4, StickCP2.Lcd.height() - 20, WHITE);
        }
    }

    StickCP2.Lcd.setTextDatum(BC_DATUM);
    StickCP2.Lcd.setTextSize(1);
    StickCP2.Lcd.drawString("Hold Front to Return", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 5);
    drawLowBatteryIndicator();
}

void displayDryFireReadyScreen() {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextDatum(MC_DATUM);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextSize(2);
    StickCP2.Lcd.drawString("Dry Fire Par", StickCP2.Lcd.width() / 2, 30);

    StickCP2.Lcd.setTextSize(1);
    StickCP2.Lcd.drawString("Press Front to Start", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 + 10);
    StickCP2.Lcd.drawString("Hold Top/Front=Exit", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 20);
    drawLowBatteryIndicator();
}

void displayDryFireRunningScreen(bool waiting, int beepNum, int totalBeeps) {
    if (!redrawMenu) return; 

    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextDatum(MC_DATUM);
    StickCP2.Lcd.setTextFont(0);

    if (waiting) {
        StickCP2.Lcd.setTextSize(3);
        StickCP2.Lcd.drawString("Waiting...", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
    } else {
        StickCP2.Lcd.setTextSize(7);
        StickCP2.Lcd.drawString(String(beepNum), StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 - 10);
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(1);
        StickCP2.Lcd.drawString("Beep / " + String(totalBeeps), StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 + 35);
    }

    StickCP2.Lcd.setTextDatum(BC_DATUM);
    StickCP2.Lcd.setTextSize(1);
    StickCP2.Lcd.drawString("Hold Top/Front=Cancel", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 10);
    drawLowBatteryIndicator();
    redrawMenu = false; 
}

void drawLowBatteryIndicator() {
    if (lowBatteryWarning) {
        StickCP2.Lcd.setTextDatum(TR_DATUM); 
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(1);
        StickCP2.Lcd.setTextColor(RED, BLACK);
        StickCP2.Lcd.drawString("(Bat)", StickCP2.Lcd.width() - 5, 5); 
        StickCP2.Lcd.setTextColor(WHITE, BLACK); 
        StickCP2.Lcd.setTextDatum(TL_DATUM); 
    }
}

void displayBluetoothScanResults() {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextDatum(TC_DATUM);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextSize(2);
    // Dynamically change title based on scan state
    if (scanInProgress) {
        StickCP2.Lcd.drawString("Scanning...", StickCP2.Lcd.width() / 2, 10);
    } else {
        StickCP2.Lcd.drawString("Scan Results", StickCP2.Lcd.width() / 2, 10);
    }
    

    StickCP2.Lcd.setTextDatum(TL_DATUM);
    int y_pos = 35;
    int itemsPerScreen = MENU_ITEMS_PER_SCREEN_PORTRAIT + 2;
    int itemHeight = MENU_ITEM_HEIGHT_PORTRAIT - 3;
    int textSize = 1;

    StickCP2.Lcd.setTextSize(textSize);

    if (discoveredBtDevices.empty() && !scanInProgress) { // Show "No devices" only if scan is finished
        StickCP2.Lcd.setTextDatum(MC_DATUM);
        StickCP2.Lcd.drawString("No devices found.", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
        StickCP2.Lcd.drawString("Hold Front to go Back.", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 + 15);
         StickCP2.Lcd.setTextDatum(TL_DATUM); 
    } else {
        int startIdx = scanMenuScrollOffset;
        int endIdx = min((int)(scanMenuScrollOffset + itemsPerScreen), (int)discoveredBtDevices.size());

        for (int i = startIdx; i < endIdx; ++i) {
            if (i >= discoveredBtDevices.size()) break;

            int display_y = y_pos + (i - startIdx) * itemHeight;
            String deviceName = discoveredBtDevices[i].name;
            if (deviceName.isEmpty()) {
                deviceName = discoveredBtDevices[i].address; // Fallback to address
            }
            int maxDisplayChars = (StickCP2.Lcd.width() - 20) / 6; 
            if (deviceName.length() > maxDisplayChars && maxDisplayChars > 3) { 
                 deviceName = deviceName.substring(0, maxDisplayChars - 3) + "...";
            }

            if (i == scanMenuSelection) {
                StickCP2.Lcd.setTextColor(BLACK, WHITE);
                StickCP2.Lcd.fillRect(5, display_y - 2, StickCP2.Lcd.width() - 10, itemHeight + 1 , WHITE);
                StickCP2.Lcd.drawString(deviceName, 10, display_y);
                StickCP2.Lcd.setTextColor(WHITE, BLACK);
            } else {
                StickCP2.Lcd.drawString(deviceName, 10, display_y);
            }
        }
        
        // Show instructions only when scan is finished
        if (!scanInProgress) {
            StickCP2.Lcd.setTextDatum(BC_DATUM);
            StickCP2.Lcd.setTextSize(1);
            StickCP2.Lcd.drawString("Press=Connect / Hold=Back", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 5);
            StickCP2.Lcd.setTextDatum(TL_DATUM); 
        } else {
             // Optionally show different text while scanning, e.g., "Scanning... Hold=Cancel"
            StickCP2.Lcd.setTextDatum(BC_DATUM);
            StickCP2.Lcd.setTextSize(1);
            StickCP2.Lcd.drawString("Scanning... Hold=Cancel", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 5);
            StickCP2.Lcd.setTextDatum(TL_DATUM); 
        }
    }

    // Scroll indicators
    if (scanMenuScrollOffset > 0) {
        StickCP2.Lcd.fillTriangle(StickCP2.Lcd.width() / 2, 28, StickCP2.Lcd.width() / 2 - 4, 33, StickCP2.Lcd.width() / 2 + 4, 33, WHITE);
    }
    if (scanMenuScrollOffset + itemsPerScreen < discoveredBtDevices.size() && !discoveredBtDevices.empty()) {
        StickCP2.Lcd.fillTriangle(StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 15, StickCP2.Lcd.width() / 2 - 4, StickCP2.Lcd.height() - 20, StickCP2.Lcd.width() / 2 + 4, StickCP2.Lcd.height() - 20, WHITE);
    }

    drawLowBatteryIndicator();
    StickCP2.Lcd.setTextDatum(TL_DATUM); 
}
