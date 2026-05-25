/*
 * PlugNSat - Blink Wallet API
 *
 * API: https://dev.blink.sv/
 *
 * WORKFLOW:
 * Creates Lightning invoices and checks payment status via GraphQL.
 * API key configured on web portal during setup.
 *
 * DIFFERENCES FROM BTCPAY:
 * - GraphQL API (not REST)
 * - Fixed endpoint: https://api.blink.sv/graphql
 * - Auth via X-API-KEY header (not token-based)
 * - Returns BOLT11 paymentRequest (~257 chars) not LNURL (~115 chars)
 * - QR code is denser but scannable at ~20-30cm on T-Display S3
 * - Status check uses paymentRequest (not invoice ID)
 * - Status values: PENDING, PAID, EXPIRED (mapped for state machine compat)
 *
 * LIBRARIES:
 * - ArduinoJson by Benoit Blanchon (JSON parsing)
 * - WiFiClientSecure (HTTPS requests)
 *
 * License: MIT © 2026
 * Author: ezmo-dev (PlugNSat)
 */

#ifndef BLINK_H
#define BLINK_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

//
// CREATE INVOICE
//
// Calls lnInvoiceCreate mutation.
// outInvoiceId receives the paymentHash (short, for logging).
// outLNURL receives the paymentRequest (BOLT11, for QR display + status checks).
//

bool blinkCreateInvoice(
  String apiKey, String walletId,
  int amountSats,
  String &outInvoiceId, String &outLNURL
) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, BLINK_GRAPHQL_URL)) {
    Serial.println("Blink: connection failed");
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-KEY", apiKey);
  http.setTimeout(10000);

  JsonDocument doc;
  doc["query"] = "mutation LnInvoiceCreate($input: LnInvoiceCreateInput!) {"
                 "  lnInvoiceCreate(input: $input) {"
                 "    invoice { paymentRequest paymentHash satoshis }"
                 "    errors { message }"
                 "  }"
                 "}";
  JsonObject variables = doc["variables"].to<JsonObject>();
  JsonObject input = variables["input"].to<JsonObject>();
  input["walletId"] = walletId;
  input["amount"] = amountSats;

  String body;
  serializeJson(doc, body);

  int code = http.POST(body);

  if (code != 200) {
    String err = http.getString();
    Serial.println("Blink: invoice error " + String(code) + ": " + err);
    http.end();
    return false;
  }

  String response = http.getString();
  http.end();

  JsonDocument resp;
  if (deserializeJson(resp, response)) {
    Serial.println("Blink: JSON parse failed");
    return false;
  }

  JsonArray errors = resp["data"]["lnInvoiceCreate"]["errors"].as<JsonArray>();
  if (errors && errors.size() > 0) {
    String errMsg = errors[0]["message"].as<String>();
    Serial.println("Blink: API error: " + errMsg);
    return false;
  }

  String paymentRequest = resp["data"]["lnInvoiceCreate"]["invoice"]["paymentRequest"].as<String>();
  String paymentHash    = resp["data"]["lnInvoiceCreate"]["invoice"]["paymentHash"].as<String>();

  if (paymentRequest.length() == 0) {
    Serial.println("Blink: no paymentRequest in response");
    return false;
  }

  outInvoiceId = paymentHash;
  outLNURL     = paymentRequest;

  Serial.println("Blink: invoice created (" + String(paymentRequest.length()) + " chars)");
  return true;
}

//
// CHECK INVOICE STATUS
//
// Uses lnInvoicePaymentStatus query with the paymentRequest (BOLT11).
// The paymentRequest comes via the invoicePayload parameter (= currentQRData).
//
// Maps Blink status to BTCPay-compatible values for the state machine:
//   "PAID"    -> "Settled"   (triggers payment flow)
//   "PENDING" -> "New"       (keep polling)
//   "EXPIRED" -> "Expired"   (triggers QR refresh)
//

String blinkCheckInvoice(
  String apiKey, const String &paymentRequest
) {
  if (paymentRequest.length() == 0) return "ERROR";

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, BLINK_GRAPHQL_URL)) return "ERROR";

  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-KEY", apiKey);
  http.setTimeout(3000);

  JsonDocument doc;
  doc["query"] = "query LnInvoicePaymentStatus($input: LnInvoicePaymentStatusInput!) {"
                 "  lnInvoicePaymentStatus(input: $input) {"
                 "    status"
                 "  }"
                 "}";
  JsonObject variables = doc["variables"].to<JsonObject>();
  JsonObject input = variables["input"].to<JsonObject>();
  input["paymentRequest"] = paymentRequest;

  String body;
  serializeJson(doc, body);

  int code = http.POST(body);
  if (code != 200) { http.end(); return "ERROR"; }

  String response = http.getString();
  http.end();

  JsonDocument resp;
  if (deserializeJson(resp, response)) return "ERROR";

  String status = resp["data"]["lnInvoicePaymentStatus"]["status"].as<String>();

  if (status == "PAID")    return "Settled";
  if (status == "PENDING") return "New";
  if (status == "EXPIRED") return "Expired";

  Serial.println("Blink: unknown status: " + status);
  return "ERROR";
}

#endif
