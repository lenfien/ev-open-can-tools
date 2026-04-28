#pragma once
// ── 模块：FSD 注入处理器 ───────────────────────────────────────────────────
// 三种硬件模式的处理器，负责读取、修改并重传 FSD CAN 帧。
// 选择逻辑位于 handlers.h（handleMessage）。
//
// 处理的 CAN ID（接收 + 修改重传）：
//   Legacy  — 0x045 (69)   档杆位置  [只读]
//             0x3EE (1006) FSD 帧    [mux 0/1 被修改]
//   HW3     — 0x3F8 (1016) 档杆      [只读]
//             0x3FD (1021) FSD 帧    [mux 0/1/2 被修改]
//   HW4     — 0x399 (921)  ISA 提示音 [被修改]
//             0x3F8 (1016) 档杆      [只读]
//             0x3FD (1021) FSD 帧    [mux 0/1/2 被修改]

#include <algorithm>
#include "can_frame_types.h"
#include "drivers/can_driver.h"
#include "can_helpers.h"
#include "fsd_config.h"

extern FSDConfig cfg;  // defined in handlers.h

// ── HW3 自动速度策略（对应 tesla-open-can-mod hw3_speed_policy.h）──
// 针对公制仪表盘的实测请求下限值。自动目标值、分桶形状及切换点
// 定义在 fsd_config.h，以便默认值与策略共用同一数据源。
//
// 线路编码（0x3FD mux-2 data[0][6:7] + data[1][0:5]，8-bit 原始值）：
//   Tesla 将原始值解码为限速百分比：pct = raw / 4。
//   固件上限为 50%，故 raw 范围 [0, 200]。来源：tesla-open-can-mod
//   include/app.h:211（manualSpeedOffset = pct * 4）及 include/can_helpers.h:113
//   （offsetPct 0-50）。
static constexpr int kHw3SpeedOffsetMaxPct = 50;  // 线路 raw 上限 = 200

// ── HW3 offset 斜率限制器 ─────────────────────────────────────────────────
// 当道路限速突然降低（如 80 → 40 kph），计算出的 raw offset 字节也会骤降。
// 立即发送新的低值会导致车辆急刹。改为以 ≤ rate pct/sec 的速率逐步降低
// 输出字节（可通过 cfg.hw3SlewRatePctPerSec 调节）。上升方向不限速。
//
// 单位：pct/sec（每秒占限速的百分比）。与限速无关，因此同一数值在
// 40 / 60 / 80 kph 道路上感受一致。
// 默认 5 pct/sec ≈ 在 60 kph 限速下减速约 3 kph/s。
// 内部换算：raw/sec = pct/sec × 4（线路编码为 pct × 4）。
static constexpr uint8_t kHw3SlewRateMin = 1;    // 1 pct/s ≈ 60 kph 限速下 0.6 kph/s
static constexpr uint8_t kHw3SlewRateMax = 25;   // 25 pct/s ≈ 60 kph 限速下 15 kph/s
static constexpr uint8_t kHw3SlewRateDefault = 5;

static inline int computeHW3MinimumTargetSpeedKph(int fusedLimitKph) {
    if (fusedLimitKph == 60)                       return kHw3AutoTargetAt60Kph;
    if (fusedLimitKph <  kHw3AutoTargetBelow60Kph) return kHw3AutoTargetBelow60Kph;
    if (fusedLimitKph <  kHw3StockOffsetCutoverKph) return kHw3AutoTargetForVisible80Kph;
    return fusedLimitKph;
}

// 自定义模式：用户自定义目标速度，以 kHw3CustomBucketStepKph 为步长，
// 从 kHw3CustomBucketBaseKph 开始分桶查表。
// 当输入超出表格范围时返回 0，调用方回退到直通模式。
static inline int computeHW3CustomTargetSpeedKph(int fusedLimitKph) {
    if (fusedLimitKph <  kHw3CustomBucketBaseKph ||
        fusedLimitKph >= kHw3StockOffsetCutoverKph) return 0;
    int idx = (fusedLimitKph - kHw3CustomBucketBaseKph) / kHw3CustomBucketStepKph;
    return (int)cfg.hw3CustomTarget[idx];
}

// 实测发现的两种线路编码方案，由 cfg.hw3WireEncoding 选择。
enum Hw3WireEnc : uint8_t { HW3_ENC_KPH5 = 0, HW3_ENC_PCT4 = 1, HW3_ENC_MAX = HW3_ENC_PCT4 };

// KPH5 编码上限为 40 kph offset（raw 200）。
// 经 2024 款 Model Y HW3（v1.4.28 反馈者）验证正确。
static constexpr int kHw3EncKph5MaxKph = 40;
static constexpr int kHw3EncKph5Scale  = 5;  // raw = offsetKph × 5
static constexpr int kHw3EncPct4Scale  = 4;  // raw = pct × 4

// raw = pct × 4（tesla-open-can-mod 参考实现；Issue #9 反馈者）。
static inline uint8_t encodeHW3OffsetPct4(int pct) {
    int clamped = std::max(0, std::min(pct, kHw3SpeedOffsetMaxPct));
    return (uint8_t)(clamped * kHw3EncPct4Scale);
}

// raw = offsetKph × 5（v1.4.25 行为；2024 款 Model Y HW3 反馈者）。
static inline uint8_t encodeHW3OffsetKph5(int offsetKph) {
    int clamped = std::max(0, std::min(offsetKph, kHw3EncKph5MaxKph));
    return (uint8_t)(clamped * kHw3EncKph5Scale);
}

// HW3 0x3FD mux-2 offset 写入的统一入口（调用方已知 km/h 值时使用）。
// 根据 cfg.hw3WireEncoding 分发；调用方同时提供 km/h offset 和当前限速，
// 使两条路径都能获取所需数据，无需二次查表。
static inline uint8_t encodeHW3Offset(int offsetKph, int fusedLimitKph) {
    if (offsetKph <= 0 || fusedLimitKph <= 0) return 0;
    if (cfg.hw3WireEncoding == HW3_ENC_KPH5) {
        return encodeHW3OffsetKph5(offsetKph);
    }
    int pct = (offsetKph * 100 + fusedLimitKph / 2) / fusedLimitKph;
    return encodeHW3OffsetPct4(pct);
}

// 高速分支按桶存储 pct，根据线路编码分发。
// PCT4 直接编码 pct（避免 kph 往返转换的精度损失）；
// KPH5 将 pct 一次性换算为 kph，再转为 raw。
static inline uint8_t encodeHW3OffsetFromPct(int pct, int fusedLimitKph) {
    if (pct <= 0 || fusedLimitKph <= 0) return 0;
    if (cfg.hw3WireEncoding == HW3_ENC_PCT4) {
        return encodeHW3OffsetPct4(pct);
    }
    int offsetKph = (fusedLimitKph * pct + 50) / 100;
    return encodeHW3OffsetKph5(offsetKph);
}

// ── CAN ID 过滤表（供 handleMessage 使用）────────────────────────────────
static constexpr uint32_t LEGACY_IDS[] = {69, 760, 1006, 1080};
static constexpr uint32_t HW3_IDS[]    = {787, 1016, 1021};
static constexpr uint32_t HW4_IDS[]    = {921, 1016, 1021};

inline const uint32_t* getFilterIds() {
    switch (cfg.hwMode) {
        case 0:  return LEGACY_IDS;
        case 1:  return HW3_IDS;
        default: return HW4_IDS;
    }
}
inline uint8_t getFilterIdCount() {
    switch (cfg.hwMode) {
        case 0:  return 4;
        case 1:  return 3;
        default: return 3;
    }
}
inline bool isFilteredId(uint32_t id) {
    auto* ids = getFilterIds();
    auto  cnt = getFilterIdCount();
    for (uint8_t i = 0; i < cnt; i++) {
        if (ids[i] == id) return true;
    }
    return false;
}

// ── 处理器：HW3（0x3FD / 0x3F8 / 0x313）────────────────────────────────
static void handleHW3(CanFrame& frame, CanDriver& driver) {
    // 0x313 (787) — UI_trackModeSettings：回传时带 trackModeRequest=ON
    if (frame.id == 787) {
        if (!cfg.trackModeEnable) return;
        if (frame.dlc < 8) return;
        setTrackModeRequest(frame, 0x01);
        frame.data[7] = computeVehicleChecksum(frame);
        if (driver.send(frame)) cfg.modifiedCount++;
        else                    cfg.errorCount++;
        return;
    }
    // 0x3F8 (1016) — 档杆位置 → 速度档位（仅自动模式）
    if (frame.id == 1016 && cfg.profileModeAuto) {
        if (frame.dlc < 6) return;
        uint8_t fd = (frame.data[5] & 0b11100000) >> 5;
        switch (fd) {
            case 1: cfg.speedProfile = 2; break;
            case 2: cfg.speedProfile = 1; break;
            case 3: cfg.speedProfile = 0; break;
        }
        return;
    }
    // 0x3FD (1021) — FSD 激活帧（mux 0/1/2）
    // 对应 tesla-open-can-mod：mux-0 从 byte 3 读取原厂 EAP offset，
    // mux-2 将有效 offset 写回（融合限速 ≥80 kph 时直通原厂值，低于时取校准下限）。
    if (frame.id == 1021) {
        if (frame.dlc < 8) return;
        auto index = readMuxID(frame);
        if (index == 0) cfg.fsdTriggered = cfg.forceActivate || isFSDSelectedInUI(frame);
        if (index == 0 && cfg.fsdTriggered && cfg.fsdEnable) {
            // byte 3 bits 1-6 存储 Tesla 自身的原厂 offset 偏好，编码方式为
            // (kph + 30) / 5？不是——opendbc 说是类 offset_kph；此处
            // 严格遵循 tesla-open-can-mod：((d3>>1)&0x3F - 30)*5，限定 [0,100]。
            cfg.hw3SpeedOffset = std::max(std::min(((int)((frame.data[3] >> 1) & 0x3F) - 30) * 5, 100), 0);
            setBit(frame, 46, true);
            setSpeedProfileV12V13(frame, cfg.speedProfile);
            if (driver.send(frame)) cfg.modifiedCount++;
            else                    cfg.errorCount++;
        }
        if (index == 1 && cfg.fsdTriggered && cfg.fsdEnable) {
            setBit(frame, 19, false);
            driver.send(frame);  // 仅抑制 nag 提示，不计入 FSD 修改次数
        }
        if (index == 2 && cfg.fsdTriggered && cfg.fsdEnable) {
            // 从原厂 offset raw 开始（Tesla 原本会发的值——hw3SpeedOffset
            // 以 pct*5 存储在 [0,100] 范围内，与 open-can-mod 一致）。
            uint8_t activeRaw = (uint8_t)std::max(std::min((int)cfg.hw3SpeedOffset, 255), 0);

            // UI 保证 Auto 与 Custom 互斥；Custom 优先的顺序是防御性写法，
            // 以防客户端互斥逻辑失效。
            // ≥80 kph：独立的 hw3HighSpeedEnable 分支直接写 pct×4；
            // 该开关关闭时保持原有的原厂直通行为。
            {
                uint8_t fl = (cfg.fusedSpeedLimit > 0 && cfg.fusedSpeedLimit < 31) ? cfg.fusedSpeedLimit : 0;
                if (fl > 0) {
                    int fusedLimitKph = (int)fl * 5;
                    if (fusedLimitKph < kHw3StockOffsetCutoverKph) {
                        if (cfg.hw3CustomSpeed || cfg.hw3AutoSpeed) {
                            int targetSpeedKph = cfg.hw3CustomSpeed
                                ? computeHW3CustomTargetSpeedKph(fusedLimitKph)
                                : computeHW3MinimumTargetSpeedKph(fusedLimitKph);
                            if (targetSpeedKph > 0) {
                                int desiredOffsetKph = std::max(targetSpeedKph - fusedLimitKph, 0);
                                activeRaw = encodeHW3Offset(desiredOffsetKph, fusedLimitKph);
                            }
                        }
                    } else {
                        if (cfg.hw3HighSpeedEnable) {
                            int idx = (fusedLimitKph - kHw3HighSpeedBucketBaseKph)
                                      / kHw3HighSpeedBucketStepKph;
                            if (idx < 0) idx = 0;
                            if (idx >= kHw3HighSpeedBucketCount) idx = kHw3HighSpeedBucketCount - 1;
                            uint8_t pct = cfg.hw3HighSpeedTargetPct[idx];
                            if (pct > 0) {
                                activeRaw = encodeHW3OffsetFromPct((int)pct, fusedLimitKph);
                            }
                        }
                    }
                }
            }

            // 诊断：记录斜率整形前编码器请求的原始值。
            cfg.hw3OffsetTargetRaw = activeRaw;

            // 仅对下降方向做斜率限制。当道路限速突然降低（地图数据更新、
            // 进入城区等），从新限速计算出的 activeRaw 会在一帧内骤降
            // → 车辆急刹。限制每秒允许下降的幅度。
            // 上升方向不限速，以便进入更高限速路段时立即生效。
            //
            // 限速 ≥80 kph 时跳过斜率限制：该区间内 raw 字节的变化
            // 来自用户切换高速开关/修改 pct，而非影响乘坐舒适性的限速骤降。
            // 用户要求 ≥80 kph 时立即响应。
            uint8_t curFl = (cfg.fusedSpeedLimit > 0 && cfg.fusedSpeedLimit < 31) ? cfg.fusedSpeedLimit : 0;
            bool slewApplies = cfg.hw3OffsetSlew &&
                               curFl > 0 &&
                               ((int)curFl * 5 < kHw3StockOffsetCutoverKph);
            if (slewApplies) {
                uint32_t now = millis();
                uint8_t  last = cfg.hw3OffsetLastRaw;
                // 将运行时可配置的速率限定在 [min, max]；若 NVS 中存储的值
                // 过期或损坏（0 或 255），则回退到编译时默认值。
                // 单位为限速的 pct/sec，乘以 4 转换为 raw/sec。
                uint8_t  ratePctPerSec = cfg.hw3SlewRatePctPerSec;
                if (ratePctPerSec < kHw3SlewRateMin || ratePctPerSec > kHw3SlewRateMax) {
                    ratePctPerSec = kHw3SlewRateDefault;
                }
                uint32_t rateRawPerSec = (uint32_t)ratePctPerSec * 4;
                if (activeRaw < last && cfg.hw3OffsetLastSentMs != 0) {
                    uint32_t dt = now - cfg.hw3OffsetLastSentMs;
                    uint32_t maxDrop = (rateRawPerSec * dt + 500) / 1000;  // 四舍五入
                    uint8_t  floorRaw = (last > maxDrop) ? (uint8_t)(last - maxDrop) : 0;
                    if (activeRaw < floorRaw) {
                        activeRaw = floorRaw;
                        cfg.hw3OffsetSlewCount++;
                    }
                }
                cfg.hw3OffsetLastRaw    = activeRaw;
                cfg.hw3OffsetLastSentMs = now;
            } else {
                // 保持状态更新，避免稍后开启斜率限制时误判为大幅跳变；
                // 记录最后发送的字节，防止开关切换后第一帧出现瞬态尖峰。
                cfg.hw3OffsetLastRaw    = activeRaw;
                cfg.hw3OffsetLastSentMs = millis();
            }

            frame.data[0] &= ~(0b11000000);
            frame.data[1] &= ~(0b00111111);
            frame.data[0] |= (activeRaw & 0x03) << 6;
            frame.data[1] |= (activeRaw >> 2);
            if (driver.send(frame)) cfg.modifiedCount++;
            else                    cfg.errorCount++;
        }
    }
}

// ── 处理器：HW4（0x3FD / 0x3F8 / 0x399）────────────────────────────────
static void handleHW4(CanFrame& frame, CanDriver& driver) {
    // 0x399 (921) — 仅用于抑制 ISA 限速提示音
    // 限速值由 handleDASStatus 从 0x39B (923) 读取——此处不读
    if (frame.id == 921) {
        if (frame.dlc < 8) return;
        if (!cfg.isaChimeSuppress) return;
        frame.data[1] |= 0x20;
        frame.data[7] = computeVehicleChecksum(frame);
        if (driver.send(frame)) cfg.modifiedCount++;
        else                    cfg.errorCount++;
        return;
    }
    // 0x3F8 (1016) — 档杆位置 → 速度档位（仅自动模式）
    if (frame.id == 1016 && cfg.profileModeAuto) {
        if (frame.dlc < 6) return;
        auto fd = (frame.data[5] & 0b11100000) >> 5;
        switch (fd) {
            case 1: cfg.speedProfile = 3; break;
            case 2: cfg.speedProfile = 2; break;
            case 3: cfg.speedProfile = 1; break;
            case 4: cfg.speedProfile = 0; break;
            case 5: cfg.speedProfile = 4; break;
        }
        return;
    }
    // 0x3FD (1021) — FSD 激活帧（mux 0/1/2）
    if (frame.id == 1021) {
        if (frame.dlc < 8) return;
        auto index = readMuxID(frame);
        if (index == 0) {
            cfg.fsdTriggered = cfg.forceActivate || isFSDSelectedInUI(frame);
#ifdef DEBUG_MODE
            for (int i = 0; i < 8; i++) cfg.dbgFrame[i] = frame.data[i];
            cfg.dbgFrameCaptured = true;
#endif
        }
        if (index == 0 && cfg.fsdTriggered && cfg.fsdEnable) {
            setBit(frame, 46, true);
            setBit(frame, 60, true);
            if (cfg.emergencyDetection) setBit(frame, 59, true);
            if (driver.send(frame)) cfg.modifiedCount++;
            else                    cfg.errorCount++;
        }
        if (index == 1 && cfg.fsdTriggered && cfg.fsdEnable) {
            // bit19=false：抑制 nag 提示（与 HW3 相同）
            // bit47=true：HW4 专有 FSD 就绪信号；计入修改次数（与 HW3 仅抑制 nag 不同）
            setBit(frame, 19, false);
            setBit(frame, 47, true);
            if (driver.send(frame)) cfg.modifiedCount++;
            else                    cfg.errorCount++;
        }
        if (index == 2 && cfg.fsdTriggered && cfg.fsdEnable) {
            frame.data[7] &= ~(0x07 << 4);
            frame.data[7] |= (cfg.speedProfile & 0x07) << 4;
            if (cfg.hw4OffsetRaw > 0)
                frame.data[1] = (frame.data[1] & 0xC0) | (cfg.hw4OffsetRaw & 0x3F);
            if (driver.send(frame)) cfg.modifiedCount++;
            else                    cfg.errorCount++;
        }
    }
}