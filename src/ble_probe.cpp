#include "ble_probe.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <esp_system.h>

static constexpr const char *BLE_DEVICE_NAME = "TeslaWakeProbe";
static constexpr const char *DEFAULT_UUID = "f4a2b6a1-2c7e-4c79-9f1d-6f7d9a21c8b4";
static constexpr uint32_t MIN_WINDOW_SEC = 1;
static constexpr uint32_t MAX_WINDOW_SEC = 3600;
static constexpr uint32_t MIN_DURATION_SEC = 1;
static constexpr uint32_t MAX_DURATION_SEC = 3600;

static BleProbeStatus g_ble;
static bool g_ble_ready = false;
static uint32_t g_cycle_start_ms = 0;
static uint32_t g_adv_start_ms = 0;

static uint32_t clampSec(uint32_t v, uint32_t lo, uint32_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

bool BleProbeUuidValid(const String &uuid) {
    if (uuid.length() != 36) return false;
    for (int i = 0; i < 36; i++) {
        char c = uuid[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') return false;
            continue;
        }
        bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        if (!hex) return false;
    }
    return true;
}

String BleProbeGenerateUuid() {
    uint8_t b[16];
    for (uint8_t &v : b)
        v = (uint8_t)(esp_random() & 0xff);
    b[6] = (uint8_t)((b[6] & 0x0f) | 0x40);
    b[8] = (uint8_t)((b[8] & 0x3f) | 0x80);

    char out[37];
    snprintf(out, sizeof(out),
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
             b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
    return String(out);
}

static void stopAdvertising() {
    if (!g_ble_ready || !g_ble.advertising) return;
    BLEDevice::getAdvertising()->stop();
    g_ble.advertising = false;
    Serial.println("[BLE] advertising stopped");
}

static void startAdvertising() {
    if (!g_ble_ready || g_ble.advertising || !g_ble.enabled) return;

    BLEAdvertising *adv = BLEDevice::getAdvertising();
    adv->stop();
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);
    adv->setMaxPreferred(0x12);

    BLEAdvertisementData advData;
    advData.setFlags(0x06);
    advData.setCompleteServices(BLEUUID(g_ble.uuid));
    adv->setAdvertisementData(advData);

    BLEAdvertisementData scanData;
    scanData.setName(BLE_DEVICE_NAME);
    adv->setScanResponseData(scanData);
    adv->start();

    g_ble.advertising = true;
    g_adv_start_ms = millis();
    Serial.printf("[BLE] advertising UUID %s\n", g_ble.uuid);
}

void BleProbeLoad(Preferences &prefs) {
    String uuid = prefs.getString("ble_uuid", DEFAULT_UUID);
    if (!BleProbeUuidValid(uuid))
        uuid = DEFAULT_UUID;
    strlcpy(g_ble.uuid, uuid.c_str(), sizeof(g_ble.uuid));
    g_ble.window_sec = clampSec(prefs.getUInt("ble_win_s", 30), MIN_WINDOW_SEC, MAX_WINDOW_SEC);
    g_ble.duration_sec = clampSec(prefs.getUInt("ble_dur_s", 10), MIN_DURATION_SEC, MAX_DURATION_SEC);
    if (g_ble.duration_sec > g_ble.window_sec)
        g_ble.duration_sec = g_ble.window_sec;
    g_ble.enabled = prefs.getBool("ble_enabled", true);
}

void BleProbeSave(Preferences &prefs) {
    prefs.putString("ble_uuid", g_ble.uuid);
    prefs.putUInt("ble_win_s", g_ble.window_sec);
    prefs.putUInt("ble_dur_s", g_ble.duration_sec);
    prefs.putBool("ble_enabled", g_ble.enabled);
}

void BleProbeSetup() {
    BLEDevice::init(BLE_DEVICE_NAME);
    BLEDevice::setPower(ESP_PWR_LVL_P9);
    g_ble_ready = true;
    g_cycle_start_ms = millis();
    startAdvertising();
}

bool BleProbeSetConfig(const String &uuid, uint32_t window_sec, uint32_t duration_sec, bool enabled) {
    if (!BleProbeUuidValid(uuid)) return false;
    stopAdvertising();
    strlcpy(g_ble.uuid, uuid.c_str(), sizeof(g_ble.uuid));
    g_ble.window_sec = clampSec(window_sec, MIN_WINDOW_SEC, MAX_WINDOW_SEC);
    g_ble.duration_sec = clampSec(duration_sec, MIN_DURATION_SEC, MAX_DURATION_SEC);
    if (g_ble.duration_sec > g_ble.window_sec)
        g_ble.duration_sec = g_ble.window_sec;
    g_ble.enabled = enabled;
    g_cycle_start_ms = millis();
    g_adv_start_ms = 0;
    if (g_ble.enabled)
        startAdvertising();
    return true;
}

void BleProbeLoop() {
    if (!g_ble_ready) return;

    uint32_t now = millis();
    uint32_t window_ms = g_ble.window_sec * 1000u;
    uint32_t duration_ms = g_ble.duration_sec * 1000u;
    if (duration_ms > window_ms)
        duration_ms = window_ms;

    if (!g_ble.enabled) {
        stopAdvertising();
        g_ble.cycle_remaining_sec = 0;
        g_ble.adv_remaining_sec = 0;
        return;
    }

    if (now - g_cycle_start_ms >= window_ms) {
        g_cycle_start_ms = now;
        startAdvertising();
    }

    uint32_t cycle_elapsed = now - g_cycle_start_ms;
    if (g_ble.advertising && cycle_elapsed >= duration_ms)
        stopAdvertising();
    else if (!g_ble.advertising && cycle_elapsed < duration_ms)
        startAdvertising();

    cycle_elapsed = now - g_cycle_start_ms;
    uint32_t adv_elapsed = g_ble.advertising ? (now - g_adv_start_ms) : 0;
    g_ble.cycle_remaining_sec = (cycle_elapsed < window_ms) ? ((window_ms - cycle_elapsed + 999u) / 1000u) : 0;
    g_ble.adv_remaining_sec = (g_ble.advertising && adv_elapsed < duration_ms) ? ((duration_ms - adv_elapsed + 999u) / 1000u) : 0;
}

BleProbeStatus BleProbeGetStatus() {
    return g_ble;
}
