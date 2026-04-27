#pragma once

#include "can_frame_types.h"
#include "drivers/can_driver.h"
#include <Arduino.h>
#include <vector>

class Preferences;

struct SpeedLimitToPercent {
    uint32_t speed_limit    = 0;
    uint32_t offset_percent = 0;
};

struct CanConf {
    uint32_t version = 1;

    uint32_t enable_inject    = false;
    uint32_t enable_fsd       = false;
    uint32_t enable_print     = true;

    uint32_t use_hw3_code                              = false;
    uint32_t enable_ban_shield                         = true;
    uint32_t enable_nag_suppress                       = false;
    uint32_t enable_summon_unlock                      = false;
    uint32_t enable_enhanced_autopilot_runtime         = false;
    uint32_t disable_camera                            = false;
    uint32_t enable_emergency_vehicle_detection_runtime = true;
    uint32_t enable_isa_speed_chime_suppress_runtime   = false;

    // speed profile
    uint32_t speed_profile_use_follow_distance = true; // true=follow distance stalk, false=web
    uint32_t speed_profile_from_web               = 2;    // 1(fastest)–5(slowest), used when source=web
    uint32_t enable_set_hw3_profile               = true;

    // speed offset
    uint32_t speed_offset_enable_override    = 0;
    uint32_t speed_offset_use_fix_or_dynamic = 0; // 0=fixed value, 1=auto table
    uint32_t speed_offset_fix_from_web       = 0; // 0–50, used when mode=fixed

    // auto offset table: speed_limit(km/h) → offset_percent(%), sorted ascending
    SpeedLimitToPercent speed_limit_auto_cfg[12] = {
        {0, 60}, {20, 60}, {30, 60}, {40, 50}, {50, 40}, {60, 33},
        {70, 12}, {80, 11}, {90, 10}, {100, 10}, {110, 10}, {120, 10}
    };
};

struct CanState {
    uint32_t follow_distance      = 1; // 1–5, from CAN 1016 data[5][7:5]

    uint32_t speed_profile_to_hw3 = 2; // 0–2, written to 1021 mux-0 data[6][2:1]
    uint32_t speed_profile_to_hw4 = 3; // 0–4, written to 1021 mux-2 data[7][6:4]

    uint32_t frame_cnt  = 0; // total frames received
    uint32_t frame_sent = 0; // total frames injected

    uint32_t ban_shield_cnt       = 0; // frames forwarded by ban shield (changed mux)
    uint32_t ban_shield_check_cnt = 0; // total ban shield comparisons

    uint32_t speed_limit_fused      = 0; // km/h, from CAN 921 data[1][4:0]*5
    uint32_t speed_limit_vision_only = 0; // km/h, from CAN 921 data[2][4:0]*5

    uint32_t speed_offset     = 0; // current applied offset value
    uint32_t gateway_autopilot = 0; // from CAN 2047 mux-2 data[5][4:2]
};

struct CanHandler {
public:
    // Main entry point — called for every received frame.
    // Returns true if the (possibly modified) frame should be re-injected onto the bus.
    bool
    Handle(CanFrame &frame, CanDriver &driver);

    // CAN hardware filter ID list passed to the driver on init.
    const uint32_t *
    GetFilterIDs() const { return filter_can_id_list.data(); }

    uint8_t
    GetFilterIDCnt() const { return filter_can_id_list.size(); }

    // Clamp value to [min, max].
    int
    Clamp(int value, int min, int max);

    // Linear re-map from [min,max] to [t_min,t_max].
    int
    ReRange(int value, int min, int max, int t_min, int t_max);

    // Look up the offset percent for the current speed_limit_fused using speed_limit_auto_cfg.
    // Returns the percent of the last table entry whose speed_limit <= current fused speed.
    uint8_t
    CalcCustomOffset() const;

    // Convert raw gateway autopilot value (0–4) to a human-readable string.
    const char *
    GetGTWAutopilotStr(uint8_t value);

    // Compute the Tesla CAN checksum byte for a frame (sum of id bytes + all non-checksum data bytes).
    uint8_t
    ComputeVehicleChecksum(const CanFrame &frame, uint8_t checksumByteIndex = 7);

    // Persist m_cnf to NVS via prefs.putBytes("can_cnf", ...).
    void
    SaveConf(Preferences &pref);

    // Load m_cnf from NVS; only applied if the stored version matches m_cnf.version.
    void
    LoadConf(Preferences &pref);

    // Print all m_cnf fields to Serial.
    void
    PrintCnf();

    // Print all m_state fields to Serial.
    void
    PrintState();

private:
    // CAN 921  — decode speed limits (fused + vision-only) from data[1]/data[2].
    void Handle921(CanFrame &frame);

    // CAN 1016 — decode follow distance from data[5][7:5].
    void Handle1016(CanFrame &frame);

    // CAN 2047 — update gateway_autopilot (mux 2) and run ban shield comparison.
    //            Returns true if the frame changed and should be forwarded.
    bool Handle2047(CanFrame &frame);

    // CAN 1021 — dispatch to the appropriate mux handler below.
    bool Handle1021(CanFrame &frame);

    // CAN 1021 mux 0 — set FSD enable bits (46, 60), HW3 speed profile (data[6][2:1]),
    //                   and emergency vehicle detection bit (59).
    bool Handle1021Mux0(CanFrame &frame);

    // CAN 1021 mux 1 — clear nag bit (19), set nag override (47); optionally clear camera bit (43).
    bool Handle1021Mux1(CanFrame &frame);

    // CAN 1021 mux 2 — set HW4 speed profile (data[7][6:4]) and write speed offset byte.
    bool Handle1021Mux2(CanFrame &frame);

public:
    std::vector<uint32_t> filter_can_id_list = {880, 921, 1016, 1021, 2047, 838, 601};

    // Reference frames for ban shield — mux index → expected clean frame content.
    // A mux frame is only forwarded when its content differs from the stored reference.
    CanFrame m_gtw_protector[10] = {           //0          8          16         24         32         40         48         56
        {.id = 0x7FF, .dlc = 8, {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000}}, // 0
        {.id = 0x7FF, .dlc = 8, {0b00000001,0b10100101,0b01001110,0b01000011,0b01000101,0b10100101,0b01010100,0b00000001}}, // 1
        {.id = 0x7FF, .dlc = 8, {0b00000010,0b01000111,0b01010110,0b01101101,0b10001110,0b01001101,0b11000110,0b00100111}}, // 2
        {.id = 0x7FF, .dlc = 8, {0b00000011,0b00110011,0b00001100,0b01001101,0b11100001,0b00100000,0b00101000,0b00110100}}, // 3
        {.id = 0x7FF, .dlc = 8, {0b00000100,0b01110111,0b11000001,0b10010011,0b01101000,0b10101010,0b10111100,0b00110000}}, // 4
        {.id = 0x7FF, .dlc = 8, {0b00000101,0b00010010,0b00100000,0b01100001,0b01000000,0b00000100,0b10000000,0b01000010}}, // 5
        {.id = 0x7FF, .dlc = 8, {0b00000110,0b01011000,0b10110101,0b01011011,0b00000111,0b10110000,0b11001100,0b11001000}}, // 6
        {.id = 0x7FF, .dlc = 8, {0b00000111,0b00100110,0b00000000,0b10000101,0b00100000,0b00000100,0b00100011,0b01110000}}, // 7
        {.id = 0x7FF, .dlc = 8, {0b00001000,0b00000000,0b01000010,0b00000010,0b10010000,0b01000010,0b00010100,0b00000000}}, // 8
        {.id = 0x7FF, .dlc = 8, {0b00001001,0b11101111,0b00000000,0b00100000,0b00000000,0b10000000,0b00000000,0b00000000}}  // 9
    };

    CanConf  m_cnf;
    CanState m_state;
};
