//
// Created by hong- on 2026/4/26.
//

#include "handlers.h"
#include <Preferences.h>
#include <cstddef>
#include <cstring>

// ── Schema ────────────────────────────────────────────────────────
// 主页上的可写开关 / 只读状态都在这里声明。
//   可写字段（渲染成表单控件）→ kCnfSchema[]
//   只读字段（渲染成顶部小卡片）→ kStateSchema[]
//
// 新增一个简单开关：
//   1. 在 CanConf  里加一个 uint32_t 字段
//   2. 在 kCnfSchema 里加一行
// 新增一个只读状态：
//   1. 在 CanState 里加一个 uint32_t 字段
//   2. 在 kStateSchema 里加一行
// C++ handler 里照常读写 m_cnf.xxx / m_state.xxx；CAN bit 映射手写在各 handler 里。

// ── cnf（可写）─────────────────────────────────────────────────────
// 一个纯 bool 开关
// 注意：#KEY 把裸标识符字符串化为 "enable_fsd" 这样的 key；offsetof 仍用裸名字。
#define CNF_BOOL(KEY, LABEL, GROUP) \
    { #KEY, LABEL, GROUP, FT_BOOL, WT_CHECKBOX, 0, 1, 1, nullptr, offsetof(CanConf, KEY), false }

// 前端有自定义控件（不走自动渲染）的 cnf 字段：hidden=true
// 后端 /status、/config、/schema 仍照常遍历；前端 renderSchemaGroups() 会跳过自动渲染。
#define CNF_HIDDEN_BOOL(KEY) \
    { #KEY, #KEY, nullptr, FT_BOOL, WT_CHECKBOX, 0, 1, 1, nullptr, offsetof(CanConf, KEY), true }

#define CNF_HIDDEN_NUM(KEY, MINV, MAXV, STEPV) \
    { #KEY, #KEY, nullptr, FT_NUMBER, WT_INPUT, MINV, MAXV, STEPV, nullptr, offsetof(CanConf, KEY), true }

// ── state（只读；显示在主页顶部小卡片）──────────────────────────
// 用法约定：
//   - tile == nullptr  → 字段独占一个 tile，标题取 tile_label（若空则取 label）。
//   - tile != nullptr  → 同 tile 的字段按 schema 顺序合并到一个 tile；
//                        标题取第一个非空的 tile_label；字段之间用 tile_sep 拼接；单位 unit 跟在值后。
// 数值（显示原始数字 + 可选单位）
#define STATE_NUM(KEY, LABEL, UNIT, TILE, TILE_LBL, SEP) \
    { #KEY, LABEL, FT_NUMBER, offsetof(CanState, KEY), UNIT, TILE, TILE_LBL, SEP, nullptr }

// 用 "离线|在线" 或 "无|高速|..." 的轻量枚举映射显示（无需单独定义 EnumOption 数组）
#define STATE_ENUM(KEY, LABEL, LABELS, TILE, TILE_LBL, SEP) \
    { #KEY, LABEL, FT_ENUM, offsetof(CanState, KEY), nullptr, TILE, TILE_LBL, SEP, LABELS }

const StateFieldDesc kStateSchema[] = {
    STATE_ENUM(can_online,               "CAN 总线",     "离线|在线",                  nullptr, "CAN 总线",           " / "),
    STATE_NUM (frame_rx_rate,            "帧/s 收",      nullptr, "frm", "帧/s 收 / 发",      " / "),
    STATE_NUM (frame_tx_rate,            "帧/s 发",      nullptr, "frm", "帧/s 收 / 发",      " / "),
    STATE_NUM (follow_distance,          "跟车距离",      nullptr, nullptr, "跟车距离",         " / "),
    STATE_NUM (speed_profile_to_hw3,     "档位 HW3",     nullptr, "sp",  "速度档位 HW3/HW4",   " / "),
    STATE_NUM (speed_profile_to_hw4,     "档位 HW4",     nullptr, "sp",  "速度档位 HW3/HW4",   " / "),
    STATE_NUM (speed_limit_fused,        "限速 融合",     "km/h",  "sl",  "限速 融合/视觉",     " / "),
    STATE_NUM (speed_limit_vision_only,  "限速 视觉",     "km/h",  "sl",  "限速 融合/视觉",     " / "),
    STATE_NUM (speed_offset,             "速度偏移",      "%",     nullptr, "速度偏移",         " / "),
    STATE_ENUM(gateway_autopilot,        "网关自动驾驶", "无|高速|增强|自动驾驶|基础",         nullptr, "网关自动驾驶",       " / "),
    STATE_NUM (ban_shield_cnt,           "Ban 命中",     nullptr, "bs",  "Ban 盾 命中/检查",    " / "),
    STATE_NUM (ban_shield_check_cnt,     "Ban 检查",     nullptr, "bs",  "Ban 盾 命中/检查",    " / "),
    STATE_ENUM(das_ap_state,             "DAS AP 状态", "关闭|不可用|就绪|工作|受限|导航|-|-|中止中|已中止|-|-|-|-|故障|无效", "das", "DAS AP 状态", " / "),
    STATE_ENUM(shift_status,               "档位", "-|-|-|P|-|R|-|N|-|D|-|-|-|-|-|-", "档位", "档位状态", " / ")
};

const size_t kStateSchemaCount = sizeof(kStateSchema) / sizeof(kStateSchema[0]);

const CnfFieldDesc kCnfSchema[] = {
    // ── FSD ──
    CNF_BOOL(enable_fsd,                                  "FSD 启用",          "FSD"),
    CNF_BOOL(use_hw3_code,                                "使用 HW3 代码",      "FSD"),
    CNF_BOOL(start_from_park,                             "驻车启动",           "FSD"),
    CNF_BOOL(ap_first,                                    "APFirst 模式",      "FSD"),
    CNF_BOOL(enable_fsd_only_in_D,                        "仅D/R档启用",          "FSD"),

    // ── 安全 ──
    CNF_BOOL(enable_ban_shield,                           "Ban 盾保护",                        "安全"),
    CNF_BOOL(enable_nag_suppress,                         "消除提示音",                         "安全"),
    CNF_BOOL(disable_camera,                              "禁用摄像头",                         "安全"),
    CNF_BOOL(enable_isa_speed_chime_suppress_runtime,     "ISA 提示音抑制",                     "安全"),
    CNF_BOOL(camera_by_distance,                          "根据距离关闭摄像头(>1:关闭;1:打开)",    "安全"),

    // ── 系统 ──
    CNF_BOOL(enable_print,                                "串口日志",                          "系统"),

    // ── Hidden（前端自定义 UI；后端通过 schema 自动收发）──
    CNF_HIDDEN_BOOL(enable_inject),
    CNF_HIDDEN_BOOL(speed_profile_use_follow_distance),
    CNF_HIDDEN_BOOL(enable_set_hw3_profile),
    CNF_HIDDEN_BOOL(speed_offset_enable_override),
    CNF_HIDDEN_BOOL(speed_offset_use_fix_or_dynamic),
    CNF_HIDDEN_NUM (speed_profile_from_web,    1,  5, 1),
    CNF_HIDDEN_NUM (speed_offset_fix_from_web, 0, 50, 1),
};
const size_t kCnfSchemaCount = sizeof(kCnfSchema) / sizeof(kCnfSchema[0]);

// 通过 key 找到 cnf schema 条目；没找到返回 nullptr。
const CnfFieldDesc *CnfSchemaFindByKey(const char *key) {
    for (size_t i = 0; i < kCnfSchemaCount; ++i) {
        if (strcmp(kCnfSchema[i].key, key) == 0) return &kCnfSchema[i];
    }
    return nullptr;
}

// 按 schema 条目把值写入 CanConf。值统一按 uint32_t 存储。
void SchemaApplyValue(const CnfFieldDesc &f, CanConf &cnf, uint32_t v) {
    // 对数值/枚举做范围 clamp
    if (f.type == FT_NUMBER && f.max_val > f.min_val) {
        int32_t sv = (int32_t)v;
        if (sv < f.min_val) sv = f.min_val;
        if (sv > f.max_val) sv = f.max_val;
        v = (uint32_t)sv;
    }
    uint32_t *slot = (uint32_t *)((char *)&cnf + f.conf_offset);
    *slot = v;
}

// ── CAN 921 — speed limits ────────────────────────────────────────

bool CanHandler::
Handle921(CanFrame &frame) {
    if (frame.dlc < 8) return false;

    bool need_send = false;
    m_state.speed_limit_fused       = (frame.data[1] & 0x1F) * 5;
    m_state.speed_limit_vision_only = (frame.data[2] & 0x1F) * 5;

    m_state.das_ap_state = (frame.data[0] & 0x0F);

    if (m_cnf.enable_isa_speed_chime_suppress_runtime) {
        frame.data[1] |= 0x20;
        frame.data[7] = ComputeVehicleChecksum(frame);
        need_send = true;
    }

    return need_send;
}

// ── CAN 1016 — follow distance ────────────────────────────────────

bool CanHandler::
Handle1016(CanFrame &frame) {
    if (frame.dlc < 8) return false;
    m_state.follow_distance = (frame.data[5] & 0b11100000) >> 5;

    if (m_cnf.camera_by_distance)
        m_cnf.disable_camera = m_state.follow_distance > 1;

    return false;
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
    frame.SetBit(46, true);
    frame.SetBit(59, true);

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

    return true;
}

// ── CAN 1021 mux 1 — nag suppress + camera disable ───────────────

bool CanHandler::
Handle1021Mux1(CanFrame &frame) {
    bool should_send = false;

    if (m_cnf.disable_camera) {
        frame.SetBit(43, false);
        should_send = true;
    }

    if (m_cnf.enable_nag_suppress) {
        frame.SetBit(19, false);
        should_send = true;
    }

    return should_send;
}

// ── CAN 1021 mux 2 — HW4 speed profile + speed offset ────────────

bool CanHandler::
Handle1021Mux2(CanFrame &frame) {
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

    if (!m_cnf.enable_set_hw3_profile) {
        frame.data[7] &= ~(0x07 << 4);
        frame.data[7] |= (m_state.speed_profile_to_hw4 & 0x07) << 4;
    }

    if (m_cnf.speed_offset_enable_override) {
        m_state.speed_offset = m_cnf.speed_offset_fix_from_web;
        if (m_cnf.speed_offset_use_fix_or_dynamic)
            m_state.speed_offset = (uint8_t)CalcCustomOffset();

        if (m_state.speed_offset > 0) {
            frame.data[1] = (frame.data[1] & 0xC0) | (m_state.speed_offset & 0x3F);
        }
    }

    if (m_cnf.start_from_park) {
        frame.SetBit(6, true);
        frame.SetBit(7, true);
    }

    return true;
}
bool CanHandler::
Handle280(CanFrame &frame) {
    m_state.shift_status = (frame.data[2] >> 4) & 0x0F;
    return false;
}

// ── CAN 1021 — dispatch by mux ────────────────────────────────────

bool CanHandler::
Handle1021(CanFrame &frame) {
    if (frame.dlc < 8) return false;

    if (!m_cnf.enable_fsd)
        return false;

    if (m_cnf.enable_fsd_only_in_D && m_state.shift_status != EGearStatus::EGearStatus_D && m_state.shift_status != EGearStatus::EGearStatus_R)
        return false;

    switch (frame.GetMux()) {
        case 0:  return Handle1021Mux0(frame);
        case 1:  return Handle1021Mux1(frame);
        case 2:  return Handle1021Mux2(frame);
        default: return false;
    }
}

// ── Main dispatch ─────────────────────────────────────────────────

bool CanHandler::Handle923(const CanFrame &frame) {
    if (frame.dlc < 7)
        return false;
    // m_state.das_ap_state = (frame.data[1] >> 4) & 0x0F;
    return false;
}

bool CanHandler::
Handle(CanFrame &frame, CanDriver &driver) {
    bool should_send = false;

    m_state.frame_rx_rate += 1;

    switch (frame.id) {
        case 921:  should_send = Handle921(frame);   break;
        case 1016: should_send = Handle1016(frame);  break;
        case 2047: should_send = Handle2047(frame);  break;
        case 1021: should_send = Handle1021(frame);  break;
        case 923:  should_send = Handle923(frame);      break;
        case 280: should_send = Handle280(frame);    break;
    }

    if (should_send)
        m_state.frame_tx_rate += 1;

    if (m_state.last_reset_frame_sec < millis() / 1000)
    {
        m_state.last_reset_frame_sec = millis() / 1000;
        m_state.frame_rx_rate = 0;
        m_state.frame_tx_rate = 0;
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

// 应用调试覆盖：按 mask/value 位级写入 frame，返回是否实际修改了任何 bit。
// 安全红线：1021 mux 选择器 (bit 0/1/2) 与 ban protection bit 52 永不接受覆盖。
bool CanHandler::
ApplyDebugOverride(CanFrame &frame, const DebugOverride &ovr) {
    // 拷贝一份 mask 并强制清掉红线位
    uint8_t mask[8];
    memcpy(mask, ovr.mask, 8);
    // 禁止覆盖 mux 选择器 data[0] bit[0..2]
    mask[0] &= static_cast<uint8_t>(~0x07);
    // 禁止覆盖 ban protection bit 52 = data[6] bit 4
    mask[6] &= static_cast<uint8_t>(~(1U << 4));

    bool changed = false;
    for (int i = 0; i < 8; i++) {
        if (!mask[i]) continue;
        uint8_t before = frame.data[i];
        frame.data[i]  = static_cast<uint8_t>((before & ~mask[i]) | (ovr.value[i] & mask[i]));
        if (frame.data[i] != before) changed = true;
    }
    return changed;
}

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
    // Serial.printf("CanConf: version=%u inject=%u fsd=%u print=%u hw3_code=%u ban_shield=%u nag=%u summon=%u eap=%u no_cam=%u evd=%u isa=%u\n",
    //     m_cnf.version, m_cnf.enable_inject, m_cnf.enable_fsd, m_cnf.enable_print,
    //     m_cnf.use_hw3_code, m_cnf.enable_ban_shield,
    //     m_cnf.enable_nag_suppress,
    //     m_cnf.enable_summon_unlock,
    //     m_cnf.enable_enhanced_autopilot_runtime,
    //     m_cnf.disable_camera,
    //     m_cnf.enable_emergency_vehicle_detection_runtime,
    //     m_cnf.enable_isa_speed_chime_suppress_runtime);
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
    Serial.printf("CanState: ban_shield=%u ban_check=%u spd_fused=%u spd_vision=%u spd_offset=%u gtwap=%s gear:%d\n",
        m_state.ban_shield_cnt, m_state.ban_shield_check_cnt,
        m_state.speed_limit_fused, m_state.speed_limit_vision_only,
        m_state.speed_offset, GetGTWAutopilotStr(m_state.gateway_autopilot),
        m_state.shift_status);
}
