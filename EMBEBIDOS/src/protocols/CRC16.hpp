#pragma once

#include <cstdint>
#include <cstddef>

namespace Protocols
{
    class CRC16
    {
    public:
        // CRC16-CCITT / XMODEM:
        //   poly = 0x1021, init = 0xFFFF, no XOR final, sin reflexión
        static uint16_t compute(
            const uint8_t* data,
            size_t length,
            uint16_t initial = 0xFFFF
        );

        // Versión incremental para serializar/parsear streamings
        static uint16_t updateByte(uint16_t crc, uint8_t byte);
    };
}