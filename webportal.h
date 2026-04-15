/*
 * PlugNSat - Web Configuration Portal
 *
 * Served by ESP32 at http://{ip}/ for setup and config changes.
 *
 * LIBRARIES:
 * - WebServer (ESP32 built-in HTTP server)
 * - ArduinoJson by Benoit Blanchon
 * - Preferences (ESP32 NVS flash storage)
 * 
 * License: MIT © 2026
 * Author: ezmo-dev (PlugNSat)
 */

#ifndef WEBPORTAL_H
#define WEBPORTAL_H

#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "config.h"

void saveConfig();  // Defined in main .ino

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>PlugNSat Setup</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{font-family:-apple-system,system-ui,sans-serif;background:#1a1a2e;color:#e0e0e0;padding:20px;max-width:480px;margin:0 auto}
    h1{color:#f7931a;text-align:center;margin-bottom:8px;font-size:28px}
    .sub{text-align:center;color:#888;margin-bottom:24px;font-size:14px}
    .sec{background:#16213e;border-radius:12px;padding:20px;margin-bottom:16px}
    .sec h2{color:#f7931a;font-size:16px;margin-bottom:16px;padding-bottom:8px;border-bottom:1px solid #2a2a4a}
    label{display:block;font-size:13px;color:#aaa;margin:12px 0 4px}
    label:first-of-type{margin-top:0}
    input[type="text"],input[type="password"],input[type="number"]{width:100%;padding:10px 12px;background:#0f3460;border:1px solid #2a2a4a;border-radius:8px;color:#fff;font-size:15px;outline:none}
    input:focus{border-color:#f7931a}
    select{width:100%;padding:10px 12px;background:#0f3460;border:1px solid #2a2a4a;border-radius:8px;color:#fff;font-size:14px;outline:none;margin-bottom:8px;cursor:pointer}
    select:focus{border-color:#f7931a}
    .hint{font-size:11px;color:#666;margin-top:4px}
    button{width:100%;padding:14px;background:#f7931a;color:#1a1a2e;border:none;border-radius:8px;font-size:16px;font-weight:600;cursor:pointer;margin-top:20px}
    button:hover{background:#e8850f}
    .tbtn{background:#2a2a4a;color:#e0e0e0;padding:8px 16px;border:1px solid #444;border-radius:6px;font-size:13px;cursor:pointer;margin-top:8px;width:auto}
    .tbtn:hover{background:#3a3a5a}
    .footer{text-align:center;color:#444;font-size:12px;margin-top:24px}
    .ok{background:#0a3d2a;color:#4caf50;padding:12px;border-radius:8px;margin-top:16px;text-align:center}
  </style>
</head>
<body>
  <h1>PlugNSat</h1>
  <p class="sub">Lightning Smart Plug Controller</p>
  
  <form action="/save" method="POST">
    <div class="sec">
      <h2>WiFi</h2>
      <label>SSID</label>
      <input type="text" name="wifi_ssid" value="%WIFI_SSID%">
      <label>Password</label>
      <input type="password" name="wifi_pass" value="%WIFI_PASS%">
    </div>
    
    <div class="sec">
      <h2>BTCPay Server</h2>
      <label>Server URL</label>
      <input type="text" name="btcpay_url" value="%BTCPAY_URL%" placeholder="e.g. https://btcpay.mydomain.com">
      <div class="hint">No trailing slash</div>
      <label>API Key</label>
      <input type="password" name="btcpay_key" value="%BTCPAY_KEY%">
      <div class="hint">Permissions: cancreateinvoice, canviewinvoices</div>
      <label>Store ID</label>
      <input type="text" name="btcpay_store" value="%BTCPAY_STORE%">
    </div>
    
    <div class="sec">
      <h2>Shelly Plug</h2>
      <label>Shelly hostname or IP</label>
      <button type="button" class="tbtn" onclick="scanShelly()">Scan network</button>
      <div id="scan-st" class="hint" style="margin-top:6px"></div>
      <select id="shelly-sel" style="display:none;margin-top:8px" onchange="selectShelly(this)"></select>
      <input type="text" name="shelly_host" id="shelly-host" value="%SHELLY_HOST%" placeholder="shellyplugsg3-xxxxxxxxxxxx.local" style="margin-top:8px">
      <button type="button" class="tbtn" onclick="testShelly()" style="margin-top:8px">Test connection</button>
      <div id="sst"></div>
    </div>
    
    <div class="sec">
      <h2>Device</h2>
      <label>Name</label>
      <input type="text" name="dev_name" value="%DEV_NAME%" placeholder="PlugNSat">
      <label>Price (satoshis)</label>
      <input type="number" name="price_sats" value="%PRICE_SATS%" min="1" max="1000000">
      <label>Activation duration (seconds)</label>
      <input type="number" name="duration" value="%DURATION%" min="1" max="86400">
      <button type="button" class="tbtn" onclick="testPayment()">Simulate payment (free)</button>
      <div id="tpst"></div>
    </div>
    
    <button type="submit">Save and restart</button>
  </form>
  
  <p class="footer">PlugNSat v0.1.0</p>

  <script>
  function scanShelly(){
    var el=document.getElementById('scan-st');
    var sel=document.getElementById('shelly-sel');
    el.innerHTML='Scanning...';el.style.color='#888';
    sel.style.display='none';
    fetch('/api/scan-shelly')
    .then(function(r){return r.json()})
    .then(function(d){
      if(!d.length){
        el.innerHTML='No Shelly found. Check that your Shelly is connected to the same WiFi network.';
        el.style.color='#f44336';
      } else {
        el.innerHTML='';
        sel.innerHTML='<option value="">-- select a device --</option>';
        d.forEach(function(s){
          var o=document.createElement('option');
          o.value=s.hostname;
          o.text=s.name+' ('+s.ip+')';
          sel.appendChild(o);
        });
        sel.style.display='block';
      }
    }).catch(function(){el.innerHTML='Scan failed';el.style.color='#f44336'});
  }
  function selectShelly(sel){
    if(sel.value)document.getElementById('shelly-host').value=sel.value;
  }
  function testShelly(){
    var host=document.getElementById('shelly-host').value;
    var el=document.getElementById('sst');
    el.innerHTML='Testing...';el.style.color='#888';
    fetch('/test-shelly?host='+encodeURIComponent(host))
    .then(function(r){return r.json()}).then(function(d){
      el.innerHTML=d.ok?'OK! Power: '+d.power+'W':'Failed';
      el.style.color=d.ok?'#4caf50':'#f44336';
    }).catch(function(){el.innerHTML='Error';el.style.color='#f44336'});
  }
  function testPayment(){
    var el=document.getElementById('tpst');
    el.innerHTML='Activating...';el.style.color='#888';
    fetch('/test-payment')
    .then(function(r){return r.json()}).then(function(d){
      el.innerHTML=d.ok?'Shelly ON for '+d.duration+'s!':'Failed: '+(d.error||'unknown');
      el.style.color=d.ok?'#4caf50':'#f44336';
    }).catch(function(){el.innerHTML='Error';el.style.color='#f44336'});
  }
  </script>
</body>
</html>
)rawliteral";

String processTemplate(PlugNSatConfig &config) {
  String html = String(HTML_PAGE);
  html.replace("%WIFI_SSID%",   config.wifiSsid);
  html.replace("%WIFI_PASS%",   config.wifiPass);
  html.replace("%BTCPAY_URL%",  config.btcpayUrl);
  html.replace("%BTCPAY_KEY%",  config.btcpayApiKey);
  html.replace("%BTCPAY_STORE%", config.btcpayStoreId);
  html.replace("%SHELLY_HOST%", config.shellyHost);
  html.replace("%PRICE_SATS%",  String(config.priceSats));
  html.replace("%DURATION%",    String(config.activationDuration));
  html.replace("%DEV_NAME%",    config.deviceName);
  return html;
}

void setupWebPortal(WebServer &server, PlugNSatConfig &config, Preferences &prefs) {
  
  server.on("/", HTTP_GET, [&config, &server]() {
    server.send(200, "text/html", processTemplate(config));
  });
  
  server.on("/save", HTTP_POST, [&config, &server]() {
    config.wifiSsid           = server.arg("wifi_ssid");
    config.wifiPass           = server.arg("wifi_pass");
    config.btcpayUrl          = server.arg("btcpay_url");
    config.btcpayApiKey       = server.arg("btcpay_key");
    config.btcpayStoreId      = server.arg("btcpay_store");
    config.shellyHost         = server.arg("shelly_host");
    config.priceSats          = server.arg("price_sats").toInt();
    config.activationDuration = server.arg("duration").toInt();
    config.deviceName         = server.arg("dev_name");
    
    if (config.priceSats < 1) config.priceSats = 100;
    if (config.activationDuration < 1) config.activationDuration = 60;
    if (config.deviceName.length() == 0) config.deviceName = "PlugNSat";
    if (config.btcpayUrl.endsWith("/")) {
      config.btcpayUrl = config.btcpayUrl.substring(0, config.btcpayUrl.length() - 1);
    }
    
    saveConfig();
    
    server.send(200, "text/html", 
      "<html><body style='background:#1a1a2e;color:#4caf50;text-align:center;"
      "padding:60px;font-family:sans-serif'>"
      "<h1>Saved!</h1><p>Restarting...</p></body></html>");
    delay(3000);
    ESP.restart();
  });


// Test Shelly connection

  server.on("/test-shelly", HTTP_GET, [&server]() {
    String host = server.arg("host");
    if (host.length() == 0) {
      server.send(400, "application/json", "{\"ok\":false}");
      return;
    }
    HTTPClient http;
    if (!http.begin("http://" + host + "/rpc/Switch.GetStatus?id=0")) {
      server.send(200, "application/json", "{\"ok\":false}");
      return;
    }
    http.setTimeout(3000);
    int code = http.GET();
    if (code == 200) {
      String r = http.getString(); http.end();
      JsonDocument doc; deserializeJson(doc, r);
      float p = doc["apower"] | 0.0f;
      server.send(200, "application/json", "{\"ok\":true,\"power\":" + String(p, 1) + "}");
    } else {
      http.end();
      server.send(200, "application/json", "{\"ok\":false}");
    }
  });

// Test payment without BTCPay server to setup the duration
//

  server.on("/test-payment", HTTP_GET, [&config, &server]() {
  HTTPClient http;
  String url = "http://" + config.shellyHost
               + "/rpc/switch.set?id=0&on=true&toggle_after="
               + String(config.activationDuration);
  
  if (!http.begin(url)) {
    server.send(200, "application/json", "{\"ok\":false,\"error\":\"Connection failed\"}");
    return;
  }
  http.setTimeout(5000);
  int code = http.GET();
  http.end();
  
  if (code == 200) {
    server.send(200, "application/json", 
      "{\"ok\":true,\"duration\":" + String(config.activationDuration) + "}");
  } else {
    server.send(200, "application/json", "{\"ok\":false,\"error\":\"Shelly error\"}");
  }
});
  
// Scan local network for Shelly devices via mDNS

  server.on("/api/scan-shelly", HTTP_GET, [&server]() {
    int n = MDNS.queryService("http", "tcp", 3000);
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < n; i++) {
      String h = MDNS.hostname(i);
      if (h.length() > 0 && h.startsWith("shelly")) {
        JsonObject obj = arr.add<JsonObject>();
        obj["hostname"] = h + ".local";
        obj["ip"]       = MDNS.IP(i).toString();
        obj["name"]     = h;
      }
    }
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });

  server.on("/api/status", HTTP_GET, [&config, &server]() {
    JsonDocument doc;
    doc["device"] = config.deviceName;
    doc["price"] = config.priceSats;
    doc["duration"] = config.activationDuration;
    doc["wifi"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
    doc["uptime"] = millis() / 1000;
    doc["version"] = "0.2.0";
    String json; serializeJson(doc, json);
    server.send(200, "application/json", json);
  });
}

#endif
