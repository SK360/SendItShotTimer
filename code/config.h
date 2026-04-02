#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> // For String, PI etc.

// --- Firmware Version ---
#define FIRMWARE_VERSION "1.0.0"

// --- Configuration Constants (These are generally safe in headers as const) ---
const unsigned long LONG_PRESS_DURATION_MS = 750;
const unsigned long SHOT_REFRACTORY_MS = 150;
const unsigned long TIMEOUT_DURATION_MS = 15000;
const unsigned long BEEP_NOTE_DURATION_MS = 150;
const unsigned long BEEP_NOTE_DELAY_MS = 50;
const unsigned long BATTERY_CHECK_INTERVAL_MS = 60000;
const float BATTERY_LOW_PERCENTAGE = 0.78f;
const int MAX_SHOTS_LIMIT = 99;
const int MENU_ITEM_HEIGHT_LANDSCAPE = 25;
const int MENU_ITEM_HEIGHT_PORTRAIT = 18;
const int MENU_ITEMS_PER_SCREEN_LANDSCAPE = 3;
const int MENU_ITEMS_PER_SCREEN_PORTRAIT = 5;
const unsigned long DEFAULT_POST_BEEP_DELAY_MS = 200; // Default post-beep delay before timing starts
const unsigned long MESSAGE_DISPLAY_MS = 2000;
const unsigned long DRY_FIRE_RANDOM_DELAY_MIN_MS = 2000;
const unsigned long DRY_FIRE_RANDOM_DELAY_MAX_MS = 5000;
const int MAX_PAR_BEEPS = 10;
const unsigned long RECOIL_DETECTION_WINDOW_MS = 100;
const unsigned long DEFAULT_MIN_FIRST_SHOT_TIME_MS = 100; // Default min time after start for first shot
const int DEFAULT_START_DELAY_MIN_MS = 2000;
const int DEFAULT_START_DELAY_MAX_MS = 5000;
const int MAX_START_DELAY_MS = 10000;
const int AUTO_SLEEP_OPTIONS[] = {0, 1, 2, 5, 10};
const int AUTO_SLEEP_OPTIONS_COUNT = 5;
const unsigned long SLEEP_MESSAGE_DELAY_MS = 1500;
const unsigned long WIFI_TIMEOUT_MS = 10UL * 60UL * 1000UL; // 10 minutes
const unsigned long BT_SCAN_DURATION_S = 10;
const int MAX_BT_DEVICES_DISPLAY = 20;
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 100;
const int BT_AUDIO_OFFSET_STEP_MS = 50; 
const int BUZZER_QUEUE_LENGTH = 10; 
const int BUZZER_TASK_STACK_SIZE = 2048; 

// --- Buzzer Pins (External) ---
#define BUZZER_PIN 25
#define BUZZER_PIN_2 2

// --- NVS Keys (Declarations) ---
extern const char* NVS_NAMESPACE;
extern const char* KEY_MAX_SHOTS;
extern const char* KEY_BEEP_DUR;
extern const char* KEY_BEEP_HZ;
extern const char* KEY_SHOT_THRESH;
extern const char* KEY_DF_BEEP_CNT;
extern const char* KEY_NR_RECOIL;
extern const char* KEY_PEAK_BATT;
extern const char* KEY_ROTATION;
extern const char* KEY_AUTO_SLEEP;
extern const char* KEY_BT_DEVICE_NAME;
extern const char* KEY_BT_AUTO_RECONNECT;
extern const char* KEY_BT_VOLUME;
extern const char* KEY_BT_AUDIO_OFFSET;
extern const char* KEY_UI_SOUNDS;

extern const char* KEY_MIN_FIRST_SHOT;
extern const char* KEY_POST_BEEP_DELAY;
extern const char* KEY_START_DELAY_MIN;
extern const char* KEY_START_DELAY_MAX;

// --- Timer States ---
enum TimerState {
    MODE_SELECTION,
    LIVE_FIRE_READY,
    LIVE_FIRE_GET_READY,
    LIVE_FIRE_TIMING,
    LIVE_FIRE_STOPPED,
    DRY_FIRE_READY,
    DRY_FIRE_RUNNING,
    NOISY_RANGE_READY,
    NOISY_RANGE_GET_READY,
    NOISY_RANGE_TIMING,
    SETTINGS_MENU_MAIN,
    SETTINGS_MENU_GENERAL,
    SETTINGS_MENU_BEEP,
    SETTINGS_MENU_DRYFIRE,
    SETTINGS_MENU_NOISY,
    SETTINGS_MENU_BLUETOOTH,
    SETTINGS_MENU_DEVICE,
    BLUETOOTH_SCANNING,
    DEVICE_STATUS,
    EDIT_SETTING,
    CALIBRATE_THRESHOLD,
    CALIBRATE_RECOIL,
    OTA_UPDATE
};

// --- Operating Modes ---
enum OperatingMode {
    MODE_LIVE_FIRE,
    MODE_DRY_FIRE,
    MODE_NOISY_RANGE
};

// --- Editable Settings Enum ---
enum EditableSetting {
    EDIT_NONE,
    EDIT_MAX_SHOTS,
    EDIT_BEEP_DURATION,
    EDIT_BEEP_TONE,
    EDIT_SHOT_THRESHOLD,
    EDIT_PAR_BEEP_COUNT,
    EDIT_PAR_TIME_ARRAY,
    EDIT_RECOIL_THRESHOLD,
    EDIT_ROTATION,
    EDIT_AUTO_SLEEP,
    EDIT_BT_AUTO_RECONNECT,
    EDIT_BT_VOLUME,
    EDIT_BT_AUDIO_OFFSET,

    EDIT_MIN_FIRST_SHOT,
    EDIT_POST_BEEP_DELAY,
    EDIT_TONE_SWEEP,
    EDIT_UI_SOUNDS,
    EDIT_START_DELAY_MIN,
    EDIT_START_DELAY_MAX
};

// --- Struct for Buzzer Task Queue ---
typedef struct {
    int frequency;
    int duration;
} BuzzerRequest;


#endif // CONFIG_H
