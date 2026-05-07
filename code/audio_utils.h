#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <M5Unified.h>
#include "config.h"

// --- Mic ---
void initMic();
void updateMic();
float getMicPeak();
void resetMicPeak();

// --- Audio playback ---
void playTone(int freq, int duration);
void playFeedbackTone(int freq, int duration);
void playSuccessBeeps();
void playUnsuccessBeeps();


#endif // AUDIO_UTILS_H
