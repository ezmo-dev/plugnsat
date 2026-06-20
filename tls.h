#ifndef TLS_H
#define TLS_H

#include <WiFiClientSecure.h>
#include <esp_crt_bundle.h>

extern const uint8_t x509_crt_imported_bundle_bin_start[] asm("_binary_x509_crt_bundle_start");
extern const uint8_t x509_crt_imported_bundle_bin_end[]   asm("_binary_x509_crt_bundle_end");

inline void applyTls(WiFiClientSecure& client, bool allowSelfSigned) {
  if (allowSelfSigned) {
    client.setInsecure();
  } else {
    client.setCACertBundle(x509_crt_imported_bundle_bin_start,
                           x509_crt_imported_bundle_bin_end - x509_crt_imported_bundle_bin_start);
  }
}

#endif
