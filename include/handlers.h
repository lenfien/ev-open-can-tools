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
    Shared<int> speedOffset{0};
    Shared<uint32_t> banShieldCnt{0};
    Shared<uint32_t> banShieldCheckCnt{0};
    Shared<int> speedLimit{0};

    void (*onFrame)(const CanFrame &) = nullptr;
    void (*onSend)(uint8_t mux, bool ok) = nullptr;
    bool (*checkAD)() = nullptr;
    bool (*checkNag)() = nullptr;
    bool (*checkSummon)() = nullptr;
    bool (*checkIsa)() = nullptr;
    bool (*checkEvd)() = nullptr;

    virtual void handleMessage(CanFrame &frame, CanDriver &driver) = 0;
    virtual const uint32_t *filterIds() const = 0;
    virtual uint8_t filterIdCount() const = 0;
    virtual ~CarManagerBase() = default;
};

struct LegacyHandler : public CarManagerBase
{
    const uint32_t *filterIds() const override
    {
        static constexpr uint32_t ids[] = {69, 1006};
        return ids;
    }
    uint8_t filterIdCount() const override { return 2; }

    void handleMessage(CanFrame &frame, CanDriver &driver) override
    {
        if (onFrame)
            onFrame(frame);
        // STW_ACTN_RQ (0x045 = 69): Follow-Distance-Stalk as Source for Profile Mapping
        // byte[1]: 0x00=Pos1, 0x21=Pos2, 0x42=Pos3, 0x64=Pos4, 0x85=Pos5, 0xA6=Pos6, 0xC8=Pos7
        if (frame.id == 69)
        {
            if (frame.dlc < 2)
                return;
            uint8_t pos = frame.data[1] >> 5;
            if (pos <= 1)
                speedProfile = 2;
            else if (pos == 2)
                speedProfile = 1;
            else
                speedProfile = 0;
            return;
        }
        if (frame.id == 1006)
        {
            if (frame.dlc < 8)
                return;
            auto index = readMuxID(frame);
            if (index == 0)
                ADEnabled = isADSelectedInUI(frame) && (!checkAD || checkAD());
            if (index == 0 && ADEnabled && (!checkAD || checkAD()))
            {
                setBit(frame, 46, true);
                setSpeedProfileV12V13(frame, speedProfile);
                framesSent++;
                driver.send(frame);
                if (onSend)
                    onSend(0, true);
            }
            if (index == 1 && (!checkNag || checkNag()))
            {
                setBit(frame, 19, false);
                framesSent++;
                driver.send(frame);
                if (onSend)
                    onSend(1, true);
            }
            if (index == 0 && enablePrint)
            {
                char buf[LogRingBuffer::kMaxMsgLen];
                snprintf(buf, sizeof(buf), "LegacyHandler: AD: %d, Profile: %d",
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
    }
};

struct HW3Handler : public CarManagerBase
{
    const uint32_t *filterIds() const override
    {
#if defined(ESP32_DASHBOARD)
        static constexpr uint32_t ids[] = {787, 880, 1016, 1021, 2047};
        return ids;
    }
    uint8_t filterIdCount() const override { return 5; }
#else
        static constexpr uint32_t ids[] = {787, 1016, 1021, 2047};
        return ids;
    }
    uint8_t filterIdCount() const override { return 4; }
#endif

    void handleMessage(CanFrame &frame, CanDriver &driver) override
    {
        if (onFrame)
            onFrame(frame);
#if defined(ESP32_DASHBOARD)
        if (nagKillerRuntime && frame.id == 880 && frame.dlc >= 8)
        {
            if ((frame.data[4] >> 6 & 0x03) == 0)
            {
                CanFrame echo;
                echo.id = 880;
                echo.dlc = 8;
                echo.data[0] = frame.data[0];
                echo.data[1] = frame.data[1];
                echo.data[2] = (frame.data[2] & 0xF0) | 0x08;
                echo.data[3] = 0xB6;
                echo.data[4] = frame.data[4] | 0x40;
                echo.data[5] = frame.data[5];
                echo.data[6] = (frame.data[6] & 0xF0) | ((frame.data[6] + 1) & 0x0F);
                uint16_t s = echo.data[0] + echo.data[1] + echo.data[2] + echo.data[3] + echo.data[4] + echo.data[5] + echo.data[6];
                echo.data[7] = static_cast<uint8_t>((s + 0x73) & 0xFF);
                framesSent++;
                driver.send(echo);
                if (onSend)
                    onSend(3, true);
            }
            return;
        }
#endif
        if (frame.id == 787)
        {
            if (frame.dlc < 8)
                return;
            setTrackModeRequest(frame, kTrackModeRequestOn);
            frame.data[7] = computeVehicleChecksum(frame);
            framesSent++;
            driver.send(frame);
            return;
        }
        if (frame.id == 1016)
        {
            if (frame.dlc < 6)
                return;
            if (!speedProfileLocked)
            {
                uint8_t followDistance = (frame.data[5] & 0b11100000) >> 5;
                switch (followDistance)
                {
                case 1:
                    speedProfile = 2;
                    break;
                case 2:
                    speedProfile = 1;
                    break;
                case 3:
                    speedProfile = 0;
                    break;
                default:
                    break;
                }
            }
            return;
        }
        if (frame.id == 2047)
        {
            if (frame.dlc < 6)
                return;
            if (readMuxID(frame) != 2)
                return;

            uint8_t next = readGTWAutopilot(frame);
            int prev = gatewayAutopilot;
            gatewayAutopilot = next;

            if (enablePrint && prev != next)
            {
                char buf[LogRingBuffer::kMaxMsgLen];
                snprintf(buf, sizeof(buf), "HW3Handler: GTW_autopilot: %d -> %u (%s)",
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
            return;
        }
        if (frame.id == 1021)
        {
            if (frame.dlc < 8)
                return;
            auto index = readMuxID(frame);
            if (index == 0)
                ADEnabled = isADSelectedInUI(frame) && (!checkAD || checkAD());
            if (index == 0 && ADEnabled && (!checkAD || checkAD()))
            {
                speedOffset = std::max(std::min(((uint8_t)((frame.data[3] >> 1) & 0x3F) - 30) * 5, 100), 0);
                setBit(frame, 46, true);
                setSpeedProfileV12V13(frame, speedProfile);
                framesSent++;
                driver.send(frame);
                if (onSend)
                    onSend(0, true);
            }
            if (index == 1)
            {
                bool modified = false;
#if defined(ENHANCED_AUTOPILOT) || defined(ESP32_DASHBOARD)
                if (enhancedAutopilotRuntime)
                {
                    setBit(frame, 19, false);
                    setBit(frame, 46, true);
                    modified = true;
                }
#else
                // No dashboard: nag suppression always-on (RP2040, M4)
                setBit(frame, 19, false);
                modified = true;
#endif
                if (modified)
                {
                    framesSent++;
                    driver.send(frame);
                    if (onSend)
                        onSend(1, true);
                }
            }
            if (index == 2 && ADEnabled && (!checkAD || checkAD()))
            {
                frame.data[0] &= ~(0b11000000);
                frame.data[1] &= ~(0b00111111);
                frame.data[0] |= (speedOffset & 0x03) << 6;
                frame.data[1] |= (speedOffset >> 2);
                framesSent++;
                driver.send(frame);
                if (onSend)
                    onSend(2, true);
            }
            if (index == 0 && enablePrint)
            {
                char buf[LogRingBuffer::kMaxMsgLen];
                snprintf(buf, sizeof(buf), "HW3Handler: AD: %d, Profile: %d, Offset: %d",
                         (bool)ADEnabled, (int)speedProfile, (int)speedOffset);
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
    }
};

/**
 * NagHandler — Autosteer nag suppression (counter+1 echo method)
 *
 * Replicates the Chinese TSL6P module behavior:
 * - Listens for CAN 880 (0x370) = EPAS3P_sysStatus
 * - When handsOnLevel = 0 (nag would trigger):
 *   1. Copies the real frame
 *   2. Sets byte 3 = 0xB6 (fixed torsionBarTorque = 1.80 Nm)
 *   3. Sets byte 4 |= 0x40 (handsOnLevel = 1)
 *   4. Increments counter (byte 6 lower nibble + 1)
 *   5. Recalculates checksum (byte 7)
 * - The real EPAS frame with the same counter arrives AFTER -> rejected as duplicate
 *
 * Tested: Model Y Performance 2022 HW3, Basic Autopilot
 * Bus: X179 pin 2/3 (CAN bus 4)
 *
 * Enable with build flag: -D NAG_KILLER
 */
struct NagHandler : public CarManagerBase
{
    Shared<bool> nagKillerActive{true};
    Shared<uint32_t> nagEchoCount{0};

    const uint32_t *filterIds() const override
    {
        static constexpr uint32_t ids[] = {880};
        return ids;
    }
    uint8_t filterIdCount() const override { return 1; }

    void handleMessage(CanFrame &frame, CanDriver &driver) override
    {
        if (frame.id != 880 || frame.dlc < 8)
            return;

        uint8_t handsOn = (frame.data[4] >> 6) & 0x03;

        if (!nagKillerActive || !nagKillerRuntime || handsOn != 0)
            return;

        CanFrame echo;
        echo.id = 880;
        echo.dlc = 8;

        echo.data[0] = frame.data[0];
        echo.data[1] = frame.data[1];
        echo.data[2] = (frame.data[2] & 0xF0) | 0x08;
        echo.data[5] = frame.data[5];

        // Fixed torque = 1.80 Nm (tRaw = 0x08B6)
        echo.data[3] = 0xB6;

        // handsOnLevel = 1
        echo.data[4] = frame.data[4] | 0x40;

        // Counter + 1
        uint8_t cnt = (frame.data[6] & 0x0F);
        cnt = (cnt + 1) & 0x0F;
        echo.data[6] = (frame.data[6] & 0xF0) | cnt;

        // Checksum: sum(byte0..byte6) + 0x73
        uint16_t sum = echo.data[0] + echo.data[1] + echo.data[2] + echo.data[3] + echo.data[4] + echo.data[5] + echo.data[6];
        echo.data[7] = static_cast<uint8_t>((sum + 0x73) & 0xFF);

        framesSent++;
        nagEchoCount++;
        driver.send(echo);

        if (enablePrint && (nagEchoCount % 500 == 1))
        {
            char buf[LogRingBuffer::kMaxMsgLen];
            snprintf(buf, sizeof(buf), "NagHandler: echo=%u",
                     (unsigned int)(uint32_t)nagEchoCount);
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

    void handleMessage(CanFrame &frame, CanDriver &driver) override
    {
        if (onFrame)
            onFrame(frame);
#if defined(ESP32_DASHBOARD)
        if (nagKillerRuntime && frame.id == 880 && frame.dlc >= 8)
        {
            if ((frame.data[4] >> 6 & 0x03) == 0)
            {
                CanFrame echo;
                echo.id = 880;
                echo.dlc = 8;
                echo.data[0] = frame.data[0];
                echo.data[1] = frame.data[1];
                echo.data[2] = (frame.data[2] & 0xF0) | 0x08;
                echo.data[3] = 0xB6;
                echo.data[4] = frame.data[4] | 0x40;
                echo.data[5] = frame.data[5];
                echo.data[6] = (frame.data[6] & 0xF0) | ((frame.data[6] + 1) & 0x0F);
                uint16_t s = echo.data[0] + echo.data[1] + echo.data[2] + echo.data[3] + echo.data[4] + echo.data[5] + echo.data[6];
                echo.data[7] = static_cast<uint8_t>((s + 0x73) & 0xFF);
                framesSent++;
                driver.send(echo);
                if (onSend)
                    onSend(3, true);
            }
            return;
        }
#endif


#if defined(ISA_SPEED_CHIME_SUPPRESS) || defined(ESP32_DASHBOARD)

        if (frame.id == 921)
        {
            if (frame.dlc < 8)
                return;

            speedLimit = (frame.data[1] & 0x1F) * 5;

            if (!isaSpeedChimeSuppressRuntime)
                return;
            frame.data[1] |= 0x20;
            uint8_t sum = 0;
            for (int i = 0; i < 7; i++)
                sum += frame.data[i];
            sum += (921 & 0xFF) + (921 >> 8);
            frame.data[7] = sum & 0xFF;
            framesSent++;
            driver.send(frame);
            if (onSend)
                onSend(0, true);
            return;
        }
#endif
        if (frame.id == 1016)
        {
            if (frame.dlc < 6)
                return;
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
            if (frame.dlc < 6)
                return;

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
                    return;

                // Serial.printf("BanShield: 0x7FF: %d : NewFrame: %s, Shield: %s\n", mux, ToHexString(frame).c_str(), ToHexString(saved_frame).c_str());
                framesSent++;
                banShieldCnt++;

                driver.send(saved_frame);
                if (onSend)
                    onSend(mux, true);
            }

            return;
        }
        if (frame.id == 1021)
        {
            if (frame.dlc < 8)
                return;
            auto index = readMuxID(frame);
            if (index == 0)
                ADEnabled = isADSelectedInUI(frame) && (!checkAD || checkAD());
            if (index == 0 && ADEnabled && (!checkAD || checkAD()))
            {
                setBit(frame, 46, true);
                setBit(frame, 60, true);
#if defined(EMERGENCY_VEHICLE_DETECTION) || defined(ESP32_DASHBOARD)
                if (emergencyVehicleDetectionRuntime)
                    setBit(frame, 59, true);
#endif
                framesSent++;
                driver.send(frame);
                if (onSend)
                    onSend(0, true);
            }
            if (index == 1)
            {
                bool modified = false;
#if defined(ENHANCED_AUTOPILOT) || defined(ESP32_DASHBOARD)
                if (enhancedAutopilotRuntime)
                {
                    setBit(frame, 19, false);
                    setBit(frame, 47, true);
                    modified = true;
                }
#else
                // No dashboard: nag suppression always-on (RP2040, M4)
                setBit(frame, 19, false);
                setBit(frame, 47, true);
                modified = true;
#endif

                if (!enableCamera)
                    setBit(frame, 43, false);

                if (modified)
                {
                    framesSent++;
                    driver.send(frame);
                    if (onSend)
                        onSend(1, true);
                }
            }
            if (index == 2 && ADEnabled && (!checkAD || checkAD()))
            {
                frame.data[7] &= ~(0x07 << 4);
                frame.data[7] |= (speedProfile & 0x07) << 4;

                uint8_t off = 0;
                if (h4oTab == 0)
                    off = hw4OffsetRuntime;
                else
                    off = (uint8_t)GetSpeedOffset();

                if (off > 0)
                    frame.data[1] = (frame.data[1] & 0xC0) | (off & 0x3F);
                framesSent++;
                driver.send(frame);
                if (onSend)
                    onSend(2, true);
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
