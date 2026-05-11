/*
 * PlugNSat - Configuration
 * 
 * Global instuctions, constants, timing with the QRcode et config structure
 * 
 * License: MIT © 2026
 * Author: ezmo-dev (PlugNSat)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Timing
#define POLL_INTERVAL_MS   5000      // Check payment every 5s (was 2s)
#define QR_REFRESH_MS      285000    // Refresh QR at 4min45s (before 5min expiry)
#define INVOICE_EXPIRY_MIN 5         // BTCPay invoice expiration

// Display (T-Display S3 landscape)
#define SCREEN_W 320
#define SCREEN_H 170

// QR code
#define QR_VERSION 10  // LNURL mode uses verson 5-6 (~115 chars) -> For LED screen (impossible to scan BOLT11 QR due to the reflections) 
                      
// Colors
#define COLOR_BG       TFT_BLACK
#define COLOR_TEXT     TFT_WHITE
#define COLOR_ACCENT   0xFD20    // Bitcoin orange
#define COLOR_SUCCESS  0x07E0    // Green
#define COLOR_ERROR    0xF800    // Red
#define COLOR_QR_FG    TFT_BLACK
#define COLOR_QR_BG    TFT_WHITE
#define COLOR_GRAY     0x7BEF

// Config struct
struct PlugNSatConfig {
  String wifiSsid;
  String wifiPass;
  String btcpayUrl;        // Instance URL, e.g. https://btcpay.mydomain.com"
  String btcpayApiKey;
  String btcpayStoreId;
  String shellyHost;        // IP address or mDNS hostname, e.g. 192.168.1.42 or shellyplugsg3-xxxxxxxxxxxx.local
  int priceSats;           // Price per activation
  int activationDuration;  // Seconds relay stays ON after payment
  String deviceName;       // Shown on screen
  int brightness;          // Screen brightness (10-255)
  String pin;              // 4-digit PIN to protect Price/Duration (empty = disabled)
  bool showName;           // Show device name on QR screen
  bool showPrice;          // Show price on QR screen
};

#endif
