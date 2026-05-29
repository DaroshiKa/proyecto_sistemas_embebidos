#include "services/ConfigService.hpp"

#include "services/IMUService.hpp"
#include "services/EMGService.hpp"

#include "utils/CRC32.hpp"

#include "esp_log.h"
#include <cstring>

namespace Services
{
    static constexpr const char* TAG = "ConfigSvc";

    ConfigService::ConfigService(
        Interfaces::IPersistentStorage& storage,
        const Targets& targets
    )
        : storage_(storage),
          targets_(targets)
    {
    }

    uint32_t ConfigService::computeCRC(const Models::PersistentConfig& cfg)
    {
        // CRC sobre todo menos el propio campo crc (último uint32_t).
        const size_t len = sizeof(Models::PersistentConfig) - sizeof(uint32_t);
        return Utils::CRC32::compute(&cfg, len);
    }

    bool ConfigService::validate(const Models::PersistentConfig& cfg) const
    {
        if (cfg.magic != Models::PERSISTENT_CONFIG_MAGIC)
        {
            ESP_LOGW(TAG, "bad magic 0x%08lx",
                     static_cast<unsigned long>(cfg.magic));
            return false;
        }
        if (cfg.version != Models::PERSISTENT_CONFIG_VERSION)
        {
            ESP_LOGW(TAG, "version mismatch: nvs=%u expect=%u",
                     cfg.version, Models::PERSISTENT_CONFIG_VERSION);
            return false;
        }
        const uint32_t expected = computeCRC(cfg);
        if (cfg.crc != expected)
        {
            ESP_LOGW(TAG, "crc mismatch nvs=0x%08lx calc=0x%08lx",
                     static_cast<unsigned long>(cfg.crc),
                     static_cast<unsigned long>(expected));
            return false;
        }
        return true;
    }

    bool ConfigService::initialize()
    {
        mutex_ = xSemaphoreCreateMutex();
        if (mutex_ == nullptr) return false;

        // Defaults primero (por si no hay nada en NVS)
        current_ = {};
        // Si tenemos punteros, sembramos los defaults de los Configs
        // dentro de current_ ANTES de intentar leer de NVS.
        // (Los Configs ya traen valores buenos en su default ctor.)
        if (targets_.imuCfg != nullptr)
        {
            current_.imu.gyroFs             = static_cast<uint8_t>(targets_.imuCfg->gyroFs);
            current_.imu.accelFs            = static_cast<uint8_t>(targets_.imuCfg->accelFs);
            current_.imu.dlpf               = static_cast<uint8_t>(targets_.imuCfg->dlpf);
            current_.imu.sampleRateDiv      = targets_.imuCfg->sampleRateDiv;
            current_.imu.complementaryAlpha = targets_.imuCfg->complementaryAlpha;
            current_.imu.planeThresholdG    = targets_.imuCfg->planeThresholdG;
            current_.imu.planeHysteresisG   = targets_.imuCfg->planeHysteresisG;
            current_.imu.calibrationSamples = targets_.imuCfg->calibrationSamples;
        }
        if (targets_.emgCfg != nullptr)
        {
            current_.emg.thresholdOn         = targets_.emgCfg->thresholdOn;
            current_.emg.thresholdOff        = targets_.emgCfg->thresholdOff;
            current_.emg.debounceMs          = targets_.emgCfg->debounceMs;
            current_.emg.doublePulseWindowMs = targets_.emgCfg->doublePulseWindowMs;
            current_.emg.singlePulseMinMs    = targets_.emgCfg->singlePulseMinMs;
            current_.emg.longHoldMs          = targets_.emgCfg->longHoldMs;
            current_.emg.relaxMs             = targets_.emgCfg->relaxMs;
            current_.emg.movingAvgWindow     = targets_.emgCfg->movingAvgWindow;
            current_.emg.envelopeCutoffHz    = targets_.emgCfg->envelopeCutoffHz;
            current_.emg.calibrationSamples  = targets_.emgCfg->calibrationSamples;
        }
        if (targets_.motionCfg != nullptr)
        {
            current_.motion.defaultSpeedDps = targets_.motionCfg->defaultSpeedDps;
            current_.motion.handOpenAngle   = targets_.motionCfg->handOpenAngle;
            current_.motion.handCloseAngle  = targets_.motionCfg->handCloseAngle;
            current_.motion.wristLeftAngle  = targets_.motionCfg->wristLeftAngle;
            current_.motion.wristRightAngle = targets_.motionCfg->wristRightAngle;
            current_.motion.elbowXyAngle    = targets_.motionCfg->elbowXyAngle;
            current_.motion.elbowXzAngle    = targets_.motionCfg->elbowXzAngle;
            current_.motion.elbowYzAngle    = targets_.motionCfg->elbowYzAngle;
            for (size_t i = 0; i < 5; ++i)
            {
                current_.motion.homeAngles[i] =
                    targets_.motionCfg->joints[i].homeAngle;
            }
        }

        // Intento de carga desde NVS
        if (storage_.isReady())
        {
            Models::PersistentConfig fromNvs {};
            const size_t got = storage_.readBlob(
                NVS_KEY, &fromNvs, sizeof(fromNvs));

            if (got == sizeof(fromNvs) && validate(fromNvs))
            {
                current_       = fromNvs;
                loadedFromNvs_ = true;
                ESP_LOGI(TAG, "config loaded from NVS");
            }
            else if (got != 0)
            {
                ESP_LOGW(TAG, "stored config rejected → using defaults");
            }
            else
            {
                ESP_LOGI(TAG, "no stored config → defaults");
            }
        }
        else
        {
            ESP_LOGW(TAG, "storage not ready → defaults only");
        }

        // Aplicar (defaults o NVS) a los Configs/servicios
        if (xSemaphoreTake(mutex_, portMAX_DELAY) == pdTRUE)
        {
            applyToConfigsLocked();
            xSemaphoreGive(mutex_);
        }

        return true;
    }

    void ConfigService::applyToConfigsLocked()
    {
        // Aplicar IMU
        if (targets_.imuCfg != nullptr)
        {
            targets_.imuCfg->gyroFs        =
                static_cast<Models::GyroFullScale>(current_.imu.gyroFs);
            targets_.imuCfg->accelFs       =
                static_cast<Models::AccelFullScale>(current_.imu.accelFs);
            targets_.imuCfg->dlpf          =
                static_cast<Models::DLPFMode>(current_.imu.dlpf);
            targets_.imuCfg->sampleRateDiv      = current_.imu.sampleRateDiv;
            targets_.imuCfg->complementaryAlpha = current_.imu.complementaryAlpha;
            targets_.imuCfg->planeThresholdG    = current_.imu.planeThresholdG;
            targets_.imuCfg->planeHysteresisG   = current_.imu.planeHysteresisG;
            targets_.imuCfg->calibrationSamples = current_.imu.calibrationSamples;
        }

        // Aplicar EMG
        if (targets_.emgCfg != nullptr)
        {
            targets_.emgCfg->thresholdOn         = current_.emg.thresholdOn;
            targets_.emgCfg->thresholdOff        = current_.emg.thresholdOff;
            targets_.emgCfg->debounceMs          = current_.emg.debounceMs;
            targets_.emgCfg->doublePulseWindowMs = current_.emg.doublePulseWindowMs;
            targets_.emgCfg->singlePulseMinMs    = current_.emg.singlePulseMinMs;
            targets_.emgCfg->longHoldMs          = current_.emg.longHoldMs;
            targets_.emgCfg->relaxMs             = current_.emg.relaxMs;
            targets_.emgCfg->movingAvgWindow     = current_.emg.movingAvgWindow;
            targets_.emgCfg->envelopeCutoffHz    = current_.emg.envelopeCutoffHz;
            targets_.emgCfg->calibrationSamples  = current_.emg.calibrationSamples;
        }

        // EMG runtime: si el servicio ya está corriendo, aplicar umbrales
        // en caliente.
        if (targets_.emgSvc != nullptr)
        {
            targets_.emgSvc->setThresholds(
                current_.emg.thresholdOn,
                current_.emg.thresholdOff
            );
        }

        // Aplicar Motion
        if (targets_.motionCfg != nullptr)
        {
            targets_.motionCfg->defaultSpeedDps = current_.motion.defaultSpeedDps;
            targets_.motionCfg->handOpenAngle   = current_.motion.handOpenAngle;
            targets_.motionCfg->handCloseAngle  = current_.motion.handCloseAngle;
            targets_.motionCfg->wristLeftAngle  = current_.motion.wristLeftAngle;
            targets_.motionCfg->wristRightAngle = current_.motion.wristRightAngle;
            targets_.motionCfg->elbowXyAngle    = current_.motion.elbowXyAngle;
            targets_.motionCfg->elbowXzAngle    = current_.motion.elbowXzAngle;
            targets_.motionCfg->elbowYzAngle    = current_.motion.elbowYzAngle;
            for (size_t i = 0; i < 5; ++i)
            {
                targets_.motionCfg->joints[i].homeAngle =
                    current_.motion.homeAngles[i];
            }
        }
    }

    Models::PersistentConfig ConfigService::snapshot() const
    {
        Models::PersistentConfig copy {};
        if (mutex_ == nullptr) return copy;

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            copy = current_;
            xSemaphoreGive(mutex_);
        }
        return copy;
    }

    bool ConfigService::apply(const Models::PersistentConfig& cfg)
    {
        if (mutex_ == nullptr) return false;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) != pdTRUE) return false;

        // Aceptamos cualquier cfg con magic+version correctos.
        // CRC se recomputa al guardar.
        if (cfg.magic   != Models::PERSISTENT_CONFIG_MAGIC ||
            cfg.version != Models::PERSISTENT_CONFIG_VERSION)
        {
            xSemaphoreGive(mutex_);
            return false;
        }

        current_       = cfg;
        current_.crc   = computeCRC(current_);
        loadedFromNvs_ = false;

        applyToConfigsLocked();
        xSemaphoreGive(mutex_);
        ESP_LOGI(TAG, "config applied (RAM only — call save() to persist)");
        return true;
    }

    bool ConfigService::save()
    {
        if (mutex_ == nullptr || !storage_.isReady()) return false;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) != pdTRUE) return false;

        // Sellar header + CRC antes de escribir
        current_.magic   = Models::PERSISTENT_CONFIG_MAGIC;
        current_.version = Models::PERSISTENT_CONFIG_VERSION;
        current_.crc     = computeCRC(current_);

        const bool ok = storage_.writeBlob(
            NVS_KEY, &current_, sizeof(current_));

        xSemaphoreGive(mutex_);

        if (ok) ESP_LOGI(TAG, "config saved");
        else    ESP_LOGE(TAG, "config save FAILED");
        return ok;
    }

    bool ConfigService::reload()
    {
        if (mutex_ == nullptr || !storage_.isReady()) return false;

        Models::PersistentConfig fromNvs {};
        const size_t got = storage_.readBlob(
            NVS_KEY, &fromNvs, sizeof(fromNvs));
        if (got != sizeof(fromNvs))            return false;
        if (!validate(fromNvs))                return false;

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) != pdTRUE) return false;
        current_       = fromNvs;
        loadedFromNvs_ = true;
        applyToConfigsLocked();
        xSemaphoreGive(mutex_);

        ESP_LOGI(TAG, "config reloaded from NVS");
        return true;
    }

    void ConfigService::resetToDefaults()
    {
        if (mutex_ == nullptr) return;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) != pdTRUE) return;

        current_       = {};   // defaults del POD
        current_.crc   = computeCRC(current_);
        loadedFromNvs_ = false;

        applyToConfigsLocked();
        xSemaphoreGive(mutex_);
        ESP_LOGI(TAG, "defaults restored (not yet saved)");
    }
}