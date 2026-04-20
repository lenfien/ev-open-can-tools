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

struct CarManagerBase
{
    Shared<int> speedProfile{1};
    Shared<bool> ADEnabled{false};
    Shared<int> gatewayAutopilot{-1};
    Shared<bool> enablePrint{true};
    Shared<uint32_t> frameCount{0};
    Shared<uint32_t> framesSent{0};
    Shared<uint32_t> banShieldCnt{0};
    Shared<uint32_t> banShieldCheckCnt{0};
    Shared<int> speedOffset{0};
    Shared<int> speedLimit{0};
    Shared<int> speedLimitVisionOnly{0};

    void (*onFrame)(const CanFrame &) = nullptr;
    void (*onSend)(uint8_t mux, bool ok) = nullptr;
    bool (*checkAD)() = nullptr;
    bool (*checkNag)() = nullptr;
    bool (*checkSummon)() = nullptr;
    bool (*checkIsa)() = nullptr;
    bool (*checkEvd)() = nullptr;

    virtual bool handleMessage(CanFrame &frame, CanDriver &driver) = 0;
    virtual const uint32_t *filterIds() const = 0;
    virtual uint8_t filterIdCount() const = 0;
    virtual ~CarManagerBase() = default;
};

struct HW4Handler : public CarManagerBase
{
    const uint32_t *filterIds() const override
    {
#if defined(ESP32_DASHBOARD)
        static constexpr uint32_t ids[] = {880, 921, 1016, 1021, 2047};
        return ids;
    }
    uint8_t filterIdCount() const override { return 5; }
#elif defined(ISA_SPEED_CHIME_SUPPRESS)
        static constexpr uint32_t ids[] = {921, 1016, 1021, 2047};
        return ids;
    }
    uint8_t filterIdCount() const override { return 4; }
#else
            static constexpr uint32_t ids[] = {1016, 1021, 2047};
            return ids;
        }
        uint8_t filterIdCount() const override { return 3; }
#endif

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
                    snprintf(buf, sizeof(buf), "HW4Handler: GTW_autopilot: %d -> %u (%s)",
                             prev, (unsigned int)next, describeGTWAutopilot(next));
                    logRing.push(buf,
    #ifndef NATIVE_BUILD
                                 millis()
    #else
                                 0
    #endif
                    );
#ifndef NATIVE_BUILD
                    Serial.println(buf);
#endif
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
            if (index == 0)
                ADEnabled = (!checkAD || checkAD());

            if (index == 0 && ADEnabled && (!checkAD || checkAD()))
            {
                setBit(frame, 46, true);
                setBit(frame, 60, true);

                if (emergencyVehicleDetectionRuntime)
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
                    }

                    frame.data[6] &= ~0x06;
                    frame.data[6] |= (speed_profile_for_hw3 << 1);
                }

                should_send = true;
            }

            if (index == 1)
            {
                bool modified = false;
                if (enhancedAutopilotRuntime)
                {
                    setBit(frame, 19, false);
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

            if (index == 2 && ADEnabled && (!checkAD || checkAD()))
            {
                frame.data[7] &= ~(0x07 << 4);
                frame.data[7] |= (speedProfile & 0x07) << 4;

                if (h4oTab == 0)
                    speedOffset = hw4OffsetRuntime;
                else
                    speedOffset = (uint8_t)GetSpeedOffset();

                if (speedOffset > 0)
                    frame.data[1] = (frame.data[1] & 0xC0) | (speedOffset & 0x3F);

                should_send = true;

                speedLimitVisionOnly = frame.data[1];
            }

            if (index == 0 && enablePrint)
            {
                char buf[LogRingBuffer::kMaxMsgLen];
                snprintf(buf, sizeof(buf), "HW4Handler: AD: %d, Profile: %d",
                         (bool)ADEnabled, (int)speedProfile);
                logRing.push(buf,
#ifndef NATIVE_BUILD
                             millis()
#else
                             0
#endif
                );
#ifndef NATIVE_BUILD
                Serial.println(buf);
#endif
            }
        }

        return should_send;
    }

private:
    uint8_t
    GetSpeedOffset() const
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

    // std::string
    // ToHexString(const CanFrame &frame) {
    //     std::ostringstream result;
    //     for (size_t i = 0; i < sizeof(frame.data); ++i) {
    //         result << "0x"
    //                 << std::hex
    //                 << std::uppercase // 可选：大写 A-F
    //                 << std::hex
    //                 << std::setw(2) // 宽度 2
    //                 << std::setfill('0')
    //                 << (int) frame.data[i];
    //
    //         if (i < sizeof(frame.data) - 1)
    //             result << ",";
    //     }
    //     return result.str();
    // }
    //
    // __attribute__((optimize("O3"))) std::string
    // ToBinaryString(uint8_t i) {
    //     std::string b;
    //     b.reserve(8);
    //     for (int index = 0; index < sizeof(i) * 8; index += 1)
    //         b += ((i << index) & 0b10000000) ? "1" : "0";
    //
    //     return b;
    // }
    //
    // __attribute__((optimize("O3"))) std::string
    // ToBinaryString(const CanFrame &frame) {
    //     std::ostringstream result;
    //     for (size_t i = 0; i < sizeof(frame.data); ++i)
    //         result << i * 8 << ":" << ToBinaryString(frame.data[i]) << ";";
    //
    //     return result.str();
    // }

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
