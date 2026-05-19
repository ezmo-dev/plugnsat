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
- [X] Better error messages and info indicators on all screens
- [X] Info tooltips on web portal fields
- [X] QR display modes: Show name/price toggles on QR screen (configurable via web portal)
- [X] First test video
- [X] Documentation structure: create outline with all major sections
- [X] README v1 on GitHub
- [X] Full code cleanup before moving forward

### v0.2 — Blink integration and enclosures (current)
- [ ] Blink wallet API integration
- [ ] Backend choice on web portal: BTCPay or Blink
- [ ] Web portal adapts fields based on backend choice
- [ ] Update info indicators on web portal for both backends
- [ ] Enclosure prototypes: desk stand, magnetic mount, wall mount
- [ ] Test prints for each enclosure
- [ ] UI polish: review all screens for improvements and new integrations
- [ ] Continue documentation: Blink guide, BTCPay guide

### v0.3 - OTA, landing page, and beta testing (USB model)
- [ ] OTA firmware updates via WiFi
- [ ] "Check for updates" button in web portal
- [ ] Auto-update option on boot
- [ ] Finalize USB enclosures and publish STL files on GitHub
- [ ] UI improvements on screen: price and device name alongside QR, WiFi signal indicator
- [ ] Full design coherence review: screens, web portal, enclosures, docs
- [ ] Landing page v1 on plugnsat.com
- [ ] Product photos for landing page: device, enclosures, screens, setup flow
- [ ] Product video for landing page
- [ ] Quick-start guide (printed in box)
- [ ] Troubleshooting page
- [ ] Beta testing USB model: send units to trusted people
- [ ] Collect feedback on setup, documentation, enclosure, daily use
- [ ] Code cleanup

### v0.4 - Battery mode prototype
- [ ] Battery-powered prototype (LiPo integration with T-Display S3)
- [ ] Power management: sleep mode, low battery indicator on screen
- [ ] Battery level indicator on screen and web portal
- [ ] Fix issues reported by USB beta testers
- [ ] Update documentation and landing page with beta feedback

### v0.5 - Battery enclosures, NFC prototype, and packaging
- [ ] Enclosures for battery model: desk stand, magnetic mount, wall mount
- [ ] Test prints for battery enclosures
- [ ] STL files for battery enclosures on GitHub
- [ ] NFC module integration (PN532 or similar)
- [ ] Tap-to-pay flow: NFC reads LNURL, wallet opens, pay
- [ ] NFC + QR dual mode on screen
- [ ] Enclosure prototypes for NFC model (antenna positioning)
- [ ] Packaging design: box, insert, printed quick-start guide, sticker
- [ ] Update documentation: battery mode guide, NFC setup guide
- [ ] Beta testing battery model with trusted people

### v0.6 - Finalization
- [ ] Finalize NFC enclosures and publish STL files
- [ ] Beta testing NFC model with trusted people
- [ ] Finalize packaging for all models
- [ ] Finalize all documentation
- [ ] Update landing page with full product range (USB, battery, NFC)
- [ ] Product photos and video for all three models

### v0.7 - Multi-device support
- [ ] New web portal section: device manager with "+" button to add PlugNSat units
- [ ] Per-device configuration: name, price, duration, Shelly host
- [ ] Device list view with status (online/offline, last payment, uptime)
- [ ] Edit and remove devices from the portal
- [ ] Multi-device firmware support (one BTCPay store, multiple devices)
- [ ] Multi-device documentation
- [ ] Beta testing multi-device with hub managers and coworking spaces

### v0.8 - Community sharing and final feedback
- [ ] Share on Bitcoin communities: Twitter, Nostr, Telegram groups, forums
- [ ] Share on Reddit: r/bitcoin, r/lightningnetwork, r/esp32
- [ ] Present at local meetups
- [ ] Post on Plan B Network, Stacker News
- [ ] Collect feedback, testimonials, and early interest
- [ ] Final round of beta testing with all three models
- [ ] Fix all reported issues
- [ ] All documentation reviewed and finalized

### v0.9 - Pre-launch
- [ ] Market study: target segments, pricing per model, competitors
- [ ] Pricing strategy finalized: USB, battery, NFC (kit vs pre-assembled)
- [ ] Sales channels defined: plugnsat.com, conferences, resellers, partner shops
- [ ] Landing page updated with final pricing and buy CTAs
- [ ] Packaging production ready
- [ ] Collect testimonials from beta testers for landing page

### v0.9.1 - Production ready
- [ ] Full documentation site (docs.plugnsat.com)
- [ ] CE compliance documentation package for resellers
- [ ] Firmware changelog page on plugnsat.com
- [ ] Stable, battle-tested product
- [ ] All models shipping reliably

### v1.0 - Sales and distribution
- [ ] Public sales on plugnsat.com: kit and pre-assembled options
- [ ] Contact Bitcoin shops and resellers
- [ ] Send demo units to potential partners
- [ ] Present at Bitcoin conferences
- [ ] Explore distribution partnerships (hubs, coworking chains, event organizers)

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
- **PlugNSat USB** - Base model (USB-C powered, kit or pre-assembled)
- **PlugNSat Battery** - Portable model (LiPo rechargeable, kit or pre-assembled)
- **PlugNSat NFC** - Premium model (QR + tap-to-pay, USB-C or battery, kit or pre-assembled)
- **Enclosures** - Desk stand, magnetic mount, wall mount (for each model)
- **Accessories** - Extra Shelly Plug S Gen3, replacement USB-C cables, sticker pack
- **Free resources** - STL files on GitHub, online documentation, open-source firmware