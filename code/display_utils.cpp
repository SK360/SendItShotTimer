#include "display_utils.h"
#include "globals.h" // Access to global variables
#include "config.h"  // Access to constants and enums
#include "logo.h"    // Boot logo image data
#include <float.h>   // Added for FLT_MAX

void displayBootLogo() {
    M5.Lcd.fillScreen(BLACK);
    int x = (M5.Lcd.width() - LOGO_WIDTH) / 2;
    int y = (M5.Lcd.height() - LOGO_HEIGHT) / 2;
    M5.Lcd.pushImage(x, y, LOGO_WIDTH, LOGO_HEIGHT, logo);
}

void displayBootScreen(const char* line1a, const char* line1b, const char* line2) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString(line1a, M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 25);
    M5.Lcd.drawString(line1b, M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 5);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString(line2, M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 25);
}

String getUpButtonLabel() {
    return (M5.Lcd.getRotation() == 3) ? "Top" : "Bottom";
}

String getDownButtonLabel() {
    return (M5.Lcd.getRotation() == 3) ? "Bottom" : "Top";
}

void displayMenu(const char* title, const char* items[], int count, int selection, int scrollOffset) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString(title, M5.Lcd.width() / 2, 10);

    int y_pos = 45;

    if (strcmp(title, "Select Mode") == 0) {
        M5.Lcd.setTextDatum(TR_DATUM);
        M5.Lcd.setTextFont(0);
        M5.Lcd.setTextSize(1);
        int batt_pct = M5.Power.getBatteryLevel();
        String statusText = String(batt_pct) + "%";

        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.drawString(statusText, M5.Lcd.width() - 5, 5);
    }

    M5.Lcd.setTextDatum(TL_DATUM);
    int rotation = M5.Lcd.getRotation();
    int itemsPerScreen = MENU_ITEMS_PER_SCREEN_LANDSCAPE;
    int itemHeight = MENU_ITEM_HEIGHT_LANDSCAPE;
    int textSize = 2;

    M5.Lcd.setTextSize(textSize);

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
                              strcmp(items[i], "Tone Sweep") == 0 ||
                              strcmp(items[i], "Exit") == 0 ||
                              strcmp(items[i], "WiFi Settings") == 0 ||
                              strcmp(items[i], "Settings") == 0);

        bool isParTimeSetting = (settingsMenuLevel == 2 && strncmp(items[i], "Par Time", 8) == 0);

        if (settingsMenuLevel > 0 && !isNavOrAction && !isParTimeSetting) {
            itemText += ": ";
            if (strcmp(items[i], "Max Shots") == 0) { if (currentMaxShots == 0) itemText += "Off"; else itemText += currentMaxShots; }
            else if (strcmp(items[i], "Beep Duration") == 0) itemText += currentBeepDuration;
            else if (strcmp(items[i], "Beep Tone") == 0) itemText += currentBeepToneHz;
            else if (strcmp(items[i], "Shot Threshold") == 0) itemText += shotThresholdRms;
            else if (strcmp(items[i], "Par Beep Count") == 0) itemText += dryFireParBeepCount;
            else if (strcmp(items[i], "Recoil Threshold") == 0) itemText += String(recoilThreshold, 1);
            else if (strcmp(items[i], "Orientation") == 0) itemText += (screenRotationSetting == 1 ? "Right" : "Left");
            else if (strcmp(items[i], "Auto Sleep") == 0) {
                if (autoSleepMinutes == 0) itemText += "Off";
                else { itemText += autoSleepMinutes; itemText += " min"; }
            }
            else if (strcmp(items[i], "UI Sounds") == 0) itemText += (enableUISounds ? "On" : "Off");
            else if (strcmp(items[i], "Volume") == 0) itemText += currentSpeakerVolume;
            else if (strcmp(items[i], "Shot Timeout") == 0) { if (shotTimeoutMs == 0) itemText += "Off"; else { itemText += (shotTimeoutMs / 1000); itemText += "s"; } }
            else if (strcmp(items[i], "Min 1st Shot") == 0) { itemText += minFirstShotTimeMs; itemText += "ms"; }
            else if (strcmp(items[i], "Post Beep Delay") == 0) { itemText += postBeepDelayMs; itemText += "ms"; }
            else if (strcmp(items[i], "Start Delay Min") == 0) { itemText += startDelayMinMs; itemText += "ms"; }
            else if (strcmp(items[i], "Start Delay Max") == 0) { itemText += startDelayMaxMs; itemText += "ms"; }
        }

        if (i == selection) {
            M5.Lcd.setTextColor(BLACK, WHITE);
            M5.Lcd.fillRect(5, display_y - 2, M5.Lcd.width() - 10, (textSize == 1 ? 14 : 20), WHITE);
            M5.Lcd.drawString(itemText, 15, display_y);
            M5.Lcd.setTextColor(WHITE, BLACK);
        } else {
            M5.Lcd.drawString(itemText, 15, display_y);
        }
    }

    if (scrollOffset > 0) {
        M5.Lcd.fillTriangle(M5.Lcd.width() / 2, y_pos - itemHeight/2 - 10, M5.Lcd.width() / 2 - 5, y_pos - itemHeight/2 - 5, M5.Lcd.width() / 2 + 5, y_pos - itemHeight/2 - 5, WHITE);
    }
    if (endIdx < count) {
        M5.Lcd.fillTriangle(M5.Lcd.width() / 2, M5.Lcd.height() - 5, M5.Lcd.width() / 2 - 5, M5.Lcd.height() - 10, M5.Lcd.width() / 2 + 5, M5.Lcd.height() - 10, WHITE);
    }

    drawLowBatteryIndicator();
    M5.Lcd.setTextDatum(TL_DATUM);
}

void displayTimingScreen(float elapsedTime, int count, float lastSplit) {
    static float prevElapsedTime = -1.0f;
    static int prevCount = -1;
    static float prevLastSplit = -1.0f;
    static bool prevLowBattery = false;
    int rotation = M5.Lcd.getRotation();

    bool updateNeeded = redrawMenu ||
                        abs(elapsedTime - prevElapsedTime) > 0.01f ||
                        count != prevCount ||
                        abs(lastSplit - prevLastSplit) > 0.01f ||
                        lowBatteryWarning != prevLowBattery;

    if (redrawMenu) {
        M5.Lcd.fillScreen(BLACK);
    }
    if (!updateNeeded && !redrawMenu) {
        return;
    }

    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setTextDatum(TL_DATUM);

    if (redrawMenu || abs(elapsedTime - prevElapsedTime) > 0.01f) {
        M5.Lcd.setTextFont(7);
        M5.Lcd.setTextSize(1);
        int time_y = 15;
        M5.Lcd.fillRect(5, time_y, M5.Lcd.width() - 10 , M5.Lcd.fontHeight(7) + 4, BLACK);
        M5.Lcd.setCursor(10, time_y);
        M5.Lcd.printf("%.2f", elapsedTime);
        prevElapsedTime = elapsedTime;
    }

    int shots_y = 75;
    int split_y = shots_y + 25;
    int text_size = 2;
    int line_h = (text_size == 1) ? 14 : 20;

    if (redrawMenu || count != prevCount) {
        M5.Lcd.setTextFont(0);
        M5.Lcd.setTextSize(text_size);
        M5.Lcd.fillRect(10, shots_y, M5.Lcd.width() - 20, line_h, BLACK);
        M5.Lcd.setCursor(10, shots_y);
        if (currentMaxShots == 0) M5.Lcd.printf("Shots: %d", count);
        else M5.Lcd.printf("Shots: %d/%d", count, currentMaxShots);
        prevCount = count;
    }

    if (redrawMenu || abs(lastSplit - prevLastSplit) > 0.01f || count != prevCount) {
        M5.Lcd.setTextFont(0);
        M5.Lcd.setTextSize(text_size);
        M5.Lcd.fillRect(10, split_y, M5.Lcd.width() - 20, line_h, BLACK);
        M5.Lcd.setCursor(10, split_y);
        if (count > 0) {
            M5.Lcd.printf("Split: %.2fs", lastSplit);
        } else {
            M5.Lcd.print("Split: ---");
        }
         prevLastSplit = lastSplit;
    }

    if (redrawMenu || lowBatteryWarning != prevLowBattery) {
        M5.Lcd.fillRect(M5.Lcd.width() - 40, 5, 35, 10, BLACK);
        drawLowBatteryIndicator();
        prevLowBattery = lowBatteryWarning;
    }
    redrawMenu = false;
}

void displayStoppedScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setTextDatum(TL_DATUM);
    int text_size = 2;
    int line_h = 22;
    int y_pos = 15;

    M5.Lcd.setTextSize(text_size);

    if (shotCount > 0) {
        float totalTime = (shotTimestamps[shotCount - 1] - startTime) / 1000.0f;
        M5.Lcd.setCursor(10, y_pos);
        M5.Lcd.printf("Total: %.2fs (%d)", totalTime, shotCount);
    } else {
        M5.Lcd.setCursor(10, y_pos);
        M5.Lcd.printf("Total Shots: %d", shotCount);
    }
    y_pos += line_h;

    M5.Lcd.setCursor(10, y_pos);
    if (shotCount > 0) { M5.Lcd.printf("First: %.2fs", splitTimes[0]); }
    else { M5.Lcd.print("First: ---s"); }
    y_pos += line_h;

    M5.Lcd.setCursor(10, y_pos);
     if (shotCount > 1) { M5.Lcd.printf("Last Split: %.2fs", splitTimes[shotCount - 1]); }
     else if (shotCount == 1) { M5.Lcd.print("Last Split: N/A"); }
     else { M5.Lcd.print("Last Split: ---s"); }
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
        M5.Lcd.setCursor(10, y_pos);
        if(fastestSplitIndex != -1) {
             M5.Lcd.printf("Fastest: %.2fs (S%d)", fastestSplit, fastestSplitIndex + 1);
        } else {
             M5.Lcd.print("Fastest: N/A");
        }
        y_pos += line_h;
    } else {
        M5.Lcd.setCursor(10, y_pos);
        M5.Lcd.print("Fastest: N/A");
        y_pos += line_h;
    }

    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.setTextSize(1);
    if (shotCount > 0) {
        M5.Lcd.drawString("Scroll=Shots / Front=Reset", M5.Lcd.width() / 2, M5.Lcd.height() - 5);
    } else {
        M5.Lcd.drawString("Press Front to Reset", M5.Lcd.width() / 2, M5.Lcd.height() - 5);
    }
    drawLowBatteryIndicator();
}

void displayShotReviewScreen(int index) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextColor(WHITE, BLACK);
    int text_size = 2;
    int line_h = 22;

    // Title
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.setTextSize(text_size);
    char title[20];
    snprintf(title, sizeof(title), "Shot %d / %d", index + 1, shotCount);
    M5.Lcd.drawString(title, M5.Lcd.width() / 2, 10);

    // Shot time from buzzer
    int y_pos = 10 + line_h + 8;
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setTextSize(text_size);

    float timeFromStart = (shotTimestamps[index] - startTime) / 1000.0f;
    M5.Lcd.setCursor(10, y_pos);
    M5.Lcd.printf("Time: %.2fs", timeFromStart);
    y_pos += line_h;

    // Split time
    M5.Lcd.setCursor(10, y_pos);
    if (index == 0) {
        M5.Lcd.printf("Draw: %.2fs", splitTimes[0]);
    } else {
        M5.Lcd.printf("Split: %.2fs", splitTimes[index]);
    }

    // Navigation hint
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Scroll=Shots / Front=Reset", M5.Lcd.width() / 2, M5.Lcd.height() - 5);
    drawLowBatteryIndicator();
}

void displayEditScreen() {
    if (!redrawMenu) {
         M5.Lcd.fillRect(0, M5.Lcd.height()/2 - 25, M5.Lcd.width(), 50, BLACK);
    } else {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextDatum(TC_DATUM);
        M5.Lcd.setTextFont(0);
        M5.Lcd.setTextSize(2);
        if (settingBeingEdited == EDIT_PAR_TIME_ARRAY) {
             String titleStr = "Par Time " + String(editingIntValue + 1);
             M5.Lcd.drawString(titleStr, M5.Lcd.width() / 2, 15);
        } else {
            M5.Lcd.drawString(editingSettingName, M5.Lcd.width() / 2, 15);
        }
        M5.Lcd.setTextDatum(BC_DATUM);
        M5.Lcd.setTextFont(0);
        M5.Lcd.setTextSize(1);
        if (settingBeingEdited == EDIT_ROTATION || settingBeingEdited == EDIT_UI_SOUNDS) {
            M5.Lcd.drawString(getUpButtonLabel() + " or " + getDownButtonLabel() + " = Toggle", M5.Lcd.width() / 2, M5.Lcd.height() - 25);
        } else {
            M5.Lcd.drawString(getUpButtonLabel() + "=Up / " + getDownButtonLabel() + "=Down", M5.Lcd.width() / 2, M5.Lcd.height() - 25);
        }
        M5.Lcd.drawString("Press=OK / Hold=Cancel", M5.Lcd.width() / 2, M5.Lcd.height() - 10);
    }

    M5.Lcd.setTextDatum(MC_DATUM);

    switch(settingBeingEdited) {
        case EDIT_MAX_SHOTS:
             if (editingIntValue == 0) {
                 M5.Lcd.setTextFont(4); M5.Lcd.setTextSize(1);
                 M5.Lcd.drawString("Off", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             } else {
                 M5.Lcd.setTextFont(7); M5.Lcd.setTextSize(1);
                 M5.Lcd.drawNumber(editingIntValue, M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             }
             break;
        case EDIT_SHOT_TIMEOUT:
             if (editingIntValue == 0) {
                 M5.Lcd.setTextFont(4); M5.Lcd.setTextSize(1);
                 M5.Lcd.drawString("Off", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             } else {
                 M5.Lcd.setTextFont(7); M5.Lcd.setTextSize(1);
                 M5.Lcd.drawNumber(editingIntValue / 1000, M5.Lcd.width() / 2, M5.Lcd.height() / 2);
                 M5.Lcd.setTextFont(0); M5.Lcd.setTextSize(1);
                 M5.Lcd.drawString("s", M5.Lcd.width() / 2 + M5.Lcd.textWidth(String(editingIntValue / 1000))/2 + 15, M5.Lcd.height() / 2);
             }
             break;
        case EDIT_ROTATION:
             M5.Lcd.setTextFont(4); M5.Lcd.setTextSize(1);
             M5.Lcd.drawString(editingIntValue == 1 ? "Right Hand" : "Left Hand", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             break;
        case EDIT_BEEP_TONE:
        case EDIT_SHOT_THRESHOLD:
        case EDIT_PAR_BEEP_COUNT:
        case EDIT_SPEAKER_VOLUME:
        case EDIT_MIN_FIRST_SHOT:
        case EDIT_POST_BEEP_DELAY:
        case EDIT_START_DELAY_MIN:
        case EDIT_START_DELAY_MAX:
        case EDIT_TONE_SWEEP:
             M5.Lcd.setTextFont(7); M5.Lcd.setTextSize(1);
             M5.Lcd.drawNumber(editingIntValue, M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             if (settingBeingEdited == EDIT_MIN_FIRST_SHOT || settingBeingEdited == EDIT_POST_BEEP_DELAY ||
                 settingBeingEdited == EDIT_START_DELAY_MIN || settingBeingEdited == EDIT_START_DELAY_MAX) {
                M5.Lcd.setTextFont(0); M5.Lcd.setTextSize(1);
                M5.Lcd.drawString("ms", M5.Lcd.width() / 2 + M5.Lcd.textWidth(String(editingIntValue))/2 + 15, M5.Lcd.height() / 2);
             }
             if (settingBeingEdited == EDIT_TONE_SWEEP) {
                M5.Lcd.setTextFont(0); M5.Lcd.setTextSize(1);
                M5.Lcd.drawString("Hz", M5.Lcd.width() / 2 + M5.Lcd.textWidth(String(editingIntValue))/2 + 15, M5.Lcd.height() / 2);
             }
             break;
        case EDIT_BEEP_DURATION:
             M5.Lcd.setTextFont(7); M5.Lcd.setTextSize(1);
             M5.Lcd.drawNumber(editingULongValue, M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             break;
        case EDIT_PAR_TIME_ARRAY:
        case EDIT_RECOIL_THRESHOLD:
             M5.Lcd.setTextFont(7); M5.Lcd.setTextSize(1);
             M5.Lcd.drawFloat(editingFloatValue, 1, M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             break;
        case EDIT_AUTO_SLEEP:
             M5.Lcd.setTextFont(4); M5.Lcd.setTextSize(1);
             if (editingIntValue == 0) {
                 M5.Lcd.drawString("Off", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             } else {
                 M5.Lcd.drawString(String(editingIntValue) + " min", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             }
             break;
        case EDIT_UI_SOUNDS:
             M5.Lcd.setTextFont(4); M5.Lcd.setTextSize(1);
             M5.Lcd.drawString(editingBoolValue ? "On" : "Off", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             break;
        default:
             M5.Lcd.setTextFont(7); M5.Lcd.setTextSize(1);
             M5.Lcd.drawString("ERROR", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
             break;
    }
    drawLowBatteryIndicator();
}

void displayCalibrationScreen(const char* title, float peakValue, const char* unit) {
    if (redrawMenu) { 
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextDatum(TC_DATUM); M5.Lcd.setTextFont(0); M5.Lcd.setTextSize(2);
        M5.Lcd.drawString(title, M5.Lcd.width() / 2, 10);
        M5.Lcd.setTextDatum(BC_DATUM); M5.Lcd.setTextFont(0); M5.Lcd.setTextSize(1);
        M5.Lcd.drawString("Press Front=Save Peak", M5.Lcd.width() / 2, M5.Lcd.height() - 25);
        M5.Lcd.drawString("Hold Front=Cancel", M5.Lcd.width() / 2, M5.Lcd.height() - 10);
        if (currentState == CALIBRATE_RECOIL) {
             M5.Lcd.drawString("Trigger Recoil", M5.Lcd.width()/2, M5.Lcd.height()-45);
        }
    } else { 
        M5.Lcd.fillRect(0, M5.Lcd.height() / 2 - 25, M5.Lcd.width(), 50, BLACK);
    }

    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(3);
    String peakStr = "PEAK: " + String(peakValue, (currentState == CALIBRATE_RECOIL ? 2 : 0)); 
    M5.Lcd.drawString(peakStr, M5.Lcd.width() / 2, (M5.Lcd.height()/2));
    drawLowBatteryIndicator(); 
}


void displayDeviceStatusScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("Device Status", M5.Lcd.width() / 2, 10);

    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setTextSize(1);
    int y_pos = 35;
    int line_h = 12;

    float batt_v = M5.Power.getBatteryVoltage() / 1000.0f;
    int batt_pct = M5.Power.getBatteryLevel();
    bool charging = M5.Power.isCharging();

    M5.Lcd.setCursor(10, y_pos);
    M5.Lcd.printf("Batt: %.2fV (%d%%) %s", batt_v, batt_pct, charging ? "Chg" : "");
    y_pos += line_h;
    M5.Lcd.setCursor(10, y_pos);
    M5.Lcd.printf("Peak V: %.2fV", peakBatteryVoltage);
    y_pos += line_h + 5;

    float accX, accY, accZ, gyroX, gyroY, gyroZ, temp;
    M5.Imu.getAccelData(&accX, &accY, &accZ);

    M5.Lcd.setCursor(10, y_pos);
    M5.Lcd.print("IMU Acc (G):");
    y_pos += line_h;
    M5.Lcd.setCursor(15, y_pos);
    M5.Lcd.printf("X:%.2f, Y:%.2f, Z:%.2f", accX, accY, accZ);
    y_pos += line_h + 5;

    y_pos += line_h;

    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Hold Front to Return", M5.Lcd.width() / 2, M5.Lcd.height() - 10);
    drawLowBatteryIndicator();
}

void displayDryFireReadyScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("Dry Fire Par", M5.Lcd.width() / 2, 30);

    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Press Front to Start", M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 10);
    M5.Lcd.drawString("Hold Top/Front=Exit", M5.Lcd.width() / 2, M5.Lcd.height() - 20);
    drawLowBatteryIndicator();
}

void displayDryFireRunningScreen(bool waiting, int beepNum, int totalBeeps) {
    if (!redrawMenu) return; 

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextFont(0);

    if (waiting) {
        M5.Lcd.setTextSize(3);
        M5.Lcd.drawString("Waiting...", M5.Lcd.width() / 2, M5.Lcd.height() / 2);
    } else {
        M5.Lcd.setTextSize(7);
        M5.Lcd.drawString(String(beepNum), M5.Lcd.width() / 2, M5.Lcd.height() / 2 - 10);
        M5.Lcd.setTextFont(0);
        M5.Lcd.setTextSize(1);
        M5.Lcd.drawString("Beep / " + String(totalBeeps), M5.Lcd.width() / 2, M5.Lcd.height() / 2 + 35);
    }

    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Hold Top/Front=Cancel", M5.Lcd.width() / 2, M5.Lcd.height() - 10);
    drawLowBatteryIndicator();
    redrawMenu = false; 
}

void drawLowBatteryIndicator() {
    if (lowBatteryWarning) {
        M5.Lcd.setTextDatum(TR_DATUM); 
        M5.Lcd.setTextFont(0);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.drawString("(Bat)", M5.Lcd.width() - 5, 5); 
        M5.Lcd.setTextColor(WHITE, BLACK); 
        M5.Lcd.setTextDatum(TL_DATUM); 
    }
}

