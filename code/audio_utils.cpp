#include "audio_utils.h"
#include "globals.h"
#include "config.h"

// --- Mic ---

static int16_t _micBuf[MIC_BUFFER_SIZE];
static float   _micPeak = 0.0f;

void initMic() {
    auto cfg = M5.Mic.config();
    cfg.sample_rate = MIC_SAMPLE_RATE;
    // Disable AGC — auto-gain flattens muzzle-blast peaks.
    // Fix analog gain at a known level; recalibrate threshold empirically.
    cfg.noise_filter_level = 0;
    M5.Mic.config(cfg);
    M5.Mic.begin();
}

void updateMic() {
    if (!M5.Mic.record(_micBuf, MIC_BUFFER_SIZE, MIC_SAMPLE_RATE)) return;
    for (int i = 0; i < MIC_BUFFER_SIZE; i++) {
        float v = (float)abs(_micBuf[i]);
        if (v > _micPeak) _micPeak = v;
    }
}

float getMicPeak() {
    return _micPeak;
}

void resetMicPeak() {
    _micPeak = 0.0f;
}

// --- Audio playback ---

void playTone(int freq, int duration) {
    M5.Speaker.tone(freq, duration);
}

void playFeedbackTone(int freq, int duration) {
    if (!enableUISounds) return;
    M5.Speaker.tone(freq, duration);
}

void playSuccessBeeps() {
    if (!enableUISounds) return;
    int freqs[] = {1047, 1175, 1319, 1397, 1568};
    for (int f : freqs) {
        M5.Speaker.tone(f, BEEP_NOTE_DURATION_MS);
        vTaskDelay(pdMS_TO_TICKS(BEEP_NOTE_DURATION_MS + BEEP_NOTE_DELAY_MS));
    }
}

void playUnsuccessBeeps() {
    if (!enableUISounds) return;
    unsigned long dur = (unsigned long)(BEEP_NOTE_DURATION_MS * 1.5f);
    for (int i = 0; i < 2; i++) {
        M5.Speaker.tone(262, dur);
        vTaskDelay(pdMS_TO_TICKS(dur + BEEP_NOTE_DELAY_MS * 2));
    }
}
