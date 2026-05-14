#pragma once

#include "common.h"
#include "drivers/can_driver.h"
#include <Arduino.h>
#include <vector>

class Preferences;

struct SpeedLimitToPercent {
    uint32_t speed_limit    = 0;
    uint32_t offset_percent = 0;
};

// ── Schema 驱动的字段描述 ───────────────────────────────────────────
// 主页上的开关 / 状态卡片由 schema 统一描述；
// C++ 侧在 handlers.cpp 里静态声明 kCnfSchema[] / kStateSchema[]，Web 前端通过 /schema
// 拉取并自动渲染。
//
// 新增一个可写开关  = 在 CanConf  里加一个 uint32_t 字段，并在 kCnfSchema   里加一行。
// 新增一个只读状态 = 在 CanState 里加一个 uint32_t 字段，并在 kStateSchema 里加一行。
//
// CAN bit 映射一律由 handler.cpp 手写，不再放进 schema。
enum FieldType  : uint8_t { FT_BOOL, FT_ENUM, FT_NUMBER };
enum WidgetType : uint8_t { WT_CHECKBOX, WT_SELECT, WT_SLIDER, WT_INPUT };

struct EnumOption {
    uint32_t    value;    // 编码值
    const char *label_zh; // 中文显示；nullptr 作为数组结束标记
};

// 可写字段（CanConf 成员），前端渲染成表单控件。
struct CnfFieldDesc {
    const char *key;      // 内部 key，同时是 /config 的参数名
    const char *label_zh; // UI 上的中文标签
    const char *group;    // 分组名（如 "FSD" / "安全" / "系统"）；nullptr = 不分组
    FieldType   type;
    WidgetType  widget;

    // 数值字段范围（bool 可填 0/1/1）
    int32_t min_val;
    int32_t max_val;
    int32_t step;

    // 枚举字段（以 {0,nullptr} 结尾的数组）；type==FT_ENUM 时使用
    const EnumOption *enum_options;

    size_t conf_offset;   // offsetof(CanConf, xxx)，按 uint32_t 统一存取

    // true = 该字段在页面上有自定义控件（segmented / slider / 5 档按钮等），
    // 由手写 JS 负责展示与写入；后端 /status、/config、/schema 仍照常遍历。
    bool hidden;
};

// 只读字段（CanState 成员），前端渲染成主页顶部小卡片。
struct StateFieldDesc {
    const char *key;         // 内部 key，同时是 /status 里 JSON 字段名
    const char *label_zh;    // UI 标签（当字段独占一个 tile 且无 tile_label 时使用）
    FieldType   type;        // 仅用 FT_NUMBER / FT_ENUM 两种
    size_t      state_offset; // offsetof(CanState, xxx)

    // 显示控制
    const char *unit;        // 单位后缀，例如 "km/h"、"%"；nullptr = 不显示
    const char *tile;        // 合并 key：同 key 的多个字段会拼在同一个 tile 里；nullptr = 独占
    const char *tile_label;  // tile 标题（同 tile 的字段取第一个非空值）
    const char *tile_sep;    // 同 tile 内多字段之间的分隔符
    const char *enum_labels; // "离线|在线" 式的轻量枚举映射，仅前端显示用
};

extern const CnfFieldDesc   kCnfSchema[];
extern const size_t         kCnfSchemaCount;
extern const StateFieldDesc kStateSchema[];
extern const size_t         kStateSchemaCount;

struct CanConf {
    uint32_t version = 4;

    uint32_t enable_inject    = false;
    uint32_t enable_fsd       = false;
    uint32_t enable_print     = true;

    uint32_t use_hw3_code                              = false;
    uint32_t enable_ban_shield                         = true;
    uint32_t enable_nag_suppress                       = false;
    uint32_t disable_camera                            = false;
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
        {0, 50},
        {20, 50},
        {30, 50},
        {40, 50},
        {50, 40},
        {60, 33},
        {70, 12},
        {80, 0},
        {90, 0},
        {100, 0},
        {110, 0},
        {120, 0}
    };

    uint32_t start_from_park = 0; //
    uint32_t camera_by_distance = 0;
    uint32_t ap_first = 0;   //
    uint32_t enable_fsd_only_in_D = 0;
};

// 按 key 查找 cnf schema 条目；找不到返回 nullptr。实现见 handlers.cpp。
const CnfFieldDesc *CnfSchemaFindByKey(const char *key);

// 按 schema 条目把值写入 CanConf（含范围 clamp）。实现见 handlers.cpp。
void SchemaApplyValue(const CnfFieldDesc &f, CanConf &cnf, uint32_t v);

enum EGearStatus
{
    EGearStatus_P = 3,
    EGearStatus_R = 5,
    EGearStatus_D = 9,
    EGearStatus_N = 7,
};

struct CanState {
    uint32_t follow_distance      = 1; // 1–5, from CAN 1016 data[5][7:5]

    uint32_t speed_profile_to_hw3 = 2; // 0–2, written to 1021 mux-0 data[6][2:1]
    uint32_t speed_profile_to_hw4 = 3; // 0–4, written to 1021 mux-2 data[7][6:4]

    uint32_t frame_cnt  = 0; // total frames received
    uint32_t frame_sent = 0; // total frames injected

    // 每秒收/发帧数（由 web 层按秒采样差分得到；不由 handler 写）
    uint32_t frame_rx_rate = 0;
    uint32_t frame_tx_rate = 0;

    uint32_t ban_shield_cnt       = 0; // frames forwarded by ban shield (changed mux)
    uint32_t ban_shield_check_cnt = 0; // total ban shield comparisons

    uint32_t speed_limit_fused      = 0; // km/h, from CAN 921 data[1][4:0]*5
    uint32_t speed_limit_vision_only = 0; // km/h, from CAN 921 data[2][4:0]*5

    uint32_t speed_offset     = 0;      // current applied offset value
    uint32_t gateway_autopilot = 0;     // from CAN 2047 mux-2 data[5][4:2]
    uint32_t das_ap_state = 0;          // DAS_autopilotState (CAN 921): 0=DISABLED 1=UNAVAILABLE 2=AVAILABLE 3=ACTIVE_NOMINAL 4=ACTIVE_RESTRICTED 5=ACTIVE_NAV 8=ABORTING 9=ABORTED 14=FAULT 15=SNA

    // CAN 总线在线状态：由 web 层根据最近一次收到帧的时间戳决定（0=离线, 1=在线）
    uint32_t can_online = 0;

    uint32_t shift_status = 0; // 档位识别
    uint32_t temprature = 0;  // 温度

    uint32_t last_reset_frame_sec = 0;
};

// ── Debug override (not persisted, cleared on reboot) ─────────────
// 每个帧/mux 维护一份 8 字节的 mask/value：
//   mask[i] 的每一位 = 1 表示对应 bit 由调试页接管；
//   写帧前按位：frame.data[i] = (frame.data[i] & ~mask[i]) | (value[i] & mask[i]);
struct DebugOverride {
    uint8_t mask[8]  = {0};
    uint8_t value[8] = {0};
};

// 最近一次收到的原始帧内容，调试页用于展示"当前值"。
// seen=false 表示还没收到过，前端显示 "--"。
struct DebugLastFrame {
    bool    seen       = false;
    uint8_t data[8]    = {0};
    uint32_t updated_ms = 0;
};

struct DebugState {
    // 四组覆盖（按 1016 和 1021 的三个 mux 分别独立）
    DebugOverride ovr_1016;
    DebugOverride ovr_1021_m0;
    DebugOverride ovr_1021_m1;
    DebugOverride ovr_1021_m2;

    // 最近一次原始帧内容
    DebugLastFrame last_1016;
    DebugLastFrame last_1021_m0;
    DebugLastFrame last_1021_m1;
    DebugLastFrame last_1021_m2;
};

struct CanHandler {
public:
    bool
    Handle923(const CanFrame & frame);

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

    // 将 ovr 中被 mask 标记的位写入 frame。返回是否有任何 bit 被改写。
    bool
    ApplyDebugOverride(CanFrame &frame, const DebugOverride &ovr);

private:
    // CAN 921  — decode speed limits (fused + vision-only) from data[1]/data[2].
    bool
    Handle921(CanFrame &frame);

    // CAN 1016 — decode follow distance from data[5][7:5].
    bool
    Handle1016(CanFrame &frame);

    // CAN 2047 — update gateway_autopilot (mux 2) and run ban shield comparison.
    //            Returns true if the frame changed and should be forwarded.
    bool
    Handle2047(CanFrame &frame);

    // CAN 1021 — dispatch to the appropriate mux handler below.
    bool
    Handle1021(CanFrame &frame);

    // CAN 1021 mux 0 — set FSD enable bits (46, 60), HW3 speed profile (data[6][2:1]),
    //                   and emergency vehicle detection bit (59).
    bool
    Handle1021Mux0(CanFrame &frame);

    // CAN 1021 mux 1 — clear nag bit (19), set nag override (47); optionally clear camera bit (43).
    bool
    Handle1021Mux1(CanFrame &frame);

    // CAN 1021 mux 2 — set HW4 speed profile (data[7][6:4]) and write speed offset byte.
    bool
    Handle1021Mux2(CanFrame &frame);

    bool
    Handle280(CanFrame& frame);

public:
    std::vector<uint32_t> filter_can_id_list = {280, 921, 1016, 1021, 2047, 923};

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
    DebugState m_dbg; // 调试覆盖 / 最近一次帧（非持久化）
};