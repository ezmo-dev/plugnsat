/*
 * PlugNSat - Main Firmware
 * Lightning Smart Plug Controller
 * Version: 0.1.0
 * Hardware: LilyGO T-Display S3 (ESP32-S3 + 1.9" LCD)
 * 
 * BASED ON:
 * - BitcoinSwitch by @arcbtc: github.com/arcbtc/bitcoinSwitch
 * - LNPoS by @arcbtc: github.com/arcbtc/LNPoS
 * 
 * HARDWARE:
 * - LilyGO T-Display S3 (ESP32-S3 + ST7789 1.9" LCD)
 * - Shelly Plug S Gen3 (WiFi relay, CE certified, 2500W)
 * - BTCPay Server with Lightning (Phoenixd, LND, CLN)
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
String currentBolt11    = "";
unsigned long invoiceCreatedAt = 0;
unsigned long lastPollTime     = 0;

// Error tracking
int consecutiveErrors = 0;
unsigned long errorStartTime = 0;

// Session stats
int paymentCount = 0;

// Settings menu
int settingsIndex = 0;
unsigned long lastSettingsInput = 0;
unsigned long lastBrightnessInput = 0;
unsigned long lastPriceInput = 0;
unsigned long lastDurationInput = 0;

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

// Display
bool screenNeedsRedraw = true;

// Config
PlugNSatConfig config;

//
// SETUP
//

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== PlugNSat v0.2.0 ===");

  tft.init();
  tft.setRotation(1);
  ledcAttach(38, 5000, 8);
  ledcWrite(38, 40);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);

  loadConfig();
  ledcWrite(38, config.brightness);
  displaySplash(tft);
  delay(5000);

  if (config.wifiSsid.length() == 0 || config.btcpayUrl.length() == 0) {
    startAPMode();
  } else {
    connectToWiFi();
  }
}

//
// MAIN LOOP
//

void loop() {
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
    MDNS.begin("plugnsat");
    displayStatus(tft, "WiFi connected!", WiFi.localIP().toString());
    delay(1500);

    setupWebPortal(server, config, prefs);
    server.begin();

    generateAndShowQR();
  } else {
    Serial.println("WiFi FAILED");
    displayWiFiFailed(tft, config.wifiSsid);
    
    // Wait for user input or auto-retry
    unsigned long t = millis();
    while (millis() - t < 10000) {
      readButtons();
      if (btn1Pressed) { startAPMode(); return; }
      if (btn2Pressed) { connectToWiFi(); return; }
      server.handleClient();
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
  
  // Auto-refresh QR before expiry (4min 45s = 285000ms)
  if (millis() - invoiceCreatedAt > QR_REFRESH_MS) {
    Serial.println("QR refresh (expiry prevention)");
    generateAndShowQR();
    return;
  }
  
  // Poll BTCPay every 2s
  if (millis() - lastPollTime > POLL_INTERVAL_MS) {
    lastPollTime = millis();
    
    String status = btcpayCheckInvoice(
      config.btcpayUrl, config.btcpayApiKey,
      config.btcpayStoreId, currentInvoiceId
    );
    
    if (status == "ERROR") {
      consecutiveErrors++;
      Serial.println("Poll error #" + String(consecutiveErrors));
      if (consecutiveErrors >= 10) {
        displayError(tft, WiFi.localIP().toString(), 5);
        delay(5000);
        ESP.restart();
      }
      return;
    }
    
    consecutiveErrors = 0;
    
    if (status == "Settled" || status == "Processing") {
      Serial.println("*** PAYMENT RECEIVED ***");
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

void generateAndShowQR() {
  displayGenerating(tft);
  
  String invoiceId, bolt11;
  
  bool ok = btcpayCreateInvoice(
    config.btcpayUrl, config.btcpayApiKey,
    config.btcpayStoreId, config.priceSats,
    invoiceId, bolt11
  );
  
  if (ok && bolt11.length() > 0) {
    currentInvoiceId = invoiceId;
    currentBolt11 = bolt11;
    invoiceCreatedAt = millis();
    lastPollTime = millis();
    consecutiveErrors = 0;
    
    displayQR(tft, bolt11, config.priceSats, config.deviceName);
    currentState = STATE_QR_DISPLAY;
    
    Serial.println("QR ready. Invoice: " + invoiceId + " (" + String(bolt11.length()) + " chars)");
  } else {
    consecutiveErrors++;
    
    if (consecutiveErrors >= 3) {
      currentState = STATE_ERROR;
      errorStartTime = millis();
    } else {
      delay(2000);
      generateAndShowQR();  // Retry (max 3x via recursion)
    }
  }
}

//
// PAID
//

void loopPaid() {
  paymentCount++;

  // Max brightness for paid screen
  ledcWrite(38, 255);

  bool shellyOk = shellySwitchOn(config.shellyHost, config.activationDuration);

  if (!shellyOk) {
    Serial.println("WARNING: Shelly failed!");
  } else {
    Serial.println("Shelly ON for " + String(config.activationDuration) + "s");
  }

  for (int s = config.activationDuration; s > 0; s--) {
    displayPaid(tft, paymentCount, config.priceSats, s);
    if (!shellyOk) displayPaidShellyError(tft);
    delay(1000);
  }

  // Restore brightness
  ledcWrite(38, config.brightness);

  // Back to QR automatically
  generateAndShowQR();
}

//
// ERROR (auto-retry after 10s)
//

void loopError() {
  ledcWrite(38, 255);

  int secondsLeft = max(0, 10 - (int)((millis() - errorStartTime) / 1000));

  static int lastDisplayedSecond = -1;
  if (secondsLeft != lastDisplayedSecond) {
    lastDisplayedSecond = secondsLeft;
    displayError(tft, WiFi.localIP().toString(), secondsLeft);
  }

  if (millis() - errorStartTime > 10000) {
    consecutiveErrors = 0;
    ledcWrite(38, config.brightness);
    generateAndShowQR();
    return;
  }
  if (btn1Pressed) { ledcWrite(38, config.brightness); startAPMode(); }
  if (btn2Pressed) { ledcWrite(38, config.brightness); consecutiveErrors = 0; generateAndShowQR(); }
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
    if (currentBolt11.length() > 0) {
      displayQR(tft, currentBolt11, config.priceSats, config.deviceName);
      currentState = STATE_QR_DISPLAY;
    } else {
      generateAndShowQR();
    }
  }
}

//
// SETTINGS
//

void loopSettings() {
  // Auto-timeout 6s -> back to QR
  if (millis() - lastSettingsInput > 6000) {
    if (currentBolt11.length() > 0) {
      displayQR(tft, currentBolt11, config.priceSats, config.deviceName);
      currentState = STATE_QR_DISPLAY;
    } else {
      generateAndShowQR();
    }
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
  if (millis() - lastBrightnessInput > 3000) {
    saveConfig();
    if (currentBolt11.length() > 0) {
      displayQR(tft, currentBolt11, config.priceSats, config.deviceName);
      currentState = STATE_QR_DISPLAY;
    } else {
      generateAndShowQR();
    }
    return;
  }
  if (screenNeedsRedraw) {
    displayBrightness(tft, config.brightness, currentBolt11);
    screenNeedsRedraw = false;
  }
  if (btn1Pressed) {
    lastBrightnessInput = millis();
    config.brightness = max(10, config.brightness - 10);
    ledcWrite(38, config.brightness);
    screenNeedsRedraw = true;
  }
  if (btn2Pressed) {
    lastBrightnessInput = millis();
    config.brightness = min(255, config.brightness + 10);
    ledcWrite(38, config.brightness);
    screenNeedsRedraw = true;
  }
}

//
// PRICE
//

void loopPrice() {
  if (millis() - lastPriceInput > 3000) {
    saveConfig();
    if (currentBolt11.length() > 0) {
      displayQR(tft, currentBolt11, config.priceSats, config.deviceName);
      currentState = STATE_QR_DISPLAY;
    } else {
      generateAndShowQR();
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
  if (millis() - lastDurationInput > 3000) {
    saveConfig();
    if (currentBolt11.length() > 0) {
      displayQR(tft, currentBolt11, config.priceSats, config.deviceName);
      currentState = STATE_QR_DISPLAY;
    } else {
      generateAndShowQR();
    }
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
  if (btn1Down && (millis() - btn1DownTime > 3000)) {
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
  config.shellyHost         = prefs.getString("shelly_ip", "");
  config.priceSats          = prefs.getInt("price_sats", 100);
  config.activationDuration = prefs.getInt("duration", 60);
  config.deviceName         = prefs.getString("dev_name", "PlugNSat");
  config.brightness         = prefs.getInt("brightness", 40);
  config.pin                = prefs.getString("settings_pin", "");
  prefs.end();
}

void saveConfig() {
  prefs.begin("plugnsat", false);
  prefs.putString("wifi_ssid",    config.wifiSsid);
  prefs.putString("wifi_pass",    config.wifiPass);
  prefs.putString("btcpay_url",   config.btcpayUrl);
  prefs.putString("btcpay_key",   config.btcpayApiKey);
  prefs.putString("btcpay_store", config.btcpayStoreId);
  prefs.putString("shelly_ip",    config.shellyHost);
  prefs.putInt("price_sats",      config.priceSats);
  prefs.putInt("duration",        config.activationDuration);
  prefs.putString("dev_name",     config.deviceName);
  prefs.putInt("brightness",      config.brightness);
  prefs.putString("settings_pin", config.pin);
  prefs.end();
}
