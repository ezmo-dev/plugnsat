# PlugNSat Firmware v0.1.0

An open-source Lightning Smart Plug Controller. Turn any device into a Bitcoin-powered machine!

PlugNSat is an ESP32-based controller that displays a Lightning invoice QR code on screen.
When a customer pays, it triggers a CE-certified Shelly smart plug to power on any connected device (5V, 12V, 220V...) for a configurable duration and price.

Connect to WiFi, set your price in sats, and go!
No modification to the target device needed.

Built for Bitcoin hubs, stores, coworking spaces, companies, meetups, events, and more.

## How it works
```
[QR on screen] --> [Customer scans] --> [Pays Lightning] --> [Shelly ON] --> [New QR]
     ^                                                                          |
     |__________________________________________________________________________|
                              (automatic loop)
```

The QR code is always displayed. No button press needed to start a payment.
QR auto-refreshes every 4m45s before the 5-minute invoice expiry.

## Hardware

| Part | Price | Link |
|------|-------|------|
| LilyGO T-Display S3 | ~20 EUR | [amazon.fr](https://www.amazon.fr/dp/B0BX8Q2MJP) |
| Shelly Plug S Gen3 | ~26 EUR | [amazon.fr](https://www.amazon.fr/dp/B0DJFQXTY2) |

## Arduino IDE Setup

1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Add ESP32 boards: File > Preferences > Board Manager URLs:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Install "esp32" in Board Manager
4. Board: ESP32S3 Dev Module
5. USB CDC On Boot: Enabled
6. Flash Size: 16MB
7. Partition: Huge APP (3MB No OTA)
8. PSRAM: OPI PSRAM

### Libraries (Sketch > Include Library > Manage Libraries)

- **TFT_eSPI** by Bodmer (+ configure User_Setup_Select.h for T-Display S3)
  - Line 27: Comment `#include <User_Setup.h>` (add `//` in front)
  - Line 133: Uncomment `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>` (remove `//`)
- **ArduinoJson** by Benoit Blanchon
- **QRCode** by Richard Moore

## First Boot

1. Flash firmware via USB-C
2. Device shows "PlugNSat Setup" screen
3. Connect phone to WiFi: `PlugNSat-Setup` (password: `plugnsat21`)
4. Open `http://192.168.4.1` in browser
5. Enter WiFi, BTCPay Server, and Shelly configuration (hostname or IP)
6. Save > device restarts > connects to WiFi > QR appears

## Buttons

| Button | Short press | Long press (3s) |
|--------|-------------|-----------------|
| BTN1 (left) | Show info screen | Enter AP setup mode |
| BTN2 (right) | Force QR refresh | - |

## Files
```
plugnsat.ino   Main sketch, state machine, WiFi, buttons
config.h       Constants, colors, config struct
display.h      All screen rendering (splash, QR, paid, error, info, AP)
btcpay.h       BTCPay Server API (create invoice, check status)
shelly.h       Shelly local HTTP API (switch on/off, status)
webportal.h    Web config page (HTML/CSS/JS served by ESP32)
qrcode.h       QR code generation library header (by Richard Moore)
qrcode.c       QR code generation library source
```

All files go in the same folder. Arduino IDE compiles them together.

## Edge Cases Handled

- WiFi disconnects > auto reconnect, QR regenerated
- BTCPay unreachable > retry 3x, then error screen, auto-retry 10s
- Invoice expires > new QR generated silently
- Shelly offline > payment still accepted, warning shown
- Shelly hostname (mDNS .local) supported to avoid DHCP IP changes
- 10+ consecutive errors > ESP32 restarts itself
- Long press BTN1 from any screen > AP setup mode

## License

MIT © 2026 - [ezmo-dev](https://github.com/ezmo-dev) (PlugNSat)