#pragma once

#include <cstring>

struct CanFrame
{
    uint32_t id = 0;
    uint8_t dlc = 8;
    uint8_t data[8] = {};

public:
    inline uint8_t
    GetMux(uint32_t mux_length = 3) {
        switch (mux_length) {
            case 4:
                return data[0] & 0x0F;
            default:
                return data[0] & 0x07;
        }
    }

    void
    SetBit(int bit, bool value) {
        if (bit < 0 || bit >= 64)
            return; // bounds guard: CanFrame.data is 8 bytes

        int byteIndex = bit / 8;
        int bitIndex = bit % 8;
        uint8_t mask = static_cast<uint8_t>(1U << bitIndex);
        if (value)
            data[byteIndex] |= mask;
        else
            data[byteIndex] &= static_cast<uint8_t>(~mask);
    }
};
