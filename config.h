/*
 * PlugNSat - Configuration
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Timing
#define POLL_INTERVAL_MS   2000      // Check payment every 2s
#define QR_REFRESH_MS      285000    // Refresh QR at 4min45s (before 5min expiry)
#define INVOICE_EXPIRY_MIN 5         // BTCPay invoice expiration

// Display (T-Display S3 landscape)
#define SCREEN_W 320
#define SCREEN_H 170

// QR code
#define QR_VERSION 10  // Holds ~271 chars alphanumeric (enough for BOLT11)

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
  String btcpayUrl;        // "https://pay.ezmo.dev"
  String btcpayApiKey;
  String btcpayStoreId;
  String shellyIp;         // "192.168.1.42"
  int priceSats;           // Price per activation
  int activationDuration;  // Seconds relay stays ON
  String deviceName;       // Shown on screen
};

#endif
