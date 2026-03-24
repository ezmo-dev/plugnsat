/*
 * PlugNSat - Display Module
 * All TFT screen rendering functions
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include "config.h"
#include "qrcode.h"

// ============================================================
// SPLASH
// ============================================================

void displaySplash(TFT_eSPI &tft) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(3);
  tft.drawString("PlugNSat", SCREEN_W / 2, SCREEN_H / 2 - 20);
  
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("Lightning Smart Plug v0.2", SCREEN_W / 2, SCREEN_H / 2 + 15);
  tft.drawString("plugnsat.com", SCREEN_W / 2, SCREEN_H / 2 + 35);
}

// ============================================================
// CONNECTING
// ============================================================

void displayConnecting(TFT_eSPI &tft, String ssid) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  tft.drawString("Connecting to WiFi...", SCREEN_W / 2, SCREEN_H / 2 - 15);
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString(ssid, SCREEN_W / 2, SCREEN_H / 2 + 5);
}

// ============================================================
// STATUS (generic)
// ============================================================

void displayStatus(TFT_eSPI &tft, String line1, String line2) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  tft.drawString(line1, SCREEN_W / 2, SCREEN_H / 2 - 10);
  if (line2.length() > 0) {
    tft.setTextColor(COLOR_ACCENT);
    tft.drawString(line2, SCREEN_W / 2, SCREEN_H / 2 + 10);
  }
}

// ============================================================
// GENERATING (brief flash while creating invoice)
// ============================================================

void displayGenerating(TFT_eSPI &tft) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("Generating invoice...", SCREEN_W / 2, SCREEN_H / 2);
}

// ============================================================
// QR CODE (main screen - stays displayed permanently)
// ============================================================

void displayQR(TFT_eSPI &tft, String bolt11, int priceSats, String deviceName) {
  tft.fillScreen(COLOR_BG);
  
  // BOLT11 to uppercase for QR alphanumeric encoding (smaller QR)
  String upper = bolt11;
  upper.toUpperCase();
  
  // Add "lightning:" prefix for wallet auto-detection
  String qrData = "LIGHTNING:" + upper;
  
  // Generate QR
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(QR_VERSION)];
  
  // Try encoding. If data is too long for version, increase version
  int version = QR_VERSION;
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, qrData.c_str());
  
  int qrSize = qrcode.size;
  
  // Calculate pixel size to fit screen height with padding
  int maxH = SCREEN_H - 16;  // 8px padding top and bottom
  int pixelSize = maxH / qrSize;
  if (pixelSize < 1) pixelSize = 1;
  
  int qrPixelW = qrSize * pixelSize;
  int qrPixelH = qrSize * pixelSize;
  int qrX = 8;  // Left aligned with small margin
  int qrY = (SCREEN_H - qrPixelH) / 2;
  
  // White background behind QR with quiet zone
  int quietZone = 4;
  tft.fillRect(qrX - quietZone, qrY - quietZone, 
               qrPixelW + quietZone * 2, qrPixelH + quietZone * 2, COLOR_QR_BG);
  
  // Draw QR modules
  for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(
          qrX + x * pixelSize,
          qrY + y * pixelSize,
          pixelSize, pixelSize,
          COLOR_QR_FG
        );
      }
    }
  }
  
  // Right side text area
  int textX = qrX + qrPixelW + quietZone + 12;
  
  // Device name at top
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(1);
  tft.drawString(deviceName, textX, 8);
  
  // Price (big)
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(3);
  tft.drawString(String(priceSats), textX, 30);
  
  // "sats" label
  tft.setTextSize(1);
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("sats", textX, 58);
  
  // Lightning bolt indicator
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(2);
  tft.drawString("~", textX, 80);  // Lightning symbol placeholder
  
  // Scan instruction
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("Scan to pay", textX, 110);
  
  // WiFi status dot (bottom right)
  if (WiFi.status() == WL_CONNECTED) {
    tft.fillCircle(SCREEN_W - 10, SCREEN_H - 10, 4, COLOR_SUCCESS);
  } else {
    tft.fillCircle(SCREEN_W - 10, SCREEN_H - 10, 4, COLOR_ERROR);
  }
  
  // Button hints (very small, at bottom)
  tft.setTextColor(0x4208);  // Very dark gray
  tft.setTextDatum(BL_DATUM);
  tft.drawString("[1]Info", textX, SCREEN_H - 2);
  tft.setTextDatum(BR_DATUM);
  tft.drawString("[2]New", SCREEN_W - 20, SCREEN_H - 2);
}

// ============================================================
// PAID
// ============================================================

void displayPaid(TFT_eSPI &tft, int count) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  
  // Big checkmark using lines (thick)
  int cx = SCREEN_W / 2;
  int cy = SCREEN_H / 2 - 25;
  for (int i = -2; i <= 2; i++) {
    tft.drawLine(cx - 25, cy + i, cx - 8, cy + 17 + i, COLOR_SUCCESS);
    tft.drawLine(cx - 8, cy + 17 + i, cx + 30, cy - 18 + i, COLOR_SUCCESS);
  }
  
  tft.setTextColor(COLOR_SUCCESS);
  tft.setTextSize(2);
  tft.drawString("PAID!", cx, cy + 40);
  
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("Activating device...", cx, cy + 60);
  
  // Payment counter
  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(0x4208);
  tft.drawString("#" + String(count), SCREEN_W - 5, SCREEN_H - 5);
}

void displayPaidShellyError(TFT_eSPI &tft) {
  // Overlay warning at bottom
  tft.fillRect(0, SCREEN_H - 25, SCREEN_W, 25, COLOR_ERROR);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  tft.drawString("Warning: Shelly not responding!", SCREEN_W / 2, SCREEN_H - 12);
}

// ============================================================
// ERROR
// ============================================================

void displayError(TFT_eSPI &tft, String title, String detail) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(COLOR_ERROR);
  tft.setTextSize(2);
  tft.drawString(title, SCREEN_W / 2, SCREEN_H / 2 - 20);
  
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  tft.drawString(detail, SCREEN_W / 2, SCREEN_H / 2 + 10);
  
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("[1] Setup  [2] Retry  (auto 10s)", SCREEN_W / 2, SCREEN_H - 12);
}

// ============================================================
// WIFI FAILED
// ============================================================

void displayWiFiFailed(TFT_eSPI &tft, String ssid) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(COLOR_ERROR);
  tft.setTextSize(2);
  tft.drawString("WiFi Failed", SCREEN_W / 2, 30);
  
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  tft.drawString("Could not connect to:", SCREEN_W / 2, 65);
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString(ssid, SCREEN_W / 2, 85);
  
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("[1] Open setup page", SCREEN_W / 2, 120);
  tft.drawString("[2] Retry connection", SCREEN_W / 2, 140);
  tft.drawString("Auto-retry in 10s...", SCREEN_W / 2, SCREEN_H - 10);
}

// ============================================================
// INFO
// ============================================================

void displayInfo(TFT_eSPI &tft, PlugNSatConfig &config, int payments) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  
  int y = 8;
  int lh = 17;
  
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("PlugNSat Info", 10, y); y += lh + 4;
  
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("WiFi: " + WiFi.SSID(), 10, y); y += lh;
  tft.drawString("IP: " + WiFi.localIP().toString(), 10, y); y += lh;
  tft.drawString("RSSI: " + String(WiFi.RSSI()) + " dBm", 10, y); y += lh;
  tft.drawString("Shelly: " + config.shellyIp, 10, y); y += lh;
  tft.drawString("Price: " + String(config.priceSats) + " sats / " 
                 + String(config.activationDuration) + "s", 10, y); y += lh;
  tft.drawString("Payments this session: " + String(payments), 10, y); y += lh;
  tft.drawString("Uptime: " + String(millis() / 60000) + " min", 10, y); y += lh;
  
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("Config: http://" + WiFi.localIP().toString(), 10, y);
  
  tft.setTextDatum(BC_DATUM);
  tft.drawString("Press any button to return", SCREEN_W / 2, SCREEN_H - 5);
}

// ============================================================
// AP MODE
// ============================================================

void displayAPMode(TFT_eSPI &tft, String apIp) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(2);
  tft.drawString("PlugNSat Setup", SCREEN_W / 2, 20);
  
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  
  int y = 55;
  tft.drawString("1. Connect to WiFi:", SCREEN_W / 2, y); y += 18;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("PlugNSat-Setup", SCREEN_W / 2, y); y += 22;
  
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("2. Password:", SCREEN_W / 2, y); y += 18;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("plugnsat21", SCREEN_W / 2, y); y += 22;
  
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("3. Open in browser:", SCREEN_W / 2, y); y += 18;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("http://" + apIp, SCREEN_W / 2, y);
}

#endif
