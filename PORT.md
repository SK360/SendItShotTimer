# Port Plan: M5StickC Plus2 → M5StickS3

Code-grounded plan. Read [Arch.md](Arch.md) first for the current architecture.

**Scope: S3 only.** The Plus2 build is not touched. Branch `port/s3` may diverge freely from `main` — no parallel-board discipline, no `#if BOARD_HAS_A2DP` gates, no dual Makefile targets. If a Plus2 build is ever needed again, it's recovered from `main`.

## Scope at a glance

- **422** direct `StickCP2.*` call sites across `code/` — every one moves to `M5.*` under `M5Unified`.
- **3** subsystems get a real rewrite: audio out, microphone, Bluetooth (deletion).
- **0** new features. Parity-only with the Plus2 firmware, minus A2DP.

## Hardware delta (the only deltas that matter)

| Subsystem | Plus2 | StickS3 | Action |
|-----------|-------|---------|--------|
| MCU | ESP32-PICO-D4 | ESP32-S3-PICO-1-N8R8 | New FQBN; **no BT Classic** |
| LCD | ST7789V2 | ST7789P3 | `M5Unified` handles |
| Buttons | BtnA(37), BtnB(39), PWR(35) | BtnA(KEY1=11), BtnB(KEY2=12), PWR via M5PM1 PMIC | `M5Unified` handles, but PWR semantics need bench check |
| IMU | MPU6886 | BMI270 | `M5.Imu` transparent; **re-calibrate recoil threshold** |
| Audio out | Passive buzzer GPIO 2 + 25 (PWM) | ES8311 codec → AW8737 amp via I²S | **Rewrite [audio_utils.cpp](code/audio_utils.cpp)** |
| Mic | SPM1423 PDM (CLK 0 / DATA 34) | MEMS via ES8311 codec | **Rewrite mic path in [timer_modes.cpp](code/timer_modes.cpp)** |
| PMIC | AXP-style | M5PM1 (I²C 0x6E) | `M5.Power` transparent |
| Bluetooth | BT Classic + BLE | **BLE-only** | **Delete A2DP entirely** |
| USB | USB-UART bridge | Native USB | New flash command + serial path |
| Battery | 200 mAh | 250 mAh | Rebaseline peak-voltage NVS default |

## Three structural issues

### 1. Audio out — full rewrite

[audio_utils.cpp](code/audio_utils.cpp) drives a passive buzzer with `ledcSetup` / `ledcAttachPin` / `ledcWriteTone` on a FreeRTOS `buzzerTask` consuming a `buzzerQueue`. None of that survives:

- Delete `BUZZER_LEDC_*`, `buzzerTask`, `buzzerQueue`.
- Replace `playTone` / `playFeedbackTone` with `M5.Speaker.tone(freq, duration_ms)`.
- The "send to BT or buzzer, never both" branch in `playTone` collapses to a single call to `M5.Speaker.tone()` (no A2DP).
- Drop `playSyncCalibrationTone` — it only existed to tune the BT audio offset.
- Keep `playSuccessBeeps` / `playUnsuccessBeeps` and the public header API; they call through to the new backend.
- `M5.Speaker.setVolume(0..255)` replaces the buzzer's fixed loudness; expose a new `Volume` setting in the Beep submenu (the BT volume setting goes away with A2DP).

### 2. Microphone + shot detection — full rewrite

`M5MicPeakRMS` is PDM-specific. Replace with `M5.Mic.record(buf, samples, rate)` returning `int16_t` from the ES8311.

- Re-implement peak-RMS inline (~30 LOC). Windowed max of `abs(sample)` over a small ring; trivial.
- **Disable ES8311 AGC.** Auto-gain will flatten muzzle-blast peaks. Configure fixed analog gain at `M5.Mic.begin()`.
- **Tight buffer.** 64 samples @ 16 kHz = 4 ms. Larger buffers widen timestamp jitter beyond the ±5 ms target.
- **Audit the codec HPF.** May attenuate muzzle-blast LF energy; relax if peaks look weak on the calibration screen.
- **Re-run `CALIBRATE_THRESHOLD` on real stimulus.** Plus2 default is meaningless on the S3 mic. Commit the new default with measurement notes.

### 3. Bluetooth A2DP — delete

ESP32-S3 has no BT Classic radio. `BluetoothA2DPSource` and `ESP32-BT-Scanner` cannot link.

- Delete [bluetooth_utils.cpp](code/bluetooth_utils.cpp) and [bluetooth_utils.h](code/bluetooth_utils.h) outright.
- Delete `a2dp_source` and BT volatiles from [globals.h](code/globals.h).
- Delete the entire `SETTINGS_MENU_BLUETOOTH` state, `BLUETOOTH_SCANNING` state, and `BT Audio Offset` row in [config.h](code/config.h), [display_utils.cpp](code/display_utils.cpp), and [input_handler.cpp](code/input_handler.cpp). Remove dead menu transitions; do not just blank the screen.
- Strip BT-related NVS keys from [nvs_utils.cpp](code/nvs_utils.cpp). Old keys can stay orphaned in flash on first boot — harmless.
- BLE Audio (LC3) is **out of scope** for v1.1. Revisit only if speaker support matures.

## Strategy

Adopt **`M5Unified`** as the HAL. Single `M5.*` API on a single supported board. No board-conditional compilation — the codebase becomes S3-only.

- Branch `port/s3` from `main`. Diverge freely.
- Tag `v1.1.0-s3` ships the first S3 binary.

## Phased work (each phase ends green and flashable)

### Phase 0 — Prep (½ day)
- Branch `port/s3`. Confirm the S3 FQBN: `arduino-cli board listall | grep -i sticks3`. Bump the `m5stack:esp32` core if needed.
- Replace [code/Makefile](code/Makefile) `boardconfig` with `m5stack:esp32:m5stack_sticks3`. Update `DEVICE` to the native-USB serial path. Update the `merge` target esptool args (likely `--chip esp32s3` and different file paths under `build/m5stack.esp32.m5stack_sticks3/`).
- Snapshot Plus2 `merged.bin` from `main` as a known-good reference before diverging.
- Add `port-notes.md` to `.gitignore` for empirical scratchpad.

### Phase 1 — `M5Unified` HAL swap + delete A2DP (1.5–2 days)
One commit (or tight series). The 422 `StickCP2.*` references all change here, and dead BT code is excised in the same pass so the codebase compiles cleanly with no orphan references.
- Replace `#include <M5StickCPlus2.h>` → `#include <M5Unified.h>` everywhere.
- `StickCP2.Lcd` → `M5.Lcd`, `StickCP2.BtnA/B` → `M5.BtnA/B`, `StickCP2.Imu` → `M5.Imu`, `StickCP2.Power` → `M5.Power`.
- `M5.begin()` config: enable LCD, IMU, BtnPWR, Speaker, Mic.
- Delete [bluetooth_utils.cpp](code/bluetooth_utils.cpp) / `.h`, `a2dp_source`, BT states, BT menu, BT NVS keys, BT audio offset.
- Remove `BluetoothA2DPSource` / `ESP32-BT-Scanner` / `M5MicPeakRMS` from required-libraries doc — those uninstall later as cleanup.
- Goal: **compiles for S3.** Won't run perfectly yet; audio and mic are still stubbed against the old buzzer/PDM API.
- Commit: `refactor: migrate to M5Unified, drop A2DP for S3 port`.

### Phase 2 — Boot, display, buttons (½ day)
- Flash. Verify [logo.h](code/logo.h) splash renders, rotation works (left/right hand), backlight setting persists.
- Verify BtnA/BtnB/BtnPWR cover every menu transition in [input_handler.cpp](code/input_handler.cpp).
- Confirm BtnPWR long-press at `LONG_PRESS_DURATION_MS` (750 ms) feels right. PMIC routing may add latency — adjust the constant if needed and document.
- Commit: `feat(s3): boot, display, buttons`.

### Phase 3 — Speaker (1 day)
- Rewrite [audio_utils.cpp](code/audio_utils.cpp) per §1. Keep the public header API stable so callers in [timer_modes.cpp](code/timer_modes.cpp) don't change.
- Drop the FreeRTOS buzzer task entirely; `M5.Speaker.tone()` is non-blocking.
- Add a Volume setting (replaces BT volume); persist via NVS.
- Verify every beep path: `LIVE_FIRE_READY` start beep, par beeps, shot-detect feedback, UI clicks (`enableUISounds`), success/unsuccess melodies.
- Commit: `feat(s3): speaker via M5.Speaker`.

### Phase 4 — Microphone + shot detection (1–2 days, mostly empirical)
- Replace `M5MicPeakRMS` includes/calls in [timer_modes.cpp](code/timer_modes.cpp) with inline peak-RMS over `M5.Mic.record()`.
- Configure: fixed analog gain, AGC off, HPF auditable, 64-sample buffer @ 16 kHz.
- Bring up `CALIBRATE_THRESHOLD`; check ambient floor in a quiet room.
- Run reference stimulus (canned shot recording or `.22 cap`); commit threshold and gain in their own commit with measured values in the message.
- Verify: no false trips at idle; no double-triggers on echo; works across the threshold range.
- Commit: `feat(s3): shot detection via codec mic`.

### Phase 5 — IMU recoil mode (½ day)
- Verify `M5.Imu.getAccelData()` returns sane G on BMI270.
- Re-run `CALIBRATE_RECOIL`. Commit new default.
- Verify Noisy Range mode: dry-fire triggers, table-bumps don't trigger during `GET_READY`.
- Commit: `feat(s3): recoil detection on BMI270`.

### Phase 6 — Power, sleep, OTA (½ day)
- `M5.Power.powerOff()` via M5PM1 — verify "Power Off Now" menu item.
- Light sleep: confirm `esp_light_sleep_start()` wakes on GPIO 11/12. PMIC-routed BtnPWR wake needs verification — may need a different wake source.
- Auto-sleep timer: the BT-connected bypass is gone with A2DP, so auto-sleep is always honored.
- OTA: WiFi AP path in [ota_utils.cpp](code/ota_utils.cpp) should work unchanged. Native USB on S3 also opens a DFU path — note in README, don't implement.
- Commit: `feat(s3): power, sleep, OTA`.

### Phase 7 — Polish and release (½ day)
- README: replace "M5StickC Plus2" with "M5StickS3" throughout. New flashing instructions (native USB, different serial path, esptool `--chip esp32s3`). Drop the BT/A2DP feature row, add the speaker volume row.
- Update boot logo only if dimensions differ (S3 LCD is the same 135×240 — likely no change).
- Drop unused libraries from build instructions (`ESP32-A2DP`, `ESP32-BT-Scanner`, `M5MicPeakRMS`).
- `make release`, tag `v1.1.0-s3`.

**Estimate: 5–7 working days.** Phases 1 (HAL swap) and 4 (mic) dominate. Half the budget is empirical calibration, not coding.

## Test plan

### Bench checklist (run every phase end)
- [ ] Flash succeeds, boot to splash
- [ ] All menu screens render and navigate with all 3 buttons
- [ ] Battery % within ±10% of multimeter
- [ ] Light sleep enters after timeout, wakes on button
- [ ] NVS settings persist across reboot
- [ ] OTA upload completes, device reboots into new firmware

### Audio (Phase 3)
- [ ] Start beep audible, not clipped, at expected pitch
- [ ] Par beeps at correct intervals (±10 ms vs second timer)
- [ ] UI clicks honor `enableUISounds`
- [ ] Volume control changes loudness across full range
- [ ] Overlapping beeps don't artifact

### Mic / shot detection (Phase 4)
- [ ] Quiet-room threshold calibration converges without false trips
- [ ] Reference stimulus split times match Plus2 reference within ±5 ms
- [ ] No double-trigger on muzzle blast echo
- [ ] Detection works at both extremes of the threshold range

### IMU / recoil (Phase 5)
- [ ] Recoil calibration converges in <10 s
- [ ] Dry-fire reliably triggers; dropping device on table does not
- [ ] Pickup/rotation during `GET_READY` does not false-trigger

### UX regression (Phases 2, 7)
- [ ] Every state reachable with 3 buttons
- [ ] BtnPWR long-press "back" feels consistent across screens
- [ ] No dead-end screens after BT submenu deletion

## Iteration discipline

- **One phase per commit** (or a small phase-scoped series). Calibration constants get their own commits with measurements in the message.
- **`port-notes.md` (gitignored)** for oscilloscope shots, threshold sweeps, ES8311 register experiments.
- The Plus2 reference firmware lives at the `main` tip — re-flash a Plus2 from there if you need an A/B comparison during mic tuning.

## Open questions

- Exact S3 FQBN in the current `m5stack:esp32 @ 2.1.4` core, or do we need to bump? Run `arduino-cli board listall | grep -i sticks3`.
- Partition scheme: with A2DP gone, the BT stack is out — drop `huge_app` and reclaim flash, or keep it for OTA headroom?
- Does M5PM1 expose battery voltage at the same precision as the Plus2 path? Peak-voltage NVS default may need re-baselining.
