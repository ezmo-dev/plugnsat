#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <esp_ota_ops.h>

#define OTA_ROLLBACK_TIMEOUT_MS 60000

static bool _otaValid = false;
static bool _otaPendingVerify = false;
static unsigned long _otaBootTime = 0;

inline void otaInit() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  esp_ota_img_states_t state;
  if (esp_ota_get_state_info(running, &state) == ESP_OK &&
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
