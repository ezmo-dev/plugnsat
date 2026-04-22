/*
 * PlugNSat - Display Module
 * All TFT screen rendering functions : screen flow
 * 
 * LIBRARIES:
 * - TFT_eSPI by Bodmer (display driver for ST7789)
 * - QRCode by Richard Moore (QR code generation)
 * 
 * REFERENCES:
 * - T-Display S3 pinout: github.com/Xinyuan-LilyGO/T-Display-S3
 * - TFT_eSPI User_Setup: Setup206_LilyGo_T_Display_S3.h
 * 
 * License: MIT © 2026
 * Author: ezmo-dev (PlugNSat)
 * 
 */


#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include "config.h"

//
// SPLASH
//

void displaySplash(TFT_eSPI &tft) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(3);
  tft.drawString("PlugNSat", SCREEN_W / 2, SCREEN_H / 2 - 30);

  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(2);
  tft.drawString("Lightning Smart Plug", SCREEN_W / 2, SCREEN_H / 2 + 8);
  tft.setTextSize(1);
  tft.drawString("plugnsat.com", SCREEN_W / 2, SCREEN_H / 2 + 30);
}

//
// CONNECTING
//

void displayConnecting(TFT_eSPI &tft, String ssid) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.drawString("Connecting to WiFi...", SCREEN_W / 2, SCREEN_H / 2 - 12);
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString(ssid, SCREEN_W / 2, SCREEN_H / 2 + 12);
}

//
// STATUS (generic)
//

void displayStatus(TFT_eSPI &tft, String line1, String line2) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.drawString(line1, SCREEN_W / 2, SCREEN_H / 2 - 12);
  if (line2.length() > 0) {
    tft.setTextColor(COLOR_ACCENT);
    tft.drawString(line2, SCREEN_W / 2, SCREEN_H / 2 + 12);
  }
}

//
// GENERATING (brief flash while creating invoice)
//

void displayGenerating(TFT_eSPI &tft) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(2);
  tft.drawString("Generating invoice...", SCREEN_W / 2, SCREEN_H / 2);
}

//
// QR CODE (main screen - stays displayed permanently)
//

void displayQR(TFT_eSPI &tft, String data, int priceSats, String deviceName) {
  tft.fillScreen(TFT_BLACK);
  
  // Uppercase for alphanumeric QR encoding (smaller QR)
  String qrData = data;
  qrData.toUpperCase();
  
  // Pick QR version based on data length
  int version;
  if (qrData.length() < 50) version = 3;
  else if (qrData.length() < 85) version = 5;
  else if (qrData.length() < 120) version = 6;
  else if (qrData.length() < 180) version = 8;
  else version = QR_VERSION;
  
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, qrData.c_str());
  
  int qrSize = qrcode.size;
  
  // Full screen height
  int pixelSize = SCREEN_H / qrSize;
  if (pixelSize < 1) pixelSize = 1;
  
  int qrPixelW = qrSize * pixelSize;
  int qrPixelH = qrSize * pixelSize;
  
  int qrX = (SCREEN_W - qrPixelW) / 2;
  int qrY = (SCREEN_H - qrPixelH) / 2;
  
  for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(
          qrX + x * pixelSize,
          qrY + y * pixelSize,
          pixelSize, pixelSize,
          TFT_WHITE
        );
      }
    }
  }
}
  
//
// PAID
//

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

//
// ERROR
//

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

//
// IF WIFI FAILED
//

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

//
// INFO
//

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
  tft.drawString("Shelly: " + config.shellyHost, 10, y); y += lh;
  tft.drawString("Price: " + String(config.priceSats) + " sats / " 
                 + String(config.activationDuration) + "s", 10, y); y += lh;
  tft.drawString("Payments this session: " + String(payments), 10, y); y += lh;
  tft.drawString("Uptime: " + String(millis() / 60000) + " min", 10, y); y += lh;
  
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("Config: http://" + WiFi.localIP().toString(), 10, y);
  
  tft.setTextDatum(BC_DATUM);
  tft.drawString("Press any button to return", SCREEN_W / 2, SCREEN_H - 5);
}

//
// SETTINGS MENU
//

void displaySettings(TFT_eSPI &tft, int selectedIndex) {
  tft.fillScreen(COLOR_BG);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(1);
  tft.drawString("Settings", SCREEN_W / 2, 12);

  const char* options[] = {"Device Info", "Brightness"};
  int count = 2;
  int startY = 48;
  int rowH = 36;

  for (int i = 0; i < count; i++) {
    int y = startY + i * rowH;
    if (i == selectedIndex) {
      tft.fillRoundRect(30, y - 2, SCREEN_W - 60, 24, 5, COLOR_ACCENT);
      tft.setTextColor(COLOR_BG);
    } else {
      tft.setTextColor(COLOR_TEXT);
    }
    tft.setTextDatum(MC_DATUM);
    tft.drawString(options[i], SCREEN_W / 2, y + 10);
  }

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("BTN1: move   BTN2: select", SCREEN_W / 2, SCREEN_H - 5);
}

//
// BRIGHTNESS
//

void displayBrightness(TFT_eSPI &tft, int brightness, String qrData) {
  tft.fillScreen(COLOR_BG);

  // Left: QR code preview on white background
  int leftW = 152;
  tft.fillRect(0, 0, leftW, SCREEN_H, TFT_WHITE);

  String qr = (qrData.length() > 0) ? qrData : "PLUGNSAT";
  qr.toUpperCase();

  int version;
  if (qr.length() < 50)       version = 3;
  else if (qr.length() < 85)  version = 5;
  else if (qr.length() < 120) version = 6;
  else if (qr.length() < 180) version = 8;
  else                         version = QR_VERSION;

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, qr.c_str());

  int qrSize  = qrcode.size;
  int pixSize = (leftW - 8) / qrSize;
  if (pixSize < 1) pixSize = 1;
  int qrPixW = qrSize * pixSize;
  int qrPixH = qrSize * pixSize;
  int qrX    = (leftW - qrPixW) / 2;
  int qrY    = (SCREEN_H - qrPixH) / 2;

  for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(qrX + x * pixSize, qrY + y * pixSize,
                     pixSize, pixSize, TFT_BLACK);
      }
    }
  }

  tft.drawFastVLine(leftW, 0, SCREEN_H, COLOR_GRAY);

  // Right: brightness controls
  int cx = leftW + (SCREEN_W - leftW) / 2;  // ~236

  // "-" at top (BTN1 = decrease)
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.drawString("-", cx, 10);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("BTN1", cx, 30);

  // Vertical bar
  int barH = 60;
  int barW = 14;
  int barX = cx - barW / 2;
  int barY = 44;

  tft.fillRect(barX, barY, barW, barH, 0x1082);
  tft.drawRect(barX - 1, barY - 1, barW + 2, barH + 2, COLOR_GRAY);

  int fillH = map(brightness, 0, 255, 0, barH);
  if (fillH > 0) {
    tft.fillRect(barX, barY + barH - fillH, barW, fillH, COLOR_ACCENT);
  }

  // Percentage below bar
  int pct = map(brightness, 0, 255, 0, 100);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(1);
  tft.drawString(String(pct) + "%", cx, barY + barH + 6);

  // "+" at bottom (BTN2 = increase)
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("BTN2", cx, SCREEN_H - 28);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("+", cx, SCREEN_H - 8);
}

//
// AP MODE
//

void displayAPMode(TFT_eSPI &tft, String apIp) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextSize(2);

  int y = 20;
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("1. Connect to WiFi:", SCREEN_W / 2, y); y += 28;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("PlugNSat-Setup", SCREEN_W / 2, y); y += 28;

  tft.setTextColor(COLOR_TEXT);
  tft.drawString("2. Password:", SCREEN_W / 2, y); y += 28;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("plugnsat21", SCREEN_W / 2, y); y += 28;

  tft.setTextColor(COLOR_TEXT);
  tft.drawString("3. Open in browser:", SCREEN_W / 2, y); y += 28;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("http://" + apIp, SCREEN_W / 2, y);
}

#endif
