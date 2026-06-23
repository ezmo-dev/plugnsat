### v0.1 — Released
- [x] Basic prototype: LNURL QR, BTCPay polling, Shelly activation
- [x] Web portal configuration
- [x] Auto QR refresh before invoice expiry
- [x] Simulate payment button for testing
- [x] mDNS autodiscovery for Shelly devices
- [x] Splash screen with logo and branding
- [x] Color theme and screen brightness adjustments
- [x] Adjust price and duration from device buttons (PIN-protected)
- [x] First 3D print case
- [x] Web portal UI redesign (light theme, step-based layout)
- [x] UI polish: paid screen, error screen, info screen, settings screen
- [x] Better error messages and info indicators on all screens
- [x] Info tooltips on web portal fields
- [x] QR display modes: Show name/price toggles on QR screen (configurable via web portal)
- [x] First test video
- [x] Documentation structure: create outline with all major sections
- [x] README v1 on GitHub
- [x] Full code cleanup before moving forward

### v0.2 — Blink integration, OTA, and enclosures (current)
- [x] Blink wallet API integration
- [x] Backend choice on web portal: BTCPay or Blink
- [x] Web portal adapts fields based on backend choice
- [x] Update info indicators on web portal for both backends
- [x] Fix: infinite paid/activation loop after payment timer expires
- [x] Add descriptive memo to Blink invoices (device name + amount)
- [x] Merge Lightning Backend selector and credentials into a single configuration step
- [x] Add collapsible accordion sections to configuration page with chevron, summary line, and state persistence
- [x] Rewrite all configuration page tooltips for clarity
- [x] OTA Phase 1: partition scheme with two OTA slots, flash and reboot on the other slot, automatic rollback
- [x] OTA Phase 2: local Web OTA, upload a .bin from the portal (protected by portal password)
- [x] OTA Phase 3: firmware signing, RSA key pair, signature verified before any flash
- [x] OTA Phase 4: version check via GitHub Releases API, TLS validation with embedded root CA
- [x] OTA Phase 5: download and flash signed .bin from GitHub, opt-in via portal button
- [x] OTA Phase 6: controlled deployment, release delay and staged rollout, optional auto-update on boot
- [x] Research and select the right LiPo battery model (capacity, connector, dimensions for enclosure)
- [x] "Check for updates" button in web portal
- [x] Auto-update option on boot
- [x] Battery-powered prototype (LiPo integration with T-Display S3)
  - [x] GPIO15 power-enable for battery operation (screen on without USB)
  - [x] VBAT reading on GPIO4 with onboard divider, factory eFuse + 1.017 calibration
  - [x] Battery model flag (isBattery) in config + web portal toggle, persisted
- [x] Power management: sleep mode, low battery indicator on screen
  - [x] Manual Turn OFF in settings menu (deep sleep, wake on BTN_1 or USB)
  - [x] Software low-voltage cutoff at 3100mV (confirmed over consecutive reads)
  - [x] Low battery warning: blinking crossed-out icon, bottom-right of main screen
  - [x] RTC pull-up on wake pin to prevent immediate wake
- [x] Battery level indicator on screen and web portal
  - [x] Device Info: battery icon, voltage (X.XX/4.20V), percentage (LiPo curve)
  - [x] Charging icon when voltage trend is rising
- [x] Battery UI polish: refine design of all battery-related screen elements
- [ ] Enclosure prototypes (USB and battery): desk stand, magnetic mount, wall mount
- [ ] Test prints for each enclosure
- [x] General V0.2 audit: review all new features for correctness and consistency
- [x] Security audit: review all new features (portal auth, XSS, input validation)
- [ ] Finish documentation: User Manual, Web Portal Reference, Troubleshooting (Quick Start done; needed for landing page)

### v0.3 — Landing page and USB beta testing
- [ ] Finalize USB and battery enclosures and publish STL files on GitHub
- [ ] UI improvements on screen: price and device name alongside QR, WiFi signal indicator, battery level
- [ ] Full design coherence review: screens, web portal, enclosures, docs
- [ ] Landing page v1 on plugnsat.com (USB + battery)
- [ ] Product photos for landing page: device, enclosures, screens, setup flow
- [ ] Product video for landing page
- [ ] Quick-start guide (printed in box)
- [ ] Troubleshooting page
- [ ] Beta testing USB and battery models: send units to trusted people
- [ ] Collect feedback on setup, documentation, enclosure, daily use
- [ ] Code cleanup

### v0.4 — Product finalization and packaging
- [ ] Fix issues reported by beta testers
- [ ] Finalize battery enclosures: desk stand, magnetic mount, wall mount
- [ ] Packaging design: box, insert, printed quick-start guide, sticker
- [ ] Update documentation and landing page with beta feedback
- [ ] Product photos and video for USB and battery models

### v0.5 — Pre-launch and production ready
- [ ] Market study: target segments, pricing per model, competitors
- [ ] Pricing strategy finalized: USB, battery (kit vs pre-assembled)
- [ ] Sales channels defined: plugnsat.com, conferences, resellers, partner shops
- [ ] Landing page updated with final pricing and buy CTAs
- [ ] Collect testimonials from beta testers for landing page
- [ ] Full documentation site (docs.plugnsat.com)
- [ ] CE compliance documentation package for resellers
- [ ] Firmware changelog page on plugnsat.com
- [ ] Packaging production ready
- [ ] Stable, battle-tested product (USB and battery shipping reliably)

### v0.6 — Community sharing
- [ ] Share on Bitcoin communities: Twitter, Nostr, Telegram groups, forums
- [ ] Share on Reddit: r/bitcoin, r/lightningnetwork, r/esp32
- [ ] Present at local meetups
- [ ] Post on Plan B Network, Stacker News
- [ ] Collect feedback, testimonials, and early interest

### v1.0 — Sales and distribution
- [ ] Public sales on plugnsat.com: USB and battery (kit and pre-assembled)
- [ ] Battery model to resellers (desk stand, magnetic mount, wall mount)
- [ ] Contact Bitcoin shops and resellers
- [ ] Send demo units to potential partners
- [ ] Present at Bitcoin conferences
- [ ] Explore distribution partnerships (hubs, coworking chains, event organizers)

---
 
## Post-v1.0 (conditional on sales traction)
 
### NFC model
- [ ] NFC module integration (PN532 or similar)
- [ ] Tap-to-pay flow: NFC reads LNURL, wallet opens, pay
- [ ] NFC + QR dual mode on screen
- [ ] Enclosure prototypes for NFC model (antenna positioning)
- [ ] Finalize NFC enclosures and publish STL files
- [ ] Beta testing NFC model with trusted people
- [ ] Update documentation: NFC setup guide
- [ ] Update landing page with NFC model
- [ ] Product photos and video for NFC model
### Multi-device support
- [ ] New web portal section: device manager with "+" button to add PlugNSat units
- [ ] Per-device configuration: name, price, duration, Shelly host
- [ ] Device list view with status (online/offline, last payment, uptime)
- [ ] Edit and remove devices from the portal
- [ ] Multi-device firmware support (one BTCPay store, multiple devices)
- [ ] Multi-device documentation
- [ ] Beta testing multi-device with hub managers and coworking spaces
### Future ideas
- [ ] Sub-domain dashboard per client (client.plugnsat.com)
- [ ] Usage statistics and payment history
- [ ] Multiple Shelly devices per controller
- [ ] Multi-language support on web portal and screen
- [ ] Fiat price display (sats + EUR/USD equivalent)
- [ ] Receipt/proof of payment via email or NFC
- [ ] Referral program for existing buyers
- [ ] White-label option (custom branding per client)
- [ ] Optional OLED/AMOLED screen version for outdoor use
- [ ] Wall-flush Shelly install behind drywall
### Product lineup
- **PlugNSat USB** - Base model (USB-C powered, kit or pre-assembled) — plugnsat.com only
- **PlugNSat Battery** - Portable model (LiPo rechargeable, kit or pre-assembled) — plugnsat.com and resellers
- **PlugNSat NFC** - Premium model (QR + tap-to-pay), post-v1.0 if sales follow
- **Enclosures** - Desk stand, magnetic mount, wall mount
- **Accessories** - Extra Shelly Plug S Gen3, replacement USB-C cables, sticker pack
- **Free resources** - STL files on GitHub, online documentation, open-source firmware
 