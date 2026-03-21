#include "system_utils.h"
#include "globals.h"
#include "config.h"
#include "audio_utils.h" // For playUnsuccessBeeps on low battery
#include "nvs_utils.h"   // For savePeakVoltage

void setState(TimerState newState) {
    if (currentState != newState) {
        previousState = currentState;
        currentState = newState;
        redrawMenu = true;
        if (newState == LIVE_FIRE_STOPPED) {
            shotReviewIndex = -1;
        }
    }
}

void resetActivityTimer() {
    lastActivityTime = millis();
}

void checkBattery() {
    currentBatteryVoltage = StickCP2.Power.getBatteryVoltage() / 1000.0f;

    if (currentBatteryVoltage > peakBatteryVoltage && currentBatteryVoltage > 3.0f && !StickCP2.Power.isCharging()) { // Only update peak if not charging and voltage is plausible
        peakBatteryVoltage = currentBatteryVoltage;
        savePeakVoltage(peakBatteryVoltage);
    }

    bool previousWarningState = lowBatteryWarning;
    // Use a fixed reasonable value if peakBatteryVoltage is not yet well-calibrated or too low
    float referenceVoltage = (peakBatteryVoltage > 3.3f) ? peakBatteryVoltage : 4.15f;
    lowBatteryWarning = (currentBatteryVoltage < (referenceVoltage * BATTERY_LOW_PERCENTAGE));


    if (lowBatteryWarning && !previousWarningState) {
        playUnsuccessBeeps(); // Warning sound
        redrawMenu = true; // Force redraw of screens that show battery status
    } else if (!lowBatteryWarning && previousWarningState) {
         redrawMenu = true; // Force redraw if status changes back
    }
    lastBatteryCheckTime = millis();
}
