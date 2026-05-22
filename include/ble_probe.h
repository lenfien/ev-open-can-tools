#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct BleProbeStatus {
    bool enabled = true;
    bool advertising = false;
    char uuid[37] = "";
    uint32_t window_sec = 30;
    uint32_t duration_sec = 10;
    uint32_t interval_ms = 500;
    uint32_t cycle_remaining_sec = 0;
    uint32_t adv_remaining_sec = 0;
};

void BleProbeLoad(Preferences &prefs);
void BleProbeSave(Preferences &prefs);
void BleProbeSetup();
void BleProbeLoop();
bool BleProbeSetConfig(const String &uuid, uint32_t window_sec, uint32_t duration_sec, uint32_t interval_ms, bool enabled);
String BleProbeGenerateUuid();
BleProbeStatus BleProbeGetStatus();
bool BleProbeUuidValid(const String &uuid);
