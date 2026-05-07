#ifndef GLOBALS_H
#define GLOBALS_H

#include <M5Unified.h>
#include <vector>
#include <Preferences.h>
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// --- Global Variables ---
extern TimerState currentState;
extern TimerState previousState;
extern TimerState stateBeforeEdit;
extern OperatingMode currentMode;
extern unsigned long startTime;
extern unsigned long lastDisplayUpdateTime;
extern unsigned long lastActivityTime;

// Settings Variables
extern int currentMaxShots;
extern unsigned long currentBeepDuration;
extern int currentBeepToneHz;
extern int shotThresholdRms;
extern int dryFireParBeepCount;
extern float dryFireParTimesSec[MAX_PAR_BEEPS];
extern float recoilThreshold;
extern int screenRotationSetting;
extern int autoSleepMinutes;
extern bool enableUISounds;
extern int currentSpeakerVolume;

extern int minFirstShotTimeMs;
extern int postBeepDelayMs;
extern int startDelayMinMs;
extern int startDelayMaxMs;
extern int shotTimeoutMs;

// --- Timer State Variables ---
extern volatile bool is_listening_active;
extern volatile unsigned long beep_audio_end_time;
extern unsigned long liveFireDelayEndTime;
extern bool liveFireWaitingForDelay;

// Shot Data Arrays
extern int shotCount;
extern unsigned long shotTimestamps[MAX_SHOTS_LIMIT];
extern float splitTimes[MAX_SHOTS_LIMIT];
extern unsigned long lastShotTimestamp;
extern unsigned long lastDetectionTime;

// Menu Variables
extern int currentMenuSelection;
extern int menuScrollOffset;
extern int shotReviewIndex;
extern int settingsMenuLevel;
extern unsigned long btnTopPressTime;
extern bool btnTopHeld;
extern bool redrawMenu;

// Editing Variables
extern EditableSetting settingBeingEdited;
extern int editingIntValue;
extern unsigned long editingULongValue;
extern float editingFloatValue;
extern bool editingBoolValue;
extern const char* editingSettingName;

// Audio Level Data
extern float currentCyclePeakRMS;
extern float peakRMSOverall;

// Battery Monitoring
extern Preferences preferences;
extern float peakBatteryVoltage;
extern float currentBatteryVoltage;
extern bool lowBatteryWarning;
extern unsigned long lastBatteryCheckTime;

// Dry Fire Par Variables
extern unsigned long randomDelayStartMs;
extern unsigned long parTimerStartTime;
extern unsigned long beepSequenceStartTime;
extern int beepsPlayed;
extern unsigned long nextBeepTime;
extern unsigned long lastBeepTime;

// Noisy Range Variables
extern unsigned long lastSoundPeakTime;
extern bool checkingForRecoil;
extern float peakRecoilValue;


#endif // GLOBALS_H
