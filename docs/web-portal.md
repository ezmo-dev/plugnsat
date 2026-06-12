# Web Portal Reference

The web portal is the configuration interface served by the PlugNSat itself. It runs directly on the ESP32 and is accessible from any browser on the same network.

## How to access the web portal

There are two ways to reach the web portal depending on the device's current state:

**During first setup (AP mode):**

1. Connect your phone to WiFi: `PlugNSat-Setup` (password: `plugnsat21`)
2. Open `http://192.168.4.1` in your browser

**After setup (normal mode):**

1. Connect to the same WiFi network as the PlugNSat
2. Find the device's IP address (shown on the Device Info screen: BTN1 from QR > Device Info)
3. Open `http://<device-ip>` in your browser, or use `http://plugnsat.local` (mDNS)

> The web portal works on any modern browser: Safari, Chrome, Firefox, on both mobile and desktop. No app needed.

> In AP mode the portal is never password-protected, so you can never lock yourself out during setup. A Web Access Password (see Step 5) only applies in normal mode on your WiFi.

---