#include "nvs_utils.h"
#include "globals.h"
#include "config.h"

void loadSettings() {
    preferences.begin(NVS_NAMESPACE, false);

    currentMaxShots = preferences.getInt(KEY_MAX_SHOTS, 10);
    if (currentMaxShots > MAX_SHOTS_LIMIT) currentMaxShots = MAX_SHOTS_LIMIT;
    else if (currentMaxShots < 0) currentMaxShots = 0;

    currentBeepDuration = preferences.getULong(KEY_BEEP_DUR, 150);
    currentBeepToneHz = preferences.getInt(KEY_BEEP_HZ, 2000);
    shotThresholdRms = preferences.getInt(KEY_SHOT_THRESH, 15311);
    dryFireParBeepCount = preferences.getInt(KEY_DF_BEEP_CNT, 3);
    if (dryFireParBeepCount < 1) dryFireParBeepCount = 1;
    if (dryFireParBeepCount > MAX_PAR_BEEPS) dryFireParBeepCount = MAX_PAR_BEEPS;

    for (int i = 0; i < MAX_PAR_BEEPS; ++i) {
        char key[12];
        sprintf(key, "dfParT_%d", i);
        dryFireParTimesSec[i] = preferences.getFloat(key, 1.0f);
    }

    recoilThreshold = preferences.getFloat(KEY_NR_RECOIL, 1.5f);
    screenRotationSetting = preferences.getInt(KEY_ROTATION, 3);
    if (screenRotationSetting != 1 && screenRotationSetting != 3) screenRotationSetting = 3;
    autoSleepMinutes = preferences.getInt(KEY_AUTO_SLEEP, 1);
    bool validSleep = false;
    for (int i = 0; i < AUTO_SLEEP_OPTIONS_COUNT; i++) {
        if (autoSleepMinutes == AUTO_SLEEP_OPTIONS[i]) { validSleep = true; break; }
    }
    if (!validSleep) autoSleepMinutes = 1;
    enableUISounds = preferences.getBool(KEY_UI_SOUNDS, true);
    currentSpeakerVolume = preferences.getInt(KEY_SPEAKER_VOLUME, DEFAULT_SPEAKER_VOLUME);
    if (currentSpeakerVolume < 0) currentSpeakerVolume = 0;
    if (currentSpeakerVolume > 255) currentSpeakerVolume = 255;

    minFirstShotTimeMs = preferences.getInt(KEY_MIN_FIRST_SHOT, DEFAULT_MIN_FIRST_SHOT_TIME_MS);
    if (minFirstShotTimeMs < 0) minFirstShotTimeMs = 0;
    if (minFirstShotTimeMs > 500) minFirstShotTimeMs = 500;
    postBeepDelayMs = preferences.getInt(KEY_POST_BEEP_DELAY, DEFAULT_POST_BEEP_DELAY_MS);
    if (postBeepDelayMs < 50) postBeepDelayMs = 50;
    if (postBeepDelayMs > 1000) postBeepDelayMs = 1000;

    startDelayMinMs = preferences.getInt(KEY_START_DELAY_MIN, DEFAULT_START_DELAY_MIN_MS);
    if (startDelayMinMs < 0) startDelayMinMs = 0;
    if (startDelayMinMs > MAX_START_DELAY_MS) startDelayMinMs = MAX_START_DELAY_MS;
    startDelayMaxMs = preferences.getInt(KEY_START_DELAY_MAX, DEFAULT_START_DELAY_MAX_MS);
    if (startDelayMaxMs < 0) startDelayMaxMs = 0;
    if (startDelayMaxMs > MAX_START_DELAY_MS) startDelayMaxMs = MAX_START_DELAY_MS;
    if (startDelayMaxMs < startDelayMinMs) startDelayMaxMs = startDelayMinMs;

    shotTimeoutMs = preferences.getInt(KEY_SHOT_TIMEOUT, (int)DEFAULT_TIMEOUT_DURATION_MS);
    if (shotTimeoutMs < 0) shotTimeoutMs = 0;
    if (shotTimeoutMs > 60000) shotTimeoutMs = 60000;
    if (shotTimeoutMs > 0 && shotTimeoutMs < 5000) shotTimeoutMs = 5000;

    peakBatteryVoltage = preferences.getFloat(KEY_PEAK_BATT, 4.2f);
}

void saveSettings() {
    preferences.putInt(KEY_MAX_SHOTS, currentMaxShots);
    preferences.putULong(KEY_BEEP_DUR, currentBeepDuration);
    preferences.putInt(KEY_BEEP_HZ, currentBeepToneHz);
    preferences.putInt(KEY_SHOT_THRESH, shotThresholdRms);
    preferences.putInt(KEY_DF_BEEP_CNT, dryFireParBeepCount);
    for (int i = 0; i < MAX_PAR_BEEPS; ++i) {
        char key[12];
        sprintf(key, "dfParT_%d", i);
        preferences.putFloat(key, dryFireParTimesSec[i]);
    }
    preferences.putFloat(KEY_NR_RECOIL, recoilThreshold);
    preferences.putInt(KEY_ROTATION, screenRotationSetting);
    preferences.putInt(KEY_AUTO_SLEEP, autoSleepMinutes);
    preferences.putBool(KEY_UI_SOUNDS, enableUISounds);
    preferences.putInt(KEY_SPEAKER_VOLUME, currentSpeakerVolume);

    preferences.putInt(KEY_MIN_FIRST_SHOT, minFirstShotTimeMs);
    preferences.putInt(KEY_POST_BEEP_DELAY, postBeepDelayMs);
    preferences.putInt(KEY_START_DELAY_MIN, startDelayMinMs);
    preferences.putInt(KEY_START_DELAY_MAX, startDelayMaxMs);
    preferences.putInt(KEY_SHOT_TIMEOUT, shotTimeoutMs);
}

void savePeakVoltage(float voltage) {
    preferences.putFloat(KEY_PEAK_BATT, voltage);
}
