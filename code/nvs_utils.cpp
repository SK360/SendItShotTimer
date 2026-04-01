#include "nvs_utils.h"
#include "globals.h" // Access to global variables and preferences object
#include "config.h"  // Access to NVS_NAMESPACE and KEY_ constants

void loadSettings() {
    preferences.begin(NVS_NAMESPACE, false); // Open NVS

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
    // Validate: must be one of the allowed values (0, 1, 2, 5, 10)
    bool validSleep = false;
    for (int i = 0; i < AUTO_SLEEP_OPTIONS_COUNT; i++) {
        if (autoSleepMinutes == AUTO_SLEEP_OPTIONS[i]) { validSleep = true; break; }
    }
    if (!validSleep) autoSleepMinutes = 1;

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

    currentBluetoothDeviceName = preferences.getString(KEY_BT_DEVICE_NAME, "LEXON MINO L");
    currentBluetoothAutoReconnect = preferences.getBool(KEY_BT_AUTO_RECONNECT, false);
    currentBluetoothVolume = preferences.getInt(KEY_BT_VOLUME, 80);
    currentBluetoothAudioOffsetMs = preferences.getInt(KEY_BT_AUDIO_OFFSET, 0);

    peakBatteryVoltage = preferences.getFloat(KEY_PEAK_BATT, 4.2f); // Also load peak battery here

    // preferences.end(); // Keep NVS open if frequently accessed, or close if done for now
}

void saveSettings() {
    // preferences.begin(NVS_NAMESPACE, false); // Ensure NVS is open if not already

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

    preferences.putInt(KEY_MIN_FIRST_SHOT, minFirstShotTimeMs);
    preferences.putInt(KEY_POST_BEEP_DELAY, postBeepDelayMs);
    preferences.putInt(KEY_START_DELAY_MIN, startDelayMinMs);
    preferences.putInt(KEY_START_DELAY_MAX, startDelayMaxMs);

    preferences.putString(KEY_BT_DEVICE_NAME, currentBluetoothDeviceName);
    preferences.putBool(KEY_BT_AUTO_RECONNECT, currentBluetoothAutoReconnect);
    preferences.putInt(KEY_BT_VOLUME, currentBluetoothVolume);
    preferences.putInt(KEY_BT_AUDIO_OFFSET, currentBluetoothAudioOffsetMs);

    // preferences.end(); // Close NVS if done with batch of writes
}

void savePeakVoltage(float voltage) {
    // preferences.begin(NVS_NAMESPACE, false); // Ensure NVS is open
    preferences.putFloat(KEY_PEAK_BATT, voltage);
    // preferences.end();
}
