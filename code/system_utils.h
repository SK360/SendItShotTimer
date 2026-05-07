#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <M5Unified.h>
#include "config.h" // For TimerState enum
// For sleep functions
#include "driver/rtc_io.h"
#include "esp_sleep.h"


void setState(TimerState newState);
void resetActivityTimer();
void checkBattery();
// setupSleep and wakeUp functions could be here if more complex

#endif // SYSTEM_UTILS_H
