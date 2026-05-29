#include "protocols/PacketParser.hpp"
#include "protocols/CRC16.hpp"

namespace Protocols
{
    PacketParser::PacketParser() = default;

    void PacketParser::reset()
    {
        state_       = ParserState::WAIT_SOF;
        expectedLen_ = 0;
        payloadIdx_  = 0;
        crcReceived_ = 0;
        inFlight_    = ProtocolFrame {};
    }

    ParseResult PacketParser::feed(uint8_t byte, ProtocolFrame& outFrame)
    {
        switch (state_)
        {
            case ParserState::WAIT_SOF:
                if (byte == PROTO_SOF)
                {
                    inFlight_  = ProtocolFrame {};
                    state_     = ParserState::WAIT_LEN;
                }
                return ParseResult::NEED_MORE_DATA;

            case ParserState::WAIT_LEN:
                if (byte > PROTO_MAX_PAYLOAD)
                {
                    ++stats_.framesTooLong;
                    ++stats_.resyncs;
                    state_ = ParserState::WAIT_SOF;
                    return ParseResult::BAD_LENGTH;
                }
                expectedLen_       = byte;
                inFlight_.length   = byte;
                state_             = ParserState::WAIT_TYPE;
                return ParseResult::NEED_MORE_DATA;

            case ParserState::WAIT_TYPE:
                inFlight_.type = static_cast<MessageType>(byte);
                state_         = ParserState::WAIT_SEQ;
                return ParseResult::NEED_MORE_DATA;

            case ParserState::WAIT_SEQ:
                inFlight_.seq = byte;
                payloadIdx_   = 0;

                if (expectedLen_ == 0)
                {
                    state_ = ParserState::WAIT_CRC_LO;
                }
                else
                {
                    state_ = ParserState::WAIT_PAYLOAD;
                }
                return ParseResult::NEED_MORE_DATA;

            case ParserState::WAIT_PAYLOAD:
                inFlight_.payload[payloadIdx_++] = byte;
                if (payloadIdx_ >= expectedLen_)
                {
                    state_ = ParserState::WAIT_CRC_LO;
                }
                return ParseResult::NEED_MORE_DATA;

            case ParserState::WAIT_CRC_LO:
                crcReceived_ = byte;
                state_       = ParserState::WAIT_CRC_HI;
                return ParseResult::NEED_MORE_DATA;

            case ParserState::WAIT_CRC_HI:
                crcReceived_ |= static_cast<uint16_t>(byte) << 8;
                state_        = ParserState::WAIT_EOF;
                return ParseResult::NEED_MORE_DATA;

            case ParserState::WAIT_EOF:
            {
                if (byte != PROTO_EOF)
                {
                    ++stats_.framesBadEof;
                    ++stats_.resyncs;
                    state_ = ParserState::WAIT_SOF;
                    return ParseResult::BAD_FRAMING;
                }

                // Verificar CRC: armamos la región sobre la que se calculó.
                uint8_t buf[3 + PROTO_MAX_PAYLOAD];
                buf[0] = inFlight_.length;
                buf[1] = static_cast<uint8_t>(inFlight_.type);
                buf[2] = inFlight_.seq;
                for (uint8_t k = 0; k < inFlight_.length; ++k)
                {
                    buf[3 + k] = inFlight_.payload[k];
                }

                const uint16_t crcCalc = CRC16::compute(
                    buf, 3 + inFlight_.length
                );

                state_ = ParserState::WAIT_SOF;

                if (crcCalc != crcReceived_)
                {
                    ++stats_.framesBadCrc;
                    return ParseResult::BAD_CRC;
                }

                ++stats_.framesOk;
                outFrame = inFlight_;
                return ParseResult::FRAME_COMPLETE;
            }
        }

        // Inalcanzable, pero por defensa:
        state_ = ParserState::WAIT_SOF;
        return ParseResult::BAD_FRAMING;
    }
}