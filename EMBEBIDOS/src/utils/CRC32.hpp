#pragma once

#include <cstdint>
#include <cstddef>

namespace Utils
{
    // CRC32 / IEEE 802.3 / poly 0xEDB88320, init 0xFFFFFFFF, refIn/Out true,
    // xorOut 0xFFFFFFFF. Mismo CRC que zlib.
    class CRC32
    {
    public:
        static uint32_t compute(const void* data, size_t len);
    };
}