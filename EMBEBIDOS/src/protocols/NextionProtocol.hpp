#pragma once

#include <cstdint>
#include <cstddef>

namespace Protocols
{
    // Constantes del framing
    static constexpr uint8_t  PROTO_SOF = 0xAA;
    static constexpr uint8_t  PROTO_EOF = 0x55;
    static constexpr size_t   PROTO_HEADER_BYTES  = 4;  // SOF+LEN+TYPE+SEQ
    static constexpr size_t   PROTO_TRAILER_BYTES = 3;  // CRC16+EOF
    static constexpr size_t   PROTO_OVERHEAD      = PROTO_HEADER_BYTES + PROTO_TRAILER_BYTES;
    static constexpr size_t   PROTO_MAX_PAYLOAD   = 64;
    static constexpr size_t   PROTO_MAX_FRAME     = PROTO_OVERHEAD + PROTO_MAX_PAYLOAD;

    // Tipos de mensaje
    enum class MessageType : uint8_t
    {
        NONE              = 0x00,

        // Display → MCU (comandos y eventos)
        CMD_MOTION        = 0x01,
        CMD_SERVO         = 0x02,
        CMD_QUERY_STATUS  = 0x03,
        EVT_BUTTON        = 0x10,

        // MCU → Display (telemetría)
        TLM_IMU           = 0x20,
        TLM_EMG           = 0x21,
        TLM_SERVOS        = 0x22,
        TLM_SYSTEM        = 0x23,
        EVT_ALARM         = 0x30,

        // Bidireccional
        ACK               = 0x40,
        NACK              = 0x41
    };

    // Códigos de NACK
    enum class NackError : uint8_t
    {
        UNKNOWN         = 0,
        BAD_CRC         = 1,
        BAD_LENGTH      = 2,
        UNSUPPORTED     = 3,
        QUEUE_FULL      = 4,
        TIMEOUT         = 5,
        INTERNAL        = 6
    };

    // Frame "parseado" en memoria. El payload está copiado en el buffer.
    struct ProtocolFrame
    {
        MessageType type    { MessageType::NONE };
        uint8_t     seq     { 0 };
        uint8_t     length  { 0 };
        uint8_t     payload[PROTO_MAX_PAYLOAD] {};
    };
}