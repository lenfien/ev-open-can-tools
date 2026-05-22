#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct BleProbeStatus {
    bool enabled = true;
    bool advertising = false;
    char uuid[37] = "";
    uint32_t window_ms = 1000;
    uint32_t duration_ms = 1000;
    uint32_t interval_ms = 20;
    uint32_t cycle_remaining_ms = 0;
    uint32_t adv_remaining_ms = 0;
};

void BleProbeLoad(Preferences &prefs);
void BleProbeSave(Preferences &prefs);
void BleProbeSetup();
void BleProbeLoop();
bool BleProbeSetConfig(const String &uuid, uint32_t window_ms, uint32_t duration_ms, uint32_t interval_ms, bool enabled);
String BleProbeGenerateUuid();
BleProbeStatus BleProbeGetStatus();
bool BleProbeUuidValid(const String &uuid);
