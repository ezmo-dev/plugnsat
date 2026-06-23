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

### The Shelly didn't turn on after payment

**Symptoms:** The "PAID" screen appeared but the Shelly did not activate, or the "Shelly not found" screen appeared.

**Things to check:**

1. **Is the Shelly plugged in?** Check that it has power (LED indicator on the Shelly).
2. **Is the Shelly on the same WiFi?** Both devices must be on the same local network.
3. **Is the hostname correct?** Go to Device Info (BTN1 from QR) and check the "Shelly" field. If it shows an IP address, the Shelly may have changed IP via DHCP. Use the mDNS hostname instead (`.local`).
4. **Can the Shelly be reached?** Open the Shelly hostname or IP in a browser on the same network. You should see the Shelly web interface.

> **Note:** Even if the Shelly is offline, the customer's payment is still processed. The Lightning invoice was settled. You may need to activate the Shelly manually (via the Shelly app or by pressing the button on the Shelly itself).

### "Error" screen keeps appearing

This means the PlugNSat cannot reach the BTCPay Server (or Blink). The device retries automatically every 10 seconds.

**Things to check:**

1. **Is the BTCPay Server online?** Open the server URL in your browser.
2. **Is the API key valid?** API keys can be revoked. Generate a new one if needed.
3. **Did the server URL change?** If you migrated your BTCPay instance, update the URL in the web portal.
4. **Is the Lightning node working?** Log into BTCPay > Settings > Lightning and check the status.

If the device encounters 10 consecutive errors, it restarts itself automatically to recover from potential memory or connection issues.

---

## Shelly issues

### "Scan network" doesn't find the Shelly

**Possible causes:**

1. **Shelly is on a different network.** Both devices must be on the same WiFi network and subnet.
2. **mDNS is blocked.** Some routers block multicast DNS traffic. Try entering the Shelly's IP address manually instead.
3. **Shelly is not powered on.** Make sure the Shelly has power (check its LED).

**Manual alternative:** Open the Shelly app, find your device, and note the hostname or IP address. Enter it manually in the web portal's Shelly hostname field.

### The Shelly turns on but the connected device doesn't

1. **Check the device plugged into the Shelly.** Is it actually turned on (power switch in the "on" position)? Some devices have their own on/off switch.
2. **Check the Shelly's max power rating.** The Shelly Plug S Gen3 supports up to 2500W. If your device draws more, the Shelly will not switch.
3. **Check the wall outlet.** Try plugging the device directly into the wall to confirm it works.

---

## Web portal issues

### Can't access the web portal at 192.168.4.1

This address only works in **AP Setup mode** (when the device is broadcasting its own WiFi network). If the device is connected to your regular WiFi:

1. Find the device's IP address on the Device Info screen (BTN1 from QR)
2. Open `http://<device-ip>` in your browser on the same network

### Can't access the web portal at the device's IP

1. **Are you on the same WiFi?** The portal is only accessible on the local network.
2. **Try refreshing.** The first connection can take a second while the ESP32 serves the page.
3. **Try a different browser.** Some browsers aggressively redirect HTTP to HTTPS. The web portal uses HTTP (no SSL). Try typing the URL manually including `http://`.

### Settings don't save

If you click "Save and restart" but the settings revert to old values:

1. Make sure you filled in all required fields (WiFi SSID, BTCPay URL, API Key, Store ID)
2. Try again from AP mode (long press BTN1) for a fresh connection

---

## AP mode issues

### I'm stuck in AP mode

If the device keeps entering AP mode on every boot, it means no WiFi credentials are saved. Enter the web portal (`http://192.168.4.1`), fill in your WiFi credentials, and click Save and restart.

### I can't find "PlugNSat-Setup" in my WiFi list

1. Wait 10 seconds after the AP mode screen appears. The WiFi broadcast can take a moment.
2. On your phone, pull down to refresh the WiFi list.
3. Move closer to the PlugNSat (within 2 meters).
4. If still not visible, try power-cycling the PlugNSat (unplug and replug USB-C).

---

## BTCPay Server specific issues

### "Invoice error 401" or "Invoice error 403"

This means the API key is invalid or doesn't have the right permissions.

1. Go to BTCPay Server > Account > Manage Account > API Keys
2. Check that your key has: `cancreateinvoice`, `canviewinvoices`, `canuselightningnode`
3. If unsure, revoke the old key and create a new one

### "No LNURL found"

This means the BTCPay Server returned the invoice but the LNURL payment method is missing.

1. Make sure your store has Lightning enabled (Settings > Lightning)
2. Make sure LNURL is enabled in your store's Lightning settings
3. Check that the payment method ID is `BTC-LNURL` (recent BTCPay versions changed the naming from `BTC-LightningNetwork`)

---

## Factory reset

To reset the PlugNSat to its default state:

1. Long press BTN1 (3 seconds) to enter AP Setup mode
2. Connect to `PlugNSat-Setup` WiFi
3. Open `http://192.168.4.1`
4. Clear all fields and save, or simply re-enter new credentials

> There is no "erase all" button yet. All settings are stored in the ESP32's non-volatile memory. Reflashing the firmware also clears all settings.

---

## Getting help

If none of the above solves your problem:

1. Open an issue on [GitHub](https://github.com/ezmo-dev/plugnsat/issues) with:
   - Your firmware version (shown on the splash screen)
   - A description of the issue
   - Photos of the screen if relevant
2. Check the [PlugNSat website](https://plugnsat.com) for updates
3. Reach out on Twitter/Nostr to [@ezmo-dev](https://twitter.com/ezmo_dev)
