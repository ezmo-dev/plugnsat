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
// SPLASH - Pixel art neon logo (matches new banner)
//
 
void displaySplash(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);
 
  // Colors
  uint16_t orange = tft.color565(247, 147, 26);
  uint16_t cyan   = tft.color565(0, 229, 255);
  uint16_t yellow = tft.color565(255, 215, 0);     // bolt + gold accents
  uint16_t lgray  = tft.color565(200, 200, 210);    // "Lightning Smart Plug" (bright gray)
  uint16_t gold   = tft.color565(255, 215, 0);      // "plugnsat.com" + pill badge
 
  // Pixel font (5 wide x 7 tall)
  static const uint8_t font_P[] = {0xF0,0x88,0x88,0xF0,0x80,0x80,0x80};
  static const uint8_t font_L[] = {0x80,0x80,0x80,0x80,0x80,0x80,0xF8};
  static const uint8_t font_U[] = {0x88,0x88,0x88,0x88,0x88,0x88,0x70};
  static const uint8_t font_G[] = {0x70,0x88,0x80,0xB8,0x88,0x88,0x70};
  static const uint8_t font_S[] = {0x70,0x88,0x80,0x70,0x08,0x88,0x70};
  static const uint8_t font_A[] = {0x70,0x88,0x88,0xF8,0x88,0x88,0x88};
  static const uint8_t font_T[] = {0xF8,0x20,0x20,0x20,0x20,0x20,0x20};
 
  // Lightning bolt (5 wide x 10 tall, matches banner)
  static const uint8_t bolt[] = {
    0x38,  // ..XXX
    0x70,  // .XXX.
    0x70,  // .XXX.
    0x60,  // .XX..
    0xF8,  // XXXXX
    0x70,  // .XXX.
    0x60,  // .XX..
    0x40,  // .X...
    0xC0,  // XX...
    0x80,  // X....
  };
 
  int ps = 5;   // letter pixel size
  int gap = ps;
  int charW = 5 * ps + gap;  // 30
  int bps = 5;  // bolt pixel size (solid, no gaps)
  int boltCols = 5;
  int boltRows = 10;
  int boltW = boltCols * bps;  // 25
  int boltGap = ps * 2;  // 10
 
  int totalW = 4 * charW + boltGap + boltW + boltGap + 3 * charW - gap;
  int logoH = 7 * ps;  // 35
  int boltH = boltRows * bps;  // 50
  int visibleH = (logoH > boltH) ? logoH : boltH;
 
  // Layout vertically centered: block = logo(50) + gaps + line1(16) + line2(8) + pill(17) = 124px
  // topY = (170 - 124) / 2 = 23
  int topY = 23;
  int startX = (SCREEN_W - totalW) / 2;
 
  // Helper: draw a 5x7 font character (with 1px gap between pixels)
  auto drawChar = [&](const uint8_t* f, int cx, int cy, uint16_t color) {
    for (int row = 0; row < 7; row++) {
      uint8_t bits = f[row];
      for (int col = 0; col < 5; col++) {
        if (bits & (0x80 >> col)) {
          tft.fillRect(cx + col * ps, cy + row * ps, ps - 1, ps - 1, color);
        }
      }
    }
  };
 
  int x = startX;
  int letterY = topY + (visibleH - logoH) / 2;
 
  // "PLUG" in orange
  drawChar(font_P, x, letterY, orange); x += charW;
  drawChar(font_L, x, letterY, orange); x += charW;
  drawChar(font_U, x, letterY, orange); x += charW;
  drawChar(font_G, x, letterY, orange); x += charW;
 
  // Lightning bolt in yellow (solid blocks)
  x += boltGap - gap;
  int boltY = topY + (visibleH - boltH) / 2;
  for (int row = 0; row < boltRows; row++) {
    uint8_t bits = bolt[row];
    for (int col = 0; col < boltCols; col++) {
      if (bits & (0x80 >> col)) {
        tft.fillRect(x + col * bps, boltY + row * bps, bps, bps, yellow);
      }
    }
  }
  x += boltW + boltGap;
 
  // "SAT" in cyan
  drawChar(font_S, x, letterY, cyan); x += charW;
  drawChar(font_A, x, letterY, cyan); x += charW;
  drawChar(font_T, x, letterY, cyan);
 
  // === Bottom text block ===
  int cx = SCREEN_W / 2;
 
  // 1. "Lightning Smart Plug" - light gray, textSize 2 (bold/bigger)
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.setTextColor(lgray);
  int line1Y = topY + visibleH + 16;
  tft.drawString("Lightning Smart Plug", cx, line1Y);
 
  // 2. "plugnsat.com" - gold (same as bolt)
  tft.setTextSize(1);
  tft.setTextColor(gold);
  int line2Y = line1Y + 22;
  tft.drawString("plugnsat.com", cx, line2Y);
 
  // 3. "v0.1.0" pill badge - gold outline + gold text
  int pillY = line2Y + 27;
  tft.setTextColor(gold);
  String ver = "v0.1.0";
  int verW = tft.textWidth(ver);
  int pillPadX = 8;
  int pillPadY = 3;
  int pillW = verW + pillPadX * 2;
  int pillH = 11 + pillPadY * 2;
  int pillX = cx - pillW / 2;
  int pillYtop = pillY - pillH / 2;
 
  tft.drawRoundRect(pillX, pillYtop, pillW, pillH, pillH / 2, gold);
  tft.drawString(ver, cx, pillY);
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

void displayPaid(TFT_eSPI &tft, int count, int amountSats, int secondsLeft) {
  tft.fillScreen(COLOR_BG);

  // Layout anchored from bottom: activating=131(fixed), amount=104, PAID!=73, checkmark cy=37
  // Gaps: checkmark→PAID! 5px, PAID!→amount 11px, amount→activating 11px
  int cx = SCREEN_W / 2;
  int cy = 37;
  for (int i = -2; i <= 2; i++) {
    tft.drawLine(cx - 25, cy + i, cx - 8, cy + 17 + i, COLOR_SUCCESS);
    tft.drawLine(cx - 8, cy + 17 + i, cx + 30, cy - 18 + i, COLOR_SUCCESS);
  }

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_SUCCESS);
  tft.setTextSize(3);
  tft.drawString("PAID!", cx, 73);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.drawString(String(amountSats) + " sats", cx, 104);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(2);
  tft.drawString("Activating for " + String(secondsLeft) + "s...", cx, 131);

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

void displayError(TFT_eSPI &tft, String ip, int secondsLeft) {
  tft.fillScreen(COLOR_BG);

  // Layout: circle cy=40, "Error" y=76, config y=103, countdown y=131
  int cx = SCREEN_W / 2;
  int cy = 40;

  // Red ring, 4px thick (radii 19–22)
  for (int r = 19; r <= 22; r++) tft.drawCircle(cx, cy, r, COLOR_ERROR);

  // "!" drawn manually for pixel-perfect centering inside the ring
  // Bar: 4px wide, 12px tall, centered at (cx, cy-2)
  tft.fillRect(cx - 2, cy - 10, 4, 12, COLOR_ERROR);
  // Dot: 4px wide, 4px tall, 3px below bar
  tft.fillRect(cx - 2, cy + 5, 4, 4, COLOR_ERROR);

  // "Error" title — same style as "PAID!" on paid screen
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ERROR);
  tft.setTextSize(3);
  tft.drawString("Error", cx, 76);

  // Config URL
  if (ip.length() > 0 && ip != "0.0.0.0") {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    tft.drawString("http://" + ip, cx, 106);
  }

  // Countdown — same anchor as "Activating" on paid screen
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(2);
  tft.drawString("Auto-retry " + String(secondsLeft) + "s...", cx, 131);
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
  tft.setTextSize(1);

  uint16_t colOrange = tft.color565(247, 147, 26);
  uint16_t colYellow = tft.color565(255, 215, 0);
  uint16_t colCyan   = tft.color565(0, 229, 255);

  // Header: "Plug" orange | "N" yellow | "sat" cyan — size 2 = 12px/char
  tft.setTextDatum(TL_DATUM); tft.setTextSize(2);
  tft.setTextColor(colOrange); tft.drawString("Plug", 10, 6);
  tft.setTextColor(colYellow); tft.drawString("N",    58, 6); // 10 + 4*12
  tft.setTextColor(colCyan);   tft.drawString("sat",  70, 6); // 58 + 1*12

  // Version top-right in yellow, size 1 (small contrast with title)
  tft.setTextDatum(TR_DATUM); tft.setTextSize(1);
  tft.setTextColor(colYellow);
  tft.drawString("v0.1.0", SCREEN_W - 8, 12);

  // Separator "─── Network ──────────────────"  (y=38, more room below header)
  tft.drawFastHLine(8, 34, SCREEN_W - 16, colYellow);
  tft.fillRect(15, 29, 56, 10, COLOR_BG);
  tft.setTextDatum(TL_DATUM); tft.setTextColor(colYellow); tft.setTextSize(1);
  tft.drawString("Network", 17, 30);

  // Layout constants
  int lx = 10;   // label col
  int vx = 80;   // value col
  int lh = 13;   // line height
  int y  = 50;

  // --- Block 1: Connection ---
  tft.setTextDatum(TL_DATUM);

  // WiFi + RSSI badge
  tft.setTextColor(COLOR_GRAY);  tft.drawString("WiFi", lx, y);
  String ssid = WiFi.SSID();
  if (ssid.length() > 16) ssid = ssid.substring(0, 15) + "~";
  tft.setTextColor(COLOR_TEXT);  tft.drawString(ssid, vx, y);
  int rssi = WiFi.RSSI();
  uint16_t rssiCol = (rssi > -50) ? COLOR_SUCCESS
                   : (rssi > -70) ? tft.color565(255, 165, 0)
                                  : COLOR_ERROR;
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(rssiCol);
  tft.drawString(String(rssi) + "dBm", SCREEN_W - 8, y);

  // WiFi signal bars (4 bars, bottom-aligned, white=active gray=inactive)
  // 4/4 > -50 | 3/4 -50→-65 | 2/4 -65→-75 | 1/4 -75→-85 | 0/4 < -85
  int activeBars = (rssi > -50) ? 4 : (rssi > -65) ? 3 : (rssi > -75) ? 2 : (rssi > -85) ? 1 : 0;
  int bx = SCREEN_W - 80;
  int by = y + 7;
  int barHeights[] = {4, 7, 10, 13};
  for (int b = 0; b < 4; b++) {
    uint16_t col = (b < activeBars) ? COLOR_TEXT : COLOR_GRAY;
    tft.fillRect(bx + b * 6, by - barHeights[b] + 1, 4, barHeights[b], col);
  }
  y += lh;

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_GRAY);  tft.drawString("IP", lx, y);
  tft.setTextColor(COLOR_TEXT);  tft.drawString(WiFi.localIP().toString(), vx, y);
  y += lh;

  tft.setTextColor(COLOR_GRAY);  tft.drawString("Shelly", lx, y);
  tft.setTextColor(COLOR_TEXT);  tft.drawString(config.shellyHost, vx, y);
  y += lh;

  // Separator "─── Session ──────────────────"  (y+10, extra breathing room)
  tft.drawFastHLine(8, y + 10, SCREEN_W - 16, colYellow);
  tft.fillRect(15, y + 5, 56, 10, COLOR_BG);
  tft.setTextDatum(TL_DATUM); tft.setTextColor(colYellow); tft.setTextSize(1);
  tft.drawString("Session", 17, y + 6);

  // --- Block 2: Metrics — 4 columns on one row ---
  // Label y=120 (size 1 gray), value y=138 (size 2 white) — shifted down for breathing room
  tft.setTextDatum(MC_DATUM);

  tft.setTextColor(COLOR_GRAY); tft.setTextSize(1);
  tft.drawString("sats", 40, 118);
  tft.setTextColor(COLOR_TEXT); tft.setTextSize(2);
  tft.drawString(String(config.priceSats), 40, 138);

  tft.setTextColor(COLOR_GRAY); tft.setTextSize(1);
  tft.drawString("sec", 120, 118);
  tft.setTextColor(COLOR_TEXT); tft.setTextSize(2);
  tft.drawString(String(config.activationDuration), 120, 138);

  tft.setTextColor(COLOR_GRAY); tft.setTextSize(1);
  tft.drawString("payments", 200, 118);
  tft.setTextColor(COLOR_TEXT); tft.setTextSize(2);
  tft.drawString(String(payments), 200, 138);

  tft.setTextColor(COLOR_GRAY); tft.setTextSize(1);
  tft.drawString("uptime", 280, 118);
  tft.setTextColor(COLOR_TEXT); tft.setTextSize(2);
  tft.drawString(String(millis() / 60000) + "m", 280, 138);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(COLOR_TEXT); tft.setTextSize(1);
  tft.drawString("Any button to exit", SCREEN_W / 2, SCREEN_H - 4);
}

//
// SETTINGS MENU
//

void displaySettings(TFT_eSPI &tft, int selectedIndex, bool pinActive) {
  tft.fillScreen(COLOR_BG);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(1);
  tft.drawString("Settings", SCREEN_W / 2, 12);

  const char* options[] = {"Device Info", "Brightness", "Price", "Duration"};
  int count = 4;
  int startY = 30;
  int rowH = 32;

  // Padlock icon: 10w x 11h total (4px shackle + 7px body)
  auto drawLock = [&](int lx, int ly, bool locked, uint16_t col) {
    tft.fillRect(lx, ly + 4, 10, 7, col);         // body
    tft.fillRect(lx + 2, ly, 6, 2, col);           // arch top bar
    tft.fillRect(lx + 6, ly, 2, 5, col);           // right leg (always)
    if (locked) {
      tft.fillRect(lx + 2, ly, 2, 5, col);         // left leg (closed only)
    }
  };

  for (int i = 0; i < count; i++) {
    int y = startY + i * rowH;
    uint16_t textColor;
    if (i == selectedIndex) {
      tft.fillRoundRect(30, y - 2, SCREEN_W - 60, 24, 5, COLOR_ACCENT);
      textColor = COLOR_BG;
    } else {
      textColor = COLOR_TEXT;
    }
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(textColor);
    tft.setTextSize(1);
    tft.drawString(options[i], SCREEN_W / 2, y + 10);

    if (i == 2 || i == 3) {
      uint16_t lockColor = (i == selectedIndex) ? COLOR_BG : (pinActive ? COLOR_ACCENT : COLOR_GRAY);
      drawLock(37, y + 4, pinActive, lockColor);
    }
  }

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("BTN1: move   BTN2: select", SCREEN_W / 2, SCREEN_H - 5);
}

//
// PRICE
//

void displayPrice(TFT_eSPI &tft, int priceSats) {
  tft.fillScreen(COLOR_BG);

  int mid = SCREEN_W / 2;
  int lcx = mid / 2;        // 80
  int rcx = mid + mid / 2;  // 240

  tft.drawFastVLine(mid, 0, SCREEN_H, COLOR_GRAY);

  // Left: price value
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(3);
  tft.drawString(String(priceSats), lcx, SCREEN_H / 2 - 12);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(2);
  tft.drawString("sats", lcx, SCREEN_H / 2 + 18);

  // Right: controls
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.drawString("-", rcx, 10);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("BTN1", rcx, 30);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("BTN2", rcx, SCREEN_H - 28);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.drawString("+", rcx, SCREEN_H - 8);
}

//
// DURATION
//

void displayDuration(TFT_eSPI &tft, int durationSeconds) {
  tft.fillScreen(COLOR_BG);

  int mid = SCREEN_W / 2;
  int lcx = mid / 2;
  int rcx = mid + mid / 2;

  tft.drawFastVLine(mid, 0, SCREEN_H, COLOR_GRAY);

  // Left: duration value
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(3);
  tft.drawString(String(durationSeconds) + "s", lcx, SCREEN_H / 2 - 14);

  // Conversion below
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  if (durationSeconds >= 3600) {
    int h = durationSeconds / 3600;
    int m = (durationSeconds % 3600) / 60;
    String conv = String(h) + "h";
    if (m > 0) conv += " " + String(m) + "min";
    tft.drawString(conv, lcx, SCREEN_H / 2 + 12);
  } else if (durationSeconds >= 60) {
    int m = durationSeconds / 60;
    tft.drawString(String(m) + " min", lcx, SCREEN_H / 2 + 12);
  }

  // Right: controls
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.drawString("-", rcx, 10);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("BTN1", rcx, 30);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("BTN2", rcx, SCREEN_H - 28);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.drawString("+", rcx, SCREEN_H - 8);
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
  tft.drawString("BTN2", cx, SCREEN_H - 38);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("+", cx, SCREEN_H - 8);
}

//
// PIN ENTRY
//

void displayPinEntry(TFT_eSPI &tft, int digits[], int digitIndex, bool wrongPin) {
  tft.fillScreen(COLOR_BG);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(1);
  tft.drawString("Enter PIN", SCREEN_W / 2, 10);

  int boxW = 44;
  int boxH = 52;
  int boxGap = 10;
  int totalW = 4 * boxW + 3 * boxGap;  // 206
  int startX = (SCREEN_W - totalW) / 2; // 57
  int boxY = 38;

  for (int i = 0; i < 4; i++) {
    int bx = startX + i * (boxW + boxGap);
    uint16_t borderColor;
    if (i == digitIndex)      borderColor = COLOR_ACCENT;
    else if (i < digitIndex)  borderColor = COLOR_GRAY;
    else                      borderColor = 0x2104;

    tft.drawRoundRect(bx, boxY, boxW, boxH, 6, borderColor);
    if (i == digitIndex) {
      tft.fillRoundRect(bx + 1, boxY + 1, boxW - 2, boxH - 2, 5, 0x0820);
    }

    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    if (i < digitIndex) {
      tft.setTextColor(COLOR_TEXT);
      tft.drawString("*", bx + boxW / 2, boxY + boxH / 2);
    } else if (i == digitIndex) {
      tft.setTextColor(COLOR_ACCENT);
      tft.drawString(String(digits[i]), bx + boxW / 2, boxY + boxH / 2);
    } else {
      tft.setTextColor(0x2104);
      tft.drawString("-", bx + boxW / 2, boxY + boxH / 2);
    }
  }

  if (wrongPin) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_ERROR);
    tft.setTextSize(1);
    tft.drawString("Wrong PIN", SCREEN_W / 2, 110);
  }

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("BTN1: change   BTN2: confirm", SCREEN_W / 2, SCREEN_H - 5);
}

//
// AP MODE
//

void displayAPMode(TFT_eSPI &tft, String apIp, bool hasWifi) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);

  int y = 14;
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("1. Connect to WiFi:", SCREEN_W / 2, y); y += 26;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("PlugNSat-Setup", SCREEN_W / 2, y); y += 26;

  tft.setTextColor(COLOR_TEXT);
  tft.drawString("2. Password:", SCREEN_W / 2, y); y += 26;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("plugnsat21", SCREEN_W / 2, y); y += 26;

  tft.setTextColor(COLOR_TEXT);
  tft.drawString("3. Open in browser:", SCREEN_W / 2, y); y += 26;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString("http://" + apIp, SCREEN_W / 2, y);

  tft.setTextDatum(BC_DATUM);
  tft.setTextSize(1);
  if (hasWifi) {
    tft.setTextColor(COLOR_ACCENT);
    tft.drawString("Press any button to reconnect", SCREEN_W / 2, SCREEN_H - 2);
  } else {
    tft.setTextColor(COLOR_GRAY);
    tft.drawString("Follow the 3 steps above to connect", SCREEN_W / 2, SCREEN_H - 2);
  }
}

#endif
