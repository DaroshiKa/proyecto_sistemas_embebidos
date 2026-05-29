#pragma once

#include <cstdint>
#include "esp_adc/adc_oneshot.h"

namespace Models
{
    struct EMGConfig
    {
        // ----- Hardware -----
        adc_channel_t adcChannel       { ADC_CHANNEL_0 };  // GPIO36 (VP)
        adc_unit_t    adcUnit          { ADC_UNIT_1 };
        adc_atten_t   adcAttenuation   { ADC_ATTEN_DB_12 }; // 0..3.1V

        // ----- Muestreo -----
        uint32_t      sampleRateHz     { 20000 };
        uint32_t      dmaBufferBytes   { 2048 };
        uint32_t      dmaFrameBytes    { 256 };// 128 muestras de 16 bits
        // ----- Filtros (frecuencias en Hz) -----
        float         hpfCutoffHz      { 20.0f };
        float         lpfCutoffHz      { 450.0f };
        float         notchHz          { 50.0f };  // 60 Hz para zonas NTSC
        float         notchQ           { 30.0f };
        float         envelopeCutoffHz { 5.0f };
        uint16_t      movingAvgWindow  { 50 };     // 50 ms a 1 kHz

        // ----- Detector -----
        float         thresholdOn      { 0.35f };  // [0..1] sobre envolvente
        float         thresholdOff     { 0.20f };
        uint16_t      debounceMs       { 30 };

        // ----- FSM de gestos -----
        uint16_t      doublePulseWindowMs   { 400 };
        uint16_t      singlePulseMinMs      { 80 };
        uint16_t      longHoldMs            { 2000 };
        uint16_t      relaxMs               { 500 };

        // ----- Calibración -----
        uint16_t      calibrationSamples    { 2000 };  // 2 s a 1 kHz

        // ----- Watchdog -----
        uint32_t      readTimeoutMs         { 200 };
        uint8_t       maxConsecutiveFails   { 5 };
    };
}