/*
 * PlugNSat - Main Firmware
 * Lightning Smart Plug Controller
 * Version: see FIRMWARE_VERSION in config.h
 * Hardware: LilyGO T-Display S3 (ESP32-S3 + 1.9" LCD)
 * 
 * BASED ON:
 * - BitcoinSwitch by @arcbtc: github.com/arcbtc/bitcoinSwitch
 * - LNPoS by @arcbtc: github.com/arcbtc/LNPoS
 * 
 * HARDWARE:
 * - LilyGO T-Display S3 (ESP32-S3 + ST7789 1.9" LCD)
 * - Shelly Plug S Gen3 (WiFi relay, CE certified, 2500W)
 * - Lightning backend: BTCPay Server (Phoenixd, LND, CLN) or Blink
 * 
 * WORKFLOW:
 *   1. First boot: Welcome screen, AP mode for setup via web browser (webportal)
 *   2. Once configured: QR code LNURL displayed permanently (passive PoS)
 *   3. Customer scans and pays with any Lightning wallet
 *   4. "Payment received!" screen, Shelly activates (duration configured on webportal)
 *   5. New QR code auto-generated, cycle repeats
 *   6. QR auto-refreshes every 4m45s (before 5min expiry)
 *
 * BUTTONS (T-Display S3 built-in):
 *   - Button 1 (GPIO 0):  Settings menu / Long press = AP mode
 *   - Button 2 (GPIO 14): Force refresh QR / Confirm
 * 
 * License: MIT © 2026
 * Author: ezmo-dev (PlugNSat)
 */

// OTA NOTE (Arduino IDE):
// Board: LilyGO T-Display S3
// Partition Scheme: select "Minimal SPIFFS (1.9MB APP with OTA/128KB SPIFFS)"
// This provides two OTA app slots required for firmware updates.
// Do not use a "No OTA" partition scheme.

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <TFT_eSPI.h>
#include "qrcode.h"
#include "config.h"
#include "display.h"
#include "btcpay.h"
#include "shelly.h"
#include "backend.h"
#include "ota.h"
#include "webportal.h"

//
// GLOBALS
//

TFT_eSPI tft = TFT_eSPI();
Preferences prefs;
WebServer server(80);

// Buttons
#define BTN_1   0
#define BTN_2   14

bool btn1Pressed = false;
bool btn2Pressed = false;

// State machine
enum AppState {
  STATE_AP_SETUP,
  STATE_CONNECTING,
  STATE_QR_DISPLAY,
  STATE_PAID,
  STATE_ERROR,
  STATE_SHELLY_OFFLINE,
  STATE_INFO,
  STATE_SETTINGS,
  STATE_BRIGHTNESS,
  STATE_PRICE,
  STATE_DURATION,
  STATE_PIN_ENTRY
};
AppState currentState = STATE_CONNECTING;

// Invoice tracking
String currentInvoiceId = "";
String currentQRData     = "";
unsigned long invoiceCreatedAt = 0;
unsigned long lastPollTime     = 0;

// Error tracking
int consecutiveErrors = 0;
unsigned long errorStartTime = 0;
unsigned long shellyOfflineStartTime = 0;
unsigned long paidStartTime = 0;
bool paidShellyTriggered = false;

// Session stats
int paymentCount = 0;

// Settings menu
int settingsIndex = 0;
unsigned long lastSettingsInput = 0;
unsigned long lastBrightnessInput = 0;
unsigned long lastPriceInput = 0;
unsigned long lastDurationInput = 0;
int priceOnEnterMenu = 0;

// PIN entry
int pinEntry[4] = {0, 0, 0, 0};
int pinDigitIndex = 0;
AppState pinTargetState = STATE_PRICE;
unsigned long lastPinInput = 0;
bool pinWrong = false;
unsigned long pinWrongTime = 0;

// AP mode
unsigned long apModeStartTime = 0;

// WiFi monitoring
unsigned long lastWifiCheck = 0;

// Heap monitoring
unsigned long lastHeapLog = 0;

// Display
bool screenNeedsRedraw = true;

// Config
PlugNSatConfig config;

//
// SETUP
//

void setup() {
  otaInit();
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== PlugNSat v" FIRMWARE_VERSION " ===");

  tft.init();
  tft.setRotation(1);
  ledcAttach(BACKLIGHT_PIN, 5000, 8);
  ledcWrite(BACKLIGHT_PIN, 40);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);

  loadConfig();
  initBackend(config.backendType);
  ledcWrite(BACKLIGHT_PIN, 255);
  displaySplash(tft);
  delay(5000);
  ledcWrite(BACKLIGHT_PIN, config.brightness);

  bool hasBackendConfig = false;
  if (config.backendType == BACKEND_BTCPAY) {
    hasBackendConfig = config.btcpayUrl.length() > 0 && config.btcpayApiKey.length() > 0;
  } else if (config.backendType == BACKEND_BLINK) {
    hasBackendConfig = config.blinkApiKey.length() > 0 && config.blinkWalletId.length() > 0;
  }

  if (config.wifiSsid.length() == 0 || !hasBackendConfig) {
    startAPMode();
  } else {
    connectToWiFi();
  }
}

//
// MAIN LOOP
//

void loop() {
  otaTick();
  server.handleClient();
  readButtons();

  switch (currentState) {
    case STATE_AP_SETUP:
      loopAPSetup();
      break;

    case STATE_QR_DISPLAY:
      loopQRDisplay();
      break;

    case STATE_PAID:
      loopPaid();
      break;

    case STATE_ERROR:
      loopError();
      break;

    case STATE_SHELLY_OFFLINE:
      loopShellyOffline();
      break;

    case STATE_INFO:
      loopInfo();
      break;

    case STATE_SETTINGS:
      loopSettings();
      break;

    case STATE_BRIGHTNESS:
      loopBrightness();
      break;

    case STATE_PRICE:
      loopPrice();
      break;

    case STATE_DURATION:
      loopDuration();
      break;

    case STATE_PIN_ENTRY:
      loopPinEntry();
      break;

    default:
      break;
  }
  
  delay(10);
}

//
// WIFI
//

void connectToWiFi() {
  displayConnecting(tft, config.wifiSsid);
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(config.wifiSsid.c_str(), config.wifiPass.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi OK: " + WiFi.localIP().toString());
    otaMarkValid();
    MDNS.begin("plugnsat");
    displayStatus(tft, "WiFi connected!", WiFi.localIP().toString());
    delay(1500);

    // Auto-update on boot (opt-in)
    if (config.autoUpdate) {
      displayStatus(tft, "Checking updates...", "");
      OtaUpdateInfo upd = otaCheckUpdate();
      if (upd.error.length() == 0 && upd.available) {
        Serial.println("OTA: auto-update on boot to v" + upd.latestVersion);
        displayStatus(tft, "Updating...", "v" + upd.latestVersion);
        OtaFlashResult fr = otaDownloadAndFlash(upd.latestVersion);
        if (fr.success) {
          Serial.println("OTA: boot auto-update success, rebooting");
          delay(500);
          ESP.restart();
        } else {
          Serial.println("OTA: boot auto-update failed: " + fr.error);
          // Continue normal boot on failure
        }
      } else {
        Serial.println("OTA: no boot update (err=" + upd.error
                       + " avail=" + String(upd.available) + ")");
      }
    }

    setupWebPortal(server, config, prefs);
    server.begin();

    generateAndShowQR();
  } else {
    Serial.println("WiFi FAILED");

    unsigned long t = millis();
    int lastSecond = -1;
    while (millis() - t < 10000) {
      readButtons();
      if (btn1Pressed) { startAPMode(); return; }
      if (btn2Pressed) { connectToWiFi(); return; }
      server.handleClient();
      int secondsLeft = max(0, 10 - (int)((millis() - t) / 1000));
      if (secondsLeft != lastSecond) {
        lastSecond = secondsLeft;
        displayWiFiFailed(tft, config.wifiSsid, secondsLeft);
      }
      delay(50);
    }
    connectToWiFi();  // Auto-retry
  }
}

bool ensureWiFi() {
  if (millis() - lastWifiCheck < 30000) return WiFi.status() == WL_CONNECTED;
  lastWifiCheck = millis();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, reconnecting...");
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(config.wifiSsid.c_str(), config.wifiPass.c_str());
    
    int att = 0;
    while (WiFi.status() != WL_CONNECTED && att < 20) { delay(500); att++; }
    
    if (WiFi.status() != WL_CONNECTED) {
      currentState = STATE_ERROR;
      errorStartTime = millis();
      return false;
    }
    Serial.println("WiFi reconnected");
  }
  return true;
}

//
// QR DISPLAY (main operating loop)
//

void loopQRDisplay() {
  if (!ensureWiFi()) return;

  if (millis() - lastHeapLog > 300000) {
    lastHeapLog = millis();
    Serial.println("Heap free: " + String(ESP.getFreeHeap()) + " bytes");
  }

  // Auto-refresh QR before expiry (4min 45s = 285000ms)
  if (millis() - invoiceCreatedAt > QR_REFRESH_MS) {
    Serial.println("QR refresh (expiry prevention)");
    generateAndShowQR();
    return;
  }
  
  // BTCPay polling and Shelly activation
  if (millis() - lastPollTime > POLL_INTERVAL_MS) {
    lastPollTime = millis();
    
    String status = backend.checkInvoice(config, currentInvoiceId, currentQRData);
    
    if (status == "ERROR") {
      consecutiveErrors++;
      Serial.println("Poll error #" + String(consecutiveErrors));
      if (consecutiveErrors >= 60) {
        displayError(tft, WiFi.localIP().toString(), 5);
        delay(5000);
        ESP.restart();
      }
      return;
    }
    
    consecutiveErrors = 0;
    
    if (status == "Settled" || status == "Processing") {
      Serial.println("*** PAYMENT RECEIVED ***");
      paidStartTime = millis();
      paidShellyTriggered = false;
      paymentCount++;
      ledcWrite(BACKLIGHT_PIN, 255);
      currentState = STATE_PAID;
      return;
    }
    
    if (status == "Expired" || status == "Invalid") {
      Serial.println("Invoice expired, refreshing");
      generateAndShowQR();
      return;
    }
  }
  
  // Buttons
  if (btn1Pressed) {
    settingsIndex = 0;
    lastSettingsInput = millis();
    currentState = STATE_SETTINGS;
    screenNeedsRedraw = true;
  }
  if (btn2Pressed) {
    generateAndShowQR();  // Force refresh
  }
}

void returnToQR() {
  if (currentQRData.length() > 0) {
    showCurrentQR();
  } else {
    generateAndShowQR();
  }
}

void showCurrentQR() {
  if (config.showName || config.showPrice) {
    displayQRWithInfo(tft, currentQRData, config.priceSats, config.deviceName, config.showName, config.showPrice);
  } else {
    displayQR(tft, currentQRData, config.priceSats, config.deviceName);
  }
  currentState = STATE_QR_DISPLAY;
}

void generateAndShowQR() {
  if (currentInvoiceId.length() > 0) {
    String status = backend.checkInvoice(config, currentInvoiceId, currentQRData);
    if (status == "Settled" || status == "Processing") {
      Serial.println("*** PAYMENT DETECTED before QR refresh ***");
      paidStartTime = millis();
      paidShellyTriggered = false;
      paymentCount++;
      ledcWrite(BACKLIGHT_PIN, 255);
      currentState = STATE_PAID;
      return;
    }
  }

  for (int attempt = 0; attempt < 3; attempt++) {
    displayGenerating(tft);

    String invoiceId, lnurl;

    bool ok = backend.createInvoice(config, config.priceSats, invoiceId, lnurl);

    if (ok && lnurl.length() > 0) {
      currentInvoiceId = invoiceId;
      currentQRData = lnurl;
      invoiceCreatedAt = millis();
      lastPollTime = millis();
      consecutiveErrors = 0;

      showCurrentQR();

      Serial.println("QR ready. Invoice: " + invoiceId + " (" + String(lnurl.length()) + " chars)");
      return;
    }

    consecutiveErrors++;
    delay(2000);
  }

  currentState = STATE_ERROR;
  errorStartTime = millis();
}

//
// PAID
//

void loopPaid() {
  if (!paidShellyTriggered) {
    paidShellyTriggered = true;
    bool shellyOk = shellySwitchOn(config.shellyHost, config.activationDuration);
    if (!shellyOk) {
      Serial.println("Shelly offline at payment — showing offline screen");
      ledcWrite(BACKLIGHT_PIN, config.brightness);
      shellyOfflineStartTime = millis();
      currentState = STATE_SHELLY_OFFLINE;
      return;
    }
    Serial.println("Shelly ON for " + String(config.activationDuration) + "s");
  }

  int secondsLeft = max(0, config.activationDuration - (int)((millis() - paidStartTime) / 1000));

  static int lastDisplayedSecond = -1;
  if (secondsLeft != lastDisplayedSecond) {
    lastDisplayedSecond = secondsLeft;
    displayPaid(tft, paymentCount, config.priceSats, secondsLeft, config.activationDuration);
  }

  if (millis() - paidStartTime >= (unsigned long)config.activationDuration * 1000) {
    lastDisplayedSecond = -1;
    ledcWrite(BACKLIGHT_PIN, config.brightness);
    currentInvoiceId = "";
    currentQRData = "";
    generateAndShowQR();
  }
}

//
// ERROR (auto-retry after 10s)
//

void loopError() {
  ledcWrite(BACKLIGHT_PIN, 255);

  int secondsLeft = max(0, 10 - (int)((millis() - errorStartTime) / 1000));

  static int lastDisplayedSecond = -1;
  if (secondsLeft != lastDisplayedSecond) {
    lastDisplayedSecond = secondsLeft;
    displayError(tft, WiFi.localIP().toString(), secondsLeft);
  }

  if (millis() - errorStartTime > 10000) {
    lastDisplayedSecond = -1;
    consecutiveErrors = 0;
    ledcWrite(BACKLIGHT_PIN, config.brightness);
    generateAndShowQR();
    return;
  }
  if (btn1Pressed) { lastDisplayedSecond = -1; ledcWrite(BACKLIGHT_PIN, config.brightness); startAPMode(); }
  if (btn2Pressed) { lastDisplayedSecond = -1; ledcWrite(BACKLIGHT_PIN, config.brightness); consecutiveErrors = 0; generateAndShowQR(); }
}

//
// SHELLY OFFLINE
//

void loopShellyOffline() {
  int secondsLeft = max(0, 10 - (int)((millis() - shellyOfflineStartTime) / 1000));

  static int lastDisplayedSecond = -1;
  if (secondsLeft != lastDisplayedSecond) {
    lastDisplayedSecond = secondsLeft;
    displayShellyOffline(tft, secondsLeft);
  }

  if (btn1Pressed || btn2Pressed) {
    lastDisplayedSecond = -1;
    ledcWrite(BACKLIGHT_PIN, config.brightness);
    generateAndShowQR();
    return;
  }

  if (millis() - shellyOfflineStartTime > 10000) {
    lastDisplayedSecond = -1;
    if (shellyIsOnline(config.shellyHost)) {
      Serial.println("Shelly back online, retrying activation");
      bool ok = shellySwitchOn(config.shellyHost, config.activationDuration);
      if (ok) {
        Serial.println("Shelly ON for " + String(config.activationDuration) + "s (retry after offline)");
      }
      lastDisplayedSecond = -1;
      paidStartTime = millis();
      paidShellyTriggered = true;
      currentInvoiceId = "";
      currentQRData = "";
      ledcWrite(BACKLIGHT_PIN, 255);
      currentState = STATE_PAID;
    } else {
      Serial.println("Shelly still offline, retrying in 10s");
      shellyOfflineStartTime = millis();
    }
  }
}

//
// INFO
//

void loopInfo() {
  if (screenNeedsRedraw) {
    displayInfo(tft, config, paymentCount);
    screenNeedsRedraw = false;
  }
  if (btn1Pressed || btn2Pressed) {
    returnToQR();
  }
}

//
// SETTINGS
//

void loopSettings() {
  // Auto-timeout 6s -> back to QR
  if (millis() - lastSettingsInput > MENU_TIMEOUT_MS) {
    returnToQR();
    return;
  }
  if (screenNeedsRedraw) {
    displaySettings(tft, settingsIndex, config.pin.length() == 4);
    screenNeedsRedraw = false;
  }
  if (btn1Pressed) {
    lastSettingsInput = millis();
    settingsIndex = (settingsIndex + 1) % 4;
    screenNeedsRedraw = true;
  }
  if (btn2Pressed) {
    lastSettingsInput = millis();
    if (settingsIndex == 0) {
      currentState = STATE_INFO;
      screenNeedsRedraw = true;
    } else if (settingsIndex == 1) {
      lastBrightnessInput = millis();
      currentState = STATE_BRIGHTNESS;
      screenNeedsRedraw = true;
    } else if (settingsIndex == 2) {
      if (config.pin.length() == 4) {
        pinDigitIndex = 0;
        for (int i = 0; i < 4; i++) pinEntry[i] = 0;
        pinWrong = false;
        pinTargetState = STATE_PRICE;
        lastPinInput = millis();
        currentState = STATE_PIN_ENTRY;
      } else {
        priceOnEnterMenu = config.priceSats;
        lastPriceInput = millis();
        currentState = STATE_PRICE;
      }
      screenNeedsRedraw = true;
    } else {
      if (config.pin.length() == 4) {
        pinDigitIndex = 0;
        for (int i = 0; i < 4; i++) pinEntry[i] = 0;
        pinWrong = false;
        pinTargetState = STATE_DURATION;
        lastPinInput = millis();
        currentState = STATE_PIN_ENTRY;
      } else {
        lastDurationInput = millis();
        currentState = STATE_DURATION;
      }
      screenNeedsRedraw = true;
    }
  }
}

//
// BRIGHTNESS
//

void loopBrightness() {
  // Auto-save and return to QR after 3s without input
  if (millis() - lastBrightnessInput > MENU_TIMEOUT_MS) {
    saveConfig();
    returnToQR();
    return;
  }
  if (screenNeedsRedraw) {
    displayBrightness(tft, config.brightness, currentQRData);
    screenNeedsRedraw = false;
  }
  if (btn1Pressed) {
    lastBrightnessInput = millis();
    config.brightness = max(10, config.brightness - 10);
    ledcWrite(BACKLIGHT_PIN, config.brightness);
    screenNeedsRedraw = true;
  }
  if (btn2Pressed) {
    lastBrightnessInput = millis();
    config.brightness = min(255, config.brightness + 10);
    ledcWrite(BACKLIGHT_PIN, config.brightness);
    screenNeedsRedraw = true;
  }
}

//
// PRICE
//

void loopPrice() {
  if (millis() - lastPriceInput > MENU_TIMEOUT_MS) {
    saveConfig();
    if (config.priceSats != priceOnEnterMenu) {
      generateAndShowQR();
    } else {
      returnToQR();
    }
    return;
  }
  if (screenNeedsRedraw) {
    displayPrice(tft, config.priceSats);
    screenNeedsRedraw = false;
  }
  if (btn1Pressed) {
    lastPriceInput = millis();
    int step = (config.priceSats <= 1000) ? 10 : 100;
    config.priceSats = max(1, config.priceSats - step);
    screenNeedsRedraw = true;
  }
  if (btn2Pressed) {
    lastPriceInput = millis();
    int step = (config.priceSats <= 1000) ? 10 : 100;
    config.priceSats += step;
    screenNeedsRedraw = true;
  }
}

//
// DURATION
//

void loopDuration() {
  if (millis() - lastDurationInput > MENU_TIMEOUT_MS) {
    saveConfig();
    returnToQR();
    return;
  }
  if (screenNeedsRedraw) {
    displayDuration(tft, config.activationDuration);
    screenNeedsRedraw = false;
  }
  if (btn1Pressed) {
    lastDurationInput = millis();
    int step = (config.activationDuration <= 300) ? 10 : 60;
    config.activationDuration = max(1, config.activationDuration - step);
    screenNeedsRedraw = true;
  }
  if (btn2Pressed) {
    lastDurationInput = millis();
    int step = (config.activationDuration <= 300) ? 10 : 60;
    config.activationDuration = min(86400, config.activationDuration + step);
    screenNeedsRedraw = true;
  }
}

//
// PIN ENTRY
//

void loopPinEntry() {
  // 15s timeout -> back to settings
  if (millis() - lastPinInput > 15000) {
    currentState = STATE_SETTINGS;
    screenNeedsRedraw = true;
    return;
  }
  // Clear wrong PIN message after 1.5s and reset
  if (pinWrong && millis() - pinWrongTime > 1500) {
    pinWrong = false;
    pinDigitIndex = 0;
    for (int i = 0; i < 4; i++) pinEntry[i] = 0;
    screenNeedsRedraw = true;
  }
  if (screenNeedsRedraw) {
    displayPinEntry(tft, pinEntry, pinDigitIndex, pinWrong);
    screenNeedsRedraw = false;
  }
  if (btn1Pressed && !pinWrong) {
    lastPinInput = millis();
    pinEntry[pinDigitIndex] = (pinEntry[pinDigitIndex] + 1) % 10;
    screenNeedsRedraw = true;
  }
  if (btn2Pressed && !pinWrong) {
    lastPinInput = millis();
    if (pinDigitIndex < 3) {
      pinDigitIndex++;
      screenNeedsRedraw = true;
    } else {
      String entered = "";
      for (int i = 0; i < 4; i++) entered += String(pinEntry[i]);
      if (entered == config.pin) {
        if (pinTargetState == STATE_PRICE) {
          priceOnEnterMenu = config.priceSats;
          lastPriceInput = millis();
          currentState = STATE_PRICE;
        } else {
          lastDurationInput = millis();
          currentState = STATE_DURATION;
        }
        screenNeedsRedraw = true;
      } else {
        pinWrong = true;
        pinWrongTime = millis();
        screenNeedsRedraw = true;
      }
    }
  }
}

//
// AP MODE
//

void startAPMode() {
  WiFi.disconnect();
  delay(500);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("PlugNSat-Setup", "plugnsat21");
  
  setupWebPortal(server, config, prefs);
  server.begin();
  
  currentState = STATE_AP_SETUP;
  screenNeedsRedraw = true;
  apModeStartTime = millis();

  Serial.println("AP: PlugNSat-Setup / plugnsat21 / http://" + WiFi.softAPIP().toString());
}

//
// AP SETUP
//

void loopAPSetup() {
  bool hasWifi = (config.wifiSsid.length() > 0);
  if (screenNeedsRedraw) {
    displayAPMode(tft, WiFi.softAPIP().toString(), hasWifi);
    screenNeedsRedraw = false;
  }
  if ((btn1Pressed || btn2Pressed) && millis() - apModeStartTime > 3000) {
    if (hasWifi) {
      WiFi.softAPdisconnect(true);
      connectToWiFi();
    }
  }
}

//
// BUTTONS
//

void readButtons() {
  btn1Pressed = false;
  btn2Pressed = false;
  
  static bool btn1WasDown = false;
  static bool btn2WasDown = false;
  static unsigned long btn1DownTime = 0;
  static unsigned long btn2DownTime = 0;
  
  bool btn1Down = (digitalRead(BTN_1) == LOW);
  bool btn2Down = (digitalRead(BTN_2) == LOW);
  
  // BTN1: detect press and release
  if (btn1Down && !btn1WasDown) {
    btn1DownTime = millis();  // Just pressed
  }
  if (!btn1Down && btn1WasDown) {
    // Just released
    unsigned long held = millis() - btn1DownTime;
    if (held < 2000 && held > 30) {
      btn1Pressed = true;  // Short press (between 50ms and 2s)
    }
  }
  btn1WasDown = btn1Down;
  
  // BTN2: detect press and release
  if (btn2Down && !btn2WasDown) {
    btn2DownTime = millis();
  }
  if (!btn2Down && btn2WasDown) {
    unsigned long held = millis() - btn2DownTime;
    if (held < 2000 && held > 30) {
      btn2Pressed = true;
    }
  }
  btn2WasDown = btn2Down;
  
  // Long press BTN_1 (3s) = AP mode from any state
  if (btn1Down && (millis() - btn1DownTime > LONG_PRESS_MS)) {
    btn1DownTime = millis();  // Reset to avoid retriggering
    Serial.println("Long press -> AP mode");
    startAPMode();
  }
}

//
// CONFIG
//

void loadConfig() {
  prefs.begin("plugnsat", false);
  config.wifiSsid           = prefs.getString("wifi_ssid", "");
  config.wifiPass           = prefs.getString("wifi_pass", "");
  config.btcpayUrl          = prefs.getString("btcpay_url", "");
  config.btcpayApiKey       = prefs.getString("btcpay_key", "");
  config.btcpayStoreId      = prefs.getString("btcpay_store", "");
  config.blinkApiKey        = prefs.getString("blink_key", "");
  config.blinkWalletId      = prefs.getString("blink_wid", "");
  config.shellyHost         = prefs.getString("shelly_ip", "");
  config.priceSats          = prefs.getInt("price_sats", 100);
  config.activationDuration = prefs.getInt("duration", 60);
  config.deviceName         = prefs.getString("dev_name", "PlugNSat");
  config.brightness         = prefs.getInt("brightness", 40);
  config.pin                = prefs.getString("settings_pin", "");
  config.showName           = prefs.getBool("show_name",  false);
  config.showPrice          = prefs.getBool("show_price", false);
  config.backendType        = (BackendType)prefs.getInt("backend", 0);
  if (config.backendType < 0 || config.backendType >= BACKEND_COUNT)
    config.backendType = BACKEND_BTCPAY;
  config.portalPassword     = prefs.getString("portal_pw", "");
  config.autoUpdate         = prefs.getBool("auto_update", false);
  prefs.end();
}

void saveConfig() {
  prefs.begin("plugnsat", false);
  prefs.putString("wifi_ssid",    config.wifiSsid);
  prefs.putString("wifi_pass",    config.wifiPass);
  prefs.putString("btcpay_url",   config.btcpayUrl);
  prefs.putString("btcpay_key",   config.btcpayApiKey);
  prefs.putString("btcpay_store", config.btcpayStoreId);
  prefs.putString("blink_key",    config.blinkApiKey);
  prefs.putString("blink_wid",    config.blinkWalletId);
  prefs.putString("shelly_ip",    config.shellyHost);
  prefs.putInt("price_sats",      config.priceSats);
  prefs.putInt("duration",        config.activationDuration);
  prefs.putString("dev_name",     config.deviceName);
  prefs.putInt("brightness",      config.brightness);
  prefs.putString("settings_pin", config.pin);
  prefs.putBool("show_name",      config.showName);
  prefs.putBool("show_price",     config.showPrice);
  prefs.putInt("backend",         (int)config.backendType);
  prefs.putString("portal_pw",    config.portalPassword);
  prefs.putBool("auto_update",    config.autoUpdate);
  prefs.end();
}
