#ifndef TIMER_MODES_H
#define TIMER_MODES_H

#include <M5Unified.h>

void handleLiveFireReady();
void handleLiveFireGetReady();
void handleLiveFireTiming();
// LIVE_FIRE_STOPPED is handled in main loop's state machine, but its display is in display_utils

void handleDryFireReadyInput(); // Renamed from handleDryFireReady for consistency
void handleDryFireRunning();

void handleNoisyRangeReadyInput(); // Renamed from handleNoisyRangeReady
void handleNoisyRangeGetReady();
void handleNoisyRangeTiming();

void resetShotData();

#endif // TIMER_MODES_H
