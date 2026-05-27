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
#include "blink.h"

typedef bool   (*CreateInvoiceFn)(PlugNSatConfig &config, int amountSats, String &outInvoiceId, String &outLNURL);
typedef String (*CheckInvoiceFn) (PlugNSatConfig &config, const String &invoiceId, const String &invoicePayload);

struct BackendInterface {
  CreateInvoiceFn createInvoice;
  CheckInvoiceFn  checkInvoice;
};

// Global instance - only include backend.h from plugnsat.ino.
// If included from multiple .cpp files, change to extern + definition in .ino.
BackendInterface backend;

// --- BTCPay wrappers ---

bool btcpayCreateInvoiceWrapper(PlugNSatConfig &config, int amountSats, String &outInvoiceId, String &outLNURL) {
  return btcpayCreateInvoice(config.btcpayUrl, config.btcpayApiKey, config.btcpayStoreId, amountSats, outInvoiceId, outLNURL);
}

String btcpayCheckInvoiceWrapper(PlugNSatConfig &config, const String &invoiceId, const String &invoicePayload) {
  return btcpayCheckInvoice(config.btcpayUrl, config.btcpayApiKey, config.btcpayStoreId, invoiceId);
}

// --- Blink wrappers ---

bool blinkCreateInvoiceWrapper(PlugNSatConfig &config, int amountSats, String &outInvoiceId, String &outLNURL) {
  return blinkCreateInvoice(config.blinkApiKey, config.blinkWalletId, amountSats, config.deviceName, outInvoiceId, outLNURL);
}

String blinkCheckInvoiceWrapper(PlugNSatConfig &config, const String &invoiceId, const String &invoicePayload) {
  // Blink uses the paymentRequest (in invoicePayload / currentQRData) for status checks
  return blinkCheckInvoice(config.blinkApiKey, invoicePayload);
}

// --- Init ---

void initBackend(BackendType type) {
  switch (type) {
    case BACKEND_BLINK:
      backend.createInvoice = blinkCreateInvoiceWrapper;
      backend.checkInvoice  = blinkCheckInvoiceWrapper;
      break;
    case BACKEND_BTCPAY:
    default:
      backend.createInvoice = btcpayCreateInvoiceWrapper;
      backend.checkInvoice  = btcpayCheckInvoiceWrapper;
      break;
  }
}

#endif
