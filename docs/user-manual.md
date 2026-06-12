# User Manual

This document covers everything you need to know about using your PlugNSat day-to-day: what each screen means, how the buttons work, and how to adjust settings directly from the device.

## Overview

PlugNSat operates in a continuous loop:

1. The QR code is displayed permanently on screen
2. A customer scans and pays with any Lightning wallet
3. The screen shows "PAID" and the Shelly plug turns on
4. After the configured duration, the Shelly turns off
5. A new QR code appears automatically

No interaction from the operator is needed. The device runs autonomously.

---

## Buttons

The PlugNSat has two physical buttons on the top edge of the device.

| Button | Position | GPIO |
|--------|----------|------|
| BTN1 | Left (bottom on screen) | GPIO 0 |
| BTN2 | Right (top on screen) | GPIO 14 |

### Button actions

| Context | BTN1 (short press) | BTN1 (long press 3s) | BTN2 (short press) |
|---------|--------------------|-----------------------|--------------------|
| QR screen | Open Settings menu | Enter AP Setup mode | Force QR refresh |
| Settings menu | Move cursor down | Enter AP Setup mode | Select option |
| Brightness | Decrease (-) | Enter AP Setup mode | Increase (+) |
| Price | Decrease (-) | Enter AP Setup mode | Increase (+) |
| Duration | Decrease (-) | Enter AP Setup mode | Increase (+) |
| PIN entry | Change digit (0-9) | Enter AP Setup mode | Confirm digit / Submit |
| Info screen | Return to QR | Enter AP Setup mode | Return to QR |
| AP mode screen | Reconnect to WiFi (if WiFi was configured) | n/a | Reconnect to WiFi |
| WiFi failed screen | Enter AP Setup mode | n/a | Retry connection |
| Error screen | Enter AP Setup mode | Enter AP Setup mode | Retry now (new QR) |
| Shelly offline screen | Return to QR (cancels retry) | Enter AP Setup mode | Return to QR (cancels retry) |

> **Long press BTN1 (3 seconds)** always enters AP Setup mode, from any screen. This is the universal "reconfigure" action. Use it if you need to change WiFi, backend, or Shelly settings.

---

## Screens

### Splash screen

<img src="images/plugnsat-screen-splash.png" width="280">

Displayed for 5 seconds on every boot. Shows the PLUG⚡SAT pixel art logo, the tagline "Lightning Smart Plug", the website URL, and the firmware version.

### Connecting

<p>
    <img src="images/plugnsat-screen-connecting.png" width="280">
    <img src="images/plugnsat-screen-connected.png" width="280">
<p>

Appears while the device is connecting to your WiFi network. Shows the SSID it is trying to reach. If the connection succeeds, you briefly see the device's IP address before the QR screen loads.

If **Auto-update on boot** is enabled in the web portal, two extra screens may appear after the WiFi connection: "Checking updates..." and, if a new version is found, "Updating... vX.X.X" followed by an automatic reboot. See [Firmware updates](#firmware-updates).