#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <esp_ota_ops.h>
#include <mbedtls/pk.h>
#include <mbedtls/md.h>
#include <mbedtls/error.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <esp_crt_bundle.h>

#define OTA_ROLLBACK_TIMEOUT_MS 60000

extern const uint8_t x509_crt_imported_bundle_bin_start[] asm("_binary_x509_crt_bundle_start");
extern const uint8_t x509_crt_imported_bundle_bin_end[]   asm("_binary_x509_crt_bundle_end");


struct OtaUpdateInfo {
  bool available;
  String latestVersion;
  String error;
};

inline bool otaParseVersion(const String& v, int& major, int& minor, int& patch) {
  major = minor = patch = 0;
  int firstDot = v.indexOf('.');
  if (firstDot < 0) return false;
  int secondDot = v.indexOf('.', firstDot + 1);
  if (secondDot < 0) return false;
  major = v.substring(0, firstDot).toInt();
  minor = v.substring(firstDot + 1, secondDot).toInt();
  patch = v.substring(secondDot + 1).toInt();
  return true;
}

inline bool otaIsNewer(const String& remote, const String& current) {
  int rMaj, rMin, rPat, cMaj, cMin, cPat;
  if (!otaParseVersion(remote, rMaj, rMin, rPat)) return false;
  if (!otaParseVersion(current, cMaj, cMin, cPat)) return false;
  if (rMaj != cMaj) return rMaj > cMaj;
  if (rMin != cMin) return rMin > cMin;
  return rPat > cPat;
}

inline OtaUpdateInfo otaCheckUpdate() {
  OtaUpdateInfo result;
  result.available = false;

  WiFiClientSecure client;
  client.setCACertBundle(x509_crt_imported_bundle_bin_start,
                         x509_crt_imported_bundle_bin_end - x509_crt_imported_bundle_bin_start);
  client.setTimeout(10);

  HTTPClient http;
  if (!http.begin(client, "https://api.github.com/repos/ezmo-dev/plugnsat/releases/latest")) {
    result.error = "HTTP begin failed";
    return result;
  }

  http.addHeader("User-Agent", "PlugNSat/" FIRMWARE_VERSION);
  http.addHeader("Accept", "application/vnd.github+json");

  int code = http.GET();
  if (code != 200) {
    result.error = "HTTP " + String(code);
    http.end();
    return result;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    result.error = "JSON parse error";
    return result;
  }

  String tag = doc["tag_name"] | "";
  if (tag.length() == 0) {
    result.error = "No tag_name in response";
    return result;
  }

  if (tag.startsWith("v") || tag.startsWith("V")) {
    tag = tag.substring(1);
  }

  result.latestVersion = tag;
  // Compare: available only if remote version is strictly newer (semver)
  result.available = otaIsNewer(tag, String(FIRMWARE_VERSION));

  Serial.println("OTA: current=" + String(FIRMWARE_VERSION)
                 + " latest=" + tag
                 + " available=" + String(result.available));
  return result;
}

static const unsigned char OTA_PUBLIC_KEY_DER[] = {
  0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
  0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xb7, 0x46, 0x2d,
  0x2e, 0xbc, 0x8f, 0x12, 0x89, 0xc6, 0x0c, 0xd1, 0x64, 0x78, 0xd7, 0xb2,
  0xcb, 0x71, 0xa5, 0x96, 0xe2, 0x3d, 0x53, 0xc2, 0xb8, 0x6f, 0xba, 0x6d,
  0xf4, 0xe0, 0x26, 0x00, 0x31, 0x90, 0x98, 0xe8, 0xba, 0xaf, 0xe5, 0x7f,
  0x3f, 0x49, 0x9d, 0xd4, 0x9a, 0xa4, 0x98, 0xb8, 0x5c, 0xbf, 0xcb, 0x61,
  0xae, 0x52, 0x07, 0x82, 0xeb, 0xc8, 0x57, 0x7c, 0x52, 0xba, 0x38, 0x81,
  0x97, 0xa8, 0xa3, 0x5f, 0xaf, 0xcd, 0xe8, 0xcd, 0x92, 0x23, 0x8d, 0x79,
  0x00, 0x9f, 0xef, 0x81, 0x40, 0x51, 0xb8, 0x13, 0xe9, 0x7a, 0x96, 0x1d,
  0x8a, 0xd2, 0xed, 0xe0, 0x45, 0x66, 0x77, 0x9b, 0x0c, 0x78, 0xd1, 0xdd,
  0x0e, 0x88, 0x18, 0x08, 0xd1, 0xdb, 0x8c, 0xfe, 0x43, 0xbc, 0x41, 0x24,
  0xdd, 0xbf, 0x50, 0x41, 0x06, 0x99, 0x10, 0x9b, 0x46, 0x1a, 0xfd, 0x80,
  0xe1, 0xde, 0x1c, 0x70, 0xb3, 0x0b, 0x79, 0x70, 0x52, 0x1c, 0xe0, 0xac,
  0x36, 0x5c, 0x41, 0x05, 0xc3, 0xd4, 0xca, 0x4a, 0x55, 0x02, 0x2b, 0x71,
  0xcb, 0xf6, 0x4b, 0x56, 0x21, 0xe3, 0xda, 0x2b, 0x1e, 0x51, 0x27, 0x47,
  0xa8, 0x84, 0xbd, 0x31, 0x6d, 0xf7, 0xa1, 0x79, 0xd4, 0x61, 0x31, 0xb5,
  0xa6, 0x40, 0x6f, 0x1b, 0xf4, 0xfc, 0x6e, 0xd2, 0x7e, 0xa7, 0x87, 0x7f,
  0x85, 0xe6, 0x44, 0x62, 0x50, 0x0b, 0x62, 0x97, 0xc9, 0x2b, 0x5b, 0x78,
  0xcc, 0x1d, 0xee, 0xa9, 0xed, 0x1c, 0x42, 0x50, 0xc7, 0x17, 0x83, 0x0d,
  0xb3, 0xff, 0x66, 0x8d, 0x12, 0x8b, 0xeb, 0x00, 0xec, 0x01, 0x76, 0xb8,
  0xa0, 0xea, 0xf0, 0xcd, 0x2b, 0xd8, 0xac, 0x40, 0xaf, 0x2d, 0x56, 0x98,
  0x07, 0x9a, 0xfd, 0x60, 0x6f, 0x6c, 0xec, 0xf1, 0x75, 0xd6, 0x7c, 0x96,
  0xd0, 0xe4, 0x09, 0xe3, 0x57, 0xad, 0x7f, 0xc5, 0xd5, 0x47, 0xee, 0x74,
  0xbb, 0x02, 0x03, 0x01, 0x00, 0x01
};
static const size_t OTA_PUBLIC_KEY_DER_LEN = sizeof(OTA_PUBLIC_KEY_DER);

inline bool otaVerifySignature(const uint8_t* firmwareData, size_t firmwareLen,
                                const uint8_t* sigData, size_t sigLen) {
  mbedtls_pk_context pk;
  mbedtls_pk_init(&pk);

  int ret = mbedtls_pk_parse_public_key(&pk, OTA_PUBLIC_KEY_DER, OTA_PUBLIC_KEY_DER_LEN);
  if (ret != 0) {
    Serial.println("OTA: public key parse failed");
    mbedtls_pk_free(&pk);
    return false;
  }

  uint8_t hash[32];
  ret = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                   firmwareData, firmwareLen, hash);
  if (ret != 0) {
    Serial.println("OTA: SHA256 hash failed");
    mbedtls_pk_free(&pk);
    return false;
  }

  ret = mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, sizeof(hash), sigData, sigLen);
  mbedtls_pk_free(&pk);

  if (ret != 0) {
    char errbuf[64];
    mbedtls_strerror(ret, errbuf, sizeof(errbuf));
    Serial.println("OTA: signature invalid: " + String(errbuf));
    return false;
  }

  Serial.println("OTA: signature valid");
  return true;
}

inline bool otaVerifySignatureFromHash(const uint8_t* hash, size_t hashLen,
                                        const uint8_t* sigData, size_t sigLen) {
  mbedtls_pk_context pk;
  mbedtls_pk_init(&pk);

  int ret = mbedtls_pk_parse_public_key(&pk, OTA_PUBLIC_KEY_DER, OTA_PUBLIC_KEY_DER_LEN);
  if (ret != 0) {
    Serial.println("OTA: public key parse failed");
    mbedtls_pk_free(&pk);
    return false;
  }

  ret = mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, hashLen, sigData, sigLen);
  mbedtls_pk_free(&pk);

  if (ret != 0) {
    char errbuf[64];
    mbedtls_strerror(ret, errbuf, sizeof(errbuf));
    Serial.println("OTA: signature invalid: " + String(errbuf));
    return false;
  }

  Serial.println("OTA: signature valid");
  return true;
}

struct OtaFlashResult {
  bool success;
  String error;
};

inline OtaFlashResult otaDownloadAndFlash(const String& version) {
  OtaFlashResult result;
  result.success = false;

  String base = "https://github.com/ezmo-dev/plugnsat/releases/download/v"
                + version + "/";
  String binUrl = base + "firmware.bin";
  String sigUrl = base + "firmware.bin.sig";

  WiFiClientSecure client;
  client.setCACertBundle(x509_crt_imported_bundle_bin_start,
                         x509_crt_imported_bundle_bin_end - x509_crt_imported_bundle_bin_start);
  client.setTimeout(60);

  // --- Download .sig first (small, 256 bytes) ---
  HTTPClient http;
  Serial.println("OTA: downloading sig from " + sigUrl);
  if (!http.begin(client, sigUrl)) {
    result.error = "Sig HTTP begin failed";
    return result;
  }
  http.addHeader("User-Agent", "PlugNSat/" FIRMWARE_VERSION);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int sigCode = http.GET();
  if (sigCode != 200) {
    result.error = "Sig download failed: HTTP " + String(sigCode);
    http.end();
    return result;
  }

  static uint8_t sigBuf[256];
  size_t sigLen = 0;
  WiFiClient* sigStream = http.getStreamPtr();
  unsigned long sigTimeout = millis() + 5000;
  while (sigLen < sizeof(sigBuf) && millis() < sigTimeout) {
    if (sigStream->available()) {
      sigBuf[sigLen++] = sigStream->read();
    }
  }
  http.end();
  Serial.println("OTA: sig received, " + String(sigLen) + " bytes");

  if (sigLen == 0) {
    result.error = "Sig empty";
    return result;
  }

  // --- Download .bin and stream directly to flash partition ---
  Serial.println("OTA: downloading firmware from " + binUrl);
  WiFiClientSecure client2;
  client2.setCACertBundle(x509_crt_imported_bundle_bin_start,
                          x509_crt_imported_bundle_bin_end - x509_crt_imported_bundle_bin_start);
  client2.setTimeout(60);
  HTTPClient http2;

  if (!http2.begin(client2, binUrl)) {
    result.error = "Bin HTTP begin failed";
    return result;
  }
  http2.addHeader("User-Agent", "PlugNSat/" FIRMWARE_VERSION);
  http2.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int binCode = http2.GET();
  if (binCode != 200) {
    result.error = "Bin download failed: HTTP " + String(binCode);
    http2.end();
    return result;
  }

  int contentLen = http2.getSize();
  Serial.println("OTA: firmware size: " + String(contentLen) + " bytes");

  if (contentLen <= 0 || contentLen > 1900000) {
    result.error = "Invalid firmware size: " + String(contentLen);
    http2.end();
    return result;
  }

  // Begin OTA flash with the known size
  if (!Update.begin(contentLen)) {
    result.error = "Update.begin failed";
    http2.end();
    return result;
  }

  // Set up streaming SHA256
  mbedtls_md_context_t shaCtx;
  mbedtls_md_init(&shaCtx);
  mbedtls_md_setup(&shaCtx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
  mbedtls_md_starts(&shaCtx);

  WiFiClient* binStream = http2.getStreamPtr();
  uint8_t chunk[1024];
  int totalRead = 0;
  unsigned long binTimeout = millis() + 90000;

  while (totalRead < contentLen && millis() < binTimeout) {
    size_t avail = binStream->available();
    if (avail > 0) {
      size_t toRead = (avail > sizeof(chunk)) ? sizeof(chunk) : avail;
      if ((int)(totalRead + toRead) > contentLen) {
        toRead = contentLen - totalRead;
      }
      int r = binStream->readBytes(chunk, toRead);
      if (r > 0) {
        // Write to flash
        if (Update.write(chunk, r) != (size_t)r) {
          Update.abort();
          mbedtls_md_free(&shaCtx);
          http2.end();
          result.error = "Flash write error during stream";
          return result;
        }
        // Update hash
        mbedtls_md_update(&shaCtx, chunk, r);
        totalRead += r;
      }
    }
    yield();
  }
  http2.end();
  Serial.println("OTA: firmware streamed, " + String(totalRead) + " bytes");

  if (totalRead != contentLen) {
    Update.abort();
    mbedtls_md_free(&shaCtx);
    result.error = "Download incomplete: " + String(totalRead)
                   + "/" + String(contentLen);
    return result;
  }

  // Finalize hash
  uint8_t hash[32];
  mbedtls_md_finish(&shaCtx, hash);
  mbedtls_md_free(&shaCtx);

  // Verify signature against the streamed hash BEFORE committing the flash
  bool sigOk = otaVerifySignatureFromHash(hash, sizeof(hash), sigBuf, sigLen);
  if (!sigOk) {
    Update.abort();
    result.error = "Signature invalid - flash aborted";
    return result;
  }

  // Commit the flash
  if (!Update.end(true)) {
    result.error = "Update.end failed";
    return result;
  }

  Serial.println("OTA: flash complete");
  result.success = true;
  return result;
}

static bool _otaValid = false;
static bool _otaPendingVerify = false;
static unsigned long _otaBootTime = 0;

inline void otaInit() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  esp_ota_img_states_t state;
  if (esp_ota_get_state_partition(running, &state) == ESP_OK &&
      state == ESP_OTA_IMG_PENDING_VERIFY) {
    _otaPendingVerify = true;
    _otaBootTime = millis();
    Serial.println("OTA: post-update boot detected, rollback window open (60s)");
  }
}

inline void otaMarkValid() {
  _otaValid = true;
  esp_ota_mark_app_valid_cancel_rollback();
  Serial.println("OTA: firmware marked valid, rollback cancelled");
}

inline void otaTick() {
  if (!_otaPendingVerify || _otaValid) return;
  if (millis() - _otaBootTime >= OTA_ROLLBACK_TIMEOUT_MS) {
    Serial.println("OTA: validation timeout, rolling back");
    esp_ota_mark_app_invalid_rollback_and_reboot();
  }
}

#endif
