#pragma once

#include "protocols/NextionProtocol.hpp"

namespace Protocols
{
    class PacketSerializer
    {
    public:
        // Serializa un frame al buffer de salida. Devuelve el número de bytes
        // escritos o 0 si el buffer es demasiado pequeño o el frame es inválido.
        static size_t serialize(
            const ProtocolFrame& frame,
            uint8_t* outBuffer,
            size_t outBufferSize
        );
    };
}