#include "audio_utils.h"
#include "globals.h"
#include "config.h"

// --- Mic ---

static int16_t _micBuf[MIC_BUFFER_SIZE];
static float   _micPeak = 0.0f;

void initMic() {
    auto cfg = M5.Mic.config();
    cfg.sample_rate = MIC_SAMPLE_RATE;
    cfg.noise_filter_level = 0;  // disable AGC — auto-gain flattens muzzle-blast peaks
    cfg.magnification = 1;       // raw ADC values; ES8311 hardware gain is already high on S3
    M5.Mic.config(cfg);
    // Do NOT call M5.Mic.begin() here — speaker and mic share the ES8311
    // codec on M5StickS3. Begin mic only when recording is active.
}

void updateMic() {
    if (!M5.Mic.isRunning()) {
        M5.Speaker.end();
        M5.Mic.begin();
    }
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

static void _ensureSpeaker() {
    if (M5.Mic.isRunning()) {
        M5.Mic.end();
    }
    if (!M5.Speaker.isRunning()) {
        M5.Speaker.begin();
        M5.Speaker.setVolume(currentSpeakerVolume);
    }
}

void playTone(int freq, int duration) {
    _ensureSpeaker();
    M5.Speaker.tone(freq, duration);
}

void playFeedbackTone(int freq, int duration) {
    if (!enableUISounds) return;
    _ensureSpeaker();
    M5.Speaker.tone(freq, duration);
}

void playSuccessBeeps() {
    if (!enableUISounds) return;
    _ensureSpeaker();
    int freqs[] = {1047, 1175, 1319, 1397, 1568};
    for (int f : freqs) {
        M5.Speaker.tone(f, BEEP_NOTE_DURATION_MS);
        vTaskDelay(pdMS_TO_TICKS(BEEP_NOTE_DURATION_MS + BEEP_NOTE_DELAY_MS));
    }
}

void playUnsuccessBeeps() {
    if (!enableUISounds) return;
    _ensureSpeaker();
    unsigned long dur = (unsigned long)(BEEP_NOTE_DURATION_MS * 1.5f);
    for (int i = 0; i < 2; i++) {
        M5.Speaker.tone(262, dur);
        vTaskDelay(pdMS_TO_TICKS(dur + BEEP_NOTE_DELAY_MS * 2));
    }
}
