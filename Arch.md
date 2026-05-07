# HeyManNiceShotTimer — Architecture

A shot timer (firearms) firmware running on M5Stack hardware. Arduino framework, ESP32 target.

## Target Hardware (current)

- **Board:** M5StickC Plus2 (EOL) — ESP32-PICO-D4, 135×240 ST7789V2 LCD, MPU6886 IMU, SPM1423 PDM mic, passive buzzer on GPIO 2, AXP-style PMIC wrapper
- **Planned target:** M5StickS3 (ESP32-S3) — see [Plan.md](Plan.md)

## Build & Toolchain

- **Framework:** Arduino, built via `arduino-cli` (not PlatformIO)
- **Entry point:** [code/code.ino](code/code.ino)
- **Makefile:** [code/Makefile](code/Makefile) — `build`, `flash`, `merge`, `monitor`, `all`, `release`
- **FQBN:** `m5stack:esp32:m5stack_stickc_plus2`
- **Release artifact:** merged `.bin` (bootloader + partitions + app) via esptool

## Source Layout

All code lives in `code/`.

| File | Responsibility | Hardware |
|------|---------------|----------|
| [code.ino](code/code.ino) | `setup()`, `loop()`, state dispatch, BT/battery polling, sleep | orchestrator |
| [config.h](code/config.h) / [config.cpp](code/config.cpp) | Enums, constants, NVS keys | — |
| [globals.h](code/globals.h) | Extern declarations, FreeRTOS handles | — |
| [display_utils.cpp](code/display_utils.cpp) | Menu and screen rendering, layout | `StickCP2.Lcd` |
| [input_handler.cpp](code/input_handler.cpp) | Button routing per state, long-press | `BtnA`, `BtnB`, `M5.BtnPWR` |
| [timer_modes.cpp](code/timer_modes.cpp) | Shot detection, par timers, recoil timing | mic, buzzer, display |
| [audio_utils.cpp](code/audio_utils.cpp) | Tone playback, buzzer task, BT sync offsets | GPIO 25/2 PWM, BT A2DP |
| [bluetooth_utils.cpp](code/bluetooth_utils.cpp) | A2DP source init, scan, callbacks | BT Classic / A2DP |
| [nvs_utils.cpp](code/nvs_utils.cpp) | `Preferences` load/save, peak voltage tracking | NVS |
| [system_utils.cpp](code/system_utils.cpp) | State setter, battery check, activity timer | power |
| [ota_utils.cpp](code/ota_utils.cpp) | WiFi OTA update | WiFi |
| [logo.h](code/logo.h) | Boot splash bitmap | display |

## Runtime Model

### Main loop
- 10 ms tick (`vTaskDelay`); `switch(currentState)` dispatches handlers.

### State machine (21 states)
- **Live Fire:** `LIVE_FIRE_READY` → `GET_READY` → `TIMING` → `STOPPED`
- **Dry Fire:** `DRY_FIRE_READY` → `RUNNING`
- **Noisy Range (recoil-based):** `NOISY_RANGE_READY` → `GET_READY` → `TIMING` → `STOPPED`
- **UI:** `MODE_SELECTION`, 7× `SETTINGS_MENU_*`, `EDIT_SETTING`, `DEVICE_STATUS`, `BLUETOOTH_SCANNING`, `CALIBRATE_THRESHOLD`, `CALIBRATE_RECOIL`, `OTA_UPDATE`

### FreeRTOS
- **Buzzer task** (core 0, stack 2048) consumes a `buzzerQueue` (length 10) populated by main loop and BT callbacks. Defined in [audio_utils.cpp](code/audio_utils.cpp).
- **Light sleep** via `esp_light_sleep_start()` with GPIO wake on BtnA/B/PWR.

### Persistence
- `Preferences` API, single namespace (`NVS_NAMESPACE` in `config.cpp`).
- ~13 keys: max shots, beep duration/tone, thresholds, rotation, auto-sleep, BT device/volume/offset, UI sounds, timing delays, peak battery voltage.

## Audio Path

Two output paths, both fed from [audio_utils.cpp](code/audio_utils.cpp):

1. **Local buzzer** — LEDC PWM on GPIO 25 and GPIO 2 (Plus2 has both a Grove pin and internal buzzer). Driven by the buzzer FreeRTOS task.
2. **Bluetooth A2DP source** — `BluetoothA2DPSource` library streams tones to a paired BT speaker. A tunable `currentBluetoothAudioOffsetMs` compensates for A2DP latency so beeps line up with local buzzer.

Start-beep timing is shared between both paths so the shooter hears a synchronized tone.

## Input Path

- `StickCP2.BtnA` and `StickCP2.BtnB`: context-dependent per state.
- `M5.BtnPWR`: long-press (750 ms) typically exits back one level; short-press wakes from sleep.
- All routing centralized in [input_handler.cpp](code/input_handler.cpp).

## Sensor Path

- **Microphone** (shot detection in Live Fire): custom `M5MicPeakRMS` external library wraps the PDM mic; peak RMS compared against threshold (calibrated in `CALIBRATE_THRESHOLD`).
- **IMU** (shot detection in Noisy Range mode): `StickCP2.Imu` accel magnitude compared against a recoil threshold (calibrated in `CALIBRATE_RECOIL`).

## External Libraries

- `M5StickCPlus2` — board support
- `M5MicPeakRMS` — PDM microphone peak/RMS
- `BluetoothA2DPSource` — A2DP audio source
- `ESP32BluetoothScanner` — BT device discovery

## Abstraction

**Minimal.** `StickCP2.*` calls are scattered directly through display, input, and timer modules. No HAL boundary. This is the single biggest factor in the S3 port — see [Plan.md](Plan.md).
