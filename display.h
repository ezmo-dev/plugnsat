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
        tft.fillRect(x + col * bps, boltY + row * bps, bps - 1, bps - 1, yellow);
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
// QR RENDER UTILITY
//

struct QRResult { int qrPixW; int qrPixH; };

// Returns the rendered pixel side length without initialising a full QRCode object.
// Used by callers that need to compute position before rendering.
static int qrSide(const String &data, int maxH) {
  String d = data; d.toUpperCase();
  int version = (d.length() < 50) ? 3 : (d.length() < 85) ? 5 :
                (d.length() < 120) ? 6 : (d.length() < 180) ? 8 : QR_VERSION;
  int qrSize  = 4 * version + 17;
  int ps      = maxH / qrSize;
  if (ps < 1) ps = 1;
  return qrSize * ps;
}

// Renders a QR code at (x, y) top-left, constrained to maxH pixels tall.
QRResult renderQR(TFT_eSPI &tft, String data, int x, int y, int maxH) {
  data.toUpperCase();
  int version = (data.length() < 50) ? 3 : (data.length() < 85) ? 5 :
                (data.length() < 120) ? 6 : (data.length() < 180) ? 8 : QR_VERSION;

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, data.c_str());

  int qrSize    = qrcode.size;
  int pixelSize = maxH / qrSize;
  if (pixelSize < 1) pixelSize = 1;
  int qrPixW = qrSize * pixelSize;
  int qrPixH = qrSize * pixelSize;

  for (int row = 0; row < qrSize; row++) {
    for (int col = 0; col < qrSize; col++) {
      if (qrcode_getModule(&qrcode, col, row)) {
        tft.fillRect(x + col * pixelSize, y + row * pixelSize, pixelSize, pixelSize, TFT_WHITE);
      }
    }
  }
  return {qrPixW, qrPixH};
}

//
// QR CODE (main screen - stays displayed permanently)
//

void displayQR(TFT_eSPI &tft, String data, int priceSats, String deviceName) {
  tft.fillScreen(TFT_BLACK);
  
  int side = qrSide(data, SCREEN_H - 2);
  renderQR(tft, data, (SCREEN_W - side) / 2, (SCREEN_H - side) / 2, SCREEN_H - 2);
}
  
//
// QR CODE WITH INFO PANEL (right side: mini logo + optional name/price)
//

void displayQRWithInfo(TFT_eSPI &tft, String data, int priceSats, String deviceName, bool showName, bool showPrice) {
  tft.fillScreen(TFT_BLACK);

  int side        = qrSide(data, SCREEN_H - 2);
  int qrX         = 7;
  int qrY         = (SCREEN_H - side) / 2;
  QRResult qrDim  = renderQR(tft, data, qrX, qrY, SCREEN_H - 2);

  // Right info panel — starts at the actual right edge of the QR (qrX + qrPixW)
  int panelLeft = qrX + qrDim.qrPixW;
  int panelCX   = (panelLeft + SCREEN_W) / 2;

  uint16_t orange = tft.color565(247, 147, 26);
  uint16_t yellow = tft.color565(255, 215, 0);
  uint16_t cyan   = tft.color565(0, 229, 255);

  static const uint8_t font_P[] = {0xF0,0x88,0x88,0xF0,0x80,0x80,0x80};
  static const uint8_t font_L[] = {0x80,0x80,0x80,0x80,0x80,0x80,0xF8};
  static const uint8_t font_U[] = {0x88,0x88,0x88,0x88,0x88,0x88,0x70};
  static const uint8_t font_G[] = {0x70,0x88,0x80,0xB8,0x88,0x88,0x70};
  static const uint8_t font_S[] = {0x70,0x88,0x80,0x70,0x08,0x88,0x70};
  static const uint8_t font_A[] = {0x70,0x88,0x88,0xF8,0x88,0x88,0x88};
  static const uint8_t font_T[] = {0xF8,0x20,0x20,0x20,0x20,0x20,0x20};
  static const uint8_t bolt[]   = {0x38,0x70,0x70,0x60,0xF8,0x70,0x60,0x40,0xC0,0x80};

  int ps      = 2;
  int gap     = ps;
  int charW   = 5 * ps + gap;   // 12
  int bps     = 2;
  int boltW   = 5 * bps;        // 10
  int boltGap = ps * 2;         // 4
  // totalW = 4*12 + 4 + 10 + 4 + 3*12 - 2 = 100
  int totalW  = 4 * charW + boltGap + boltW + boltGap + 3 * charW - gap;
  int logoH   = 7 * ps;         // 14
  int boltH   = 10 * bps;       // 20
  int visibleH = boltH;         // bolt is tallest

  int startX = max(panelLeft + 1, panelCX - totalW / 2);
  int topY   = 14;

  auto drawChar = [&](const uint8_t* f, int cx, int cy, uint16_t color) {
    for (int row = 0; row < 7; row++) {
      uint8_t bits = f[row];
      for (int col = 0; col < 5; col++) {
        if (bits & (0x80 >> col))
          tft.fillRect(cx + col * ps, cy + row * ps, ps - 1, ps - 1, color);
      }
    }
  };

  int x       = startX;
  int letterY = topY + (visibleH - logoH) / 2;

  drawChar(font_P, x, letterY, orange); x += charW;
  drawChar(font_L, x, letterY, orange); x += charW;
  drawChar(font_U, x, letterY, orange); x += charW;
  drawChar(font_G, x, letterY, orange); x += charW;

  x += boltGap - gap;
  for (int row = 0; row < 10; row++) {
    uint8_t bits = bolt[row];
    for (int col = 0; col < 5; col++) {
      if (bits & (0x80 >> col))
        tft.fillRect(x + col * bps, topY + row * bps, bps - 1, bps - 1, yellow);
    }
  }
  x += boltW + boltGap;

  drawChar(font_S, x, letterY, cyan); x += charW;
  drawChar(font_A, x, letterY, cyan); x += charW;
  drawChar(font_T, x, letterY, cyan);

  // Char-based split: max 9 chars per line (enforced by webportal validation)
  bool name2Lines = (showName && (int)deviceName.length() > 9);
  int nameSplit = 9;
  if (name2Lines) {
    for (int i = 8; i >= 1; i--) {
      if (deviceName[i] == ' ') { nameSplit = i; break; }
    }
  }

  // Vertical positioning:
  // - Both active  → stack normally, block centered in remaining space
  // - One active   → that element centered at the mid-point of the remaining space
  int remaining = SCREEN_H - (topY + visibleH);
  int centerY   = topY + visibleH + remaining / 2 - 8;  // mid-point shifted up
  int curY;
  bool hasName  = showName && deviceName.length() > 0;
  if (hasName && showPrice) {
    int contentH = 10 + (name2Lines ? 34 : 16) + 12 + 54;
    curY = topY + visibleH + max(0, (remaining - contentH) / 2) - 8;
  } else if (hasName) {
    curY = centerY - 10 - (name2Lines ? 17 : 8);
  } else if (showPrice) {
    curY = centerY - 37;
  } else {
    curY = topY + visibleH;
  }

  tft.setTextDatum(MC_DATUM);

  if (showName && deviceName.length() > 0) {
    curY += 10;
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    if (!name2Lines) {
      tft.drawString(deviceName, panelCX, curY + 8);
      curY += 16;
    } else {
      String line1 = deviceName.substring(0, nameSplit);
      bool atSpace = (nameSplit < (int)deviceName.length() && deviceName[nameSplit] == ' ');
      String line2 = deviceName.substring(atSpace ? nameSplit + 1 : nameSplit);
      if ((int)line2.length() > 9) line2 = line2.substring(0, 9);
      tft.drawString(line1, panelCX, curY + 8);
      tft.drawString(line2, panelCX, curY + 26);
      curY += 34;
    }
  }

  if (showPrice) {
    curY += 12;
    tft.setTextColor(COLOR_ACCENT);
    tft.setTextSize(3);
    tft.drawString(String(priceSats), panelCX, curY + 12);
    curY += 34;
    tft.setTextColor(COLOR_ACCENT);
    tft.setTextSize(1);
    tft.drawString("sats", panelCX, curY + 4);
  }
}

//
// PAID
//

void displayPaid(TFT_eSPI &tft, int count, int amountSats, int secondsLeft, int totalDuration) {
  tft.fillScreen(COLOR_BG);

  // Vertical divider (gray, visible)
  tft.drawFastVLine(160, 20, 130, 0x4208);

  // --- LEFT ZONE (cx=80) ---
  // lcy=73: outer ring top=30, PAID bottom=140 → block centered at 85 in zone[20,150]
  int lcx = 80;
  int lcy = 73;

  uint16_t darkGreen = tft.color565(0, 55, 0);
  tft.drawCircle(lcx, lcy, 43, darkGreen);   // second outer ring
  tft.drawCircle(lcx, lcy, 35, darkGreen);   // first outer ring
  tft.fillCircle(lcx, lcy, 28, tft.color565(10, 46, 20));
  tft.drawCircle(lcx, lcy, 28, COLOR_SUCCESS);

  // Checkmark (3px thick)
  for (int i = -1; i <= 1; i++) {
    tft.drawLine(lcx - 15, lcy + i,      lcx - 3,  lcy + 12 + i, COLOR_SUCCESS);
    tft.drawLine(lcx - 3,  lcy + 12 + i, lcx + 17, lcy - 8 + i,  COLOR_SUCCESS);
  }

  // "PAID" — 8px gap below outer ring bottom (73+43=116 → text center at 132)
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_SUCCESS);
  tft.setTextSize(2);
  tft.drawString("P A I D", lcx, 132);

  // --- RIGHT ZONE (cx=240) ---
  int rcx = 240;

  // Amount (larger)
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(4);
  tft.drawString(String(amountSats), rcx, 46);

  // "SATS"
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("SATS", rcx, 72);

  // Countdown
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(3);
  tft.drawString(String(secondsLeft) + "s", rcx, 112);

  // Progress bar
  tft.fillRect(190, 140, 100, 3, 0x1082);
  int fillW = (totalDuration > 0) ? map(secondsLeft, 0, totalDuration, 0, 100) : 0;
  if (fillW > 0) tft.fillRect(190, 140, fillW, 3, COLOR_ACCENT);

  // Payment counter
  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(0x3186);
  tft.setTextSize(1);
  tft.drawString("#" + String(count), 315, 165);
}


void displayShellyOffline(TFT_eSPI &tft, int secondsLeft) {
  tft.fillScreen(COLOR_BG);

  // Vertical divider
  tft.drawFastVLine(160, 20, 130, 0x4208);

  // --- LEFT ZONE (cx=80) — same style as displayPaid ---
  // lcy=66: ring bottom at 109, text starts at 122 (13px gap)
  int lcx = 80, lcy = 66;
  uint16_t darkRed = tft.color565(55, 0, 0);
  tft.drawCircle(lcx, lcy, 43, darkRed);
  tft.drawCircle(lcx, lcy, 35, darkRed);
  tft.fillCircle(lcx, lcy, 28, tft.color565(46, 10, 10));
  tft.drawCircle(lcx, lcy, 28, COLOR_ERROR);

  // X mark (3px thick, centered on lcy)
  for (int i = -1; i <= 1; i++) {
    tft.drawLine(lcx - 15 + i, lcy - 15, lcx + 15 + i, lcy + 15, COLOR_ERROR);
    tft.drawLine(lcx + 15 + i, lcy - 15, lcx - 15 + i, lcy + 15, COLOR_ERROR);
  }

  // "Shelly not found" — two lines with extra spacing
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ERROR);
  tft.setTextSize(2);
  tft.drawString("Shelly", lcx, 122);
  tft.drawString("not found", lcx, 143);

  // --- RIGHT ZONE (cx=240) ---
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("Payments paused", 240, 62);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(3);
  tft.drawString(String(secondsLeft) + "s", 240, 92);

  // Progress bar (red)
  tft.fillRect(190, 114, 100, 3, 0x1082);
  int fillW = map(secondsLeft, 0, 10, 0, 100);
  if (fillW > 0) tft.fillRect(190, 114, fillW, 3, COLOR_ERROR);

  // "Press to exit" hint
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(0x3186);
  tft.setTextSize(1);
  tft.drawString("Press to exit", 240, 128);
}

//
// ERROR
//

void displayError(TFT_eSPI &tft, String ip, int secondsLeft) {
  tft.fillScreen(COLOR_BG);

  // Vertical divider
  tft.drawFastVLine(160, 20, 130, 0x4208);

  // --- LEFT ZONE — same style as displayShellyOffline ---
  int lcx = 80, lcy = 66;
  uint16_t darkRed = tft.color565(55, 0, 0);
  tft.drawCircle(lcx, lcy, 43, darkRed);
  tft.drawCircle(lcx, lcy, 35, darkRed);
  tft.fillCircle(lcx, lcy, 28, tft.color565(46, 10, 10));
  tft.drawCircle(lcx, lcy, 28, COLOR_ERROR);

  // X mark (3px thick, centered on lcy)
  for (int i = -1; i <= 1; i++) {
    tft.drawLine(lcx - 15 + i, lcy - 15, lcx + 15 + i, lcy + 15, COLOR_ERROR);
    tft.drawLine(lcx + 15 + i, lcy - 15, lcx - 15 + i, lcy + 15, COLOR_ERROR);
  }

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ERROR);
  tft.setTextSize(2);
  tft.drawString("Server", lcx, 119);
  tft.drawString("error", lcx, 140);

  // --- RIGHT ZONE ---
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("Connection failed", 240, 62);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(3);
  tft.drawString(String(secondsLeft) + "s", 240, 92);

  // Progress bar (red)
  tft.fillRect(190, 114, 100, 3, 0x1082);
  int fillW = map(secondsLeft, 0, 10, 0, 100);
  if (fillW > 0) tft.fillRect(190, 114, fillW, 3, COLOR_ERROR);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(0x3186);
  tft.setTextSize(1);
  tft.drawString("Press to exit", 240, 128);
}

//
// IF WIFI FAILED
//

void displayWiFiFailed(TFT_eSPI &tft, String ssid, int secondsLeft) {
  tft.fillScreen(COLOR_BG);

  // Vertical divider
  tft.drawFastVLine(160, 20, 130, 0x4208);

  // --- LEFT ZONE ---
  // Block: circle_top=34, bar_bottom=72, texts at 84/102/124, bar=133 → center=85
  // 4 WiFi signal bars (gray, bottom=72, left edge x=61, 4×6px + 3×5px = 39px wide)
  int barH[] = {8, 14, 20, 26};
  for (int b = 0; b < 4; b++) {
    tft.fillRoundRect(61 + b * 11, 72 - barH[b], 6, barH[b], 2, COLOR_GRAY);
  }

  // Red X circle (smaller: r=7) shifted right, X centered (±3px)
  tft.fillCircle(110, 41, 7, tft.color565(60, 0, 0));
  tft.drawCircle(110, 41, 7, COLOR_ERROR);
  for (int i = -1; i <= 1; i++) {
    tft.drawLine(107 + i, 38, 113 + i, 44, COLOR_ERROR);
    tft.drawLine(113 + i, 38, 107 + i, 44, COLOR_ERROR);
  }

  // Text lines — 18px then 22px spacing
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(1);
  tft.drawString("Could not connect to", 80, 84);

  String displaySsid = ssid.length() > 22 ? ssid.substring(0, 22) : ssid;
  tft.setTextColor(COLOR_ACCENT);
  tft.drawString(displaySsid, 80, 102);

  tft.setTextColor(COLOR_GRAY);
  tft.drawString("Retry in " + String(secondsLeft) + "s", 80, 124);

  // Progress bar (red, 120px centered at x=80)
  tft.fillRect(20, 133, 120, 3, 0x1082);
  int fillW = map(secondsLeft, 0, 10, 0, 120);
  if (fillW > 0) tft.fillRect(20, 133, fillW, 3, COLOR_ERROR);

  // --- RIGHT ZONE --- V icons at same position as displaySettings, text to their left
  uint16_t colCyan = tft.color565(0, 229, 255);
  int rx = 294;  // same as settings screen

  // BTN2 (top physical button) — ✓ at y≈31, same coords as settings
  for (int i = 0; i <= 1; i++) {
    tft.drawLine(rx - 5, 31 + i, rx - 1, 36 + i, colCyan);
    tft.drawLine(rx - 1, 36 + i, rx + 5, 25 + i, colCyan);
  }
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  tft.drawString("Open setup page", rx - 12, 31);

  // BTN1 (bottom physical button) — ✓ at y≈143, same zone as settings triangles
  for (int i = 0; i <= 1; i++) {
    tft.drawLine(rx - 5, 139 + i, rx - 1, 144 + i, colCyan);
    tft.drawLine(rx - 1, 144 + i, rx + 5, 132 + i, colCyan);
  }
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("Retry connection", rx - 12, 141);
}

//
// INFO
//

void displayInfo(TFT_eSPI &tft, PlugNSatConfig &config, int payments) {
  tft.fillScreen(COLOR_BG);
  tft.setTextSize(1);

  uint16_t colOrange = tft.color565(247, 147, 26);
  uint16_t colCyan   = tft.color565(0, 229, 255);

  // Header: "Plug" orange | "N" yellow | "sat" cyan — size 2 = 12px/char
  tft.setTextDatum(TL_DATUM); tft.setTextSize(2);
  tft.setTextColor(colOrange); tft.drawString("Plug", 10, 6);
  tft.setTextColor(colOrange); tft.drawString("N",    58, 6); // 10 + 4*12
  tft.setTextColor(colCyan);   tft.drawString("sat",  70, 6); // 58 + 1*12

  // Version top-right in gray
  tft.setTextDatum(TR_DATUM); tft.setTextSize(1);
  tft.setTextColor(COLOR_GRAY);
  tft.drawString("v0.1.0", SCREEN_W - 8, 12);

  // Separator "─── Network ──────────────────"  (y=38, more room below header)
  tft.drawFastHLine(8, 34, SCREEN_W - 16, colOrange);
  tft.fillRect(15, 29, 56, 10, COLOR_BG);
  tft.setTextDatum(TL_DATUM); tft.setTextColor(colOrange); tft.setTextSize(1);
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
  tft.drawFastHLine(8, y + 10, SCREEN_W - 16, colOrange);
  tft.fillRect(15, y + 5, 56, 10, COLOR_BG);
  tft.setTextDatum(TL_DATUM); tft.setTextColor(colOrange); tft.setTextSize(1);
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
  tft.setTextColor(colCyan); tft.setTextSize(1);
  tft.drawString("Any button to exit", SCREEN_W / 2, SCREEN_H - 4);
}

//
// SETTINGS MENU
//

void displaySettings(TFT_eSPI &tft, int selectedIndex, bool pinActive) {
  tft.fillScreen(COLOR_BG);

  uint16_t colOrange = tft.color565(247, 147, 26);
  uint16_t colCyan   = tft.color565(0, 229, 255);

  // Padlock: 10w x 11h (4px shackle + 7px body)
  auto drawLock = [&](int lx, int ly, bool locked, uint16_t col) {
    tft.fillRect(lx, ly + 4, 10, 7, col);
    tft.fillRect(lx + 2, ly, 6, 2, col);
    tft.fillRect(lx + 6, ly, 2, 5, col);
    if (locked) tft.fillRect(lx + 2, ly, 2, 5, col);
  };

  const char* options[] = {"Device Info", "Brightness", "Price", "Duration"};
  // Vertically centered: 130px content in 170px screen → topY=20 → cy=34,68,102,136
  int itemCY[]    = {34, 68, 102, 136};
  const int rectH = 28;
  const int rectX = 8;
  const int rectW = SCREEN_W - 60;   // 260px, right edge at 268
  const int btnCX = rectX + rectW / 2; // 138 = horizontal center of button zone
  const int rx    = 294;              // center of right margin (268..320)

  for (int i = 0; i < 4; i++) {
    int cy = itemCY[i];
    bool selected = (i == selectedIndex);
    uint16_t textColor = selected ? colOrange : COLOR_TEXT;

    if (selected) {
      tft.drawRoundRect(rectX, cy - rectH / 2, rectW, rectH, 6, colOrange);
    }

    tft.setTextSize(2);
    bool hasPadlock = (i == 2 || i == 3);

    if (!hasPadlock) {
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(textColor);
      tft.drawString(options[i], btnCX, cy);
    } else {
      int textW  = strlen(options[i]) * 12;
      int groupW = 10 + 8 + textW;
      int gx     = btnCX - groupW / 2;
      drawLock(gx, cy - 5, pinActive, textColor);
      tft.setTextDatum(TL_DATUM);
      tft.setTextColor(textColor);
      tft.drawString(options[i], gx + 18, cy - 8);
    }
  }

  // Right margin indicators (cyan = "sat" color)
  // ✓ — facing BTN2 (top button), 2px thick
  for (int i = 0; i <= 1; i++) {
    tft.drawLine(rx - 5, 31 + i, rx - 1, 36 + i, colCyan);
    tft.drawLine(rx - 1, 36 + i, rx + 5, 25 + i, colCyan);
  }

  // ▲▼ — facing BTN1 (bottom button), 6px gap between shapes
  tft.fillTriangle(rx,     134, rx - 5, 140, rx + 5, 140, colCyan);
  tft.fillTriangle(rx,     152, rx - 5, 146, rx + 5, 146, colCyan);
}

//
// PRICE
//

void displayPrice(TFT_eSPI &tft, int priceSats) {
  tft.fillScreen(COLOR_BG);

  uint16_t colCyan = tft.color565(0, 229, 255);
  int cx = SCREEN_W / 2;
  int rx = 294;

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(4);
  tft.drawString(String(priceSats), cx, SCREEN_H / 2 - 20);
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(3);
  tft.drawString("sats", cx, SCREEN_H / 2 + 24);

  // "+" — BTN2 (top), same position as settings checkmark
  for (int t = 0; t < 2; t++) {
    tft.drawFastHLine(rx - 5, 30 + t, 11, colCyan);
    tft.drawFastVLine(rx + t, 25, 12, colCyan);
  }

  // "−" — BTN1 (bottom), same y-zone as settings triangles
  for (int t = 0; t < 2; t++) {
    tft.drawFastHLine(rx - 5, 142 + t, 11, colCyan);
  }
}

//
// DURATION
//

void displayDuration(TFT_eSPI &tft, int durationSeconds) {
  tft.fillScreen(COLOR_BG);

  uint16_t colCyan = tft.color565(0, 229, 255);
  int cx = SCREEN_W / 2;
  int rx = 294;

  // Centered duration value
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(4);
  tft.drawString(String(durationSeconds) + "s", cx, SCREEN_H / 2 - 20);

  // Conversion below
  tft.setTextColor(COLOR_GRAY);
  tft.setTextSize(2);
  if (durationSeconds >= 3600) {
    int h = durationSeconds / 3600;
    int m = (durationSeconds % 3600) / 60;
    String conv = String(h) + "h";
    if (m > 0) conv += " " + String(m) + "min";
    tft.drawString(conv, cx, SCREEN_H / 2 + 22);
  } else if (durationSeconds >= 60) {
    int m = durationSeconds / 60;
    tft.drawString(String(m) + " min", cx, SCREEN_H / 2 + 22);
  }

  // "+" — BTN2 (top), same position as settings checkmark
  for (int t = 0; t < 2; t++) {
    tft.drawFastHLine(rx - 5, 30 + t, 11, colCyan);
    tft.drawFastVLine(rx + t, 25, 12, colCyan);
  }

  // "−" — BTN1 (bottom), same y-zone as settings triangles
  for (int t = 0; t < 2; t++) {
    tft.drawFastHLine(rx - 5, 142 + t, 11, colCyan);
  }
}

//
// BRIGHTNESS
//

void displayBrightness(TFT_eSPI &tft, int brightness, String qrData) {
  tft.fillScreen(COLOR_BG);

  String qr      = (qrData.length() > 0) ? qrData : "PLUGNSAT";
  int side       = qrSide(qr, SCREEN_H - 2);
  int qrX        = (SCREEN_W - side) / 2;
  int qrY        = (SCREEN_H - side) / 2;
  QRResult qrDim = renderQR(tft, qr, qrX, qrY, SCREEN_H - 2);

  // Center brightness bar between QR right edge and screen right edge
  int cx = (qrX + qrDim.qrPixW + SCREEN_W) / 2;

  uint16_t colCyan = tft.color565(0, 229, 255);
  int rx = cx;  // icons centered above/below bar

  // "+" — BTN2 (top), slightly higher for breathing room
  for (int t = 0; t < 2; t++) {
    tft.drawFastHLine(rx - 5, 22 + t, 11, colCyan);
    tft.drawFastVLine(rx + t, 17, 12, colCyan);
  }

  int barH = 80;
  int barW = 14;
  int barX = cx - barW / 2;
  int barY = 44;  // centered between "+" (ends y=28) and "−" (starts y=153)

  tft.fillRect(barX, barY, barW, barH, 0x1082);
  tft.drawRect(barX - 1, barY - 1, barW + 2, barH + 2, COLOR_GRAY);

  int fillH = map(brightness, 0, 255, 0, barH);
  if (fillH > 0) {
    tft.fillRect(barX, barY + barH - fillH, barW, fillH, COLOR_ACCENT);
  }

  // "%" between bar and "−", no overlap (bar ends y=124, % y=129–137, "−" y=145)
  int pct = map(brightness, 0, 255, 0, 100);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(1);
  tft.drawString(String(pct) + "%", cx, barY + barH + 4);

  // "−" — BTN1 (bottom), slightly lower for breathing room
  for (int t = 0; t < 2; t++) {
    tft.drawFastHLine(rx - 5, 153 + t, 11, colCyan);
  }
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
  int totalW = 4 * boxW + 3 * boxGap;          // 206
  int startX = (SCREEN_W - totalW) / 2;         // 57
  int boxY   = (SCREEN_H - boxH) / 2;           // vertically centered = 59

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
    tft.drawString("Wrong PIN", SCREEN_W / 2, boxY + boxH + 12);
  }

  // Right margin indicators — same positions as settings screen
  uint16_t colCyan = tft.color565(0, 229, 255);
  int rx = 294;

  for (int i = 0; i <= 1; i++) {
    tft.drawLine(rx - 5, 31 + i, rx - 1, 36 + i, colCyan);
    tft.drawLine(rx - 1, 36 + i, rx + 5, 25 + i, colCyan);
  }

  tft.fillTriangle(rx,     134, rx - 5, 140, rx + 5, 140, colCyan);
  tft.fillTriangle(rx,     152, rx - 5, 146, rx + 5, 146, colCyan);
}

//
// AP MODE
//

void displayAPMode(TFT_eSPI &tft, String apIp, bool hasWifi) {
  tft.fillScreen(COLOR_BG);

  uint16_t colGold = tft.color565(0, 229, 255);
  int cx   = SCREEN_W / 2;
  int boxX = 20;
  int boxW = SCREEN_W - 40;  // 280
  int boxH = 46;
  int boxR = 8;

  // 3 boxes + 2 gaps fit in 160px, 5px top/bottom margin, footer in last 10px
  // layout: 5 + 46 + 6 + 46 + 6 + 46 + 5 = 160, footer y=160..170
  int boxY[3] = { 5, 57, 109 };

  const char* labels[] = {
    "1 - Connect to Wifi:",
    "2 - Password:",
    "3 - Open in browser:"
  };
  String values[] = {
    "PlugNSat-Setup",
    "plugnsat21",
    "http://" + apIp
  };

  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);

  for (int i = 0; i < 3; i++) {
    tft.drawRoundRect(boxX, boxY[i], boxW, boxH, boxR, colGold);
    tft.setTextColor(COLOR_TEXT);
    tft.drawString(labels[i], cx, boxY[i] + boxH / 2 - 10);
    tft.setTextColor(colGold);
    tft.drawString(values[i], cx, boxY[i] + boxH / 2 + 10);
  }

  if (hasWifi) {
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(colGold);
    tft.setTextSize(1);
    tft.drawString("Any button to reconnect", cx, SCREEN_H - 3);
  }
}

#endif
