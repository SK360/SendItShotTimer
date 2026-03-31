#include "ota_utils.h"
#include "globals.h"
#include "config.h"
#include "system_utils.h"
#include "audio_utils.h"
#include "nvs_utils.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoJson.h>

static WebServer* server = nullptr;
static bool otaInProgress = false;
static bool otaSuccess = false;

static const char* AP_SSID = "ShotTimer";
static const char* AP_PASS = "shottimer";

static const char SETTINGS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Shot Timer Settings</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#111;color:#e0e0e0;padding:16px;max-width:480px;margin:0 auto}
h1{color:#4CAF50;text-align:center;font-size:24px;margin-bottom:4px}
.subtitle{text-align:center;color:#888;font-size:13px;margin-bottom:20px}
details{background:#1e1e1e;border-radius:8px;margin-bottom:10px;border:1px solid #333}
summary{padding:14px 16px;font-weight:600;font-size:15px;cursor:pointer;list-style:none;display:flex;justify-content:space-between;align-items:center;user-select:none}
summary::-webkit-details-marker{display:none}
summary::after{content:'▸';color:#4CAF50;font-size:18px;transition:transform .2s}
details[open]>summary::after{transform:rotate(90deg)}
details[open]{border-color:#4CAF50}
.fields{padding:12px 16px 16px}
.field{margin-bottom:14px}
.field:last-child{margin-bottom:0}
label{display:block;font-size:13px;color:#aaa;margin-bottom:4px}
input[type="number"],select{width:100%;padding:10px 12px;background:#2a2a2a;border:1px solid #444;border-radius:6px;color:#fff;font-size:15px;-webkit-appearance:none}
input[type="number"]:focus,select:focus{outline:none;border-color:#4CAF50}
.toggle{display:flex;align-items:center;justify-content:space-between}
.toggle label{margin:0;flex:1}
.switch{position:relative;width:48px;height:26px;flex-shrink:0}
.switch input{opacity:0;width:0;height:0}
.slider{position:absolute;cursor:pointer;inset:0;background:#444;border-radius:26px;transition:.3s}
.slider::before{content:'';position:absolute;height:20px;width:20px;left:3px;bottom:3px;background:#fff;border-radius:50%;transition:.3s}
.switch input:checked+.slider{background:#4CAF50}
.switch input:checked+.slider::before{transform:translateX(22px)}
.par-time{display:none}
.par-time.visible{display:block}
.btn{display:block;width:100%;padding:14px;border:none;border-radius:8px;font-size:16px;font-weight:600;cursor:pointer;margin-top:10px}
.btn-save{background:#4CAF50;color:#fff}
.btn-save:active{background:#388E3C}
.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%);padding:12px 24px;border-radius:8px;font-size:14px;font-weight:500;opacity:0;transition:opacity .3s;pointer-events:none;z-index:100}
.toast.show{opacity:1}
.toast.ok{background:#388E3C;color:#fff}
.toast.err{background:#c62828;color:#fff}
.ota-section{margin-top:10px}
.ota-section form{display:flex;flex-direction:column;gap:12px}
.file-input{color:#aaa;font-size:14px}
.btn-upload{background:#1565C0;color:#fff}
.btn-upload:active{background:#0D47A1}
.warn{color:#ff9800;font-size:12px;text-align:center;margin-top:8px}
.hint{color:#666;font-size:11px;margin-top:2px}
</style>
</head>
<body>
<h1>Shot Timer</h1>
<p class="subtitle">Settings &amp; Firmware</p>

<details open>
<summary>Live Fire</summary>
<div class="fields">
<div class="field"><label>Max Shots <span class="hint">(0 = Off / unlimited)</span></label><input type="number" id="maxShots" min="0" max="99" step="1"></div>
<div class="field"><label>Shot Threshold</label><input type="number" id="shotThreshold" min="100" max="32000" step="500"></div>
<div class="field"><label>Min 1st Shot (ms)</label><input type="number" id="minFirstShot" min="0" max="500" step="10"></div>
<div class="field"><label>Start Delay Min (ms)</label><input type="number" id="startDelayMin" min="0" max="10000" step="250"></div>
<div class="field"><label>Start Delay Max (ms)</label><input type="number" id="startDelayMax" min="0" max="10000" step="250"></div>
</div>
</details>

<details>
<summary>Dry Fire</summary>
<div class="fields">
<div class="field"><label>Par Beep Count</label><input type="number" id="parBeepCount" min="1" max="10" step="1"></div>
<div id="parTimesContainer"></div>
</div>
</details>

<details>
<summary>Noisy Range</summary>
<div class="fields">
<div class="field"><label>Recoil Threshold (G)</label><input type="number" id="recoilThreshold" min="0.5" max="5.0" step="0.1"></div>
</div>
</details>

<details>
<summary>Beep Settings</summary>
<div class="fields">
<div class="field"><label>Beep Duration (ms)</label><input type="number" id="beepDuration" min="50" max="2000" step="50"></div>
<div class="field"><label>Beep Tone (Hz)</label><input type="number" id="beepTone" min="500" max="8000" step="100"></div>
<div class="field"><label>Post Beep Delay (ms)</label><input type="number" id="postBeepDelay" min="50" max="1000" step="25"></div>
</div>
</details>

<details>
<summary>Bluetooth</summary>
<div class="fields">
<div class="field"><label>Volume</label><input type="number" id="btVolume" min="0" max="127" step="5"></div>
<div class="field"><label>Audio Offset (ms)</label><input type="number" id="btAudioOffset" min="-1000" max="500" step="50"></div>
<div class="field toggle"><label>Auto Reconnect</label><label class="switch"><input type="checkbox" id="btAutoReconnect"><span class="slider"></span></label></div>
</div>
</details>

<details>
<summary>Device</summary>
<div class="fields">
<div class="field"><label>Screen Rotation</label><select id="screenRotation"><option value="0">0</option><option value="1">1</option><option value="2">2</option><option value="3">3</option></select></div>
<div class="field toggle"><label>Boot Animation</label><label class="switch"><input type="checkbox" id="bootAnimation"><span class="slider"></span></label></div>
<div class="field toggle"><label>Auto Sleep</label><label class="switch"><input type="checkbox" id="autoSleep"><span class="slider"></span></label></div>
</div>
</details>

<button class="btn btn-save" onclick="saveSettings()">Save Settings</button>

<details class="ota-section">
<summary>Firmware Update</summary>
<div class="fields">
<form method="POST" action="/update" enctype="multipart/form-data" id="otaForm">
<input type="file" name="update" accept=".bin" required class="file-input">
<button type="submit" class="btn btn-upload">Upload Firmware</button>
</form>
<p class="warn">Do not power off during update!</p>
</div>
</details>

<div class="toast" id="toast"></div>

<script>
function toast(msg,ok){var t=document.getElementById('toast');t.textContent=msg;t.className='toast show '+(ok?'ok':'err');setTimeout(function(){t.className='toast'},3000)}
function buildParTimes(count,values){
var c=document.getElementById('parTimesContainer');c.innerHTML='';
for(var i=0;i<10;i++){
var d=document.createElement('div');d.className='field par-time'+(i<count?' visible':'');
d.innerHTML='<label>Par Time '+(i+1)+' (s)</label><input type="number" id="parTime'+i+'" min="0.1" max="10.0" step="0.1" value="'+(values&&values[i]?values[i].toFixed(1):'1.0')+'">';
c.appendChild(d)}}
document.addEventListener('DOMContentLoaded',function(){
fetch('/api/settings').then(function(r){return r.json()}).then(function(d){
document.getElementById('maxShots').value=d.liveFire.maxShots;
document.getElementById('shotThreshold').value=d.liveFire.shotThreshold;
document.getElementById('minFirstShot').value=d.liveFire.minFirstShot;
document.getElementById('startDelayMin').value=d.liveFire.startDelayMin;
document.getElementById('startDelayMax').value=d.liveFire.startDelayMax;
document.getElementById('parBeepCount').value=d.dryFire.parBeepCount;
buildParTimes(d.dryFire.parBeepCount,d.dryFire.parTimes);
document.getElementById('recoilThreshold').value=d.noisyRange.recoilThreshold.toFixed(1);
document.getElementById('beepDuration').value=d.beep.beepDuration;
document.getElementById('beepTone').value=d.beep.beepTone;
document.getElementById('postBeepDelay').value=d.beep.postBeepDelay;
document.getElementById('btVolume').value=d.bluetooth.volume;
document.getElementById('btAudioOffset').value=d.bluetooth.audioOffset;
document.getElementById('btAutoReconnect').checked=d.bluetooth.autoReconnect;
document.getElementById('screenRotation').value=d.device.screenRotation;
document.getElementById('bootAnimation').checked=d.device.bootAnimation;
document.getElementById('autoSleep').checked=d.device.autoSleep;
}).catch(function(){toast('Failed to load settings',false)});
document.getElementById('parBeepCount').addEventListener('change',function(){
var n=parseInt(this.value)||1;
var pts=document.querySelectorAll('.par-time');
for(var i=0;i<pts.length;i++){pts[i].className='field par-time'+(i<n?' visible':'')}})});
function saveSettings(){
var parTimes=[];var n=parseInt(document.getElementById('parBeepCount').value)||1;
for(var i=0;i<10;i++){var el=document.getElementById('parTime'+i);parTimes.push(el?parseFloat(el.value)||1.0:1.0)}
var data={liveFire:{maxShots:parseInt(document.getElementById('maxShots').value)||0,shotThreshold:parseInt(document.getElementById('shotThreshold').value)||15311,minFirstShot:parseInt(document.getElementById('minFirstShot').value)||0,startDelayMin:parseInt(document.getElementById('startDelayMin').value)||0,startDelayMax:parseInt(document.getElementById('startDelayMax').value)||0},
dryFire:{parBeepCount:n,parTimes:parTimes},
noisyRange:{recoilThreshold:parseFloat(document.getElementById('recoilThreshold').value)||1.5},
beep:{beepDuration:parseInt(document.getElementById('beepDuration').value)||150,beepTone:parseInt(document.getElementById('beepTone').value)||2000,postBeepDelay:parseInt(document.getElementById('postBeepDelay').value)||200},
bluetooth:{volume:parseInt(document.getElementById('btVolume').value)||80,audioOffset:parseInt(document.getElementById('btAudioOffset').value)||0,autoReconnect:document.getElementById('btAutoReconnect').checked},
device:{screenRotation:parseInt(document.getElementById('screenRotation').value)||3,bootAnimation:document.getElementById('bootAnimation').checked,autoSleep:document.getElementById('autoSleep').checked}};
fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)}).then(function(r){return r.json()}).then(function(d){
if(d.status==='ok'){toast('Settings saved!',true)}else{toast('Save failed: '+(d.message||'unknown error'),false)}}).catch(function(){toast('Save failed',false)})}
</script>
</body>
</html>
)rawliteral";

static void displayOtaScreen() {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextColor(WHITE, BLACK);
    StickCP2.Lcd.setTextDatum(TC_DATUM);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextSize(2);
    StickCP2.Lcd.drawString("WiFi Settings", StickCP2.Lcd.width() / 2, 5);

    StickCP2.Lcd.setTextSize(1);
    StickCP2.Lcd.setTextDatum(TL_DATUM);
    int y = 30;
    int lineH = 14;

    StickCP2.Lcd.drawString("WiFi Network:", 10, y); y += lineH;
    StickCP2.Lcd.setTextColor(GREEN, BLACK);
    StickCP2.Lcd.drawString(AP_SSID, 10, y); y += lineH + 2;

    StickCP2.Lcd.setTextColor(WHITE, BLACK);
    StickCP2.Lcd.drawString("Password:", 10, y); y += lineH;
    StickCP2.Lcd.setTextColor(GREEN, BLACK);
    StickCP2.Lcd.drawString(AP_PASS, 10, y); y += lineH + 2;

    StickCP2.Lcd.setTextColor(WHITE, BLACK);
    StickCP2.Lcd.drawString("Open in browser:", 10, y); y += lineH;
    StickCP2.Lcd.setTextColor(GREEN, BLACK);
    StickCP2.Lcd.drawString("http://192.168.4.1", 10, y);

    StickCP2.Lcd.setTextColor(WHITE, BLACK);
    StickCP2.Lcd.setTextDatum(BC_DATUM);
    StickCP2.Lcd.drawString("Hold Front to Cancel", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() - 5);
}

static void displayOtaUploading() {
    StickCP2.Lcd.fillScreen(BLACK);
    StickCP2.Lcd.setTextColor(WHITE, BLACK);
    StickCP2.Lcd.setTextDatum(MC_DATUM);
    StickCP2.Lcd.setTextFont(0);
    StickCP2.Lcd.setTextSize(3);
    StickCP2.Lcd.drawString("Uploading...", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2);
}

static void handleRoot() {
    server->send(200, "text/html", SETTINGS_HTML);
}

static void handleGetSettings() {
    JsonDocument doc;

    JsonObject lf = doc["liveFire"].to<JsonObject>();
    lf["maxShots"] = currentMaxShots;
    lf["shotThreshold"] = shotThresholdRms;
    lf["minFirstShot"] = minFirstShotTimeMs;
    lf["startDelayMin"] = startDelayMinMs;
    lf["startDelayMax"] = startDelayMaxMs;

    JsonObject df = doc["dryFire"].to<JsonObject>();
    df["parBeepCount"] = dryFireParBeepCount;
    JsonArray pt = df["parTimes"].to<JsonArray>();
    for (int i = 0; i < MAX_PAR_BEEPS; i++) {
        pt.add(dryFireParTimesSec[i]);
    }

    JsonObject nr = doc["noisyRange"].to<JsonObject>();
    nr["recoilThreshold"] = recoilThreshold;

    JsonObject bp = doc["beep"].to<JsonObject>();
    bp["beepDuration"] = (int)currentBeepDuration;
    bp["beepTone"] = currentBeepToneHz;
    bp["postBeepDelay"] = postBeepDelayMs;

    JsonObject bt = doc["bluetooth"].to<JsonObject>();
    bt["volume"] = currentBluetoothVolume;
    bt["audioOffset"] = currentBluetoothAudioOffsetMs;
    bt["autoReconnect"] = currentBluetoothAutoReconnect;

    JsonObject dv = doc["device"].to<JsonObject>();
    dv["screenRotation"] = screenRotationSetting;
    dv["bootAnimation"] = playBootAnimation;
    dv["autoSleep"] = enableAutoSleep;

    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

static void handlePostSettings() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"No body\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server->arg("plain"));
    if (err) {
        server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        return;
    }

    // Live Fire
    if (doc["liveFire"].is<JsonObject>()) {
        JsonObject lf = doc["liveFire"];
        if (lf["maxShots"].is<int>()) {
            currentMaxShots = constrain((int)lf["maxShots"], 0, MAX_SHOTS_LIMIT);
        }
        if (lf["shotThreshold"].is<int>()) {
            shotThresholdRms = constrain((int)lf["shotThreshold"], 100, 32000);
        }
        if (lf["minFirstShot"].is<int>()) {
            minFirstShotTimeMs = constrain((int)lf["minFirstShot"], 0, 500);
        }
        if (lf["startDelayMin"].is<int>()) {
            startDelayMinMs = constrain((int)lf["startDelayMin"], 0, MAX_START_DELAY_MS);
        }
        if (lf["startDelayMax"].is<int>()) {
            startDelayMaxMs = constrain((int)lf["startDelayMax"], 0, MAX_START_DELAY_MS);
        }
        if (startDelayMaxMs < startDelayMinMs) startDelayMaxMs = startDelayMinMs;
    }

    // Dry Fire
    if (doc["dryFire"].is<JsonObject>()) {
        JsonObject df = doc["dryFire"];
        if (df["parBeepCount"].is<int>()) {
            dryFireParBeepCount = constrain((int)df["parBeepCount"], 1, MAX_PAR_BEEPS);
        }
        if (df["parTimes"].is<JsonArray>()) {
            JsonArray pt = df["parTimes"];
            for (int i = 0; i < MAX_PAR_BEEPS && i < (int)pt.size(); i++) {
                if (pt[i].is<float>()) {
                    dryFireParTimesSec[i] = constrain((float)pt[i], 0.1f, 10.0f);
                }
            }
        }
    }

    // Noisy Range
    if (doc["noisyRange"].is<JsonObject>()) {
        JsonObject nr = doc["noisyRange"];
        if (nr["recoilThreshold"].is<float>()) {
            recoilThreshold = constrain((float)nr["recoilThreshold"], 0.5f, 5.0f);
        }
    }

    // Beep
    if (doc["beep"].is<JsonObject>()) {
        JsonObject bp = doc["beep"];
        if (bp["beepDuration"].is<int>()) {
            currentBeepDuration = constrain((int)bp["beepDuration"], 50, 2000);
        }
        if (bp["beepTone"].is<int>()) {
            currentBeepToneHz = constrain((int)bp["beepTone"], 500, 8000);
        }
        if (bp["postBeepDelay"].is<int>()) {
            postBeepDelayMs = constrain((int)bp["postBeepDelay"], 50, 1000);
        }
    }

    // Bluetooth
    if (doc["bluetooth"].is<JsonObject>()) {
        JsonObject bt = doc["bluetooth"];
        if (bt["volume"].is<int>()) {
            currentBluetoothVolume = constrain((int)bt["volume"], 0, 127);
            if (a2dp_source.is_connected()) {
                a2dp_source.set_volume(currentBluetoothVolume);
            }
        }
        if (bt["audioOffset"].is<int>()) {
            currentBluetoothAudioOffsetMs = constrain((int)bt["audioOffset"], -1000, 500);
        }
        if (bt["autoReconnect"].is<bool>()) {
            currentBluetoothAutoReconnect = bt["autoReconnect"];
        }
    }

    // Device
    if (doc["device"].is<JsonObject>()) {
        JsonObject dv = doc["device"];
        if (dv["screenRotation"].is<int>()) {
            screenRotationSetting = constrain((int)dv["screenRotation"], 0, 3);
            StickCP2.Lcd.setRotation(screenRotationSetting);
        }
        if (dv["bootAnimation"].is<bool>()) {
            playBootAnimation = dv["bootAnimation"];
        }
        if (dv["autoSleep"].is<bool>()) {
            enableAutoSleep = dv["autoSleep"];
        }
    }

    saveSettings();
    server->send(200, "application/json", "{\"status\":\"ok\"}");
}

static void handleUpdateComplete() {
    if (otaSuccess) {
        server->send(200, "text/html", "<html><body style='background:#111;color:#4CAF50;text-align:center;padding:50px;font-family:Arial'><h1>Update Successful!</h1><p>Rebooting...</p></body></html>");
        StickCP2.Lcd.fillScreen(BLACK);
        StickCP2.Lcd.setTextColor(GREEN, BLACK);
        StickCP2.Lcd.setTextDatum(MC_DATUM);
        StickCP2.Lcd.setTextFont(0);
        StickCP2.Lcd.setTextSize(2);
        StickCP2.Lcd.drawString("Update OK!", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 - 10);
        StickCP2.Lcd.setTextSize(1);
        StickCP2.Lcd.drawString("Rebooting...", StickCP2.Lcd.width() / 2, StickCP2.Lcd.height() / 2 + 15);
        delay(2000);
        ESP.restart();
    } else {
        server->send(500, "text/html", "<html><body style='background:#111;color:#f44336;text-align:center;padding:50px;font-family:Arial'><h1>Update Failed!</h1><p><a href='/' style='color:#4CAF50'>Try Again</a></p></body></html>");
        displayOtaScreen();
    }
}

static void handleUpdateUpload() {
    HTTPUpload& upload = server->upload();

    if (upload.status == UPLOAD_FILE_START) {
        otaInProgress = true;
        displayOtaUploading();
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            otaInProgress = false;
            otaSuccess = false;
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            otaSuccess = false;
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        otaSuccess = Update.end(true);
        otaInProgress = false;
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.abort();
        otaInProgress = false;
        otaSuccess = false;
    }
}

void startOtaUpdate() {
    // Disconnect Bluetooth if connected to free up the radio
    if (a2dp_source.is_connected()) {
        a2dp_source.disconnect();
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);

    server = new WebServer(80);
    server->on("/", HTTP_GET, handleRoot);
    server->on("/api/settings", HTTP_GET, handleGetSettings);
    server->on("/api/settings", HTTP_POST, handlePostSettings);
    server->on("/update", HTTP_POST, handleUpdateComplete, handleUpdateUpload);
    server->begin();

    otaInProgress = false;
    otaSuccess = false;

    displayOtaScreen();
}

void handleOtaUpdateLoop() {
    if (server != nullptr) {
        server->handleClient();
    }

    // Allow cancel only when not mid-upload
    if (!otaInProgress && StickCP2.BtnA.pressedFor(LONG_PRESS_DURATION_MS)) {
        stopOtaUpdate();
        settingsMenuLevel = 6;
        setState(SETTINGS_MENU_DEVICE);
        currentMenuSelection = 5; // "WiFi Settings" index in deviceItems
        menuScrollOffset = 0;
        StickCP2.Lcd.fillScreen(BLACK);
        playUnsuccessBeeps();
    }
}

void stopOtaUpdate() {
    if (otaInProgress) {
        Update.abort();
    }
    if (server != nullptr) {
        server->stop();
        delete server;
        server = nullptr;
    }
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    otaInProgress = false;
    otaSuccess = false;
}
