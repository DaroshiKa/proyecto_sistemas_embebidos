#pragma once

#include "protocols/NextionProtocol.hpp"

namespace Protocols
{
    enum class ParserState : uint8_t
    {
        WAIT_SOF = 0,
        WAIT_LEN,
        WAIT_TYPE,
        WAIT_SEQ,
        WAIT_PAYLOAD,
        WAIT_CRC_LO,
        WAIT_CRC_HI,
        WAIT_EOF
    };

    enum class ParseResult : uint8_t
    {
        NEED_MORE_DATA = 0,
        FRAME_COMPLETE,
        BAD_CRC,
        BAD_FRAMING,
        BAD_LENGTH
    };

    struct ParserStats
    {
        uint32_t framesOk     { 0 };
        uint32_t framesBadCrc { 0 };
        uint32_t framesBadEof { 0 };
        uint32_t framesTooLong{ 0 };
        uint32_t resyncs      { 0 };
    };

    class PacketParser
    {
    public:
        PacketParser();

        void reset();

        // Procesa un byte recibido. Cuando un frame queda completo
        // devuelve FRAME_COMPLETE y `outFrame` queda válido.
        ParseResult feed(
            uint8_t byte,
            ProtocolFrame& outFrame
        );

        // Procesa un buffer entero. Cada vez que un frame quede completo,
        // invoca el callback. Devuelve cuántos frames OK se procesaron.
        // Útil cuando UART devuelve un bloque de N bytes.
        template <typename Callback>
        size_t feedBuffer(
            const uint8_t* buffer,
            size_t length,
            Callback&& onFrame
        );

        ParserState  state() const { return state_; }
        ParserStats  stats() const { return stats_; }

    private:
        ParserState   state_       { ParserState::WAIT_SOF };
        uint8_t       expectedLen_ { 0 };
        uint8_t       payloadIdx_  { 0 };
        uint16_t      crcReceived_ { 0 };
        ProtocolFrame inFlight_    {};
        ParserStats   stats_       {};
    };

    template <typename Callback>
    size_t PacketParser::feedBuffer(
        const uint8_t* buffer,
        size_t length,
        Callback&& onFrame
    )
    {
        if (buffer == nullptr) return 0;

        size_t ok = 0;
        ProtocolFrame frame {};

        for (size_t i = 0; i < length; ++i)
        {
            const ParseResult r = feed(buffer[i], frame);
            if (r == ParseResult::FRAME_COMPLETE)
            {
                onFrame(frame);
                ++ok;
            }
        }
        return ok;
    }
}