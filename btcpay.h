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

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Forward declarations
bool btcpayGetBolt11(String url, String key, String store, String id, String &bolt11);
bool btcpayGetLNURL(String url, String key, String store, String id, String &lnurl);

// 
// CREATE INVOICE
//

bool btcpayCreateInvoice(
  String btcpayUrl, String apiKey, String storeId,
  int amountSats,
  String &outInvoiceId, String &outBolt11
) {
  WiFiClientSecure client;
  client.setInsecure();
  
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
  
  // Get LNURL from payment methods (short QR, scannable on LCD)
  return btcpayGetLNURL(btcpayUrl, apiKey, storeId, outInvoiceId, outBolt11);
}

//
// GET LNURL (preferred - short QR code)
//

bool btcpayGetLNURL(
  String btcpayUrl, String apiKey, String storeId,
  String invoiceId, String &outLNURL
) {
  WiFiClientSecure client;
  client.setInsecure();
  
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
  
  // Fallback: try BOLT11
  Serial.println("BTCPay: no LNURL found, trying BOLT11");
  return btcpayGetBolt11(btcpayUrl, apiKey, storeId, invoiceId, outLNURL);
}

//
// GET BOLT11 (fallback, or for OLED screens later)
//

bool btcpayGetBolt11(
  String btcpayUrl, String apiKey, String storeId,
  String invoiceId, String &outBolt11
) {
  WiFiClientSecure client;
  client.setInsecure();
  
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
    if (pm.indexOf("Lightning") >= 0 || pm.indexOf("LN") >= 0) {
      if (pm == "BTC-LNURL") continue;  // Skip LNURL, we want BOLT11
      
      if (method.containsKey("destination")) {
        outBolt11 = method["destination"].as<String>();
      } else if (method.containsKey("paymentLink")) {
        outBolt11 = method["paymentLink"].as<String>();
      }
      
      if (outBolt11.length() > 0) {
        if (outBolt11.startsWith("lightning:")) {
          outBolt11 = outBolt11.substring(10);
        }
        if (outBolt11.startsWith("LIGHTNING:")) {
          outBolt11 = outBolt11.substring(10);
        }
        return true;
      }
    }
  }
  
  Serial.println("BTCPay: no Lightning method found in invoice");
  return false;
}

// 
// CHECK INVOICE STATUS
//

String btcpayCheckInvoice(
  String btcpayUrl, String apiKey, String storeId, String invoiceId
) {
  if (invoiceId.length() == 0) return "ERROR";
  
  WiFiClientSecure client;
  client.setInsecure();
  
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