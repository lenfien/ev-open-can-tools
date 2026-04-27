//
// Created by hong- on 2026/4/26.
//

#include "handlers.h"
#include <Preferences.h>

// ── CAN 921 — speed limits ────────────────────────────────────────

void CanHandler::
Handle921(CanFrame &frame) {
    if (frame.dlc < 8) return;
    m_state.speed_limit_fused       = (frame.data[1] & 0x1F) * 5;
    m_state.speed_limit_vision_only = (frame.data[2] & 0x1F) * 5;
}

// ── CAN 1016 — follow distance ────────────────────────────────────

void CanHandler::
Handle1016(CanFrame &frame) {
    if (frame.dlc < 8) return;
    m_state.follow_distance = (frame.data[5] & 0b11100000) >> 5;
}

// ── CAN 2047 — gateway autopilot + ban shield ─────────────────────

bool CanHandler::
Handle2047(CanFrame &frame) {
    if (frame.dlc < 8) return false;

    int32_t mux = frame.data[0] & 0x0F;
    if (mux == 2)
        m_state.gateway_autopilot = static_cast<uint8_t>((frame.data[5] >> 2) & 0x07);

    if (!m_cnf.enable_ban_shield)
        return false;

    if (mux >= (int32_t)(sizeof(m_gtw_protector) / sizeof(m_gtw_protector[0])))
        return false;

    auto &saved = m_gtw_protector[mux];
    bool is_same = true;
    for (int i = 0; i < 8; i++) {
        if (saved.data[i] != frame.data[i]) {
            is_same = false;
            break;
        }
    }

    m_state.ban_shield_check_cnt++;
    if (is_same) return false;

    m_state.ban_shield_cnt++;
    return true;
}

// ── CAN 1021 mux 0 — FSD enable + HW3 speed profile ──────────────

bool CanHandler::
Handle1021Mux0(CanFrame &frame) {
    if (m_cnf.enable_fsd) {
        frame.SetBit(46, true);
        if (!m_cnf.use_hw3_code)
            frame.SetBit(60, true);

        if (m_cnf.speed_profile_use_follow_distance) {
            switch (m_state.follow_distance) {
                case 1:  m_state.speed_profile_to_hw3 = 2; break;
                case 2:  m_state.speed_profile_to_hw3 = 1; break;
                default: m_state.speed_profile_to_hw3 = 0; break;
            }
        }
        else {
            switch (m_cnf.speed_profile_from_web) {
            case 5:
            case 4:
                m_state.speed_profile_to_hw3 = 2;
                break;
            case 3:
                m_state.speed_profile_to_hw3 = 1;
                break;
            default:
                m_state.speed_profile_to_hw3 = 0;
                break;
            }
        }

        if (m_cnf.enable_set_hw3_profile) {
            frame.data[6] &= ~0x06;
            frame.data[6] |= (m_state.speed_profile_to_hw3 << 1);
        }
    }

    if (m_cnf.enable_emergency_vehicle_detection_runtime)
        frame.SetBit(59, true);

    return true;
}

// ── CAN 1021 mux 1 — nag suppress + camera disable ───────────────

bool CanHandler::
Handle1021Mux1(CanFrame &frame) {
    bool should_send = false;

    if (m_cnf.enable_nag_suppress) {
        frame.SetBit(19, false);
        frame.SetBit(47, true);
        should_send = true;
    }

    if (m_cnf.disable_camera) {
        frame.SetBit(43, false);
        should_send = true;
    }

    return should_send;
}

// ── CAN 1021 mux 2 — HW4 speed profile + speed offset ────────────

bool CanHandler::
Handle1021Mux2(CanFrame &frame) {
    if (!m_cnf.enable_fsd)
        return true;

    if (m_cnf.speed_profile_use_follow_distance) {
        switch (m_state.follow_distance) {
            case 1:  m_state.speed_profile_to_hw4 = 3; break;
            case 2:  m_state.speed_profile_to_hw4 = 2; break;
            case 3:  m_state.speed_profile_to_hw4 = 1; break;
            case 4:  m_state.speed_profile_to_hw4 = 0; break;
            default: m_state.speed_profile_to_hw4 = 4; break;
        }
    }
    else {
        switch (m_cnf.speed_profile_from_web) {
            case 5:  m_state.speed_profile_to_hw4 = 3; break;
            case 4:  m_state.speed_profile_to_hw4 = 2; break;
            case 3:  m_state.speed_profile_to_hw4 = 1; break;
            case 2:  m_state.speed_profile_to_hw4 = 0; break;
            default: m_state.speed_profile_to_hw4 = 4; break;
        }
    }

    frame.data[7] &= ~(0x07 << 4);
    frame.data[7] |= (m_state.speed_profile_to_hw4 & 0x07) << 4;

    if (m_cnf.speed_offset_enable_override) {
        m_state.speed_offset = m_cnf.speed_offset_fix_from_web;
        if (m_cnf.speed_offset_use_fix_or_dynamic)
            m_state.speed_offset = (uint8_t)CalcCustomOffset();

        if (m_state.speed_offset > 0) {
            if (!m_cnf.use_hw3_code) {
                frame.data[1] = (frame.data[1] & 0xC0) | (m_state.speed_offset & 0x3F);
            } else {
                m_state.speed_offset = ReRange(Clamp(m_state.speed_offset, 0, 60), 0, 60, 0, 240);
                frame.data[1] &= ~(0b00111111);
                frame.data[1] |= (m_state.speed_offset >> 2);
            }
        }
    }

    return true;
}

// ── CAN 1021 — dispatch by mux ────────────────────────────────────

bool CanHandler::
Handle1021(CanFrame &frame) {
    if (frame.dlc < 8) return false;

    switch (frame.GetMux()) {
        case 0:  return Handle1021Mux0(frame);
        case 1:  return Handle1021Mux1(frame);
        case 2:  return Handle1021Mux2(frame);
        default: return false;
    }
}

// ── Main dispatch ─────────────────────────────────────────────────

bool CanHandler::
Handle(CanFrame &frame, CanDriver &driver) {
    bool should_send = false;

    switch (frame.id) {
        case 921:  Handle921(frame);                 break;
        case 1016: Handle1016(frame);                break;
        case 2047: should_send = Handle2047(frame);  break;
        case 1021: should_send = Handle1021(frame);  break;
    }

    if (m_cnf.enable_print) {
        static unsigned long lastPrintMs = 0;
        unsigned long now = millis();
        if (now - lastPrintMs >= 1000) {
            lastPrintMs = now;
            PrintCnf();
            PrintState();
        }
    }

    return should_send;
}

// ── Helpers ───────────────────────────────────────────────────────

int CanHandler::
Clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int CanHandler::
ReRange(int value, int min, int max, int t_min, int t_max) {
    int t_len = t_max - t_min;
    int len   = max - min;
    int p     = (value - min) * 100 / len;
    return t_min + p * t_len / 100;
}

uint8_t CanHandler::
CalcCustomOffset() const {
    uint8_t result = 0;
    for (auto &one : m_cnf.speed_limit_auto_cfg) {
        if (one.speed_limit == 0 && one.offset_percent == 0) continue;
        if (m_state.speed_limit_fused >= one.speed_limit)
            result = one.offset_percent;
    }
    return result;
}

const char * CanHandler::
GetGTWAutopilotStr(uint8_t value) {
    switch (value) {
        case 0: return "NONE";
        case 1: return "HIGHWAY";
        case 2: return "ENHANCED";
        case 3: return "SELF_DRIVING";
        case 4: return "BASIC";
        default: return "UNKNOWN";
    }
}

uint8_t CanHandler::
ComputeVehicleChecksum(const CanFrame &frame, uint8_t checksumByteIndex) {
    if (checksumByteIndex >= frame.dlc) return 0;
    uint16_t sum = static_cast<uint16_t>(frame.id & 0xFF)
                 + static_cast<uint16_t>((frame.id >> 8) & 0xFF);
    for (uint8_t i = 0; i < frame.dlc; ++i) {
        if (i == checksumByteIndex) continue;
        sum += frame.data[i];
    }
    return static_cast<uint8_t>(sum & 0xFF);
}

void CanHandler::
SaveConf(Preferences &prefs) {
    prefs.putBytes("can_cnf", (byte *)&m_cnf, sizeof(m_cnf));
}

void CanHandler::
LoadConf(Preferences &prefs) {
    CanConf can_conf_tmp;
    prefs.getBytes("can_cnf", &can_conf_tmp, sizeof(can_conf_tmp));
    if (can_conf_tmp.version == m_cnf.version)
        m_cnf = can_conf_tmp;
}

void CanHandler::
PrintCnf() {
    Serial.printf("CanConf: version=%u inject=%u fsd=%u print=%u hw3_code=%u ban_shield=%u nag=%u summon=%u eap=%u no_cam=%u evd=%u isa=%u\n",
        m_cnf.version, m_cnf.enable_inject, m_cnf.enable_fsd, m_cnf.enable_print,
        m_cnf.use_hw3_code, m_cnf.enable_ban_shield, m_cnf.enable_nag_suppress,
        m_cnf.enable_summon_unlock, m_cnf.enable_enhanced_autopilot_runtime,
        m_cnf.disable_camera, m_cnf.enable_emergency_vehicle_detection_runtime,
        m_cnf.enable_isa_speed_chime_suppress_runtime);
    Serial.printf("CanConf speed: profile_by_dist=%u profile_web=%u set_hw3=%u offset_en=%u offset_fix_dyn=%u offset_val=%u\n",
        m_cnf.speed_profile_use_follow_distance, m_cnf.speed_profile_from_web,
        m_cnf.enable_set_hw3_profile, m_cnf.speed_offset_enable_override,
        m_cnf.speed_offset_use_fix_or_dynamic, m_cnf.speed_offset_fix_from_web);
    Serial.printf("CanConf speed_limit_auto_cfg:");
    for (const auto &s : m_cnf.speed_limit_auto_cfg)
        Serial.printf(" %u->%u%%", s.speed_limit, s.offset_percent);
    Serial.printf("\n");
}

void CanHandler::
PrintState() {
    Serial.printf("CanState: follow_dist=%u profile_hw3=%u profile_hw4=%u frames=%u sent=%u\n",
        m_state.follow_distance, m_state.speed_profile_to_hw3,
        m_state.speed_profile_to_hw4, m_state.frame_cnt, m_state.frame_sent);
    Serial.printf("CanState: ban_shield=%u ban_check=%u spd_fused=%u spd_vision=%u spd_offset=%u gtwap=%s\n",
        m_state.ban_shield_cnt, m_state.ban_shield_check_cnt,
        m_state.speed_limit_fused, m_state.speed_limit_vision_only,
        m_state.speed_offset, GetGTWAutopilotStr(m_state.gateway_autopilot));
}
