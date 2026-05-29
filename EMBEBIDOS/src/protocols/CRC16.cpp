#include "protocols/CRC16.hpp"

namespace Protocols
{
    uint16_t CRC16::updateByte(uint16_t crc, uint8_t byte)
    {
        crc ^= static_cast<uint16_t>(byte) << 8;

        for (int i = 0; i < 8; ++i)
        {
            if (crc & 0x8000)
            {
                crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
            }
            else
            {
                crc = static_cast<uint16_t>(crc << 1);
            }
        }

        return crc;
    }

    uint16_t CRC16::compute(
        const uint8_t* data,
        size_t length,
        uint16_t initial
    )
    {
        if (data == nullptr) return initial;

        uint16_t crc = initial;
        for (size_t i = 0; i < length; ++i)
        {
            crc = updateByte(crc, data[i]);
        }
        return crc;
    }
}