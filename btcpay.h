/*
 * PlugNSat - BTCPay Server API
 * Creates Lightning invoices and checks payment status.
 * API: https://docs.btcpayserver.org/API/Greenfield/v1/
 */

#ifndef BTCPAY_H
#define BTCPAY_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Forward declaration
bool btcpayGetBolt11(String url, String key, String store, String id, String &bolt11);

// ============================================================
// CREATE INVOICE
// ============================================================

bool btcpayCreateInvoice(
  String btcpayUrl, String apiKey, String storeId,
  int amountSats,
  String &outInvoiceId, String &outBolt11
) {
  WiFiClientSecure client;
  client.setInsecure();  // Skip cert verify (prototype)
  
  HTTPClient http;
  String url = btcpayUrl + "/api/v1/stores/" + storeId + "/invoices";
  
  if (!http.begin(client, url)) {
    Serial.println("BTCPay: connection failed");
    return false;
  }
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "token " + apiKey);
  http.setTimeout(10000);  // 10s timeout
  
  // Build invoice request
  JsonDocument doc;
  double amountBtc = (double)amountSats / 100000000.0;
  doc["amount"] = serialized(String(amountBtc, 8));  // Avoid float precision issues
  doc["currency"] = "BTC";
  
  JsonObject checkout = doc["checkout"].to<JsonObject>();
  JsonArray methods = checkout["paymentMethods"].to<JsonArray>();
  methods.add("BTC-LightningNetwork");
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
  
  // Parse invoice ID
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
  
  // Get BOLT11 from payment methods
  return btcpayGetBolt11(btcpayUrl, apiKey, storeId, outInvoiceId, outBolt11);
}

// ============================================================
// GET BOLT11
// ============================================================

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
  
  // Find Lightning method in array
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject method : arr) {
    String pm = method["paymentMethodId"].as<String>();
    // BTCPay uses different field names across versions
    if (pm.indexOf("Lightning") >= 0 || pm.indexOf("lightning") >= 0) {
      // Try "destination" first (newer API), then "paymentLink"
      if (method.containsKey("destination")) {
        outBolt11 = method["destination"].as<String>();
      } else if (method.containsKey("paymentLink")) {
        outBolt11 = method["paymentLink"].as<String>();
      }
      
      if (outBolt11.length() > 0) {
        // Remove "lightning:" prefix if present
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

// ============================================================
// CHECK INVOICE STATUS
// Returns: "New", "Processing", "Settled", "Expired", "Invalid", "ERROR"
// ============================================================

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
  http.setTimeout(5000);  // Short timeout for polling
  
  int code = http.GET();
  if (code != 200) { http.end(); return "ERROR"; }
  
  String response = http.getString();
  http.end();
  
  JsonDocument doc;
  if (deserializeJson(doc, response)) return "ERROR";
  
  return doc["status"].as<String>();
}

#endif
