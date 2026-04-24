#pragma once

#include <memory>
#include <algorithm>
#include "can_frame_types.h"
#include "drivers/can_driver.h"
#include "can_helpers.h"
#include "shared_types.h"
#include "log_buffer.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

inline LogRingBuffer logRing;
//
// // ── HW3 policy constants (shared with mod_fsd.h, defaults below, and UI labels) ──
// // Auto targets (field-calibrated — metric cluster under-delivers vs request):
// static constexpr int kHw3AutoTargetBelow60Kph      = 64;   // fusedLimit <60 kph  → request 64 (visible ~50)
// static constexpr int kHw3AutoTargetAt60Kph         = 100;  // fusedLimit =60 kph  → request 100 (visible ~80)
// static constexpr int kHw3AutoTargetForVisible80Kph = 85;   // 60<fusedLimit<80    → request 85 (visible ~70)
// static constexpr int kHw3StockOffsetCutoverKph     = 80;   // ≥80 kph → passthrough stock EAP offset
// // Custom mode: user-defined target-speed lookup bucketed by 10 kph starting at 30.
// static constexpr int kHw3CustomBucketBaseKph = 30;
// static constexpr int kHw3CustomBucketStepKph = 10;
// static constexpr int kHw3CustomTargetCount   = 5;  // 30/40/50/60/70 kph limit buckets
// static_assert((kHw3StockOffsetCutoverKph - kHw3CustomBucketBaseKph) / kHw3CustomBucketStepKph == kHw3CustomTargetCount, "Custom target table must cover [base, cutover) in step-sized buckets");
//
// // ── HW3 auto speed policy (mirrors tesla-open-can-mod hw3_speed_policy.h) ──
// // Field-calibrated request floors for metric clusters. Auto targets / bucket
// // shape / cutover live in fsd_config.h so defaults + policy share one source.
// //
// // Wire encoding (0x3FD mux-2 data[0][6:7] + data[1][0:5], 8-bit raw):
// //   Tesla decodes raw as percentage of posted limit: pct = raw / 4.
// //   Firmware caps at 50%, so raw range [0, 200]. Source: tesla-open-can-mod
// //   include/app.h:211 (manualSpeedOffset = pct * 4) and include/can_helpers.h:113
// //   (offsetPct 0-50).
// static constexpr int kHw3SpeedOffsetMaxPct = 50;  // wire raw cap = 200
//
// static inline int computeHW3MinimumTargetSpeedKph(int fusedLimitKph) {
//     if (fusedLimitKph == 60)                       return kHw3AutoTargetAt60Kph;
//     if (fusedLimitKph <  kHw3AutoTargetBelow60Kph) return kHw3AutoTargetBelow60Kph;
//     if (fusedLimitKph <  kHw3StockOffsetCutoverKph) return kHw3AutoTargetForVisible80Kph;
//     return fusedLimitKph;
// }
//
// // Custom mode: user-defined target-speed lookup bucketed by
// // kHw3CustomBucketStepKph starting at kHw3CustomBucketBaseKph.
// // Returns 0 when input is outside the table range — caller falls back to passthrough.
// static inline int computeHW3CustomTargetSpeedKph(int fusedLimitKph) {
//     if (fusedLimitKph <  kHw3CustomBucketBaseKph || fusedLimitKph >= kHw3StockOffsetCutoverKph) return 0;
//     int idx = (fusedLimitKph - kHw3CustomBucketBaseKph) / kHw3CustomBucketStepKph;
//     return (int)cfg.hw3CustomTarget[idx];
// }
//
// static inline uint8_t encodeHW3OffsetRawFromPct(int pct) {
//     int clamped = std::max(0, std::min(pct, kHw3SpeedOffsetMaxPct));
//     return (uint8_t)(clamped * 4);
// }
//
// // Convert a desired km/h offset at a given posted limit into the raw byte
// // written to the 0x3FD active-offset field. Rounds to nearest pct.
// static inline uint8_t encodeHW3OffsetRawFromKph(int offsetKph, int fusedLimitKph) {
//     if (fusedLimitKph <= 0 || offsetKph <= 0) return 0;
//     int pct = (offsetKph * 100 + fusedLimitKph / 2) / fusedLimitKph;
//     return encodeHW3OffsetRawFromPct(pct);
// }
//

struct CarManagerBase
{
    bool useHW3Code{true};
    bool InjectActive{false};
    bool ADEnabled{false};
    bool enablePrint{true};
    bool enableBanShield{true};
    bool enableNagSuppress{true};
    bool summonUnlock{true};
    bool enableEnhancedAutopilotRuntime{true};
    bool speedProfileLocked{true};
    int  speedProfile{1};
    int  speedOffset{0};
    uint32_t  speedOffsetRaw{0};
    bool enableCamera{true};
    bool emergencyVehicleDetectionRuntime{true};

    uint8_t offsetType = 0; // 0=preset, 1=custom
    uint8_t h4oCustomSl[H4O_CUSTOM_COUNT] = {20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120};
    uint8_t h4oCustomV [H4O_CUSTOM_COUNT] = {60, 60, 50, 40, 33, 12, 11, 10,  10,   9,   8};

    // ------------------------------------

    // speed limit
    int speedLimit{0};
    int speedLimitVisionOnly{0};

    uint32_t frameCount{0};
    uint32_t framesSent{0};
    uint32_t banShieldCnt{0};
    uint32_t banShieldCheckCnt{0};

    // gateway autopilot
    int gatewayAutopilot{-1};

    void (*onFrame)(const CanFrame &) = nullptr;
    void (*onSend)(uint8_t mux, bool ok) = nullptr;

    virtual bool handleMessage(CanFrame &frame, CanDriver &driver) = 0;
    virtual const uint32_t *filterIds() const = 0;
    virtual uint8_t filterIdCount() const = 0;
    virtual ~CarManagerBase() = default;
};

struct HW4Handler : public CarManagerBase
{
    std::vector<uint32_t> ids = {880, 921, 1016, 1021, 2047, 838, 601};

    const uint32_t *filterIds() const override
    {
        return ids.data();
    }
    uint8_t filterIdCount() const override { return ids.size(); }

    bool handleMessage(CanFrame &frame, CanDriver &driver) override
    {
        bool should_send = false;

        if (onFrame)
            onFrame(frame);

        if (frame.id == 921)
        {
            if (frame.dlc < 8)
                return false;

            speedLimit = (frame.data[1] & 0x1F) * 5;
            // speedLimitVisionOnly = (frame.data[2] & 0x1F) * 5;

            return false;
        }

        if (frame.id == 601)
        {
            // setBit(frame, 51, true);
            // setBit(frame, 19, false);
            // setBit(frame, 20, false);
            //
            // uint32_t counter = (frame.data[6] >> 4) + 1;
            // counter += 1;
            //
            // frame.data[6] = (frame.data[6] & 0x0F) | (counter << 4);
            // frame.data[7] = computeVehicleChecksum(frame);

            should_send = false;
        }

        if (frame.id == 1016)
        {
            if (frame.dlc < 8)
                return false;

            if (!speedProfileLocked)
            {
                auto fd = (frame.data[5] & 0b11100000) >> 5;
                switch (fd)
                {
                case 1:
                    speedProfile = 3;
                    break;
                case 2:
                    speedProfile = 2;
                    break;
                case 3:
                    speedProfile = 1;
                    break;
                case 4:
                    speedProfile = 0;
                    break;
                case 5:
                    speedProfile = 4;
                    break;
                }
            }
        }

        if (frame.id == 2047)
        {
            if (frame.dlc < 8)
                return false;

            int32_t mux = frame.data[0] & 0x0F;
            if (mux == 2)
            {
                uint8_t next = readGTWAutopilot(frame);
                int prev = gatewayAutopilot;
                gatewayAutopilot = next;

                if (enablePrint && prev != next)
                {
                    char buf[LogRingBuffer::kMaxMsgLen];
                    snprintf(buf, sizeof(buf), "HW4Handler: GTW_autopilot: %d -> %u (%s)", prev, (unsigned int)next, describeGTWAutopilot(next));
                    logRing.push(buf,millis());
                    Serial.println(buf);
                }
            }

            if (enableBanShield && mux < (int32_t)(sizeof(m_gtw_protector) / sizeof(m_gtw_protector[0])))
            {
                auto& saved_frame  = m_gtw_protector[mux];
                bool is_same = true;
                for (int i = 0; i < 8; i++) {
                    if (saved_frame.data[i] != frame.data[i]) {
                        is_same = false;
                        break;
                    }
                }

                banShieldCheckCnt++;
                if (is_same)
                    return false;

                // Serial.printf("BanShield: 0x7FF: %d : NewFrame: %s, Shield: %s\n", mux, ToHexString(frame).c_str(), ToHexString(saved_frame).c_str());
                banShieldCnt++;
                should_send = true;
            }
        }

        if (frame.id == 1021)
        {
            if (frame.dlc < 8)
                return false;

            auto index = readMuxID(frame);
            if (index == 0 && ADEnabled)
            {
                setBit(frame, 46, true);

                if (!useHW3Code)
                    setBit(frame, 60, true);

                if (!useHW3Code && emergencyVehicleDetectionRuntime)
                    setBit(frame, 59, true);

                // china only logic
                {
                    uint8_t speed_profile_for_hw3 = 2;
                    switch (speedProfile)
                    {
                    case 0:
                    case 4:
                        speed_profile_for_hw3 = 0;
                        break;
                    case 1:
                        speed_profile_for_hw3 = 1;
                        break;
                    case 2:
                    case 3:
                        speed_profile_for_hw3 = 2;
                        break;
                    default:
                        speed_profile_for_hw3 = 2;
                        break;
                    }

                    frame.data[6] &= ~0x06;
                    frame.data[6] |= (speed_profile_for_hw3 << 1);
                }

                should_send = true;
            }

            if (index == 1)
            {
                bool modified = false;

                if (enableNagSuppress)
                {
                    setBit(frame, 19, false);
                }

                if (!useHW3Code && emergencyVehicleDetectionRuntime)
                {
                    setBit(frame, 47, true);
                    modified = true;
                }

                if (!enableCamera)
                {
                    setBit(frame, 43, false);
                    modified = true;
                }

                should_send = modified;
            }

            if (index == 2 && ADEnabled)
            {
                frame.data[7] &= ~(0x07 << 4);
                frame.data[7] |= (speedProfile & 0x07) << 4;

                speedOffsetRaw = speedOffset;
                if (offsetType == 1)
                    speedOffsetRaw = (uint8_t)CalcSpeedOffset();

                if (speedOffsetRaw > 0)
                {
                    if (!useHW3Code)
                        frame.data[1] = (frame.data[1] & 0xC0) | (speedOffsetRaw & 0x3F);
                    else
                    {
                        speedOffsetRaw = Rerange(Clamp(speedOffsetRaw, 0, 60), 0, 60, 0, 240);

                        frame.data[0] &= ~(0b11000000);
                        frame.data[1] &= ~(0b00111111);
                        // frame.data[0] |= (speedOffsetRaw & 0x03) << 6;
                        frame.data[1] |= (speedOffsetRaw >> 2);
                    }
                }

                should_send = true;
                speedLimitVisionOnly = frame.data[1];
            }

            if (index == 0 && enablePrint)
            {
                Serial.printf("HW4Handler: AD: %d, GTWAP: %s, Profile: %d, Injection: %d\n", (bool)ADEnabled, describeGTWAutopilot (gatewayAutopilot), speedProfile, InjectActive);
            }
        }

        return should_send;
    }

private:
    inline int
    Clamp(int value, int min, int max) {
        if (value < min)
            return min;
        if (value > max)
            return max;
        return value;
    }

    int
    Rerange(int value, int min, int max, int t_min, int t_max) {
        int t_len = t_max - t_min;
        int len = max - min;
        int p = (value - min) * 100 / len;
        return t_min + p * t_len / 100;
    }

    uint8_t
    CalcSpeedOffset() const
    {
        uint8_t result = 0;
        for (int i = 0; i < H4O_CUSTOM_COUNT; i++)
        {
            if (speedLimit >= h4oCustomSl[i])
            {
                result = h4oCustomV[i];
            }
        }

        return result;
    }

private:
    CanFrame m_gtw_protector[10] = {             //0          8          16         24         32         40         48         56
        {.id = 0x7FF, .dlc = 8, {0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000}}, // 0
        {.id = 0x7FF, .dlc = 8, {0b00000001,0b10100101,0b01001110,0b01000011,0b01000101,0b10100101,0b01010100,0b00000001}}, // 1
        {.id = 0x7FF, .dlc = 8, {0b00000010,0b01000111,0b01010110,0b01101101,0b10001110,/*FSD 2|2*/0b01001101,0b11000110,0b00100111}}, // 2
        {.id = 0x7FF, .dlc = 8, {0b00000011,0b00110011,0b00001100,0b01001101,0b11100001,0b00100000,0b00101000,0b00110100}}, // 3
        {.id = 0x7FF, .dlc = 8, {0b00000100,0b01110111,0b11000001,0b10010011,0b01101000,0b10101010,0b10111100,0b00110000}}, // 4
        {.id = 0x7FF, .dlc = 8, {0b00000101,0b00010010,0b00100000,0b01100001,0b01000000,0b00000100,0b10000000,0b01000010}}, // 5
        {.id = 0x7FF, .dlc = 8, {0b00000110,0b01011000,0b10110101,0b01011011,0b00000111,0b10110000,0b11001100,0b11001000}}, // 6
        {.id = 0x7FF, .dlc = 8, {0b00000111,0b00100110,0b00000000,0b10000101,0b00100000,0b00000100,0b00100011,0b01110000}}, // 7
        {.id = 0x7FF, .dlc = 8, {0b00001000,0b00000000,0b01000010,0b00000010,0b10010000,0b01000010,0b00010100,0b00000000}}, // 8
        {.id = 0x7FF, .dlc = 8, {0b00001001,0b11101111,0b00000000,0b00100000,0b00000000,0b10000000,0b00000000,0b00000000}}  // 9
    };

    // can_frame m_gtw_protector[10] = {
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x01,0xA5,0x4E,0x43,0x45,0xA5,0x54,0x01}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x02,0x47,0x56,0x6D,0x8E,0x4D,0xC6,0x27}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x03,0x33,0x0C,0x4D,0xE1,0x20,0x28,0x34}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x04,0x77,0xC1,0x93,0x68,0xAA,0xBC,0x30}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x05,0x12,0x20,0x61,0x40,0x04,0x80,0x42}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x06,0x58,0xB5,0x5B,0x07,0xB0,0xCC,0xC8}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x07,0x26,0x00,0x85,0x20,0x04,0x23,0x70}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x08,0x00,0x42,0x02,0x90,0x42,0x14,0x00}},
    //     {.can_id = 0x7FF, .can_dlc = 8, {0x09,0xEF,0x00,0x20,0x00,0x80,0x00,0x00}}
    // };
};
