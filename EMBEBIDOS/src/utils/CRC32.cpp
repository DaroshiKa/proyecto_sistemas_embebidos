#include "utils/CRC32.hpp"

namespace Utils
{
    uint32_t CRC32::compute(const void* data, size_t len)
    {
        const uint8_t* p = static_cast<const uint8_t*>(data);
        uint32_t crc = 0xFFFFFFFFu;

        for (size_t i = 0; i < len; ++i)
        {
            crc ^= p[i];
            for (int b = 0; b < 8; ++b)
            {
                const uint32_t mask = -static_cast<int32_t>(crc & 1u);
                crc = (crc >> 1) ^ (0xEDB88320u & mask);
            }
        }
        return ~crc;
    }
}