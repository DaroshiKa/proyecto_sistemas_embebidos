#pragma once

#include <cstdint>

#include "models/MotionCommand.hpp"

namespace Models
{
    enum class DisplayEventType : uint8_t
    {
        NONE = 0,

        // Comandos directos (botones que mapean a MotionType)
        MOTION_COMMAND,
        SERVO_COMMAND,

        // Eventos de UI puros (pueden requerir lógica en el firmware)
        BUTTON_PRESSED,
        SLIDER_CHANGED,

        // Consultas
        QUERY_STATUS,
        QUERY_CALIBRATION
    };

    struct DisplayEvent
    {
        DisplayEventType type        { DisplayEventType::NONE };
        uint32_t         timestampMs { 0 };

        // Para MOTION_COMMAND
        MotionType       motionType  { MotionType::NONE };

        // Para SERVO_COMMAND
        uint8_t          servoId     { 0 };
        float            angle       { 0.0f };

        // Para BUTTON_PRESSED / SLIDER_CHANGED
        uint8_t          pageId      { 0 };
        uint8_t          componentId { 0 };
        int16_t          value       { 0 };

        // Trazabilidad
        uint8_t          sequenceNumber { 0 };
        bool             requiresAck    { false };
    };
}