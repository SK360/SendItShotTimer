#include "ota_utils.h"
#include "globals.h"
#include "config.h"
#include "system_utils.h"
#include "audio_utils.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

static WebServer* server = nullptr;
static bool otaInProgress = false;
static bool otaSuccess = false;

static const char* AP_SSID = "ShotTimer-Update";
static const char* AP_PASS = "update1234";

static const char OTA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Shot Timer OTA Update</title>
<style>
body { font-family: Arial, sans-serif; text-align: center; padding: 20px; background: #1a1a1a; color: #fff; }
h1 { color: #4CAF50; }
.upload-form { margin: 30px auto; padding: 20px; max-width: 400px; background: #2d2d2d; border-radius: 10px; }
input[type="file"] { margin: 15px 0; color: #fff; }
input[type="submit"] { background: #4CAF50; color: white; padding: 12px 30px; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; }
input[type="submit"]:hover { background: #45a049; }
.progress { display: none; margin: 20px auto; width: 80%; background: #444; border-radius: 5px; }
.progress-bar { width: 0%; height: 30px; background: #4CAF50; border-radius: 5px; text-align: center; line-height: 30px; color: white; transition: width 0.3s; }
.warning { color: #ff9800; font-size: 14px; margin-top: 15px; }
</style>
</head>
<body>
<h1>Shot Timer</h1>
<h2>Firmware Update</h2>
<div class="upload-form">
<form method="POST" action="/update" enctype="multipart/form-data" id="uploadForm">
<input type="file" name="update" accept=".bin" required><br>
<input type="submit" value="Upload Firmware">
</form>
<div class="progress" id="progress">
<div class="progress-bar" id="progressBar">0%</div>
</div>
<p class="warning">Do not power off during update!</p>
</div>
<script>
document.getElementById('uploadForm').addEventListener('submit', function() {
    document.getElementById('progress').style.display = 'block';
    var bar = document.getElementById('progressBar');
    var pct = 0;
    var interval = setInterval(function() {
        if (pct < 90) { pct += 5; bar.style.width = pct + '%'; bar.textContent = pct + '%'; }
    }, 500);
});
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
    StickCP2.Lcd.drawString("OTA Update", StickCP2.Lcd.width() / 2, 5);

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
    server->send(200, "text/html", OTA_HTML);
}

static void handleUpdateComplete() {
    if (otaSuccess) {
        server->send(200, "text/html", "<html><body style='background:#1a1a1a;color:#4CAF50;text-align:center;padding:50px;font-family:Arial'><h1>Update Successful!</h1><p>Rebooting...</p></body></html>");
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
        server->send(500, "text/html", "<html><body style='background:#1a1a1a;color:#f44336;text-align:center;padding:50px;font-family:Arial'><h1>Update Failed!</h1><p><a href='/' style='color:#4CAF50'>Try Again</a></p></body></html>");
        // Redraw the OTA screen so user can retry
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
        if (Update.end(true)) {
            otaSuccess = true;
        } else {
            otaSuccess = false;
        }
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
        currentMenuSelection = 5; // "Update Firmware" index in deviceItems
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
