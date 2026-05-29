#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/IConfigService.hpp"
#include "interfaces/IPersistentStorage.hpp"

#include "models/PersistentConfig.hpp"
#include "models/IMUConfig.hpp"
#include "models/EMGConfig.hpp"
#include "models/MotionConfig.hpp"

namespace Services
{
    // Forward decls para evitar includes pesados.
    class IMUService;
    class EMGService;

    // Carga, persiste y aplica configuración. Es el único punto que
    // sabe cómo traducir entre PersistentConfig (subset) y los Configs
    // completos (IMUConfig/EMGConfig/MotionConfig).
    class ConfigService final : public Interfaces::IConfigService
    {
    public:
        // Targets a los que aplicará la config cuando se carga/cambia.
        // Todos opcionales: si nullptr, simplemente se ignora ese target.
        struct Targets
        {
            Models::IMUConfig*    imuCfg     { nullptr };
            Models::EMGConfig*    emgCfg     { nullptr };
            Models::MotionConfig* motionCfg  { nullptr };
            IMUService*           imuSvc     { nullptr };
            EMGService*           emgSvc     { nullptr };
        };

        ConfigService(
            Interfaces::IPersistentStorage& storage,
            const Targets& targets
        );

        // Llamar UNA VEZ en boot, tras instanciar los Configs y servicios.
        // - intenta cargar de NVS
        // - si falla, aplica defaults
        // - aplica resultado a Configs y servicios
        bool initialize();

        // IConfigService
        Models::PersistentConfig snapshot() const override;
        bool apply(const Models::PersistentConfig& cfg) override;
        bool save() override;
        bool reload() override;
        void resetToDefaults() override;

        bool isLoadedFromNvs() const { return loadedFromNvs_; }

    private:
        // Helpers internos
        static uint32_t computeCRC(const Models::PersistentConfig& cfg);
        bool validate(const Models::PersistentConfig& cfg) const;

        void applyToConfigsLocked();   // requiere mutex tomado

        Interfaces::IPersistentStorage& storage_;
        Targets                         targets_;

        mutable SemaphoreHandle_t       mutex_         { nullptr };
        Models::PersistentConfig        current_       {};
        bool                            loadedFromNvs_ { false };

        static constexpr const char* NVS_KEY = "cfg.v1";
    };
}