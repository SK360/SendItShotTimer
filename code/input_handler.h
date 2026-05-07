#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <M5Unified.h>
#include "config.h" // For TimerState enum

void handleModeSelectionInput();
void handleSettingsInput();
void handleEditSettingInput();
void handleDeviceStatusInput();
void handleCalibrationInput(TimerState calibrationType);
bool checkTimerExitButtons(); // Though its logic is now mainly global

#endif // INPUT_HANDLER_H
