# Troubleshooting

Common issues and how to fix them. If your problem is not listed here, open an issue on [GitHub](https://github.com/ezmo-dev/plugnsat/issues) or reach out on social media.

---

## WiFi issues

### The PlugNSat won't connect to WiFi

**Symptoms:** the "Connecting to WiFi..." screen stays for a while, then a "Could not connect" screen appears with a 10-second countdown before auto-retry.

**Things to check:**

1. **Is the SSID correct?** WiFi names are case-sensitive. "MyWiFi" and "mywifi" are different networks.
2. **Is the password correct?** Re-enter it in the web portal (AP Setup mode: long press BTN1 for 3 seconds, connect to `PlugNSat-Setup`, open `http://192.168.4.1`).
3. **Is it a 2.4 GHz network?** The ESP32-S3 does not support 5 GHz. If your router broadcasts both bands under the same name, try separating them in your router settings.
4. **Is the WiFi too far?** Move the PlugNSat closer to the router. Check the RSSI value on the Device Info screen (Settings menu, then Device Info). Anything weaker than -75 dBm may cause issues.
5. **Is the router blocking new devices?** Some routers have MAC filtering or a device limit. Check your router admin panel.

**Quick fix:** Long press BTN1 (3 seconds) to enter AP Setup mode, then re-enter your WiFi credentials.

> When the connection attempt fails, you have two options on the "Could not connect" screen: BTN1 enters AP Setup mode, BTN2 retries immediately. If you do nothing, the device retries automatically after the countdown.

### WiFi disconnects randomly

The PlugNSat checks the connection periodically and reconnects automatically when WiFi drops. If it happens frequently:

- Check your router's stability (are other devices also dropping?)
- Move the PlugNSat closer to the router
- Avoid placing the device behind metal objects or inside thick enclosures that block the WiFi signal

---

## QR code issues

### The QR code won't scan

**Most common causes:**

1. **Screen brightness not optimal.** Adjust brightness from the device (Settings menu, then Brightness, BTN2 to increase, BTN1 to decrease). Because this is an LED-backlit screen, maximum brightness is not always best. A setting around 21% usually gives the cleanest reading, with less glare and better contrast for wallet cameras.
2. **Screen reflections.** The LCD has a glossy surface. In bright environments (sunlight, strong overhead lights), angle the device or move it to reduce glare.
3. **Camera too close.** Hold the phone 15 to 25 cm away from the screen.
4. **Wallet doesn't support the invoice format.** With a BTCPay backend the QR uses LNURL, which most modern wallets support (Phoenix, Wallet of Satoshi, Blink, Cash App, Muun, Blue Wallet, Zeus). With a Blink backend the QR is a BOLT11 invoice, which is larger and denser. Try a different wallet if scanning fails.

### The QR code refreshes before the customer can scan

The QR auto-refreshes after 4 minutes 45 seconds (before the 5-minute invoice expiry). This is normal: it ensures the QR always shows a valid, payable invoice. If a customer sees the QR refresh while scanning, they can simply scan again. The new QR is immediately valid.

---

## Payment issues

### Payment was sent but the screen didn't change

**Possible causes:**

1. **Polling delay.** The PlugNSat checks for payment every 5 seconds. Wait a few seconds after the wallet confirms payment.
2. **Backend is slow.** Lightning invoice settlement can take a moment depending on channel liquidity and node performance. The device treats both "Settled" and "Processing" as paid, so a slow final settlement still triggers activation.
3. **WiFi issue.** If WiFi dropped between the payment and the poll, the PlugNSat may miss that poll. The device reconnects and picks up the status on the next poll.
