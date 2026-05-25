#pragma once

#include <cstdint>

namespace Models
{
    enum class OrientationPlane : uint8_t
    {
        UNKNOWN = 0,
        XY,
        XZ,
        YZ
    };

    enum class EMGGesture : uint8_t
    {
        NONE = 0,
        RELAX,
        SINGLE_CONTRACTION,
        DOUBLE_CONTRACTION,
        LONG_HOLD
    };

    enum class EMGState : uint8_t
    {
        UNINITIALIZED = 0,
        INITIALIZING,
        CALIBRATING,
        OK,
        TIMEOUT,
        FAULT
    };

    struct EMGData
    {
        float    rawValue            { 0.0f };  // muestra cruda normalizada [0..1]
        float    filteredValue       { 0.0f };  // tras BPF (sin envolvente)
        float    envelopeValue       { 0.0f };  // envolvente suavizada [0..1]
        float    smoothedValue       { 0.0f };  // tras media móvil [0..1]
        bool     contractionDetected { false };
        EMGGesture gesture           { EMGGesture::NONE };
        uint32_t timestampMs         { 0 };
        bool     valid               { false };
    };

    struct IMUData
    {
        float ax { 0.0f };
        float ay { 0.0f };
        float az { 0.0f };

        float gx { 0.0f };
        float gy { 0.0f };
        float gz { 0.0f };

        float temperatureC { 0.0f };

        float pitch { 0.0f };
        float roll  { 0.0f };
        float yaw   { 0.0f };

        OrientationPlane plane { OrientationPlane::UNKNOWN };

        uint32_t timestampMs { 0 };

        bool valid { false };
    };
}