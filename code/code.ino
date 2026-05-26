// External Libraries
#include <M5Unified.h>
#include <Preferences.h>
#include <cmath>
#include <float.h>
#include <vector>
#include "driver/gpio.h"
#include "esp_sleep.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Project Headers
#include "config.h"
#include "globals.h"
#include "display_utils.h"
#include "input_handler.h"
#include "timer_modes.h"
#include "audio_utils.h"
#include "nvs_utils.h"
#include "system_utils.h"
#include "ota_utils.h"


// --- Global Variable Definitions ---
TimerState currentState = MODE_SELECTION;
TimerState previousState = MODE_SELECTION;
TimerState stateBeforeEdit = SETTINGS_MENU_MAIN;
OperatingMode currentMode = MODE_LIVE_FIRE;
unsigned long startTime = 0;
unsigned long lastDisplayUpdateTime = 0;
unsigned long lastActivityTime = 0;

int currentMaxShots = 10;
unsigned long currentBeepDuration = 150;
int currentBeepToneHz = 2000;
int shotThresholdRms = 15311;
int dryFireParBeepCount = 3;
float dryFireParTimesSec[MAX_PAR_BEEPS];
float recoilThreshold = 1.5f;
int screenRotationSetting = 3;
int autoSleepMinutes = 1;
bool enableUISounds = true;
int currentSpeakerVolume = DEFAULT_SPEAKER_VOLUME;

int minFirstShotTimeMs = 100;
int postBeepDelayMs = 200;
int startDelayMinMs = 2000;
int startDelayMaxMs = 5000;
int shotTimeoutMs = 15000;

// --- Timer State Variables ---
volatile bool is_listening_active = false;
volatile unsigned long beep_audio_end_time = 0;
unsigned long liveFireDelayEndTime = 0;
bool liveFireWaitingForDelay = false;

int shotCount = 0;
unsigned long shotTimestamps[MAX_SHOTS_LIMIT];
float splitTimes[MAX_SHOTS_LIMIT];
unsigned long lastShotTimestamp = 0;
unsigned long lastDetectionTime = 0;

int currentMenuSelection = 0;
int menuScrollOffset = 0;
int shotReviewIndex = -1;
int settingsMenuLevel = 0;
unsigned long btnTopPressTime = 0;
bool btnTopHeld = false;
bool redrawMenu = true;

EditableSetting settingBeingEdited = EDIT_NONE;
int editingIntValue = 0;
unsigned long editingULongValue = 0;
float editingFloatValue = 0.0f;
bool editingBoolValue = false;
const char* editingSettingName = "";

float currentCyclePeakRMS = 0.0f;
float peakRMSOverall = 0.0f;

Preferences preferences;
float peakBatteryVoltage = 4.2f;
float currentBatteryVoltage = 0.0f;
bool lowBatteryWarning = false;
unsigned long lastBatteryCheckTime = 0;

unsigned long randomDelayStartMs = 0;
unsigned long parTimerStartTime = 0;
unsigned long beepSequenceStartTime = 0;
int beepsPlayed = 0;
unsigned long nextBeepTime = 0;
unsigned long lastBeepTime = 0;

unsigned long lastSoundPeakTime = 0;
bool checkingForRecoil = false;
float peakRecoilValue = 0.0f;


// --- Setup ---
void setup() {
    auto cfg = M5.config();
    cfg.fallback_board = m5::board_t::board_M5StickS3;
    cfg.pmic_button = false;  // don't auto-sleep on power button; we use it for navigation
    M5.begin(cfg);
    M5.Lcd.setBrightness(100);

    preferences.begin(NVS_NAMESPACE, false);
    loadSettings();

    M5.Lcd.setRotation(screenRotationSetting);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextFont(0);

    M5.Speaker.begin();
    M5.Speaker.setVolume(currentSpeakerVolume);
    initMic();

    if (!M5.Imu.begin()) {
        displayBootScreen("WARNING", "", "IMU Init Failed!");
        delay(2000);
    }

    checkBattery();

    displayBootLogo();
    delay(1000);

    M5.Lcd.fillScreen(BLACK);
    displayBootScreen("SendIt", "Shot Timer", "v" FIRMWARE_VERSION);
    playSuccessBeeps();
    delay(500);

    resetActivityTimer();

    delay(1000);
    setState(MODE_SELECTION);
    currentMenuSelection = (int)currentMode;
    menuScrollOffset = 0;
    M5.Lcd.fillScreen(BLACK);
}

// --- Main Loop ---
void loop() {
    M5.update();
    unsigned long currentTime = millis();

    if (autoSleepMinutes > 0 && currentState != OTA_UPDATE) {
        unsigned long autoSleepTimeoutMs = (unsigned long)autoSleepMinutes * 60UL * 1000UL;
        if (currentTime - lastActivityTime > autoSleepTimeoutMs) {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setTextDatum(MC_DATUM);
            M5.Lcd.drawString("Sleeping...", M5.Lcd.width()/2, M5.Lcd.height()/2);
            delay(SLEEP_MESSAGE_DELAY_MS);
            M5.Lcd.setBrightness(0);
            M5.Lcd.sleep();
            M5.Lcd.waitDisplay();

            // M5StickS3 button GPIOs: BtnA=37, BtnB=39, BtnPWR=35
            gpio_wakeup_enable(GPIO_NUM_37, GPIO_INTR_LOW_LEVEL);
            gpio_wakeup_enable(GPIO_NUM_39, GPIO_INTR_LOW_LEVEL);
            gpio_wakeup_enable(GPIO_NUM_35, GPIO_INTR_LOW_LEVEL);
            esp_sleep_enable_gpio_wakeup();
            esp_light_sleep_start();

            M5.Lcd.wakeup();
            M5.Lcd.setBrightness(100);
            delay(200);
            resetActivityTimer();
            redrawMenu = true;
        }
    }

    bool micUpdateNeeded = (currentState == LIVE_FIRE_TIMING ||
                            currentState == NOISY_RANGE_TIMING ||
                            currentState == CALIBRATE_THRESHOLD);
    if (micUpdateNeeded && (is_listening_active || currentState == CALIBRATE_THRESHOLD)) {
        updateMic();
    }

    if (currentTime - lastBatteryCheckTime > BATTERY_CHECK_INTERVAL_MS) {
        checkBattery();
        if (currentState == DEVICE_STATUS || currentState == MODE_SELECTION || lowBatteryWarning) {
            redrawMenu = true;
        }
    }

    if (M5.BtnB.isPressed()) {
        resetActivityTimer();
        if (btnTopPressTime == 0) {
            btnTopPressTime = currentTime;
        } else if (!btnTopHeld && (currentTime - btnTopPressTime > LONG_PRESS_DURATION_MS)) {
            btnTopHeld = true;

            bool exitToModeSelect = (currentState == LIVE_FIRE_READY || currentState == LIVE_FIRE_TIMING || currentState == LIVE_FIRE_STOPPED ||
                                     currentState == DRY_FIRE_READY || currentState == DRY_FIRE_RUNNING ||
                                     currentState == NOISY_RANGE_READY || currentState == NOISY_RANGE_TIMING || currentState == NOISY_RANGE_GET_READY);

            if (exitToModeSelect) {
                playUnsuccessBeeps();
                setState(MODE_SELECTION);
                currentMenuSelection = (int)currentMode;
                menuScrollOffset = 0;
                M5.Lcd.fillScreen(BLACK);
            }
        }
    } else {
        btnTopPressTime = 0;
        btnTopHeld = false;
    }

    switch (currentState) {
        case MODE_SELECTION:          handleModeSelectionInput(); break;
        case LIVE_FIRE_READY:         handleLiveFireReady(); break;
        case LIVE_FIRE_GET_READY:     handleLiveFireGetReady(); break;
        case LIVE_FIRE_TIMING:        handleLiveFireTiming(); break;
        case LIVE_FIRE_STOPPED:
            {
                if (redrawMenu) {
                    if (shotReviewIndex < 0) {
                        displayStoppedScreen();
                    } else {
                        displayShotReviewScreen(shotReviewIndex);
                    }
                    redrawMenu = false;
                }

                if (shotCount > 0 && M5.BtnB.wasClicked()) {
                    resetActivityTimer();
                    // BtnB cycles forward through shots; wraps back to summary
                    if (shotReviewIndex < shotCount - 1) {
                        shotReviewIndex++;
                        M5.Lcd.fillScreen(BLACK);
                        displayShotReviewScreen(shotReviewIndex);
                    } else {
                        shotReviewIndex = -1;
                        M5.Lcd.fillScreen(BLACK);
                        displayStoppedScreen();
                    }
                }

                if (M5.BtnA.wasClicked()) {
                    resetActivityTimer();
                    shotReviewIndex = -1;
                    if (previousState == NOISY_RANGE_TIMING || previousState == NOISY_RANGE_GET_READY || currentMode == MODE_NOISY_RANGE) {
                        setState(NOISY_RANGE_READY);
                    } else if (previousState == DRY_FIRE_RUNNING || currentMode == MODE_DRY_FIRE) {
                        setState(DRY_FIRE_READY);
                    } else {
                        setState(LIVE_FIRE_READY);
                    }
                    M5.Lcd.fillScreen(BLACK);
                }
            }
            break;
        case DRY_FIRE_READY:          handleDryFireReadyInput(); break;
        case DRY_FIRE_RUNNING:        handleDryFireRunning(); break;
        case NOISY_RANGE_READY:       handleNoisyRangeReadyInput(); break;
        case NOISY_RANGE_GET_READY:   handleNoisyRangeGetReady(); break;
        case NOISY_RANGE_TIMING:      handleNoisyRangeTiming(); break;
        case SETTINGS_MENU_MAIN:
        case SETTINGS_MENU_GENERAL:
        case SETTINGS_MENU_BEEP:
        case SETTINGS_MENU_DRYFIRE:
        case SETTINGS_MENU_NOISY:
        case SETTINGS_MENU_DEVICE:    handleSettingsInput(); break;
        case EDIT_SETTING:            handleEditSettingInput(); break;
        case DEVICE_STATUS:           handleDeviceStatusInput(); break;
        case CALIBRATE_THRESHOLD:
        case CALIBRATE_RECOIL:        handleCalibrationInput(currentState); break;
        case OTA_UPDATE:              handleOtaUpdateLoop(); break;
        default: break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}
