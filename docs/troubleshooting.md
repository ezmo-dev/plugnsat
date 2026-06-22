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