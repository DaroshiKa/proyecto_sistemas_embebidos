#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/IEventListener.hpp"
#include "interfaces/IPersistentStorage.hpp"

#include "models/CalibrationData.hpp"

namespace Drivers   { class MPU6050Driver; }
namespace Services  { class EMGService; }

namespace Services
{
    // Carga, persiste y aplica calibraciones de sensores.
    // - IMU: offsets ax/ay/az/gx/gy/gz → MPU6050Driver::setManualOffsets()
    // - EMG: baseline + peakNorm       → EMGService::setCalibration()
    //
    // Política de auto-save:
    //   Si autoSaveOnComplete_ = true, se suscribe al EventBus y guarda
    //   calibración tras un evento CALIBRATION_COMPLETE.
    class CalibrationManager final : public Interfaces::IEventListener
    {
    public:
        struct Targets
        {
            Drivers::MPU6050Driver* imuDriver { nullptr };
            EMGService*             emgSvc    { nullptr };
        };

        CalibrationManager(
            Interfaces::IPersistentStorage& storage,
            const Targets& targets,
            bool autoSaveOnComplete = true
        );

        bool initialize();

        // Si hay datos válidos en NVS, los aplica a los targets.
        // Retorna true si al menos uno fue aplicado.
        bool loadAndApply();

        // Captura calibración actual desde los targets y persiste a NVS.
        bool save();

        // Borra el blob del NVS y resetea calibraciones en memoria.
        bool reset();

        // Snapshot RAM
        Models::CalibrationData snapshot() const;

        // IEventListener: si autoSave_ está activo, persiste tras
        // CALIBRATION_COMPLETE.
        void onEvent(const Models::EventMessage& event) override;

    private:
        static uint32_t computeCRC(const Models::CalibrationData& d);
        bool validate(const Models::CalibrationData& d) const;

        void captureFromTargetsLocked();
        void applyToTargetsLocked();

        Interfaces::IPersistentStorage& storage_;
        Targets                         targets_;
        bool                            autoSave_;

        mutable SemaphoreHandle_t       mutex_   { nullptr };
        Models::CalibrationData         data_    {};

        static constexpr const char* NVS_KEY = "cal.v1";
    };
}