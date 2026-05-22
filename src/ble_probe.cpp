#include "ble_probe.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <esp_system.h>

static constexpr const char *BLE_DEVICE_NAME = "TeslaWakeProbe";
static constexpr const char *DEFAULT_UUID = "f4a2b6a1-2c7e-4c79-9f1d-6f7d9a21c8b4";
static constexpr uint32_t MIN_WINDOW_MS = 100;
static constexpr uint32_t MAX_WINDOW_MS = 3600000;
static constexpr uint32_t MIN_DURATION_MS = 100;
static constexpr uint32_t MAX_DURATION_MS = 3600000;
static constexpr uint32_t MIN_INTERVAL_MS = 20;
static constexpr uint32_t MAX_INTERVAL_MS = 5000;
static constexpr uint32_t DEFAULT_WINDOW_MS = 3000;
static constexpr uint32_t DEFAULT_DURATION_MS = 600;
static constexpr uint32_t DEFAULT_INTERVAL_MS = 200;
static constexpr uint32_t LEGACY_DEFAULT_WINDOW_MS = 1000;
static constexpr uint32_t LEGACY_DEFAULT_DURATION_MS = 1000;
static constexpr uint32_t LEGACY_DEFAULT_INTERVAL_MS = 20;

static BleProbeStatus g_ble;
static bool g_ble_ready = false;
static uint32_t g_adv_start_ms = 0;
static uint32_t g_next_adv_ms = 0;
static uint32_t g_adv_stop_ms = 0;
static uint32_t g_adv_seq = 0;

static uint32_t clampMs(uint32_t v, uint32_t lo, uint32_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static uint16_t intervalMsToBleUnits(uint32_t interval_ms) {
    uint32_t units = (interval_ms * 1000u + 624u) / 625u;
    if (units < 0x20) units = 0x20;
    if (units > 0x4000) units = 0x4000;
    return (uint16_t)units;
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
}

static void startAdvertising() {
    if (!g_ble_ready || g_ble.advertising || !g_ble.enabled) return;

    BLEAdvertising *adv = BLEDevice::getAdvertising();
    adv->stop();
    adv->setScanResponse(true);
    uint16_t interval = intervalMsToBleUnits(g_ble.interval_ms);
    adv->setMinInterval(interval);
    adv->setMaxInterval(interval);
    adv->setMinPreferred(0x06);
    adv->setMaxPreferred(0x12);

    uint32_t next_seq = g_adv_seq + 1;
    BLEAdvertisementData advData;
    advData.setFlags(0x06);
    advData.setCompleteServices(BLEUUID(g_ble.uuid));
    char mfg[4] = {
        (char)0xff,
        (char)0xff,
        (char)(next_seq & 0xff),
        (char)((next_seq >> 8) & 0xff),
    };
    advData.setManufacturerData(std::string(mfg, sizeof(mfg)));
    adv->setAdvertisementData(advData);

    BLEAdvertisementData scanData;
    scanData.setName(BLE_DEVICE_NAME);
    adv->setScanResponseData(scanData);

    adv->start();

    g_ble.advertising = true;
    g_adv_start_ms = millis();
    g_adv_stop_ms = g_adv_start_ms + g_ble.duration_ms;
    g_adv_seq = next_seq;
    Serial.printf("[BLE] advertising trigger #%lu period=%lums duration=%lums interval=%lums saved=%u uuid=%s\n",
                  (unsigned long)g_adv_seq,
                  (unsigned long)g_ble.window_ms,
                  (unsigned long)g_ble.duration_ms,
                  (unsigned long)g_ble.interval_ms,
                  g_ble.use_saved_timing ? 1 : 0,
                  g_ble.uuid);
}

void BleProbeLoad(Preferences &prefs) {
    String uuid = prefs.getString("ble_uuid", DEFAULT_UUID);
    if (!BleProbeUuidValid(uuid))
        uuid = DEFAULT_UUID;
    strlcpy(g_ble.uuid, uuid.c_str(), sizeof(g_ble.uuid));
    uint32_t defaultWindowMs = DEFAULT_WINDOW_MS;
    uint32_t defaultDurationMs = DEFAULT_DURATION_MS;
    if (!prefs.isKey("ble_win_ms") && prefs.isKey("ble_win_s"))
        defaultWindowMs = prefs.getUInt("ble_win_s", 30) * 1000u;
    if (!prefs.isKey("ble_dur_ms") && prefs.isKey("ble_dur_s"))
        defaultDurationMs = prefs.getUInt("ble_dur_s", 10) * 1000u;

    g_ble.use_saved_timing = prefs.getBool("ble_use_timing", false);
    g_ble.window_ms = g_ble.use_saved_timing ? clampMs(prefs.getUInt("ble_win_ms", defaultWindowMs), MIN_WINDOW_MS, MAX_WINDOW_MS)
                                             : DEFAULT_WINDOW_MS;
    g_ble.duration_ms = g_ble.use_saved_timing ? clampMs(prefs.getUInt("ble_dur_ms", defaultDurationMs), MIN_DURATION_MS, MAX_DURATION_MS)
                                               : DEFAULT_DURATION_MS;
    if (g_ble.duration_ms > g_ble.window_ms)
        g_ble.duration_ms = g_ble.window_ms;
    g_ble.interval_ms = g_ble.use_saved_timing ? clampMs(prefs.getUInt("ble_int_ms", DEFAULT_INTERVAL_MS), MIN_INTERVAL_MS, MAX_INTERVAL_MS)
                                               : DEFAULT_INTERVAL_MS;
    g_ble.enabled = prefs.getBool("ble_enabled", true);

    if (g_ble.use_saved_timing &&
        g_ble.window_ms == LEGACY_DEFAULT_WINDOW_MS &&
        g_ble.duration_ms == LEGACY_DEFAULT_DURATION_MS &&
        g_ble.interval_ms == LEGACY_DEFAULT_INTERVAL_MS) {
        g_ble.window_ms = DEFAULT_WINDOW_MS;
        g_ble.duration_ms = DEFAULT_DURATION_MS;
        g_ble.interval_ms = DEFAULT_INTERVAL_MS;
        prefs.putUInt("ble_win_ms", g_ble.window_ms);
        prefs.putUInt("ble_dur_ms", g_ble.duration_ms);
        prefs.putUInt("ble_int_ms", g_ble.interval_ms);
        Serial.println("[BLE] migrated aggressive default advertising interval");
    }
}

void BleProbeSave(Preferences &prefs) {
    prefs.putString("ble_uuid", g_ble.uuid);
    prefs.putUInt("ble_win_ms", g_ble.window_ms);
    prefs.putUInt("ble_dur_ms", g_ble.duration_ms);
    prefs.putUInt("ble_int_ms", g_ble.interval_ms);
    prefs.remove("ble_win_s");
    prefs.remove("ble_dur_s");
    prefs.putBool("ble_enabled", g_ble.enabled);
    prefs.putBool("ble_use_timing", g_ble.use_saved_timing);
}

void BleProbeSetup() {
    BLEDevice::init(BLE_DEVICE_NAME);
    BLEDevice::setPower(ESP_PWR_LVL_P6);
    g_ble_ready = true;
    g_next_adv_ms = millis();
    startAdvertising();
}

bool BleProbeSetConfig(const String &uuid, uint32_t window_ms, uint32_t duration_ms, uint32_t interval_ms, bool enabled, bool use_saved_timing) {
    if (!BleProbeUuidValid(uuid)) return false;
    stopAdvertising();
    strlcpy(g_ble.uuid, uuid.c_str(), sizeof(g_ble.uuid));
    g_ble.window_ms = clampMs(window_ms, MIN_WINDOW_MS, MAX_WINDOW_MS);
    g_ble.duration_ms = clampMs(duration_ms, MIN_DURATION_MS, MAX_DURATION_MS);
    if (g_ble.duration_ms > g_ble.window_ms)
        g_ble.duration_ms = g_ble.window_ms;
    g_ble.interval_ms = clampMs(interval_ms, MIN_INTERVAL_MS, MAX_INTERVAL_MS);
    g_ble.enabled = enabled;
    g_ble.use_saved_timing = use_saved_timing;
    g_next_adv_ms = millis();
    g_adv_start_ms = 0;
    g_adv_stop_ms = 0;
    if (g_ble.enabled)
        startAdvertising();
    return true;
}

void BleProbeLoop() {
    if (!g_ble_ready) return;

    uint32_t now = millis();
    uint32_t window_ms = g_ble.window_ms;
    uint32_t duration_ms = g_ble.duration_ms;
    if (duration_ms > window_ms)
        duration_ms = window_ms;

    if (!g_ble.enabled) {
        stopAdvertising();
        g_ble.cycle_remaining_ms = 0;
        g_ble.adv_remaining_ms = 0;
        g_next_adv_ms = millis();
        return;
    }

    if (g_ble.advertising && (int32_t)(now - g_adv_stop_ms) >= 0) {
        stopAdvertising();
        g_next_adv_ms = g_adv_start_ms + window_ms;
    }

    if (!g_ble.advertising && (int32_t)(now - g_next_adv_ms) >= 0)
        startAdvertising();

    uint32_t adv_elapsed = g_ble.advertising ? (now - g_adv_start_ms) : 0;
    uint32_t until_next = g_ble.advertising ? (window_ms - min(window_ms, adv_elapsed))
                                            : ((int32_t)(g_next_adv_ms - now) > 0 ? (g_next_adv_ms - now) : 0);
    g_ble.cycle_remaining_ms = until_next;
    g_ble.adv_remaining_ms = (g_ble.advertising && adv_elapsed < duration_ms) ? (duration_ms - adv_elapsed) : 0;

}

BleProbeStatus BleProbeGetStatus() {
    return g_ble;
}
