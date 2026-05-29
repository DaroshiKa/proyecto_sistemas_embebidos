#pragma once

#include <cstdint>
#include "models/IMUConfig.hpp"
#include "models/EMGConfig.hpp"
#include "models/MotionConfig.hpp"

namespace Models
{
    // Subset persistible de IMUConfig.
    // NO guardamos todo: gpio pins, addr I2C, etc. son hardware-fijos.
    // Sólo guardamos lo que el usuario puede tunear runtime.
    struct PersistentIMU
    {
        uint8_t  gyroFs              { 0 };    // GyroFullScale as u8
        uint8_t  accelFs             { 0 };    // AccelFullScale as u8
        uint8_t  dlpf                { 3 };    // DLPFMode as u8
        uint8_t  sampleRateDiv       { 9 };
        float    complementaryAlpha  { 0.98f };
        float    planeThresholdG     { 0.7f };
        float    planeHysteresisG    { 0.1f };
        uint16_t calibrationSamples  { 1000 };
    };

    // Subset persistible de EMGConfig.
    struct PersistentEMG
    {
        float    thresholdOn         { 0.35f };
        float    thresholdOff        { 0.20f };
        uint16_t debounceMs          { 30 };
        uint16_t doublePulseWindowMs { 400 };
        uint16_t singlePulseMinMs    { 80 };
        uint16_t longHoldMs          { 2000 };
        uint16_t relaxMs             { 500 };
        uint16_t movingAvgWindow     { 50 };
        float    envelopeCutoffHz    { 5.0f };
        uint16_t calibrationSamples  { 2000 };
    };

    // Subset persistible de MotionConfig.
    struct PersistentMotion
    {
        float    defaultSpeedDps     { 90.0f };
        float    handOpenAngle       { 0.0f };
        float    handCloseAngle      { 180.0f };
        float    wristLeftAngle      { 30.0f };
        float    wristRightAngle     { 150.0f };
        float    elbowXyAngle        { 90.0f };
        float    elbowXzAngle        { 60.0f };
        float    elbowYzAngle        { 120.0f };
        // home angles persisten también: índice = JointId (5 joints)
        float    homeAngles[5]       { 90.0f, 90.0f, 90.0f, 90.0f, 90.0f };
    };

    // Versión del schema. Si tocas cualquier campo de arriba, INCREMENTAR.
    // El loader rechaza versiones distintas (carga defaults).
    static constexpr uint16_t PERSISTENT_CONFIG_VERSION = 0x0001;
    static constexpr uint32_t PERSISTENT_CONFIG_MAGIC   = 0x434E4647; // 'CNFG'

    struct PersistentConfig
    {
        // Header
        uint32_t magic    { PERSISTENT_CONFIG_MAGIC };
        uint16_t version  { PERSISTENT_CONFIG_VERSION };
        uint16_t _pad     { 0 };          // alineación

        // Payload
        PersistentIMU    imu    {};
        PersistentEMG    emg    {};
        PersistentMotion motion {};

        // CRC32 sobre todo lo anterior (header + payload, magic incluido,
        // crc excluido). Se computa al guardar y valida al cargar.
        uint32_t crc      { 0 };
    };
}