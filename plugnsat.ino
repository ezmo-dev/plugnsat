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
 *   - Button 1 (GPIO 0):  Info screen / Long press = AP mode
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
unsigned long btn1LastPress = 0;
unsigned long btn2LastPress = 0;
#define DEBOUNCE_MS 300

// State machine
enum AppState {
  STATE_AP_SETUP,
  STATE_CONNECTING,
  STATE_QR_DISPLAY,
  STATE_PAID,
  STATE_ERROR,
  STATE_INFO
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
      if (screenNeedsRedraw) {
        displayAPMode(tft, WiFi.softAPIP().toString());
        screenNeedsRedraw = false;
      }
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

    default:
      break;
  }
  
  delay(50);
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
    displayStatus(tft, "WiFi connected!", WiFi.localIP().toString());
    delay(1500);

    MDNS.begin("plugnsat");

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
      displayError(tft, "WiFi lost", "Reconnecting...");
      currentState = STATE_ERROR;
      screenNeedsRedraw = true;
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
        displayError(tft, "Server unreachable", "Restarting...");
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
    currentState = STATE_INFO;
    screenNeedsRedraw = true;
  }
  if (btn2Pressed) {
    generateAndShowQR();  // Force refresh
  }
}

void generateAndShowQR() {
  // Brief loading (don't flash screen if just refreshing)
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
      displayError(tft, "BTCPay unreachable",
                   "Config: http://" + WiFi.localIP().toString());
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
  displayPaid(tft, paymentCount);
  
  bool shellyOk = shellySwitchOn(config.shellyHost, config.activationDuration);
  
  if (!shellyOk) {
    Serial.println("WARNING: Shelly failed!");
    displayPaidShellyError(tft);
    delay(4000);
  } else {
    Serial.println("Shelly ON for " + String(config.activationDuration) + "s");
    delay(3000);
  }
  
  // Back to QR automatically
  generateAndShowQR();
}

//
// ERROR (auto-retry after 10s)
//

void loopError() {
  if (millis() - errorStartTime > 10000) {
    consecutiveErrors = 0;
    generateAndShowQR();
    return;
  }
  if (btn1Pressed) startAPMode();
  if (btn2Pressed) { consecutiveErrors = 0; generateAndShowQR(); }
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
  
  Serial.println("AP: PlugNSat-Setup / plugnsat21 / http://" + WiFi.softAPIP().toString());
}

//
// BUTTONS
//

void readButtons() {
  btn1Pressed = false;
  btn2Pressed = false;
  
  if (digitalRead(BTN_1) == LOW && millis() - btn1LastPress > DEBOUNCE_MS) {
    btn1Pressed = true;
    btn1LastPress = millis();
  }
  if (digitalRead(BTN_2) == LOW && millis() - btn2LastPress > DEBOUNCE_MS) {
    btn2Pressed = true;
    btn2LastPress = millis();
  }
  
  // Long press BTN_1 (3s) = AP mode from any state
  static unsigned long btn1HoldStart = 0;
  if (digitalRead(BTN_1) == LOW) {
    if (btn1HoldStart == 0) btn1HoldStart = millis();
    if (millis() - btn1HoldStart > 3000) {
      btn1HoldStart = 0;
      Serial.println("Long press -> AP mode");
      startAPMode();
    }
  } else {
    btn1HoldStart = 0;
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
  prefs.end();
}
