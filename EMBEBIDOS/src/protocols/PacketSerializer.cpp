#include "protocols/PacketSerializer.hpp"
#include "protocols/CRC16.hpp"

namespace Protocols
{
    size_t PacketSerializer::serialize(
        const ProtocolFrame& frame,
        uint8_t* outBuffer,
        size_t outBufferSize
    )
    {
        if (outBuffer == nullptr) return 0;
        if (frame.length > PROTO_MAX_PAYLOAD) return 0;

        const size_t totalSize = PROTO_OVERHEAD + frame.length;
        if (outBufferSize < totalSize) return 0;

        size_t i = 0;

        outBuffer[i++] = PROTO_SOF;
        outBuffer[i++] = frame.length;
        outBuffer[i++] = static_cast<uint8_t>(frame.type);
        outBuffer[i++] = frame.seq;

        for (uint8_t k = 0; k < frame.length; ++k)
        {
            outBuffer[i++] = frame.payload[k];
        }

        // CRC sobre LEN + TYPE + SEQ + PAYLOAD (todo excepto SOF, CRC y EOF)
        const uint16_t crc = CRC16::compute(
            &outBuffer[1],
            3 + frame.length
        );

        outBuffer[i++] = static_cast<uint8_t>(crc & 0xFF);          // CRC LO
        outBuffer[i++] = static_cast<uint8_t>((crc >> 8) & 0xFF);   // CRC HI
        outBuffer[i++] = PROTO_EOF;

        return i;
    }
}