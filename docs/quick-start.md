# Quick Start Guide

Get your PlugNSat up and running in under 10 minutes. This guide covers everything from opening the box to receiving your first Lightning payment.

> **Pre-assembled unit?** Skip straight to [Step 2: Set up the Shelly Plug](#step-2--set-up-the-shelly-plug). The firmware is already flashed.

## What's in the box

![Box contents](images/box-contents.jpg)
> *Image coming soon*

- 1x PlugNSat device USB-C (LilyGO T-Display S3, pre-flashed)
- 1x 3D-printed enclosure (desk stand, magnetic mount, or wall mount)
- 1x Shelly Plug S Gen3 (CE-certified smart plug)
- 1x USB-C cable (1.5m, braided)
- 1x Quick-start card with QR code to this documentation

## Before you start

You will need:

- A WiFi network (2.4 GHz, the Shelly does not support 5 GHz)
- A smartphone to configure the Shelly and the PlugNSat
- A Lightning backend: **Blink wallet** (easy, no server needed) or **BTCPay Server** (self-hosted, advanced)
- A USB-C power source (phone charger, power bank, laptop USB port, anything)
- Something to power with the Shelly: a lamp, a coffee machine, a phone charger, anything with a plug

## Step 1 — Choose your Lightning backend

PlugNSat needs a Lightning backend to create invoices and detect payments. You have two options:

| | Blink | BTCPay Server |
|---|---|---|
| Difficulty | Easy (5 min) | Advanced (1+ hours if starting from scratch) |
| Cost | Free | Free (self-hosted) or ~5 EUR/month (hosted) |
| Requires | A Blink account | A BTCPay Server instance with Lightning |
| Custody | Blink holds your sats | You hold your keys |
| Best for | Getting started fast, demos, events | Permanent setups, sovereignty, businesses |

**Pick one and follow the matching section below.** You can always switch later from the web portal.

---

### Option A: Set up with Blink (recommended for beginners)

1. Download the **Blink wallet** ([iOS](https://apps.apple.com/app/blink-bitcoin-beach-wallet/id1531383905) / [Android](https://play.google.com/store/apps/details?id=com.galoyapp))
2. Create an account and verify your phone number
3. Go to **Settings > API Keys** in the Blink app or on [dashboard.blink.sv](https://dashboard.blink.sv)
4. Create a new API key and copy it

![Blink API key Dashboard](images/blink-api-key-dashboard.png)
![Blink API key Creation](images/blink-api-key-create.png)

5. Save your API key in a secure location (password vault) and keep it handy for step 4
6. Keep your wallet ID in a safe place (such as a password manager) and have it handy for Step 4 as well

![Blink Wallet ID](images/blink-wallet-id.png)

---

### Option B: Set up with BTCPay Server

If you already have a BTCPay Server instance running with Lightning, you need three things:

1. **Server URL** (e.g. `https://btcpay.yourdomain.com`)
2. **Store ID**
3. **API Key** with the right permissions

#### Find your Store ID

Your Store ID is the long string visible in the URL when you open your store in BTCPay Server. You can also find it in **Settings > Store Settings**.

![BTCPay Store ID](images/btcpay-store-id.png)

#### Create an API Key

1. In BTCPay Server, go to **Account > Manage Account > API Keys**
2. Click **Generate Key**
3. Check exactly these permissions:
   - `cancreateinvoice` (Create invoices)
   - `canviewinvoices` (View invoices)
   - `canuselightningnode` (Use the Lightning node)
4. Click **Generate**
5. **Copy the key immediately.** It will not be shown again.

![BTCPay API Key](images/btcpay-api-key.png)
![BTCPay API permissions](images/btcpay-api-permissions.png)

> **Don't have a BTCPay Server yet?** You can set one up for free on your own server by following the [official deployment guide](https://docs.btcpayserver.org/Deployment/), or use a hosted solution like [LunaNode](https://launchbtcpay.lunanode.com/) or [Voltage](https://voltage.cloud/). You will also need a Lightning node connected (Phoenixd, LND, or CLN).

#### Make sure Lightning is working

In your BTCPay Server store, go to **Lightning > Settings** and make sure a Lightning node is connected and operational. You should see a green status indicator.

While you are here, go to **Lightning > Settings** and make sure LNURL is enabled. PlugNSat uses LNURL to generate compact QR codes that are easier to scan on the small screen.

![BTCPay Lightning setup](images/btcpay-lightning-setup.png)

Keep the Server URL, Store ID, and API Key ready for Step 4.

---

## Step 2 — Set up the Shelly Plug
 
The Shelly Plug S Gen3 is the smart plug that PlugNSat controls. It handles the 220V switching safely, is CE-certified, and requires no modification.
 
> **Important:** The Shelly must be on the same Wi-Fi network as your PlugNSat. If you have a dual-band router, make sure both devices are on the 2.4 GHz band.
 
**Pairing the Shelly with the app**
 
1. Download the **Shelly app** ([iOS](https://apps.apple.com/app/shelly-cloud/id1147164547) / [Android](https://play.google.com/store/apps/details?id=cloud.shelly.smartcontrol))
2. Plug the Shelly into a wall outlet. It should flash blue.
3. Make sure your phone is connected to Wi-Fi and that Bluetooth is enabled.
4. Open the app and tap the blue "+" button at the bottom right. Select "Add via Bluetooth". Your Shelly Plug should be detected automatically.

<p align="center">
  <img src="images/shelly-app-add-device-1.png" alt="Shelly app home screen with the blue + button at the bottom right" width="280" style="vertical-align: middle">
  <span style="vertical-align: middle">&nbsp;&nbsp;→&nbsp;&nbsp;</span>
  <img src="images/shelly-app-add-device-2.png" alt="Add via Bluetooth option selected in the Shelly app" width="280" style="vertical-align: middle">
  <span style="vertical-align: middle">&nbsp;&nbsp;→&nbsp;&nbsp;</span>
  <img src="images/shelly-app-add-device-3.png" alt="Shelly Plug detected automatically via Bluetooth" width="280" style="vertical-align: middle">
</p>

5. Confirm your Wi-Fi network is correct, then tap "Add device" and wait for the setup to finish.

<p align="center">
  <img src="images/shelly-app-add-device-4.png" alt="Wi-Fi network confirmation screen" width="280" style="vertical-align: middle">
  <span style="vertical-align: middle">&nbsp;&nbsp;→&nbsp;&nbsp;</span>
  <img src="images/shelly-app-add-device-5.png" alt="Add device button to start the installation" width="280" style="vertical-align: middle">
  <span style="vertical-align: middle">&nbsp;&nbsp;→&nbsp;&nbsp;</span>
  <img src="images/shelly-app-add-device-6.png" alt="Installation in progress screen" width="280" style="vertical-align: middle">
</p>

6. Give your Shelly Plug a name so you can identify it if you have several.

<p align="center">
  <img src="images/shelly-app-add-device-7.png" alt="Device naming screen in the Shelly app" width="280" style="vertical-align: middle">
  <span style="vertical-align: middle">&nbsp;&nbsp;→&nbsp;&nbsp;</span>
  <img src="images/shelly-app-add-device-8.png" alt="Shelly Plug successfully added and visible in the device list" width="280" style="vertical-align: middle">
</p>

**Configuring the initial state**
 
7. Go to the "All Devices" tab and tap your Shelly Plug. Scroll down to "Settings".

<img src="images/shelly-app-initial-state-1.png" alt="All Devices tab showing the Shelly Plug" width="280">

8. Under "Input/Output settings", select: "Configure Shelly device to Turn OFF when it has power." Tap "Save".

<img src="images/shelly-app-initial-state-2.png" alt="Input/Output settings with the Turn OFF option selected" width="280">

9. Plug your target device (lamp, coffee machine, charger, etc.) into the Shelly. It should stay off.

---

## Step 3 — Power on the PlugNSat

1. Connect the PlugNSat to power using the USB-C cable
2. The splash screen appears for a few seconds

![Splash screen](images/plugnsat-screen-splash.png)

3. On first boot (or after a factory reset), the device enters **AP Setup Mode** and shows three boxes on screen:

![AP mode screen](images/plugnsat-screen-ap-mode.png)

| Step | Value |
|------|-------|
| WiFi name | `PlugNSat-Setup` |
| Password | `plugnsat21` |
| URL | `http://192.168.4.1` |

---

## Step 4 — Configure the PlugNSat

1. On your phone or computer, open WiFi settings and connect to `PlugNSat-Setup` (password: `plugnsat21`)
2. On the same device, open your browser and go to `http://192.168.4.1`
3. The web portal opens with 5 sections to complete:
   - WiFi
   - Lightning Backend
   - Shelly Plug
   - Device Settings
   - Security

### Step 1 of 5: WiFi

Enter the SSID and password of the WiFi network where the Shelly is connected.

<img src="images/webportal-step1-wifi.png" width="280">

### Step 2 of 5: Lightning Backend

Select your backend (BTCPay Server or Blink), then enter the credentials from Step 1 of this guide.

**BTCPay Server:**
- **Server URL** (no trailing slash, e.g. `https://btcpay.yourdomain.com`)
- **API Key** (the one you generated)
- **Store ID** (the long string from your store settings)

**Blink:**
- **API Key** (starts with `blink_`, generated at dashboard.blink.sv)
- **BTC Wallet ID** (find it on dashboard.blink.sv)

<p>
  <img src="images/webportal-step2-btcpay.png" width="280">
  <img src="images/webportal-step2-blink.png" width="280">
</p>

### Step 3 of 5: Shelly Plug

Click **Scan network** to automatically discover your Shelly on the local network. If it finds one, select it from the dropdown. Otherwise, type the Shelly hostname or IP address manually.

<img src="images/webportal-step3-shelly.png" width="280">

You can click **Test connection** to verify the PlugNSat can reach the Shelly.

### Step 4 of 5: Device Settings

- **Name**: a display name for your PlugNSat (up to 18 characters if shown on screen, up to 40 otherwise)
- **Show name on QR screen**: toggle to display the name next to the QR code
- **Price**: amount in satoshis the customer pays per activation
- **Duration**: how many seconds the Shelly stays on after a payment
- **Show price on QR screen**: toggle to display the price next to the QR code

You can also click **Simulate payment (free)** to test that the Shelly activates correctly.

<img src="images/webportal-step4-settings.png" width="280">

### Step 5 of 5: Security

Both fields are optional. Leave them empty to skip.

- **Device PIN** (4 digits): prevents anyone near the device from changing the price and duration using the hardware buttons
- **Web Access Password**: protects this configuration page with a login prompt (username: `admin`). Always disabled in setup mode so you can never get locked out.

<img src="images/webportal-step5-security.png" width="280">

4. Click **Save and restart**
5. The PlugNSat reboots, connects to your WiFi, and the QR code appears on screen

<p>
  <img src="images/screen-qr-full.jpg" width="280">
  <img src="images/screen-qr-minimal.jpg" width="280">
</p>

Left: name and price displayed. Right: QR code only (both toggles off).

**You're live!** Customers can now scan and pay.