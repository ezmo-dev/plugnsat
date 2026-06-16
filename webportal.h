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
#include <Update.h>

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
    select {
      background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='8' viewBox='0 0 12 8'%3E%3Cpath d='M1 1.5L6 6.5L11 1.5' stroke='%2390939B' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round' fill='none'/%3E%3C/svg%3E");
      background-repeat: no-repeat;
      background-position: right 12px center;
      padding-right: 32px;
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
      white-space: pre-line;
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
    .segmented {
      display: inline-flex;
      border: 1px solid var(--pn-border);
      border-radius: 8px;
      overflow: hidden;
      margin-top: 6px;
      margin-bottom: 4px;
    }
    .segmented input[type="radio"] { display: none; }
    .segmented label {
      padding: 8px 18px;
      font-size: 13px;
      font-weight: 600;
      color: var(--pn-fg-2);
      cursor: pointer;
      background: transparent;
      transition: background 0.15s, color 0.15s;
      margin: 0;
    }
    .segmented label + input + label { border-left: 1px solid var(--pn-border); }
    .segmented input:checked + label { background: var(--pn-cyan); color: #fff; }
    ::selection { background: rgba(247,147,26,.25); color: var(--pn-fg); }
    .sec h2 { cursor: pointer; user-select: none; }
    .chev { display: inline-block; width: 7px; height: 7px; border-right: 2px solid var(--pn-fg-3); border-bottom: 2px solid var(--pn-fg-3); transform: rotate(45deg); transition: transform .2s ease; margin-right: 9px; margin-bottom: 2px; vertical-align: middle; pointer-events: none; }
    .sec.collapsed .chev { transform: rotate(-45deg); }
    .sec-summary { display: none; font-size: 13px; color: var(--pn-fg-3); margin-top: 4px; }
    .sec.collapsed .sec-summary { display: block; }
    .sec.collapsed { border-left: 3px solid var(--pn-orange); }
    .sec-body { overflow: hidden; max-height: 1000px; transition: max-height .3s ease; }
    .sec.collapsed .sec-body { max-height: 0; }
    .sec.collapsed h2 { margin-bottom: 0; }
    .sec.collapsed h2::after { display: none; }
    @media (prefers-reduced-motion: reduce) {
      *, *::before, *::after {
        transition-duration: .001ms !important;
        animation-duration: .001ms !important;
      }
    }
    .card { background: var(--pn-surface); border-radius: var(--pn-r-card); box-shadow: var(--pn-shadow-sm); padding: 22px; }
    .card h2 { margin: 0 0 14px; font-size: 15px; font-weight: 600; color: var(--pn-fg); }
    .alert { border-radius: var(--pn-r-input); padding: 12px 14px; font-size: 13px; margin-top: 14px; }
    .alert-ok { background: var(--pn-success-soft); color: var(--pn-success); border: 1px solid #C8E6D2; }
    .alert-err { background: var(--pn-danger-soft); color: var(--pn-danger); border: 1px solid #F5C6C3; }
    .btn-check { font-size: 13px; font-weight: 500; color: var(--pn-fg); background: var(--pn-surface); border: 1px solid var(--pn-border-2); border-radius: var(--pn-r-input); padding: 8px 14px; cursor: pointer; font-family: inherit; transition: background .12s ease; -webkit-appearance: none; appearance: none; }
    .btn-check:hover { background: var(--pn-sunk); }
    .btn-check:disabled { color: var(--pn-fg-3); cursor: not-allowed; }
    .btn-install { display: inline-block; font-size: 13px; font-weight: 600; color: #0A0A0A; background: var(--pn-orange); border: none; border-radius: var(--pn-r-input); padding: 8px 14px; cursor: pointer; font-family: inherit; margin-top: 12px; transition: background .12s ease; -webkit-appearance: none; appearance: none; box-shadow: 0 2px 8px rgba(247,147,26,.28); }
    .btn-install:hover { background: var(--pn-orange-hover); }
    .btn-install:disabled { background: var(--pn-border-2); color: var(--pn-fg-3); box-shadow: none; cursor: not-allowed; }
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
    <p class="sub">Five steps. Two minutes.</p>
  </header>

  <form action="/save" method="POST">

    <div class="sec">
      <h2>WiFi <span class="tip" data-tip="The WiFi network PlugNSat connects to. Your Shelly plug must be on the same network for them to communicate.">i</span></h2>
      <label>SSID</label>
      <input type="text" name="wifi_ssid" value="%WIFI_SSID%">
      <label>Password</label>
      <input type="password" name="wifi_pass" value="%WIFI_PASS%">
    </div>

    <div class="sec">
      <h2>Lightning Backend</h2>
      <label>Backend <span class="tip" data-tip="How you receive Lightning payments.&#10;- BTCPay Server: you run your own server, full control, no middleman.&#10;- Blink: ready-to-use wallet, no server needed, sign up at blink.sv.">i</span></label>
      <select name="backend" id="backend-sel" onchange="toggleBackend()">
        <option value="0" %BACKEND_BTCPAY_SEL%>BTCPay Server</option>
        <option value="1" %BACKEND_BLINK_SEL%>Blink</option>
      </select>
      <div id="sec-btcpay">
        <div class="hint" style="margin-bottom:8px; font-weight:500; color:var(--pn-fg-2);">BTCPay Server settings</div>
        <label>Server URL <span class="tip" data-tip="Where your BTCPay Server is hosted. Enter the full URL without trailing slash. Example: https://btcpay.mydomain.com">i</span></label>
        <input type="text" name="btcpay_url" value="%BTCPAY_URL%" placeholder="e.g. https://btcpay.mydomain.com">
        <div class="hint">No trailing slash</div>
        <label>API Key <span class="tip" data-tip="A secret key that lets PlugNSat create and check invoices on your BTCPay Server. Generate one in Account > API Keys with these permissions:&#10;- cancreateinvoice&#10;- canviewinvoices&#10;- canuselightningnode">i</span></label>
        <input type="password" name="btcpay_key" value="%BTCPAY_KEY%">
        <div class="hint">Permissions: cancreateinvoice, canviewinvoices</div>
        <label>Store ID <span class="tip" data-tip="Identifies which store receives payments. Find it in your BTCPay Server under Settings > General, or in the URL bar when viewing your store.">i</span></label>
        <input type="text" name="btcpay_store" value="%BTCPAY_STORE%">
      </div>
      <div id="sec-blink">
        <div class="hint" style="margin-bottom:8px; font-weight:500; color:var(--pn-fg-2);">Blink Wallet settings</div>
        <label>API Key <span class="tip" data-tip="Lets PlugNSat create invoices on your Blink wallet. Generate one at dashboard.blink.sv under API Keys. It starts with blink_.">i</span></label>
        <input type="password" name="blink_key" value="%BLINK_KEY%">
        <div class="hint">Starts with blink_</div>
        <label>BTC Wallet ID <span class="tip" data-tip="Which wallet receives the sats. Find it on dashboard.blink.sv.">i</span></label>
        <input type="text" name="blink_wid" value="%BLINK_WID%">
      </div>
    </div>

    <div class="sec">
      <h2>Shelly Plug</h2>
      <label>Shelly hostname or IP <span class="tip" data-tip="The address of your Shelly plug on the local network. Make sure it is plugged in, on the same WiFi, and set to Off as its default state. Use Scan network to find it automatically.">i</span></label>
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
      <label>Hardware model <span class="tip" data-tip="Select USB-C for the mains-powered model, Battery for the LiPo model. Battery enables battery info and the power-off menu.">i</span></label>
      <div class="segmented">
        <input type="radio" name="is_battery" id="model-usbc" value="0" %MODEL_USBC_CHECKED%>
        <label for="model-usbc">USB-C</label>
        <input type="radio" name="is_battery" id="model-batt" value="1" %MODEL_BATT_CHECKED%>
        <label for="model-batt">Battery</label>
      </div>
      <label>Name <span class="tip" data-tip="The name shown on the device screen and on this page. Keep it short if you enable display.">i</span></label>
      <input type="text" name="dev_name" id="dev-name" value="%DEV_NAME%" placeholder="PlugNSat" oninput="validateName()">
      <div id="name-err" class="hint"></div>
      <label class="toggle">
        <input type="checkbox" name="show_name" id="show-name-cb" value="1" %SHOW_NAME_CHECKED% onchange="validateName()">
        <span class="toggle-track"><span class="toggle-thumb"></span></span>
        <span class="toggle-label">Show name on QR screen</span>
      </label>
      <div class="row">
        <div>
          <label>Price (satoshis) <span class="tip" data-tip="How many sats a customer pays per activation. 1 sat = 0.00000001 BTC.">i</span></label>
          <input type="number" name="price_sats" value="%PRICE_SATS%" min="1" max="1000000">
        </div>
        <div>
          <label>Duration (seconds) <span class="tip" data-tip="How long the plug stays on after each payment. 60 = one minute, 3600 = one hour.">i</span></label>
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
    </div>

    <div class="sec">
      <h2>Security</h2>
      <label>Device PIN (4 digits) <span class="tip" data-tip="Anyone near the PlugNSat can press its buttons to change price and duration. Set a 4-digit PIN to prevent this. Leave empty to skip.">i</span></label>
      <input type="password" name="settings_pin" value="%SETTINGS_PIN%" maxlength="4" placeholder="Optional" inputmode="numeric" pattern="[0-9]{0,4}">
      <div class="hint">Prevents price and duration changes from the device buttons</div>
      <label>Web Access Password <span class="tip" data-tip="Anyone on your WiFi network can open this page. Set a password to require login (username: admin). Leave empty to skip. Always disabled in setup mode so you never get locked out.">i</span></label>
      <input type="password" name="portal_pw" value="%PORTAL_PW%" placeholder="Optional">
      <div class="hint">Protects this configuration page on WiFi (username: admin)</div>
    </div>

    <button type="submit" id="btn-save">Save and restart &rarr;</button>

    <div class="card" style="margin-top:18px;">
      <h2 style="display:flex;align-items:baseline;justify-content:space-between;gap:8px;">
        <span>Firmware update</span>
        <span style="font-size:12px;font-weight:400;color:var(--pn-fg-3,#90939B);">current v%VERSION%</span>
      </h2>
      <label style="display:flex;align-items:flex-start;gap:10px;cursor:pointer;margin-bottom:10px;">
        <input type="checkbox" name="auto_update" id="auto_update" %AUTO_UPDATE_CHECKED% onchange="toggleUpdateBtn()" style="margin-top:3px;flex-shrink:0;">
        <span>
          <strong style="font-size:14px;">Auto-update on boot</strong><br>
          <span style="font-size:12px;color:var(--pn-fg-2,#535862);">When enabled, the device checks for a new firmware version every time it restarts and installs it automatically. Leave unchecked to stay in full control and update manually.</span>
        </span>
      </label>

      <div id="manual-update" style="border-top:1px solid var(--pn-border,#EAE7E0);padding-top:14px;margin-top:4px;">
        <button type="button" class="btn-check" id="btn-check" onclick="checkUpdate()">Check for updates</button>
        <span id="update-status" style="font-size:13px;color:var(--pn-fg-3,#90939B);margin-left:10px;"></span>
        <div class="alert alert-ok" id="update-ok" style="display:none;margin-top:12px;"></div>
        <div class="alert alert-err" id="update-err" style="display:none;margin-top:12px;"></div>
      </div>
    </div>
  </form>

  <div class="footer">PlugNSat v%VERSION%</div>

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
  function toggleBackend(){
    var v=document.getElementById('backend-sel').value;
    var bp=document.getElementById('sec-btcpay');
    var bl=document.getElementById('sec-blink');
    var show=v==='0'?bp:bl;
    var hide=v==='0'?bl:bp;
    show.style.display='';
    show.style.borderTop='1px solid var(--pn-border)';
    show.style.marginTop='16px';
    show.style.paddingTop='16px';
    hide.style.display='none';
  }
  function getSummary(s,i){
    if(i===0){var v=s.querySelector('[name=wifi_ssid]').value;return v||'Not configured';}
    if(i===1){var sel=s.querySelector('[name=backend]');return sel.options[sel.selectedIndex].text;}
    if(i===2){var v=s.querySelector('[name=shelly_host]').value;return v||'Not configured';}
    if(i===3){var n=s.querySelector('[name=dev_name]').value||'Unnamed';var p=s.querySelector('[name=price_sats]').value;var d=s.querySelector('[name=duration]').value;return n+', '+p+' sats, '+d+'s';}
    if(i===4){var pin=s.querySelector('[name=settings_pin]').value;var pw=s.querySelector('[name=portal_pw]').value;var r=[];if(pin)r.push('Device PIN set');if(pw)r.push('Web password set');return r.length?r.join(', '):'No PIN, no password';}
    return '';
  }
  function initSections(){
    var secs=document.querySelectorAll('.sec');
    var saved={};
    try{saved=JSON.parse(localStorage.getItem('plugnsat_sections')||'{}');}catch(e){}
    secs.forEach(function(s,i){
      var k='s'+i;
      var h=s.querySelector('h2');
      var body=document.createElement('div');
      body.className='sec-body';
      while(h.nextSibling){body.appendChild(h.nextSibling);}
      s.appendChild(body);
      var chev=document.createElement('span');
      chev.className='chev';
      h.insertBefore(chev,h.firstChild);
      var sum=document.createElement('div');
      sum.className='sec-summary';
      h.after(sum);
      if(saved[k]===false){s.classList.add('collapsed');sum.textContent=getSummary(s,i);}
      h.addEventListener('click',function(){
        s.classList.toggle('collapsed');
        var collapsed=s.classList.contains('collapsed');
        if(collapsed)sum.textContent=getSummary(s,i);
        var st={};
        try{st=JSON.parse(localStorage.getItem('plugnsat_sections')||'{}');}catch(e){}
        st[k]=!collapsed;
        localStorage.setItem('plugnsat_sections',JSON.stringify(st));
      });
    });
  }
  function toggleUpdateBtn() {
    var cb = document.getElementById('auto_update');
    var box = document.getElementById('manual-update');
    if (!cb || !box) return;
    box.style.display = cb.checked ? 'none' : 'block';
  }

  function checkUpdate() {
    var btn = document.getElementById('btn-check');
    var status = document.getElementById('update-status');
    var okBox = document.getElementById('update-ok');
    var errBox = document.getElementById('update-err');
    btn.disabled = true;
    btn.textContent = 'Checking...';
    status.textContent = '';
    okBox.style.display = 'none';
    errBox.style.display = 'none';
    fetch('/api/check-update')
      .then(function(r) { return r.json(); })
      .then(function(d) {
        btn.disabled = false;
        btn.textContent = 'Check for updates';
        if (!d.ok) {
          errBox.textContent = 'Check failed: ' + d.error;
          errBox.style.display = 'block';
          return;
        }
        if (d.available) {
          okBox.innerHTML = '<div>Version <strong>' + d.latest + '</strong> is available.'
            + ' You are running ' + d.current + '.</div>'
            + '<button type="button" class="btn-install" id="btn-install"'
            + ' onclick="installUpdate(\'' + d.latest + '\')">Install update</button>';
          okBox.style.display = 'block';
        } else {
          status.textContent = 'You are up to date (' + d.current + ').';
        }
      })
      .catch(function() {
        btn.disabled = false;
        btn.textContent = 'Check for updates';
        errBox.textContent = 'Network error. Check your WiFi connection.';
        errBox.style.display = 'block';
      });
  }

  function installUpdate(version) {
    var installBtn = document.getElementById('btn-install');
    var okBox = document.getElementById('update-ok');
    var errBox = document.getElementById('update-err');
    if (installBtn) installBtn.disabled = true;
    okBox.innerHTML = 'Downloading and flashing v' + version
      + '... Do not close this page. The device will reboot automatically.';
    fetch('/api/ota-update', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: 'version=' + encodeURIComponent(version)
    })
      .then(function(r) { return r.json(); })
      .then(function(d) {
        if (d.ok) {
          okBox.innerHTML = 'Update started. The device is downloading and will reboot.'
            + ' This page will stop responding during the reboot.'
            + ' Reconnect to plugnsat.local after reboot.';
        } else {
          errBox.textContent = 'Update failed: ' + d.error;
          errBox.style.display = 'block';
          if (installBtn) installBtn.disabled = false;
        }
      })
      .catch(function() {
        errBox.textContent = 'Network error during update.';
        errBox.style.display = 'block';
        if (installBtn) installBtn.disabled = false;
      });
  }

  // Set initial visibility of the manual update section on page load
  toggleUpdateBtn();
  document.addEventListener('DOMContentLoaded',function(){initSections();toggleBackend();toggleUpdateBtn();});
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

const char OTA_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>PlugNSat — Firmware Update</title>
  <style>
    :root {
      --pn-orange: #F7931A;
      --pn-bg: #FBFAF8;
      --pn-surface: #FFFFFF;
      --pn-fg: #0A0A0A;
      --pn-fg-2: #535862;
      --pn-fg-3: #90939B;
      --pn-border: #EAE7E0;
      --pn-border-2: #D9D5CB;
      --pn-success: #1F8A5B;
      --pn-success-soft: #E5F4EC;
      --pn-danger: #C8372D;
      --pn-danger-soft: #FBEAE7;
      --pn-shadow-sm: 0 1px 2px rgba(10,10,10,.04), 0 0 0 1px rgba(10,10,10,.04);
      --pn-r-card: 14px;
      --pn-r-input: 8px;
      --pn-font: -apple-system, BlinkMacSystemFont, "SF Pro Text", "Segoe UI", system-ui, "Helvetica Neue", Arial, sans-serif;
    }
    *, *::before, *::after { box-sizing: border-box; }
    html, body { margin: 0; padding: 0; background: var(--pn-bg); color: var(--pn-fg); font-family: var(--pn-font); font-size: 15px; line-height: 1.5; -webkit-font-smoothing: antialiased; }
    .container { max-width: 480px; margin: 0 auto; padding: 24px 20px 40px; }
    header { display: flex; flex-direction: column; align-items: center; gap: 14px; padding: 24px 0 32px; text-align: center; }
    .logo svg { display: block; width: 100%; max-width: 280px; margin: 0 auto; image-rendering: pixelated; image-rendering: crisp-edges; }
    h1 { margin: 6px 0 0; font-size: 26px; font-weight: 600; letter-spacing: -.02em; line-height: 1.15; color: var(--pn-fg); }
    h1 .accent { color: var(--pn-orange); font-weight: 700; }
    .card { background: var(--pn-surface); border-radius: var(--pn-r-card); box-shadow: var(--pn-shadow-sm); padding: 22px; margin-bottom: 16px; }
    .card h2 { margin: 0 0 14px; font-size: 15px; font-weight: 600; color: var(--pn-fg); border-bottom: 1px solid var(--pn-border); padding-bottom: 10px; }
    .version-line { font-size: 13px; color: var(--pn-fg-2); margin-bottom: 4px; }
    .version-line span { font-weight: 600; color: var(--pn-fg); }
    .drop-zone { border: 2px dashed var(--pn-border-2); border-radius: var(--pn-r-input); padding: 32px 20px; text-align: center; cursor: pointer; transition: border-color .15s ease, background .15s ease; margin: 14px 0; }
    .drop-zone:hover, .drop-zone.drag-over { border-color: var(--pn-orange); background: rgba(247,147,26,.04); }
    .drop-zone input[type="file"] { display: none; }
    .drop-label { font-size: 13px; color: var(--pn-fg-3); margin-top: 8px; }
    .drop-label strong { color: var(--pn-fg); }
    .file-name { font-size: 13px; color: var(--pn-fg-2); margin-top: 8px; font-weight: 500; display: none; }
    .btn-flash { display: block; width: 100%; padding: 13px 20px; font-size: 15px; font-weight: 600; color: #0A0A0A; background: var(--pn-orange); border: none; border-radius: 10px; cursor: pointer; font-family: inherit; box-shadow: 0 4px 12px rgba(247,147,26,.32); transition: background .12s ease; -webkit-appearance: none; appearance: none; }
    .btn-flash:hover { background: #E8850F; }
    .btn-flash:disabled { background: var(--pn-border-2); color: var(--pn-fg-3); box-shadow: none; cursor: not-allowed; }
    .progress-wrap { display: none; margin-top: 14px; }
    .progress-bar-bg { background: var(--pn-border); border-radius: 99px; height: 8px; overflow: hidden; }
    .progress-bar { background: var(--pn-orange); height: 8px; width: 0%; border-radius: 99px; transition: width .2s ease; }
    .progress-label { font-size: 12px; color: var(--pn-fg-3); margin-top: 6px; text-align: center; }
    .alert { display: none; border-radius: var(--pn-r-input); padding: 12px 14px; font-size: 13px; margin-top: 14px; }
    .alert-ok { background: var(--pn-success-soft); color: var(--pn-success); border: 1px solid #C8E6D2; }
    .alert-err { background: var(--pn-danger-soft); color: var(--pn-danger); border: 1px solid #F5C6C3; }
    .back { text-align: center; margin-top: 20px; font-size: 13px; color: var(--pn-fg-3); }
    .back a { color: var(--pn-orange); text-decoration: none; font-weight: 500; }
    .back a:hover { text-decoration: underline; }
    .warning-box { background: #FFF8EC; border: 1px solid #FFD97A; border-radius: var(--pn-r-input); padding: 10px 14px; font-size: 12px; color: #7A5000; margin-bottom: 14px; line-height: 1.5; }
    .btn-check { font-size: 13px; font-weight: 500; color: var(--pn-fg); background: var(--pn-surface); border: 1px solid var(--pn-border-2); border-radius: var(--pn-r-input); padding: 8px 14px; cursor: pointer; font-family: inherit; transition: background .12s ease; -webkit-appearance: none; appearance: none; }
    .btn-check:hover { background: var(--pn-sunk, #F4F2EE); }
    .btn-check:disabled { color: var(--pn-fg-3); cursor: not-allowed; }
    .btn-install { display: inline-block; font-size: 13px; font-weight: 600; color: #0A0A0A; background: var(--pn-orange); border: none; border-radius: var(--pn-r-input); padding: 8px 14px; cursor: pointer; font-family: inherit; margin-top: 12px; transition: background .12s ease; -webkit-appearance: none; appearance: none; box-shadow: 0 2px 8px rgba(247,147,26,.28); }
    .btn-install:hover { background: #E8850F; }
    .btn-install:disabled { background: var(--pn-border-2); color: var(--pn-fg-3); box-shadow: none; cursor: not-allowed; }
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
    <h1>Firmware <span class="accent">Update</span> <span style="font-size:16px;font-weight:400;color:var(--pn-fg-3);">(dev)</span>.</h1>
  </header>

  <div class="card">
    <h2>Upload new firmware</h2>
    <div class="warning-box">The device will reboot after flashing. If the new firmware fails to connect to WiFi within 60 seconds, it will roll back automatically to the previous version.</div>
    <div class="drop-zone" id="dz-fw" onclick="document.getElementById('f-fw').click()">
      <input type="file" id="f-fw" accept=".bin">
      <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="#90939B" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><polyline points="16 16 12 12 8 16"/><line x1="12" y1="12" x2="12" y2="21"/><path d="M20.39 18.39A5 5 0 0 0 18 9h-1.26A8 8 0 1 0 3 16.3"/></svg>
      <div class="drop-label"><strong>Firmware</strong> — click or drag .bin</div>
    </div>
    <div class="file-name" id="fn-fw"></div>

    <div class="drop-zone" id="dz-sig" onclick="document.getElementById('f-sig').click()" style="margin-top:10px">
      <input type="file" id="f-sig" accept=".sig">
      <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="#90939B" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
      <div class="drop-label"><strong>Signature</strong> — click or drag .sig</div>
    </div>
    <div class="file-name" id="fn-sig"></div>
    <div class="progress-wrap" id="pw">
      <div class="progress-bar-bg"><div class="progress-bar" id="pb"></div></div>
      <div class="progress-label" id="pl">Uploading...</div>
    </div>
    <div class="alert alert-ok" id="ok">Firmware flashed successfully. Device is rebooting...</div>
    <div class="alert alert-err" id="err"></div>
    <button class="btn-flash" id="btn" disabled onclick="doFlash()">Flash firmware</button>
  </div>

  <div class="back"><a href="/">&larr; Back to settings</a></div>
</div>
<script>
  var fwInput  = document.getElementById('f-fw');
  var sigInput = document.getElementById('f-sig');
  var dzFw     = document.getElementById('dz-fw');
  var dzSig    = document.getElementById('dz-sig');
  var fnFw     = document.getElementById('fn-fw');
  var fnSig    = document.getElementById('fn-sig');
  var btn      = document.getElementById('btn');
  var pw       = document.getElementById('pw');
  var pb       = document.getElementById('pb');
  var pl       = document.getElementById('pl');
  var okBox    = document.getElementById('ok');
  var errBox   = document.getElementById('err');
  var fwFile   = null;
  var sigFile  = null;

  function checkReady() {
    btn.disabled = !(fwFile && sigFile);
  }

  fwInput.addEventListener('change', function() {
    if (fwInput.files.length > 0) {
      fwFile = fwInput.files[0];
      fnFw.textContent = fwFile.name + ' (' + (fwFile.size / 1024).toFixed(1) + ' KB)';
      fnFw.style.display = 'block';
      checkReady();
    }
  });
  sigInput.addEventListener('change', function() {
    if (sigInput.files.length > 0) {
      sigFile = sigInput.files[0];
      fnSig.textContent = sigFile.name + ' (' + sigFile.size + ' bytes)';
      fnSig.style.display = 'block';
      checkReady();
    }
  });

  function setupDrop(dz, onFile) {
    dz.addEventListener('dragover', function(e) { e.preventDefault(); dz.classList.add('drag-over'); });
    dz.addEventListener('dragleave', function() { dz.classList.remove('drag-over'); });
    dz.addEventListener('drop', function(e) {
      e.preventDefault(); dz.classList.remove('drag-over');
      if (e.dataTransfer.files.length > 0) onFile(e.dataTransfer.files[0]);
    });
  }
  setupDrop(dzFw, function(f) {
    fwFile = f;
    fnFw.textContent = f.name + ' (' + (f.size / 1024).toFixed(1) + ' KB)';
    fnFw.style.display = 'block';
    checkReady();
  });
  setupDrop(dzSig, function(f) {
    sigFile = f;
    fnSig.textContent = f.name + ' (' + f.size + ' bytes)';
    fnSig.style.display = 'block';
    checkReady();
  });

  function doFlash() {
    if (!fwFile || !sigFile) return;
    btn.disabled = true;
    okBox.style.display = 'none';
    errBox.style.display = 'none';
    pw.style.display = 'block';
    pb.style.width = '0%';
    pl.textContent = 'Uploading...';
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/ota/upload', true);
    xhr.upload.onprogress = function(e) {
      if (e.lengthComputable) {
        var pct = Math.round(e.loaded / e.total * 100);
        pb.style.width = pct + '%';
        pl.textContent = 'Uploading... ' + pct + '%';
      }
    };
    xhr.onload = function() {
      pw.style.display = 'none';
      if (xhr.status === 200) {
        okBox.style.display = 'block';
      } else {
        errBox.textContent = xhr.status === 403
          ? 'Signature invalide — flash refuse.'
          : 'Erreur ' + xhr.status + ': ' + xhr.responseText;
        errBox.style.display = 'block';
        btn.disabled = false;
      }
    };
    xhr.onerror = function() {
      pw.style.display = 'none';
      errBox.textContent = 'Upload error. Check your connection and try again.';
      errBox.style.display = 'block';
      btn.disabled = false;
    };
    var fd = new FormData();
    fd.append('firmware',   fwFile,  fwFile.name);
    fd.append('signature',  sigFile, sigFile.name);
    xhr.send(fd);
  }

</script>
</body>
</html>
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
  html.replace("%BACKEND_BTCPAY_SEL%", config.backendType == BACKEND_BTCPAY ? "selected" : "");
  html.replace("%BACKEND_BLINK_SEL%",  config.backendType == BACKEND_BLINK  ? "selected" : "");
  html.replace("%BLINK_KEY%",          config.blinkApiKey.length() > 0 ? "********" : "");
  html.replace("%BLINK_WID%",          htmlEscape(config.blinkWalletId));
  html.replace("%SHELLY_HOST%", htmlEscape(config.shellyHost));
  html.replace("%PRICE_SATS%",  String(config.priceSats));
  html.replace("%DURATION%",    String(config.activationDuration));
  html.replace("%DEV_NAME%",    htmlEscape(config.deviceName));
  html.replace("%SETTINGS_PIN%",       config.pin.length() > 0 ? "****" : "");
  html.replace("%SHOW_NAME_CHECKED%",  config.showName  ? "checked" : "");
  html.replace("%MODEL_USBC_CHECKED%", config.isBattery ? "" : "checked");
  html.replace("%MODEL_BATT_CHECKED%", config.isBattery ? "checked" : "");
  html.replace("%SHOW_PRICE_CHECKED%", config.showPrice ? "checked" : "");
  html.replace("%AUTO_UPDATE_CHECKED%", config.autoUpdate ? "checked" : "");
  html.replace("%VERSION%",            FIRMWARE_VERSION);
  html.replace("%PORTAL_PW%",          config.portalPassword.length() > 0 ? "********" : "");
  return html;
}

String processOtaPage(PlugNSatConfig &config) {
  String html = String(OTA_PAGE);
  html.replace("%VERSION%",  FIRMWARE_VERSION);
  html.replace("%DEV_NAME%", htmlEscape(config.deviceName));
  return html;
}

static uint8_t* _otaFirmwareBuf = nullptr;
static size_t   _otaFirmwareBufLen = 0;
static uint8_t  _otaSigBuf[256];
static size_t   _otaSigBufLen = 0;
static bool     _otaFirmwareReady = false;
static bool     _otaSigReady = false;

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
    String newBlinkKey = server.arg("blink_key");
    if (newBlinkKey.length() > 0 && newBlinkKey != "********") config.blinkApiKey = newBlinkKey;
    config.blinkWalletId      = server.arg("blink_wid");
    config.backendType = (BackendType)server.arg("backend").toInt();
    if (config.backendType < 0 || config.backendType >= BACKEND_COUNT)
      config.backendType = BACKEND_BTCPAY;
    config.shellyHost         = server.arg("shelly_host");
    config.priceSats          = server.arg("price_sats").toInt();
    config.activationDuration = server.arg("duration").toInt();
    config.deviceName         = server.arg("dev_name");
    config.showName  = server.arg("show_name")  == "1";
    config.showPrice = server.arg("show_price") == "1";
    config.autoUpdate = server.hasArg("auto_update");
    config.isBattery  = (server.arg("is_battery") == "1");
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

  server.on("/api/check-update", HTTP_GET, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;
    OtaUpdateInfo info = otaCheckUpdate();
    String json;
    if (info.error.length() > 0) {
      json = "{\"ok\":false,\"error\":\"" + info.error + "\"}";
    } else {
      json = "{\"ok\":true,\"available\":"
             + String(info.available ? "true" : "false")
             + ",\"current\":\"" + String(FIRMWARE_VERSION) + "\""
             + ",\"latest\":\"" + info.latestVersion + "\"}";
    }
    server.send(200, "application/json", json);
  });

  server.on("/api/ota-update", HTTP_POST, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;

    String version = server.arg("version");
    if (version.length() == 0 || version.length() > 20) {
      server.send(400, "application/json",
                  "{\"ok\":false,\"error\":\"Missing or invalid version\"}");
      return;
    }

    // Validate version format: digits and dots only
    for (size_t i = 0; i < version.length(); i++) {
      char c = version.charAt(i);
      if (!isDigit(c) && c != '.') {
        server.send(400, "application/json",
                    "{\"ok\":false,\"error\":\"Invalid version format\"}");
        return;
      }
    }

    Serial.println("OTA: starting auto-update to v" + version);
    server.send(200, "application/json", "{\"ok\":true,\"status\":\"downloading\"}");

    // Download and flash runs after response is sent
    OtaFlashResult flash = otaDownloadAndFlash(version);
    if (flash.success) {
      Serial.println("OTA: rebooting into new firmware");
      delay(500);
      ESP.restart();
    } else {
      Serial.println("OTA: auto-update failed: " + flash.error);
    }
  });

  server.on("/ota/dev", HTTP_GET, [&config, &server]() {
    if (!checkPortalAuth(server, config)) return;
    server.send(200, "text/html", processOtaPage(config));
  });

  server.on("/ota/upload", HTTP_POST, [&server]() {
    // Completion handler
    if (!_otaFirmwareReady || !_otaSigReady) {
      if (_otaFirmwareBuf) { free(_otaFirmwareBuf); _otaFirmwareBuf = nullptr; }
      server.send(400, "text/plain", "Missing firmware or signature file");
      return;
    }

    bool sigOk = otaVerifySignature(_otaFirmwareBuf, _otaFirmwareBufLen,
                                     _otaSigBuf, _otaSigBufLen);
    if (!sigOk) {
      free(_otaFirmwareBuf); _otaFirmwareBuf = nullptr;
      _otaFirmwareReady = false; _otaSigReady = false;
      server.send(403, "text/plain", "Invalid signature — flash refused");
      return;
    }

    // Signature valid: flash the buffered firmware
    if (!Update.begin(_otaFirmwareBufLen)) {
      free(_otaFirmwareBuf); _otaFirmwareBuf = nullptr;
      server.send(500, "text/plain", "Flash begin failed");
      return;
    }
    size_t written = Update.write(_otaFirmwareBuf, _otaFirmwareBufLen);
    free(_otaFirmwareBuf); _otaFirmwareBuf = nullptr;
    _otaFirmwareReady = false; _otaSigReady = false;

    if (written != _otaFirmwareBufLen || !Update.end(true)) {
      server.send(500, "text/plain", "Flash write failed");
      return;
    }

    Serial.println("OTA: flash complete, rebooting");
    server.send(200, "text/plain", "OK");
    delay(500);
    ESP.restart();

  }, [&server]() {
    // Upload handler — called for each chunk of each file part
    HTTPUpload &upload = server.upload();

    if (upload.name == "firmware") {
      if (upload.status == UPLOAD_FILE_START) {
        Serial.println("OTA: receiving firmware: " + upload.filename);
        _otaFirmwareReady = false;
        if (_otaFirmwareBuf) { free(_otaFirmwareBuf); _otaFirmwareBuf = nullptr; }
        _otaFirmwareBufLen = 0;
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        _otaFirmwareBuf = (uint8_t*)realloc(_otaFirmwareBuf,
                                             _otaFirmwareBufLen + upload.currentSize);
        if (!_otaFirmwareBuf) {
          Serial.println("OTA: firmware buffer alloc failed");
          return;
        }
        memcpy(_otaFirmwareBuf + _otaFirmwareBufLen,
               upload.buf, upload.currentSize);
        _otaFirmwareBufLen += upload.currentSize;
      } else if (upload.status == UPLOAD_FILE_END) {
        _otaFirmwareReady = true;
        Serial.println("OTA: firmware buffered, " + String(_otaFirmwareBufLen) + " bytes");
      }

    } else if (upload.name == "signature") {
      if (upload.status == UPLOAD_FILE_START) {
        Serial.println("OTA: receiving signature: " + upload.filename);
        _otaSigReady = false;
        _otaSigBufLen = 0;
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (_otaSigBufLen + upload.currentSize <= sizeof(_otaSigBuf)) {
          memcpy(_otaSigBuf + _otaSigBufLen, upload.buf, upload.currentSize);
          _otaSigBufLen += upload.currentSize;
        } else {
          Serial.println("OTA: signature buffer overflow");
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        _otaSigReady = true;
        Serial.println("OTA: signature received, " + String(_otaSigBufLen) + " bytes");
      }
    }
  });
}

#endif