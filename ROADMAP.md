## Roadmap

### v0.1 (current)
- [x] Basic prototype: LNURL QR, BTCPay polling, Shelly activation
- [x] Web portal configuration
- [x] Auto QR refresh before invoice expiry
- [x] Simulate payment button for testing
- [x] mDNS autodiscovery for Shelly devices (scan network instead of manual IP)
- [X] Splash screen with logo and branding
- [X] Color theme and screen brightness adjustments
- [X] Adjust price and duration from device buttons (PIN-protected)
- [X] First 3Dprint case (test, print, upload)
- [ ] UI polish (paid screen, error screen, info screen, settings screen)
- [ ] Web portal mobile-friendly cleanup
- [ ] Web portal UI polish
- [ ] First early documentation (user flow, documentation...)
- [ ] Test video for documentation purposes

### v0.2 - Polish and UX
- [ ] UI improvements on web portal (better layout, mobile responsive)
- [ ] UI improvements on screen (price, device name alongside QR)
- [ ] Bug fixes and code cleanup
- [ ] Better error messages and info indicators on screen and web portal
- [ ] Simplify first-time setup flow for early documentation
- [ ] WiFi signal strength indicator on screen

### v0.3 - Blink integration (B2C mode)
- [ ] Blink wallet (API Galoy) as simple backend option
- [ ] Setup: create a Blink account, generate API token, done
- [ ] No server, no VPS, no Docker, no node to manage
- [ ] Choose backend during setup: BTCPay (advanced) or Blink (simple)
- [ ] Auto-discover Shelly on local network via mDNS (no manual IP needed)
- [ ] Target: small shops, cafes, individuals, meetups

### v0.4 - Multi-device (B2B mode)
- [ ] BTCPay Server as advanced backend
- [ ] Multiple PlugNSat devices on the same store
- [ ] Dashboard for monitoring all devices remotely
- [ ] Per-device pricing and activation settings
- [ ] Target: Bitcoin hubs, coworking spaces, hotels, large venues

### v0.5 - Enclosures and mounting
- [ ] Custom 3D printed enclosure with PlugNSat branding
- [ ] Magnetic mount (attach to any metal surface)
- [ ] Desk stand (angled for easy scanning)
- [ ] Wall-flush Shelly install (wired behind drywall using Shelly Plus 1 or Shelly 1 Mini, tamper-proof, CE compliant, same API)

### v1.0 - Production ready
- [ ] OTA firmware updates (update via WiFi, no USB needed)
- [ ] NFC tap-to-pay support (premium version)
- [ ] Battery-powered mode (LiPo, wireless operation)
- [ ] Full documentation and video tutorials
- [ ] Available as kit or pre-assembled
- [ ] Optional OLED/AMOLED screen version for outdoor use

### Future ideas
- [ ] Sub-domain dashboard per client (client.plugnsat.com)
- [ ] Usage statistics and payment history
- [ ] Multiple Shelly devices per controller
