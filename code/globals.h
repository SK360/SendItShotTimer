#ifndef GLOBALS_H
#define GLOBALS_H

#include <M5StickCPlus2.h>
#include "M5MicPeakRMS.h"
#include "BluetoothA2DPSource.h"
#include <ESP32BluetoothScanner.h>
#include <vector>
#include <Preferences.h>
#include "config.h" // For enum types and BuzzerRequest struct
#include <freertos/FreeRTOS.h> // For FreeRTOS types
#include <freertos/task.h>
#include <freertos/queue.h>

// --- Global Variables ---
extern TimerState currentState;
extern TimerState previousState;
extern TimerState stateBeforeEdit;
extern TimerState stateBeforeScan;
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
extern bool playBootAnimation;
extern bool enableAutoSleep;
extern bool showTotalTime;
extern int minFirstShotTimeMs;
extern int postBeepDelayMs;

// --- Bluetooth Variables ---
extern BluetoothA2DPSource a2dp_source;
extern String currentBluetoothDeviceName;
extern bool currentBluetoothAutoReconnect;
extern int currentBluetoothVolume;
extern int currentBluetoothAudioOffsetMs; 
extern bool bluetoothJustConnected;
extern bool bluetoothJustDisconnected;

// Volatile variables for A2DP audio callback, managed by audio_utils and bluetooth_utils
extern volatile int btBeepFrequency;
extern volatile unsigned long btBeepScheduledStartTime; 
extern volatile unsigned int btBeepDurationVolatile;   
extern volatile bool new_bt_beep_request;              
extern volatile bool current_bt_beep_is_active;        
extern volatile unsigned long current_bt_beep_actual_end_time; 

// --- Timer State Variables ---
extern volatile bool is_listening_active;      // Flag to enable/disable mic reading after start beep
extern volatile unsigned long beep_audio_end_time; // Calculated time when start beep audio should be finished


// Bluetooth Scanner Variables
extern ESP32BluetoothScanner btScanner;
extern std::vector<BTDevice> discoveredBtDevices;
extern int scanMenuSelection;
extern int scanMenuScrollOffset;
extern bool scanInProgress;
extern unsigned long scanStartTime;

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

// File List Variables
extern String fileListNames[MAX_FILES_LIST];
extern size_t fileListSizes[MAX_FILES_LIST];
extern int fileListCount;
extern int fileListScrollOffset;

// Audio Level Data
extern float currentCyclePeakRMS;
extern float peakRMSOverall;

// Battery Monitoring
extern Preferences preferences;
extern float peakBatteryVoltage;
extern float currentBatteryVoltage;
extern bool lowBatteryWarning;
extern unsigned long lastBatteryCheckTime;

// Create an instance of the library
extern M5MicPeakRMS micPeakRMS;

// Boot Sequence Variables
extern int currentJpgFrame;
extern bool filesystem_ok_for_boot;
extern unsigned long lastFrameTime;

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

// AVRC Metadata
extern const char *avrc_metadata[];

// --- FreeRTOS Handles ---
extern QueueHandle_t buzzerQueue; 
extern TaskHandle_t buzzerTaskHandle; 


#endif // GLOBALS_H
