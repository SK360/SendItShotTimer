#include "timer_modes.h"
#include "globals.h"
#include "config.h" 
#include "display_utils.h"
#include "audio_utils.h"
#include "system_utils.h" 

void resetShotData() {
    shotCount = 0;
    lastShotTimestamp = 0;
    lastDetectionTime = 0;
    currentCyclePeakRMS = 0.0f;
    peakRMSOverall = 0.0f;
    micPeakRMS.resetPeak();
    checkingForRecoil = false;
    lastSoundPeakTime = 0;
    for (int i = 0; i < MAX_SHOTS_LIMIT; ++i) {
        shotTimestamps[i] = 0;
        splitTimes[i] = 0.0f;
    }
}

void handleLiveFireReady() {
    if (redrawMenu) {
        displayTimingScreen(0.0, 0, 0.0);
        redrawMenu = false;
    }
    if (StickCP2.BtnA.wasClicked()) {
        resetActivityTimer();
        reset_bt_beep_state();
        is_listening_active = false;
        StickCP2.Lcd.fillScreen(BLACK);
        StickCP2.Lcd.setTextDatum(MC_DATUM);
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(3);
        StickCP2.Lcd.drawString("Ready...", StickCP2.Lcd.width()/2, StickCP2.Lcd.height()/2);
        randomSeed(micros());
        unsigned long randomDelay = random(startDelayMinMs, startDelayMaxMs + 1);
        liveFireDelayEndTime = millis() + randomDelay;
        liveFireWaitingForDelay = true;
        setState(LIVE_FIRE_GET_READY);
    }
}

void handleLiveFireGetReady() {
    if (liveFireWaitingForDelay) {
        if (StickCP2.BtnA.wasClicked()) {
            liveFireWaitingForDelay = false;
            setState(LIVE_FIRE_READY);
            StickCP2.Lcd.fillScreen(BLACK);
            playUnsuccessBeeps();
            return;
        }
        if (millis() < liveFireDelayEndTime) {
            return;
        }
        liveFireWaitingForDelay = false;
    }
    resetActivityTimer();
    unsigned long beepInitiationTime = millis();
    playTone(currentBeepToneHz, currentBeepDuration); // Only plays on BT if connected, otherwise queues for buzzer
    
    // Calculate when the audio output (whichever is active) is expected to finish.
    // If BT connected, consider offset and duration. If buzzer, consider duration.
    unsigned long audioDuration = currentBeepDuration;
    unsigned long audioStartTime = beepInitiationTime; // Buzzer starts now
    if (a2dp_source.is_connected()) {
        audioStartTime = beepInitiationTime + currentBluetoothAudioOffsetMs; // BT starts later/earlier
    }
    // Use max to handle negative offsets correctly for end time calculation
    beep_audio_end_time = max(beepInitiationTime, audioStartTime) + audioDuration + 150; // Add 150ms buffer

    is_listening_active = false; // Don't listen until beep is finished

    // Set timer start time relative to beep initiation + standard delay
    startTime = beepInitiationTime + postBeepDelayMs; 
    
    delay(postBeepDelayMs); // Wait for the standard post-beep delay
    
    resetShotData(); 
    lastDisplayUpdateTime = 0;
    StickCP2.Lcd.fillScreen(BLACK);
    setState(LIVE_FIRE_TIMING);
}

void handleLiveFireTiming() {
    unsigned long currentTime = millis();

    if (currentState != LIVE_FIRE_TIMING) return; 

    // --- Check if listening should become active ---
    if (!is_listening_active) {
        // Check if enough time has passed since the calculated end of beep audio
        // AND ensure the timer has actually started (passed postBeepDelayMs)
        if (currentTime >= beep_audio_end_time && currentTime >= startTime) { 
            is_listening_active = true;
            micPeakRMS.resetPeak(); // Reset peak *just* as listening starts
        } else {
            // Still waiting for beep audio to finish or for startTime, don't process mic input
             if (redrawMenu || currentTime - lastDisplayUpdateTime >= DISPLAY_UPDATE_INTERVAL_MS) {
                float currentElapsedTime = (startTime > 0 && currentTime > startTime) ? (currentTime - startTime) / 1000.0f : 0.0f;
                float lastSplit = (shotCount > 0) ? splitTimes[shotCount - 1] : 0.0f;
                displayTimingScreen(currentElapsedTime, shotCount, lastSplit);
                lastDisplayUpdateTime = currentTime;
                redrawMenu = false; 
            }
            return; // Skip mic processing
        }
    }

    // --- Listening is Active ---
    float currentElapsedTime = (startTime > 0 && currentTime > startTime) ? (currentTime - startTime) / 1000.0f : 0.0f;
    
    micPeakRMS.update(); 
    currentCyclePeakRMS = micPeakRMS.getPeakRMS(); 

    if (currentCyclePeakRMS > peakRMSOverall) {
        peakRMSOverall = currentCyclePeakRMS;
    }

    if (redrawMenu || currentTime - lastDisplayUpdateTime >= DISPLAY_UPDATE_INTERVAL_MS) {
        float lastSplit = (shotCount > 0) ? splitTimes[shotCount - 1] : 0.0f;
        displayTimingScreen(currentElapsedTime, shotCount, lastSplit);
        lastDisplayUpdateTime = currentTime;
        redrawMenu = false; 
    }

    // Shot Detection Logic 
    if (currentCyclePeakRMS > shotThresholdRms &&
        currentTime - lastDetectionTime > SHOT_REFRACTORY_MS &&
        shotCount < MAX_SHOTS_LIMIT &&
        (currentMaxShots == 0 || shotCount < currentMaxShots) &&
        startTime > 0 &&
        (shotCount > 0 || (currentTime - startTime) >= minFirstShotTimeMs))
    {
        unsigned long shotTimeMillis = currentTime; 
        resetActivityTimer();
        lastDetectionTime = shotTimeMillis; 
        shotTimestamps[shotCount] = shotTimeMillis; 

        float currentSplit;
        if (shotCount == 0) {
            currentSplit = (shotTimeMillis - startTime) / 1000.0f;
        } else {
            currentSplit = (lastShotTimestamp > 0) ? ((shotTimeMillis - lastShotTimestamp) / 1000.0f) : 0.0f;
        }
        lastShotTimestamp = shotTimeMillis; 
        splitTimes[shotCount] = currentSplit;
        shotCount++;
        
        displayTimingScreen(currentElapsedTime, shotCount, currentSplit); 
        lastDisplayUpdateTime = currentTime; 

        if (currentMaxShots > 0 && shotCount >= currentMaxShots) {
            is_listening_active = false;
            setState(LIVE_FIRE_STOPPED);
            StickCP2.Lcd.fillScreen(BLACK);
            displayStoppedScreen();
            if (shotCount > 0) playSuccessBeeps(); else playUnsuccessBeeps();
        }
        // Don't reset peak here, let next cycle handle it
    } 
    // Reset peak at the end of the active listening cycle if no shot was detected
    else if (is_listening_active) { 
         micPeakRMS.resetPeak(); 
    }


    // Manual Stop
    if (currentState == LIVE_FIRE_TIMING && StickCP2.BtnA.wasClicked()) {
        resetActivityTimer();
        is_listening_active = false; 
        setState(LIVE_FIRE_STOPPED);
        StickCP2.Lcd.fillScreen(BLACK);
        displayStoppedScreen();
        if (shotCount > 0) playSuccessBeeps(); else playUnsuccessBeeps();
    }

    // Timeout Stop
    if (currentState == LIVE_FIRE_TIMING && shotTimeoutMs > 0) {
        unsigned long timeSinceEvent = (shotCount == 0) ? (currentTime - startTime) : (currentTime - lastShotTimestamp);
        bool hasStarted = (startTime > 0);
        if (hasStarted && timeSinceEvent > (unsigned long)shotTimeoutMs) {
            is_listening_active = false; 
            setState(LIVE_FIRE_STOPPED);
            StickCP2.Lcd.fillScreen(BLACK);
            displayStoppedScreen();
            if (shotCount > 0) playSuccessBeeps(); else playUnsuccessBeeps();
        }
    }
}


void handleDryFireReadyInput() {
    resetActivityTimer();
    if (redrawMenu) {
        displayDryFireReadyScreen();
        redrawMenu = false;
    }

    if (StickCP2.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
        setState(MODE_SELECTION);
        currentMenuSelection = (int)MODE_DRY_FIRE;
        int itemsPerScreen = MENU_ITEMS_PER_SCREEN_LANDSCAPE;
        menuScrollOffset = max(0, currentMenuSelection - itemsPerScreen + 1);
        StickCP2.Lcd.fillScreen(BLACK);
        return;
    }

    if (StickCP2.BtnA.wasClicked()) {
        reset_bt_beep_state(); 
        randomSeed(micros());
        unsigned long randomDelay = random(DRY_FIRE_RANDOM_DELAY_MIN_MS, DRY_FIRE_RANDOM_DELAY_MAX_MS + 1);

        randomDelayStartMs = millis();
        parTimerStartTime = randomDelayStartMs + randomDelay; 
        beepSequenceStartTime = 0; 
        beepsPlayed = 0;
        nextBeepTime = 0;
        lastBeepTime = 0;

        setState(DRY_FIRE_RUNNING);
        redrawMenu = true; 
    }
}

void handleDryFireRunning() {
    resetActivityTimer();
    unsigned long currentTime = millis();

    if (StickCP2.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
        reset_bt_beep_state(); 
        setState(DRY_FIRE_READY);
        playUnsuccessBeeps();
        redrawMenu = true; 
        return;
    }

    if (beepSequenceStartTime == 0) { 
        if (redrawMenu) {
            displayDryFireRunningScreen(true, 0, dryFireParBeepCount); 
        }
        if (currentTime >= parTimerStartTime) { 
            playTone(currentBeepToneHz, currentBeepDuration); 
            beepSequenceStartTime = currentTime; 
            lastBeepTime = beepSequenceStartTime; 
            beepsPlayed = 1;
            if (beepsPlayed < dryFireParBeepCount && dryFireParBeepCount > 0) {
                 int parIndex = 0; 
                 if (parIndex < MAX_PAR_BEEPS && parIndex < dryFireParBeepCount -1) { 
                    nextBeepTime = beepSequenceStartTime + (unsigned long)(dryFireParTimesSec[parIndex] * 1000.0f);
                 } else { 
                    beepsPlayed = dryFireParBeepCount; 
                 }
            } else { 
                 beepsPlayed = dryFireParBeepCount; 
            }
            redrawMenu = true;
        }
    }
    else { 
         if (redrawMenu) {
            displayDryFireRunningScreen(false, beepsPlayed, dryFireParBeepCount);
         }
        if (beepsPlayed >= dryFireParBeepCount) {
            reset_bt_beep_state(); 
            setState(DRY_FIRE_READY);
            delay(500); 
            redrawMenu = true;
        }
        else if (currentTime >= nextBeepTime && nextBeepTime > 0) {
            playTone(currentBeepToneHz, currentBeepDuration);
            lastBeepTime = currentTime; 
            beepsPlayed++;
            if (beepsPlayed < dryFireParBeepCount) {
                int parIndex = beepsPlayed -1; 
                unsigned long cumulativeParTimeMs = 0;
                for(int k=0; k < beepsPlayed -1; ++k){ 
                    if (k < MAX_PAR_BEEPS) {
                         cumulativeParTimeMs += (unsigned long)(dryFireParTimesSec[k] * 1000.0f);
                    }
                }
                 if (parIndex >= 0 && parIndex < MAX_PAR_BEEPS && parIndex < dryFireParBeepCount -1) {
                    nextBeepTime = beepSequenceStartTime + cumulativeParTimeMs + (unsigned long)(dryFireParTimesSec[parIndex] * 1000.0f);
                 } else {
                    beepsPlayed = dryFireParBeepCount; 
                    nextBeepTime = 0;
                 }
            } else {
                nextBeepTime = 0; 
            }
            redrawMenu = true;
        }
    }
}

void handleNoisyRangeReadyInput() {
    resetActivityTimer();
    if (redrawMenu) {
        displayTimingScreen(0.0f, 0, 0.0f);
        redrawMenu = false;
    }
    if (StickCP2.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
        setState(MODE_SELECTION);
        currentMenuSelection = (int)MODE_NOISY_RANGE;
        int itemsPerScreen = MENU_ITEMS_PER_SCREEN_LANDSCAPE;
        menuScrollOffset = max(0, currentMenuSelection - itemsPerScreen + 1);
        StickCP2.Lcd.fillScreen(BLACK);
        return;
    }
    if (StickCP2.BtnA.wasClicked()) {
        reset_bt_beep_state(); 
        is_listening_active = false; 
        setState(NOISY_RANGE_GET_READY);
        StickCP2.Lcd.fillScreen(BLACK);
        StickCP2.Lcd.setTextDatum(MC_DATUM);
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(3);
        StickCP2.Lcd.drawString("Ready...", StickCP2.Lcd.width()/2, StickCP2.Lcd.height()/2);
        delay(1000);
    }
}

void handleNoisyRangeGetReady() {
    resetActivityTimer();
    unsigned long beepInitiationTime = millis();
    playTone(currentBeepToneHz, currentBeepDuration);
    
    // Calculate when the audio (BT or Buzzer) is expected to finish
    unsigned long audioDuration = currentBeepDuration;
    unsigned long audioStartTime = beepInitiationTime; // Buzzer starts now
    if (a2dp_source.is_connected()) {
        audioStartTime = beepInitiationTime + currentBluetoothAudioOffsetMs; // BT starts later/earlier
    }
    beep_audio_end_time = max(beepInitiationTime, audioStartTime) + audioDuration + 150; // Increased buffer
    is_listening_active = false; 

    startTime = beepInitiationTime + postBeepDelayMs; 

    delay(postBeepDelayMs); 
    
    resetShotData();
    lastDisplayUpdateTime = 0;
    StickCP2.Lcd.fillScreen(BLACK);
    setState(NOISY_RANGE_TIMING);
    redrawMenu = true; 
}

void handleNoisyRangeTiming() {
    unsigned long currentTime = millis();
    float accX, accY, accZ;

    if (currentState != NOISY_RANGE_TIMING) return;

    // --- Check if listening should become active ---
     if (!is_listening_active) {
        if (currentTime >= beep_audio_end_time && currentTime >= startTime) {
            is_listening_active = true;
            micPeakRMS.resetPeak(); // Start listening clean
        } else {
            // Still waiting for beep audio to finish or for startTime, don't process mic/IMU
             if (redrawMenu || currentTime - lastDisplayUpdateTime >= DISPLAY_UPDATE_INTERVAL_MS) {
                float currentElapsedTime = (startTime > 0 && currentTime > startTime) ? (currentTime - startTime) / 1000.0f : 0.0f;
                float lastSplit = (shotCount > 0) ? splitTimes[shotCount - 1] : 0.0f;
                displayTimingScreen(currentElapsedTime, shotCount, lastSplit);
                lastDisplayUpdateTime = currentTime;
                redrawMenu = false; 
            }
            return; // Skip processing
        }
    }

    // --- Listening is Active ---
    float currentElapsedTime = (startTime > 0 && currentTime > startTime) ? (currentTime - startTime) / 1000.0f : 0.0f;
    if (redrawMenu || currentTime - lastDisplayUpdateTime >= DISPLAY_UPDATE_INTERVAL_MS) {
        float lastSplit = (shotCount > 0) ? splitTimes[shotCount - 1] : 0.0f;
        displayTimingScreen(currentElapsedTime, shotCount, lastSplit);
        lastDisplayUpdateTime = currentTime;
        redrawMenu = false;
    }

    micPeakRMS.update(); // Update mic only when listening
    currentCyclePeakRMS = micPeakRMS.getPeakRMS();

    if (!checkingForRecoil &&
        currentCyclePeakRMS > shotThresholdRms &&
        currentTime - lastDetectionTime > SHOT_REFRACTORY_MS &&
        shotCount < MAX_SHOTS_LIMIT &&
        (currentMaxShots == 0 || shotCount < currentMaxShots) &&
        startTime > 0 &&
        (shotCount > 0 || (currentTime - startTime) >= minFirstShotTimeMs))
    {
        lastSoundPeakTime = currentTime;
        checkingForRecoil = true;
        // Don't reset mic peak here, wait for recoil check
    }

    if (checkingForRecoil) {
        StickCP2.Imu.getAccelData(&accX, &accY, &accZ);
        float currentRecoil = abs(accZ);

        if (currentRecoil > recoilThreshold) {
            unsigned long shotTimeMillis = lastSoundPeakTime; 
            resetActivityTimer();
            lastDetectionTime = shotTimeMillis; 
            shotTimestamps[shotCount] = shotTimeMillis;
            float currentSplit;
            if (shotCount == 0) {
                currentSplit = (shotTimeMillis - startTime) / 1000.0f;
            } else {
                currentSplit = (lastShotTimestamp > 0) ? ((shotTimeMillis - lastShotTimestamp) / 1000.0f) : 0.0f;
            }
            lastShotTimestamp = shotTimeMillis;
            splitTimes[shotCount] = currentSplit;
            shotCount++;
            displayTimingScreen(currentElapsedTime, shotCount, currentSplit); 
            lastDisplayUpdateTime = currentTime;

            checkingForRecoil = false; 
            lastSoundPeakTime = 0;
            micPeakRMS.resetPeak(); // Reset peak after successful shot registration

            if (currentMaxShots > 0 && shotCount >= currentMaxShots) {
                is_listening_active = false;
                setState(LIVE_FIRE_STOPPED);
                StickCP2.Lcd.fillScreen(BLACK);
                displayStoppedScreen();
                if (shotCount > 0) playSuccessBeeps(); else playUnsuccessBeeps();
                return;
            }
        }
        else if (currentTime - lastSoundPeakTime > RECOIL_DETECTION_WINDOW_MS) {
            checkingForRecoil = false; 
            lastSoundPeakTime = 0;
            micPeakRMS.resetPeak(); // Reset peak if recoil window expired (false alarm)
        }
    } else if (is_listening_active) { // Reset peak if not checking recoil and listening is active
         micPeakRMS.resetPeak(); 
    }

    // Manual Stop
    if (currentState == NOISY_RANGE_TIMING && StickCP2.BtnA.wasClicked()) {
        resetActivityTimer();
        is_listening_active = false;
        setState(LIVE_FIRE_STOPPED); 
        StickCP2.Lcd.fillScreen(BLACK);
        displayStoppedScreen();
        if (shotCount > 0) playSuccessBeeps(); else playUnsuccessBeeps();
        return; 
    }

    // Timeout Stop
    if (currentState == NOISY_RANGE_TIMING && shotTimeoutMs > 0) {
        unsigned long timeSinceEvent = (shotCount == 0) ? (currentTime - startTime) : (currentTime - lastShotTimestamp);
        bool hasStarted = (startTime > 0);
        if (hasStarted && timeSinceEvent > (unsigned long)shotTimeoutMs) {
            is_listening_active = false;
            setState(LIVE_FIRE_STOPPED); 
            StickCP2.Lcd.fillScreen(BLACK);
            displayStoppedScreen();
            if (shotCount > 0) playSuccessBeeps(); else playUnsuccessBeeps();
        }
    }
}
