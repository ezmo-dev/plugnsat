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

static bool portalAuthEnabled = false;

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>PlugNSat Setup</title>
  <style>
    :root {
      --pn-orange:        #F7931A;
      --pn-orange-hover:  #E8850F;
      --pn-orange-press:  #C9710A;
      --pn-gold:          #FFD700;
      --pn-cyan:          #00E5FF;
      --pn-bg:            #FBFAF8;
      --pn-surface:       #FFFFFF;
      --pn-sunk:          #F4F2EE;
      --pn-fg:            #0A0A0A;
      --pn-fg-2:          #535862;
      --pn-fg-3:          #90939B;
      --pn-border:        #EAE7E0;
      --pn-border-2:      #D9D5CB;
      --pn-success:       #1F8A5B;
      --pn-success-soft:  #E5F4EC;
      --pn-success-bd:    #C8E6D2;
      --pn-danger:        #C8372D;
      --pn-danger-soft:   #FBEAE7;
      --pn-shadow-sm:     0 1px 2px rgba(10,10,10,.04), 0 0 0 1px rgba(10,10,10,.04);
      --pn-shadow-md:     0 8px 24px rgba(10,10,10,.06), 0 1px 2px rgba(10,10,10,.04);
      --pn-focus-ring:    0 0 0 3px rgba(0,229,255,.22);
      --pn-r-input:       8px;
      --pn-r-card:        14px;
      --pn-font:          -apple-system, BlinkMacSystemFont, "SF Pro Text", "Segoe UI", system-ui, "Helvetica Neue", Arial, sans-serif;
      --pn-mono:          ui-monospace, "SF Mono", Menlo, Consolas, monospace;
    }
    *, *::before, *::after { box-sizing: border-box; }
    html, body {
      margin: 0; padding: 0;
      background: var(--pn-bg);
      color: var(--pn-fg);
      font-family: var(--pn-font);
      font-size: 15px;
      line-height: 1.5;
      -webkit-font-smoothing: antialiased;
    }
    .container {
      max-width: 480px;
      margin: 0 auto;
      padding: 24px 20px 40px;
    }
    header {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 14px;
      padding: 24px 0 32px;
      text-align: center;
    }
    .logo svg {
      display: block;
      width: 100%;
      max-width: 280px;
      margin: 0 auto;
      image-rendering: pixelated;
      image-rendering: crisp-edges;
    }
    h1 {
      margin: 6px 0 0;
      font-size: 26px;
      font-weight: 600;
      letter-spacing: -.02em;
      line-height: 1.15;
      color: var(--pn-fg);
    }
    h1 .accent { color: var(--pn-orange); font-weight: 700; }
    .sub {
      margin: 0;
      font-size: 13px;
      color: var(--pn-fg-2);
    }
    form { display: block; counter-reset: sec; }
    .sec {
      background: var(--pn-surface);
      border-radius: var(--pn-r-card);
      box-shadow: var(--pn-shadow-sm);
      padding: 22px 22px 24px;
      margin-bottom: 18px;
      position: relative;
      counter-increment: sec;
    }
    .sec h2 {
      margin: 0 0 14px;
      font-size: 18px;
      font-weight: 600;
      letter-spacing: -.01em;
      color: var(--pn-fg);
    }
    .sec h2::before {
      content: "STEP";
      display: block;
      font-size: 11px;
      font-weight: 600;
      letter-spacing: .08em;
      text-transform: uppercase;
      color: var(--pn-fg-3);
      margin-bottom: 10px;
    }
    .sec h2::after {
      content: "";
      display: block;
      height: 1px;
      background: var(--pn-border);
      margin-top: 14px;
    }
    .sec::after {
      content: counter(sec);
      position: absolute;
      top: 22px; right: 22px;
      width: 22px; height: 22px;
      border-radius: 6px;
      background: var(--pn-sunk);
      color: var(--pn-fg-2);
      font-size: 11px; font-weight: 600;
      font-variant-numeric: tabular-nums;
      display: flex; align-items: center; justify-content: center;
    }
    label {
      display: flex;
      align-items: center;
      margin-top: 12px;
      font-size: 12px;
      font-weight: 500;
      color: var(--pn-fg-2);
    }
    input[type="text"],
    input[type="password"],
    input[type="number"],
    select {
      width: 100%;
      font: inherit;
      font-size: 14px;
      color: var(--pn-fg);
      background: var(--pn-surface);
      border: 1px solid var(--pn-border-2);
      border-radius: var(--pn-r-input);
      padding: 10px 12px;
      margin-top: 6px;
      outline: none;
      transition: border-color .15s ease, box-shadow .15s ease;
      -webkit-appearance: none;
      appearance: none;
    }
    input[type="number"] { font-variant-numeric: tabular-nums; }
    input::placeholder { color: var(--pn-fg-3); }
    input:hover, select:hover { border-color: #C2BCAE; }
    input:focus, select:focus {
      border-color: var(--pn-cyan);
      box-shadow: var(--pn-focus-ring);
    }
    .hint {
      margin-top: 6px;
      font-size: 11px;
      color: var(--pn-fg-3);
      line-height: 1.45;
    }
    .row {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
    }
    button, input[type="submit"] {
      font: inherit;
      cursor: pointer;
      border: none;
      border-radius: var(--pn-r-input);
      transition: background-color .12s ease, transform .08s ease, box-shadow .12s ease;
      -webkit-appearance: none;
      appearance: none;
    }
    button:active, input[type="submit"]:active { transform: scale(.985); }
    .tbtn {
      font-size: 13px;
      font-weight: 500;
      color: var(--pn-fg);
      background: var(--pn-surface);
      border: 1px solid var(--pn-border-2);
      padding: 8px 14px;
      margin-top: 12px;
      margin-right: 8px;
      width: auto;
    }
    .tbtn:hover { background: var(--pn-sunk); }
    .tbtn:focus-visible {
      outline: none;
      box-shadow: var(--pn-focus-ring);
      border-color: var(--pn-cyan);
    }
    .tbtn-cyan { color: var(--pn-cyan); border-color: var(--pn-cyan); }
    .tbtn-cyan:hover { background: rgba(0,229,255,.06); }
    .tbtn-ok  { color: var(--pn-success); border-color: var(--pn-success); background: var(--pn-success-soft); }
    .tbtn-err { color: var(--pn-danger);  border-color: var(--pn-danger);  background: var(--pn-danger-soft); }
    .btn-row { display: flex; gap: 8px; margin-top: 12px; }
    .btn-row .tbtn { flex: 1; margin-top: 0; margin-right: 0; text-align: center; }
    button[type="submit"] {
      display: block;
      width: 100%;
      margin-top: 8px;
      padding: 14px 20px;
      font-size: 15px;
      font-weight: 600;
      color: #0A0A0A;
      background: var(--pn-orange);
      border-radius: 10px;
      box-shadow: 0 4px 12px rgba(247,147,26,.32);
    }
    button[type="submit"]:hover { background: var(--pn-orange-hover); }
    button[type="submit"]:active {
      background: var(--pn-orange-press);
      box-shadow: 0 2px 6px rgba(247,147,26,.28);
    }
    .footer {
      text-align: center;
      margin: 24px 0 0;
      font-size: 11px;
      font-family: var(--pn-mono);
      color: var(--pn-fg-3);
      letter-spacing: .01em;
    }
    .tip {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      width: 16px; height: 16px;
      border-radius: 50%;
      border: 1.5px solid var(--pn-fg-3);
      color: var(--pn-fg-3);
      font-size: 10px;
      font-style: normal;
      font-weight: 600;
      font-family: var(--pn-font);
      line-height: 1;
      cursor: default;
      position: relative;
      margin-left: 5px;
      flex-shrink: 0;
      user-select: none;
    }
    .tip::after {
      content: attr(data-tip);
      position: absolute;
      left: 0;
      top: calc(100% + 8px);
      background: var(--pn-fg);
      color: #fff;
      font-size: 12px;
      font-style: normal;
      font-family: var(--pn-font);
      font-weight: 400;
      padding: 8px 12px;
      border-radius: 8px;
      width: 240px;
      text-align: left;
      line-height: 1.45;
      opacity: 0;
      pointer-events: none;
      transition: opacity .15s ease;
      z-index: 10;
    }
    .tip:hover::after { opacity: 1; }
    .toggle {
      display: flex;
      align-items: center;
      gap: 10px;
      margin-top: 14px;
      cursor: pointer;
      user-select: none;
      font-size: inherit;
    }
    .toggle input[type="checkbox"] {
      position: absolute;
      opacity: 0;
      width: 0; height: 0;
    }
    .toggle-track {
      flex-shrink: 0;
      width: 36px; height: 20px;
      border-radius: 10px;
      background: var(--pn-border-2);
      position: relative;
      transition: background .15s ease;
    }
    .toggle input:checked + .toggle-track { background: var(--pn-cyan); }
    .toggle-thumb {
      position: absolute;
      top: 2px; left: 2px;
      width: 16px; height: 16px;
      border-radius: 50%;
      background: #fff;
      box-shadow: 0 1px 3px rgba(0,0,0,.2);
      transition: transform .15s ease;
    }
    .toggle input:checked + .toggle-track .toggle-thumb { transform: translateX(16px); }
    .toggle-label { font-size: 13px; color: var(--pn-fg-2); font-weight: 500; }
    ::selection { background: rgba(247,147,26,.25); color: var(--pn-fg); }
    @media (prefers-reduced-motion: reduce) {
      *, *::before, *::after {
        transition-duration: .001ms !important;
        animation-duration: .001ms !important;
      }
    }
  </style>
</head>
<body>

<div class="container">

  <header>
    <div class="logo" aria-label="PlugNSat">
      <svg viewBox="0 0 245 50" xmlns="http://www.w3.org/2000/svg" aria-hidden="true">
        <defs><rect id="p" width="4" height="4"/></defs>
        <g fill="#F7931A">
          <use href="#p" x="0" y="7"/><use href="#p" x="5" y="7"/><use href="#p" x="10" y="7"/><use href="#p" x="15" y="7"/>
          <use href="#p" x="0" y="12"/><use href="#p" x="20" y="12"/>
          <use href="#p" x="0" y="17"/><use href="#p" x="20" y="17"/>
          <use href="#p" x="0" y="22"/><use href="#p" x="5" y="22"/><use href="#p" x="10" y="22"/><use href="#p" x="15" y="22"/>
          <use href="#p" x="0" y="27"/><use href="#p" x="0" y="32"/><use href="#p" x="0" y="37"/>
          <use href="#p" x="30" y="7"/><use href="#p" x="30" y="12"/><use href="#p" x="30" y="17"/><use href="#p" x="30" y="22"/><use href="#p" x="30" y="27"/><use href="#p" x="30" y="32"/>
          <use href="#p" x="30" y="37"/><use href="#p" x="35" y="37"/><use href="#p" x="40" y="37"/><use href="#p" x="45" y="37"/><use href="#p" x="50" y="37"/>
          <use href="#p" x="60" y="7"/><use href="#p" x="80" y="7"/><use href="#p" x="60" y="12"/><use href="#p" x="80" y="12"/>
          <use href="#p" x="60" y="17"/><use href="#p" x="80" y="17"/><use href="#p" x="60" y="22"/><use href="#p" x="80" y="22"/>
          <use href="#p" x="60" y="27"/><use href="#p" x="80" y="27"/><use href="#p" x="60" y="32"/><use href="#p" x="80" y="32"/>
          <use href="#p" x="65" y="37"/><use href="#p" x="70" y="37"/><use href="#p" x="75" y="37"/>
          <use href="#p" x="95" y="7"/><use href="#p" x="100" y="7"/><use href="#p" x="105" y="7"/>
          <use href="#p" x="90" y="12"/><use href="#p" x="110" y="12"/><use href="#p" x="90" y="17"/>
          <use href="#p" x="90" y="22"/><use href="#p" x="100" y="22"/><use href="#p" x="105" y="22"/><use href="#p" x="110" y="22"/>
          <use href="#p" x="90" y="27"/><use href="#p" x="110" y="27"/><use href="#p" x="90" y="32"/><use href="#p" x="110" y="32"/>
          <use href="#p" x="95" y="37"/><use href="#p" x="100" y="37"/><use href="#p" x="105" y="37"/>
        </g>
        <g fill="#FFD700">
          <use href="#p" x="135" y="0"/><use href="#p" x="140" y="0"/><use href="#p" x="145" y="0"/>
          <use href="#p" x="130" y="5"/><use href="#p" x="135" y="5"/><use href="#p" x="140" y="5"/>
          <use href="#p" x="130" y="10"/><use href="#p" x="135" y="10"/><use href="#p" x="140" y="10"/>
          <use href="#p" x="130" y="15"/><use href="#p" x="135" y="15"/>
          <use href="#p" x="125" y="20"/><use href="#p" x="130" y="20"/><use href="#p" x="135" y="20"/><use href="#p" x="140" y="20"/><use href="#p" x="145" y="20"/>
          <use href="#p" x="130" y="25"/><use href="#p" x="135" y="25"/><use href="#p" x="140" y="25"/>
          <use href="#p" x="130" y="30"/><use href="#p" x="135" y="30"/>
          <use href="#p" x="130" y="35"/>
          <use href="#p" x="125" y="40"/><use href="#p" x="130" y="40"/>
          <use href="#p" x="125" y="45"/>
        </g>
        <g fill="#00E5FF">
          <use href="#p" x="165" y="7"/><use href="#p" x="170" y="7"/><use href="#p" x="175" y="7"/>
          <use href="#p" x="160" y="12"/><use href="#p" x="180" y="12"/><use href="#p" x="160" y="17"/>
          <use href="#p" x="165" y="22"/><use href="#p" x="170" y="22"/><use href="#p" x="175" y="22"/>
          <use href="#p" x="180" y="27"/><use href="#p" x="160" y="32"/><use href="#p" x="180" y="32"/>
          <use href="#p" x="165" y="37"/><use href="#p" x="170" y="37"/><use href="#p" x="175" y="37"/>
          <use href="#p" x="195" y="7"/><use href="#p" x="200" y="7"/><use href="#p" x="205" y="7"/>
          <use href="#p" x="190" y="12"/><use href="#p" x="210" y="12"/><use href="#p" x="190" y="17"/><use href="#p" x="210" y="17"/>
          <use href="#p" x="190" y="22"/><use href="#p" x="195" y="22"/><use href="#p" x="200" y="22"/><use href="#p" x="205" y="22"/><use href="#p" x="210" y="22"/>
          <use href="#p" x="190" y="27"/><use href="#p" x="210" y="27"/><use href="#p" x="190" y="32"/><use href="#p" x="210" y="32"/>
          <use href="#p" x="190" y="37"/><use href="#p" x="210" y="37"/>
          <use href="#p" x="220" y="7"/><use href="#p" x="225" y="7"/><use href="#p" x="230" y="7"/><use href="#p" x="235" y="7"/><use href="#p" x="240" y="7"/>
          <use href="#p" x="230" y="12"/><use href="#p" x="230" y="17"/><use href="#p" x="230" y="22"/><use href="#p" x="230" y="27"/><use href="#p" x="230" y="32"/><use href="#p" x="230" y="37"/>
        </g>
      </svg>
    </div>
    <h1>Configure your <span class="accent">PlugNSat</span>.</h1>
    <p class="sub">Four steps. About two minutes.</p>
  </header>

  <form action="/save" method="POST">

    <div class="sec">
      <h2>WiFi <span class="tip" data-tip="Your WiFi network name and password. The Shelly plug must be on the same network.">i</span></h2>
      <label>SSID</label>
      <input type="text" name="wifi_ssid" value="%WIFI_SSID%">
      <label>Password</label>
      <input type="password" name="wifi_pass" value="%WIFI_PASS%">
    </div>

    <div class="sec">
      <h2>BTCPay Server</h2>
      <label>Server URL <span class="tip" data-tip="The full URL of your BTCPay Server instance, without trailing slash. Example: https://btcpay.mydomain.com">i</span></label>
      <input type="text" name="btcpay_url" value="%BTCPAY_URL%" placeholder="e.g. https://btcpay.mydomain.com">
      <div class="hint">No trailing slash</div>
      <label>API Key <span class="tip" data-tip="Generate it in BTCPay Server: Account > API Keys. Required permissions: cancreateinvoice, canviewinvoices, canuselightningnode.">i</span></label>
      <input type="password" name="btcpay_key" value="%BTCPAY_KEY%">
      <div class="hint">Permissions: cancreateinvoice, canviewinvoices</div>
      <label>Store ID <span class="tip" data-tip="Found in BTCPay Server: Settings > General. It's the long alphanumeric string in the URL when viewing your store.">i</span></label>
      <input type="text" name="btcpay_store" value="%BTCPAY_STORE%">
    </div>

    <div class="sec">
      <h2>Shelly Plug</h2>
      <label>Shelly hostname or IP <span class="tip" data-tip="Make sure the Shelly is plugged in, connected to the same WiFi, and set to Off (initial state). Use 'Scan network' to find it automatically.">i</span></label>
      <div id="scan-st" class="hint" style="margin-top:6px"></div>
      <select id="shelly-sel" style="display:none;margin-top:8px" onchange="selectShelly(this)"></select>
      <input type="text" name="shelly_host" id="shelly-host" value="%SHELLY_HOST%" placeholder="shellyplugsg3-xxxxxxxxxxxx.local">
      <div class="btn-row">
        <button type="button" class="tbtn" id="btn-scan" onclick="scanShelly()">Scan network</button>
        <button type="button" class="tbtn" id="btn-test" onclick="testShelly()">Test connection</button>
      </div>
      <div id="sst" class="hint" style="margin-top:6px"></div>
    </div>

    <div class="sec">
      <h2>Device settings</h2>
      <label>Name <span class="tip" data-tip="Displayed on the device screen and on the web portal status page.">i</span></label>
      <input type="text" name="dev_name" id="dev-name" value="%DEV_NAME%" placeholder="PlugNSat" oninput="validateName()">
      <div id="name-err" class="hint"></div>
      <label class="toggle">
        <input type="checkbox" name="show_name" id="show-name-cb" value="1" %SHOW_NAME_CHECKED% onchange="validateName()">
        <span class="toggle-track"><span class="toggle-thumb"></span></span>
        <span class="toggle-label">Show name on QR screen</span>
      </label>
      <div class="row">
        <div>
          <label>Price (satoshis) <span class="tip" data-tip="Amount in satoshis the customer pays to activate the plug.">i</span></label>
          <input type="number" name="price_sats" value="%PRICE_SATS%" min="1" max="1000000">
        </div>
        <div>
          <label>Duration (seconds) <span class="tip" data-tip="How long the Shelly stays on after payment, in seconds.">i</span></label>
          <input type="number" name="duration" value="%DURATION%" min="1" max="86400">
        </div>
      </div>
      <label class="toggle">
        <input type="checkbox" name="show_price" value="1" %SHOW_PRICE_CHECKED%>
        <span class="toggle-track"><span class="toggle-thumb"></span></span>
        <span class="toggle-label">Show price on QR screen</span>
      </label>
      <button type="button" class="tbtn" onclick="testPayment()">Simulate payment (free)</button>
      <div id="tpst" class="hint" style="margin-top:6px"></div>
      <label>Settings PIN (4 digits) <span class="tip" data-tip="If set, this 4-digit PIN is required to change Price and Duration directly on the device. Leave empty to disable.">i</span></label>
      <input type="password" name="settings_pin" value="%SETTINGS_PIN%" maxlength="4" placeholder="Optional" inputmode="numeric" pattern="[0-9]{0,4}">
      <div class="hint">Protects Price and Duration settings on the device</div>
      <label>Portal Password <span class="tip" data-tip="If set, the web portal will require this password to access (username: admin). Leave empty to disable. Authentication is always disabled in AP setup mode so you can recover access.">i</span></label>
      <input type="password" name="portal_pw" value="%PORTAL_PW%" placeholder="Optional">
      <div class="hint">Protects this web portal on WiFi (username: admin)</div>
    </div>

    <button type="submit" id="btn-save">Save and restart &rarr;</button>
  </form>

  <p class="footer">v%VERSION% &middot; plugnsat.com &middot; open-source &middot; MIT</p>

</div>

  <script>
  function validateName(){
    var inp=document.getElementById('dev-name');
    var cb=document.getElementById('show-name-cb');
    var err=document.getElementById('name-err');
    var btn=document.getElementById('btn-save');
    var v=inp.value;
    var msg='';
    if(cb.checked){
      if(v.length>18||!/^[A-Za-z0-9 .\-]*$/.test(v))
        msg='Max 18 characters, no special characters';
    } else {
      if(v.length>40) msg='Max 40 characters';
    }
    inp.style.borderColor=msg?'var(--pn-danger)':'';
    inp.style.boxShadow=msg?'0 0 0 3px rgba(200,55,45,.18)':'';
    err.textContent=msg;
    err.style.color='var(--pn-danger)';
    btn.disabled=!!msg;
  }
  function scanShelly(){
    var el=document.getElementById('scan-st');
    var sel=document.getElementById('shelly-sel');
    var btn=document.getElementById('btn-scan');
    el.innerHTML='Scanning...';el.style.color='#888';
    sel.style.display='none';
    btn.className='tbtn tbtn-cyan';
    fetch('/api/scan-shelly')
    .then(function(r){return r.json()})
    .then(function(d){
      if(!d.length){
        el.innerHTML='No Shelly found. Check that your Shelly is connected to the same WiFi network.';
        el.style.color='#f44336';
        btn.className='tbtn tbtn-err';
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
        btn.className='tbtn tbtn-ok';
      }
    }).catch(function(){
      el.innerHTML='Scan failed';el.style.color='#f44336';
      btn.className='tbtn tbtn-err';
    });
  }
  function selectShelly(sel){
    if(sel.value)document.getElementById('shelly-host').value=sel.value;
  }
  function testShelly(){
    var host=document.getElementById('shelly-host').value;
    var el=document.getElementById('sst');
    var btn=document.getElementById('btn-test');
    el.innerHTML='Testing...';el.style.color='#888';
    btn.className='tbtn tbtn-cyan';
    fetch('/test-shelly?host='+encodeURIComponent(host))
    .then(function(r){return r.json()}).then(function(d){
      el.innerHTML=d.ok?'OK! Power: '+d.power+'W':'Failed';
      el.style.color=d.ok?'#4caf50':'#f44336';
      btn.className='tbtn '+(d.ok?'tbtn-ok':'tbtn-err');
    }).catch(function(){
      el.innerHTML='Error';el.style.color='#f44336';
      btn.className='tbtn tbtn-err';
    });
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

const char SAVED_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>PlugNSat</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,system-ui,sans-serif;background:#FBFAF8;min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px}
.wrap{text-align:center;max-width:340px;width:100%}
.logo svg{display:block;width:210px;margin:0 auto 32px;image-rendering:pixelated;image-rendering:crisp-edges}
.saved{font-size:42px;font-weight:800;color:#00E676;margin-bottom:20px;letter-spacing:-.02em}
.card{background:#F4F2EE;border-radius:12px;padding:14px 20px;margin-bottom:28px;font-size:14px;color:#535862}
.count{font-size:13px;color:#90939B}
.back{margin-top:18px;font-size:13px}
.back a{color:#00E5FF;text-decoration:none;font-weight:500}
.back a:hover{text-decoration:underline}
</style>
</head><body>
<div class="wrap">
<div class="logo">
<svg viewBox="0 0 245 50" xmlns="http://www.w3.org/2000/svg" aria-hidden="true">
<defs><rect id="p" width="4" height="4"/></defs>
<g fill="#F7931A">
<use href="#p" x="0" y="7"/><use href="#p" x="5" y="7"/><use href="#p" x="10" y="7"/><use href="#p" x="15" y="7"/>
<use href="#p" x="0" y="12"/><use href="#p" x="20" y="12"/>
<use href="#p" x="0" y="17"/><use href="#p" x="20" y="17"/>
<use href="#p" x="0" y="22"/><use href="#p" x="5" y="22"/><use href="#p" x="10" y="22"/><use href="#p" x="15" y="22"/>
<use href="#p" x="0" y="27"/><use href="#p" x="0" y="32"/><use href="#p" x="0" y="37"/>
<use href="#p" x="30" y="7"/><use href="#p" x="30" y="12"/><use href="#p" x="30" y="17"/><use href="#p" x="30" y="22"/><use href="#p" x="30" y="27"/><use href="#p" x="30" y="32"/>
<use href="#p" x="30" y="37"/><use href="#p" x="35" y="37"/><use href="#p" x="40" y="37"/><use href="#p" x="45" y="37"/><use href="#p" x="50" y="37"/>
<use href="#p" x="60" y="7"/><use href="#p" x="80" y="7"/><use href="#p" x="60" y="12"/><use href="#p" x="80" y="12"/>
<use href="#p" x="60" y="17"/><use href="#p" x="80" y="17"/><use href="#p" x="60" y="22"/><use href="#p" x="80" y="22"/>
<use href="#p" x="60" y="27"/><use href="#p" x="80" y="27"/><use href="#p" x="60" y="32"/><use href="#p" x="80" y="32"/>
<use href="#p" x="65" y="37"/><use href="#p" x="70" y="37"/><use href="#p" x="75" y="37"/>
<use href="#p" x="95" y="7"/><use href="#p" x="100" y="7"/><use href="#p" x="105" y="7"/>
<use href="#p" x="90" y="12"/><use href="#p" x="110" y="12"/><use href="#p" x="90" y="17"/>
<use href="#p" x="90" y="22"/><use href="#p" x="100" y="22"/><use href="#p" x="105" y="22"/><use href="#p" x="110" y="22"/>
<use href="#p" x="90" y="27"/><use href="#p" x="110" y="27"/><use href="#p" x="90" y="32"/><use href="#p" x="110" y="32"/>
<use href="#p" x="95" y="37"/><use href="#p" x="100" y="37"/><use href="#p" x="105" y="37"/>
</g>
<g fill="#FFD700">
<use href="#p" x="135" y="0"/><use href="#p" x="140" y="0"/><use href="#p" x="145" y="0"/>
<use href="#p" x="130" y="5"/><use href="#p" x="135" y="5"/><use href="#p" x="140" y="5"/>
<use href="#p" x="130" y="10"/><use href="#p" x="135" y="10"/><use href="#p" x="140" y="10"/>
<use href="#p" x="130" y="15"/><use href="#p" x="135" y="15"/>
<use href="#p" x="125" y="20"/><use href="#p" x="130" y="20"/><use href="#p" x="135" y="20"/><use href="#p" x="140" y="20"/><use href="#p" x="145" y="20"/>
<use href="#p" x="130" y="25"/><use href="#p" x="135" y="25"/><use href="#p" x="140" y="25"/>
<use href="#p" x="130" y="30"/><use href="#p" x="135" y="30"/>
<use href="#p" x="130" y="35"/>
<use href="#p" x="125" y="40"/><use href="#p" x="130" y="40"/>
<use href="#p" x="125" y="45"/>
</g>
<g fill="#00E5FF">
<use href="#p" x="165" y="7"/><use href="#p" x="170" y="7"/><use href="#p" x="175" y="7"/>
<use href="#p" x="160" y="12"/><use href="#p" x="180" y="12"/><use href="#p" x="160" y="17"/>
<use href="#p" x="165" y="22"/><use href="#p" x="170" y="22"/><use href="#p" x="175" y="22"/>
<use href="#p" x="180" y="27"/><use href="#p" x="160" y="32"/><use href="#p" x="180" y="32"/>
<use href="#p" x="165" y="37"/><use href="#p" x="170" y="37"/><use href="#p" x="175" y="37"/>
<use href="#p" x="195" y="7"/><use href="#p" x="200" y="7"/><use href="#p" x="205" y="7"/>
<use href="#p" x="190" y="12"/><use href="#p" x="210" y="12"/><use href="#p" x="190" y="17"/><use href="#p" x="210" y="17"/>
<use href="#p" x="190" y="22"/><use href="#p" x="195" y="22"/><use href="#p" x="200" y="22"/><use href="#p" x="205" y="22"/><use href="#p" x="210" y="22"/>
<use href="#p" x="190" y="27"/><use href="#p" x="210" y="27"/><use href="#p" x="190" y="32"/><use href="#p" x="210" y="32"/>
<use href="#p" x="190" y="37"/><use href="#p" x="210" y="37"/>
<use href="#p" x="220" y="7"/><use href="#p" x="225" y="7"/><use href="#p" x="230" y="7"/><use href="#p" x="235" y="7"/><use href="#p" x="240" y="7"/>
<use href="#p" x="230" y="12"/><use href="#p" x="230" y="17"/><use href="#p" x="230" y="22"/><use href="#p" x="230" y="27"/><use href="#p" x="230" y="32"/><use href="#p" x="230" y="37"/>
</g>
</svg>
</div>
<div class="saved">Saved!</div>
<div class="card">%DEV_NAME% &middot; %PRICE_SATS% sats &middot; %DURATION%s</div>
<div class="count" id="c">Restarting in 3...</div>
<p class="back"><a href="http://%IP%">Back to PlugNSat settings &rarr;</a></p>
</div>
<script>var n=3;var t=setInterval(function(){n--;if(n>0){document.getElementById('c').textContent='Restarting in '+n+'...';}else{document.getElementById('c').textContent='Restarting now...';clearInterval(t);}},1000);</script>
</body></html>
)rawliteral";

bool checkPortalAuth(WebServer &server, PlugNSatConfig &config) {
  if (!portalAuthEnabled || config.portalPassword.length() == 0) return true;
  if (!server.authenticate("admin", config.portalPassword.c_str())) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

String htmlEscape(const String &s) {
  String out;
  out.reserve(s.length());
  for (int i = 0; i < (int)s.length(); i++) {
    char c = s[i];
    switch (c) {
      case '&':  out += "&amp;";  break;
      case '<':  out += "&lt;";   break;
      case '>':  out += "&gt;";   break;
      case '"':  out += "&quot;"; break;
      case '\'': out += "&#39;";  break;
      default:   out += c;        break;
    }
  }
  return out;
}

String processSavedPage(PlugNSatConfig &config) {
  String html = String(SAVED_PAGE);
  html.replace("%DEV_NAME%",   htmlEscape(config.deviceName));
  html.replace("%PRICE_SATS%", String(config.priceSats));
  html.replace("%DURATION%",   String(config.activationDuration));
  String ip = WiFi.localIP().toString();
  if (ip == "0.0.0.0") ip = WiFi.softAPIP().toString();
  html.replace("%IP%", ip);
  return html;
}

String processTemplate(PlugNSatConfig &config) {
  String html = String(HTML_PAGE);
  html.replace("%WIFI_SSID%",   htmlEscape(config.wifiSsid));
  html.replace("%WIFI_PASS%",   config.wifiPass.length() > 0 ? "********" : "");
  html.replace("%BTCPAY_URL%",  htmlEscape(config.btcpayUrl));
  html.replace("%BTCPAY_KEY%",  config.btcpayApiKey.length() > 0 ? "********" : "");
  html.replace("%BTCPAY_STORE%", htmlEscape(config.btcpayStoreId));
  html.replace("%SHELLY_HOST%", htmlEscape(config.shellyHost));
  html.replace("%PRICE_SATS%",  String(config.priceSats));
  html.replace("%DURATION%",    String(config.activationDuration));
  html.replace("%DEV_NAME%",    htmlEscape(config.deviceName));
  html.replace("%SETTINGS_PIN%",       config.pin.length() > 0 ? "****" : "");
  html.replace("%SHOW_NAME_CHECKED%",  config.showName  ? "checked" : "");
  html.replace("%SHOW_PRICE_CHECKED%", config.showPrice ? "checked" : "");
  html.replace("%VERSION%",            FIRMWARE_VERSION);
  html.replace("%PORTAL_PW%",          config.portalPassword.length() > 0 ? "********" : "");
  return html;
}

void setupWebPortal(WebServer &server, PlugNSatConfig &config, Preferences &prefs) {
  portalAuthEnabled = (WiFi.getMode() == WIFI_STA);

  server.on("/", HTTP_GET, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;
    server.send(200, "text/html", processTemplate(config));
  });
  
  server.on("/save", HTTP_POST, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;
    config.wifiSsid           = server.arg("wifi_ssid");
    String newWifiPass = server.arg("wifi_pass");
    if (newWifiPass != "********") config.wifiPass = newWifiPass;
    config.btcpayUrl          = server.arg("btcpay_url");
    String newApiKey = server.arg("btcpay_key");
    if (newApiKey.length() > 0 && newApiKey != "********") config.btcpayApiKey = newApiKey;
    config.btcpayStoreId      = server.arg("btcpay_store");
    config.shellyHost         = server.arg("shelly_host");
    config.priceSats          = server.arg("price_sats").toInt();
    config.activationDuration = server.arg("duration").toInt();
    config.deviceName         = server.arg("dev_name");
    config.showName  = server.arg("show_name")  == "1";
    config.showPrice = server.arg("show_price") == "1";
    String newPin             = server.arg("settings_pin");
    if (newPin.length() == 0) {
      config.pin = "";
    } else if (newPin.length() == 4 && newPin != "****") {
      bool allDigits = true;
      for (int i = 0; i < 4; i++) { if (!isDigit(newPin[i])) { allDigits = false; break; } }
      if (allDigits) config.pin = newPin;
    }

    String newPortalPw = server.arg("portal_pw");
    if (newPortalPw == "********") {
      // preserve existing password
    } else if (newPortalPw.length() == 0) {
      config.portalPassword = "";
    } else {
      config.portalPassword = newPortalPw;
    }

    if (config.priceSats < 1) config.priceSats = 100;
    if (config.activationDuration < 1) config.activationDuration = 60;
    if (config.deviceName.length() == 0) config.deviceName = "PlugNSat";
    if (config.btcpayUrl.endsWith("/")) {
      config.btcpayUrl = config.btcpayUrl.substring(0, config.btcpayUrl.length() - 1);
    }
    if (config.priceSats > 1000000) config.priceSats = 1000000;
    if (config.activationDuration > 86400) config.activationDuration = 86400;
    if ((int)config.deviceName.length() > 40)
      config.deviceName = config.deviceName.substring(0, 40);
    if ((int)config.wifiSsid.length() > 64)
      config.wifiSsid = config.wifiSsid.substring(0, 64);
    if ((int)config.btcpayUrl.length() > 200)
      config.btcpayUrl = config.btcpayUrl.substring(0, 200);
    if ((int)config.shellyHost.length() > 80)
      config.shellyHost = config.shellyHost.substring(0, 80);

    saveConfig();
    
    server.send(200, "text/html", processSavedPage(config));
    delay(3000);
    ESP.restart();
  });

  server.on("/test-shelly", HTTP_GET, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;
    String host = server.arg("host");
    if (host.length() == 0 || host.length() > 80
        || host.indexOf("/") >= 0
        || host.indexOf("http") >= 0
        || host.indexOf("\\") >= 0) {
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

  server.on("/test-payment", HTTP_GET, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;
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

  server.on("/api/scan-shelly", HTTP_GET, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;
    Serial.println("mDNS scan starting...");
    int n = MDNS.queryService("http", "tcp");
    Serial.println("mDNS scan found: " + String(n) + " services total");
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < n; i++) {
      String h = MDNS.hostname(i);
      Serial.println("  [" + String(i) + "] " + h + " -> " + MDNS.address(i).toString());
      String hLower = h;
      hLower.toLowerCase();
      if (hLower.length() > 0 && hLower.startsWith("shelly")) {
        JsonObject obj = arr.add<JsonObject>();
        obj["hostname"] = hLower + ".local";
        obj["ip"]       = MDNS.address(i).toString();
        obj["name"]     = h;
      }
    }
    String json;
    serializeJson(doc, json);
    Serial.println("mDNS response: " + json);
    server.send(200, "application/json", json);
  });

  server.on("/api/status", HTTP_GET, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;
    JsonDocument doc;
    doc["device"] = config.deviceName;
    doc["price"] = config.priceSats;
    doc["duration"] = config.activationDuration;
    doc["wifi"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
    doc["uptime"] = millis() / 1000;
    doc["version"] = FIRMWARE_VERSION;
    String json; serializeJson(doc, json);
    server.send(200, "application/json", json);
  });
}

#endif