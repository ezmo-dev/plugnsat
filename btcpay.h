/*
 * PlugNSat - BTCPay Server API
 * 
 * API: https://docs.btcpayserver.org/API/Greenfield/v1/
 * 
 * WORKFLOW:
 * Creates Lightning invoices and checks payment status via JSON.
 * APIkey configured on webportal during the setup process
 * 
 * LIBRARIES:
 * - ArduinoJson by Benoit Blanchon (JSON parsing)
 * - WiFiClientSecure (HTTPS requests)
 * 
 * REFERENCES:
 * - BTCPay Greenfield API: docs.btcpayserver.org/API/Greenfield/v1/
 * - Invoice endpoint: POST /api/v1/stores/{storeId}/invoices
 * - Payment methods: GET /api/v1/stores/{storeId}/invoices/{id}/payment-methods
 * - LNURL spec: github.com/lnurl/luds
 * 
 * License: MIT © 2026
 * Author: ezmo-dev (PlugNSat)
 */

#ifndef BTCPAY_H
#define BTCPAY_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "tls.h"

// Forward declaration
bool btcpayGetLNURL(const String& url, const String& key, const String& store, const String& id, String &lnurl, bool allowSelfSigned);

// 
// CREATE INVOICE
//

bool btcpayCreateInvoice(
  const String& btcpayUrl, const String& apiKey, const String& storeId,
  int amountSats,
  String &outInvoiceId, String &outLNURL,
  bool allowSelfSigned
) {
  WiFiClientSecure client;
  applyTls(client, allowSelfSigned);
  
  HTTPClient http;
  String url = btcpayUrl + "/api/v1/stores/" + storeId + "/invoices";
  
  if (!http.begin(client, url)) {
    Serial.println("BTCPay: connection failed");
    return false;
  }
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "token " + apiKey);
  http.setTimeout(10000);
  
  JsonDocument doc;
  double amountBtc = (double)amountSats / 100000000.0;
  doc["amount"] = serialized(String(amountBtc, 8));
  doc["currency"] = "BTC";
  
  JsonObject checkout = doc["checkout"].to<JsonObject>();
  JsonArray methods = checkout["paymentMethods"].to<JsonArray>();
  methods.add("BTC-LN");
  methods.add("BTC-LNURL");
  checkout["expirationMinutes"] = INVOICE_EXPIRY_MIN;
  checkout["monitoringMinutes"] = INVOICE_EXPIRY_MIN;
  
  String body;
  serializeJson(doc, body);
  
  int code = http.POST(body);
  
  if (code != 200) {
    String err = http.getString();
    Serial.println("BTCPay: invoice error " + String(code) + ": " + err);
    http.end();
    return false;
  }
  
  String response = http.getString();
  http.end();
  
  JsonDocument resp;
  if (deserializeJson(resp, response)) {
    Serial.println("BTCPay: JSON parse failed");
    return false;
  }
  
  outInvoiceId = resp["id"].as<String>();
  
  if (outInvoiceId.length() == 0) {
    Serial.println("BTCPay: no invoice ID in response");
    return false;
  }
  
  return btcpayGetLNURL(btcpayUrl, apiKey, storeId, outInvoiceId, outLNURL, allowSelfSigned);
}

//
// GET LNURL (preferred - short QR code)
//

bool btcpayGetLNURL(
  const String& btcpayUrl, const String& apiKey, const String& storeId,
  const String& invoiceId, String &outLNURL,
  bool allowSelfSigned
) {
  WiFiClientSecure client;
  applyTls(client, allowSelfSigned);
  
  HTTPClient http;
  String url = btcpayUrl + "/api/v1/stores/" + storeId 
               + "/invoices/" + invoiceId + "/payment-methods";
  
  if (!http.begin(client, url)) return false;
  
  http.addHeader("Authorization", "token " + apiKey);
  http.setTimeout(10000);
  
  int code = http.GET();
  if (code != 200) { http.end(); return false; }
  
  String response = http.getString();
  http.end();
  
  JsonDocument doc;
  if (deserializeJson(doc, response)) return false;
  
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject method : arr) {
    String pm = method["paymentMethodId"].as<String>();
    if (pm == "BTC-LNURL") {
      String paymentLink = method["paymentLink"].as<String>();
      if (paymentLink.length() > 0) {
        if (paymentLink.startsWith("lightning:")) {
          paymentLink = paymentLink.substring(10);
        }
        if (paymentLink.startsWith("LIGHTNING:")) {
          paymentLink = paymentLink.substring(10);
        }
        outLNURL = paymentLink;
        Serial.println("BTCPay: LNURL found (" + String(outLNURL.length()) + " chars)");
        return true;
      }
    }
  }
  
  Serial.println("BTCPay: no LNURL found");
  return false;
}

//
// CHECK INVOICE STATUS
//

String btcpayCheckInvoice(
  const String& btcpayUrl, const String& apiKey, const String& storeId, const String& invoiceId,
  bool allowSelfSigned
) {
  if (invoiceId.length() == 0) return "ERROR";

  WiFiClientSecure client;
  applyTls(client, allowSelfSigned);
  
  HTTPClient http;
  String url = btcpayUrl + "/api/v1/stores/" + storeId 
               + "/invoices/" + invoiceId;
  
  if (!http.begin(client, url)) return "ERROR";
  
  http.addHeader("Authorization", "token " + apiKey);
  http.setTimeout(3000);   // was 5000
  
  int code = http.GET();
  if (code != 200) { http.end(); return "ERROR"; }
  
  String response = http.getString();
  http.end();
  
  JsonDocument doc;
  if (deserializeJson(doc, response)) return "ERROR";
  
  return doc["status"].as<String>();
}

#endif