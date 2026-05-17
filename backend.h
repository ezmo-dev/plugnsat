/*
 * PlugNSat - Backend Abstraction Layer
 *
 * Defines a common interface for Lightning backends (BTCPay, Blink, etc.)
 * so new backends can be added without touching plugnsat.ino or the state machine.
 *
 * License: MIT © 2026
 * Author: ezmo-dev (PlugNSat)
 */

#ifndef BACKEND_H
#define BACKEND_H

#include "config.h"
#include "btcpay.h"

typedef bool   (*CreateInvoiceFn)(PlugNSatConfig &config, int amountSats, String &outInvoiceId, String &outLNURL);
typedef String (*CheckInvoiceFn) (PlugNSatConfig &config, const String &invoiceId);

struct BackendInterface {
  CreateInvoiceFn createInvoice;
  CheckInvoiceFn  checkInvoice;
};

BackendInterface backend;

// --- BTCPay wrappers ---

bool btcpayCreateInvoiceWrapper(PlugNSatConfig &config, int amountSats, String &outInvoiceId, String &outLNURL) {
  return btcpayCreateInvoice(config.btcpayUrl, config.btcpayApiKey, config.btcpayStoreId, amountSats, outInvoiceId, outLNURL);
}

String btcpayCheckInvoiceWrapper(PlugNSatConfig &config, const String &invoiceId) {
  return btcpayCheckInvoice(config.btcpayUrl, config.btcpayApiKey, config.btcpayStoreId, invoiceId);
}

// --- Blink stubs (not implemented yet) ---

bool blinkCreateInvoicePlaceholder(PlugNSatConfig &config, int amountSats, String &outInvoiceId, String &outLNURL) {
  Serial.println("Blink: not implemented yet");
  return false;
}

String blinkCheckInvoicePlaceholder(PlugNSatConfig &config, const String &invoiceId) {
  Serial.println("Blink: not implemented yet");
  return "ERROR";
}

// --- Init ---

void initBackend(BackendType type) {
  switch (type) {
    case BACKEND_BLINK:
      backend.createInvoice = blinkCreateInvoicePlaceholder;
      backend.checkInvoice  = blinkCheckInvoicePlaceholder;
      break;
    case BACKEND_BTCPAY:
    default:
      backend.createInvoice = btcpayCreateInvoiceWrapper;
      backend.checkInvoice  = btcpayCheckInvoiceWrapper;
      break;
  }
}

#endif
