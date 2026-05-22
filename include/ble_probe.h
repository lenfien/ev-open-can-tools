#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct BleProbeStatus {
    bool enabled = true;
    bool advertising = false;
    bool use_saved_timing = false;
    char uuid[37] = "";
    uint32_t window_ms = 3000;
    uint32_t duration_ms = 600;
    uint32_t interval_ms = 200;
    uint32_t cycle_remaining_ms = 0;
    uint32_t adv_remaining_ms = 0;
};

void BleProbeLoad(Preferences &prefs);
void BleProbeSave(Preferences &prefs);
void BleProbeSetup();
void BleProbeLoop();
bool BleProbeSetConfig(const String &uuid, uint32_t window_ms, uint32_t duration_ms, uint32_t interval_ms, bool enabled, bool use_saved_timing);
String BleProbeGenerateUuid();
BleProbeStatus BleProbeGetStatus();
bool BleProbeUuidValid(const String &uuid);
