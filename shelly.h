/*
 * PlugNSat - Shelly Plug Control
 *
 * Local HTTP API for Shelly Gen2/Gen3 devices.
 * No firmware modification, no authentication needed on LAN.
 * 
 * LIBRARIES:
 * - HTTPClient (local HTTP requests)
 * 
 * REFERENCES:
 * - Shelly Gen2+ RPC API: shelly-api-docs.shelly.cloud/gen2/
 * - Switch component: shelly-api-docs.shelly.cloud/gen2/ComponentsAndServices/Switch/
 * - Shelly Plug S Gen3: shelly-api-docs.shelly.cloud/gen2/Devices/Gen3/ShellyPlugSG3/
 * 
 * License: MIT © 2026
 * Author: ezmo-dev (PlugNSat)
 * 
 */

#ifndef SHELLY_H
#define SHELLY_H

#include <HTTPClient.h>
#include <ArduinoJson.h>

// Turn ON with auto-off timer
bool shellySwitchOn(const String& shellyHost, int durationSeconds) {
  HTTPClient http;
  String url = "http://" + shellyHost
               + "/rpc/switch.set?id=0&on=true&toggle_after="
               + String(durationSeconds);
  
  if (!http.begin(url)) return false;
  http.setTimeout(5000);
  
  int code = http.GET();
  http.end();
  
  Serial.println("Shelly ON: " + String(code));
  return (code == 200);
}

// Turn OFF manually
bool shellySwitchOff(const String& shellyHost) {
  HTTPClient http;
  String url = "http://" + shellyHost + "/rpc/switch.set?id=0&on=false";
  
  if (!http.begin(url)) return false;
  http.setTimeout(5000);
  
  int code = http.GET();
  http.end();
  return (code == 200);
}

// Check if Shelly is reachable
bool shellyIsOnline(const String& shellyHost) {
  HTTPClient http;
  String url = "http://" + shellyHost + "/rpc/Switch.GetStatus?id=0";
  
  if (!http.begin(url)) return false;
  http.setTimeout(3000);
  
  int code = http.GET();
  http.end();
  return (code == 200);
}

#endif
