#include "services/CalibrationManager.hpp"

#include "drivers/MPU6050Driver.hpp"
#include "services/EMGService.hpp"

#include "utils/CRC32.hpp"

#include "models/EventMessage.hpp"

#include "esp_log.h"
#include <cstring>

namespace Services
{
    static constexpr const char* TAG = "Calib";

    CalibrationManager::CalibrationManager(
        Interfaces::IPersistentStorage& storage,
        const Targets& targets,
        bool autoSaveOnComplete
    )
        : storage_(storage),
          targets_(targets),
          autoSave_(autoSaveOnComplete)
    {
    }

    uint32_t CalibrationManager::computeCRC(const Models::CalibrationData& d)
    {
        const size_t len = sizeof(Models::CalibrationData) - sizeof(uint32_t);
        return Utils::CRC32::compute(&d, len);
    }

    bool CalibrationManager::validate(const Models::CalibrationData& d) const
    {
        if (d.magic   != Models::CALIBRATION_MAGIC)   return false;
        if (d.version != Models::CALIBRATION_VERSION) return false;
        return d.crc == computeCRC(d);
    }

    bool CalibrationManager::initialize()
    {
        mutex_ = xSemaphoreCreateMutex();
        if (mutex_ == nullptr) return false;
        data_ = {};
        return true;
    }

    void CalibrationManager::applyToTargetsLocked()
    {
        if (data_.imuValid && targets_.imuDriver != nullptr)
        {
            targets_.imuDriver->setManualOffsets(
                data_.imu.ax, data_.imu.ay, data_.imu.az,
                data_.imu.gx, data_.imu.gy, data_.imu.gz
            );
        }

        if (data_.emgValid && targets_.emgSvc != nullptr)
        {
            targets_.emgSvc->setCalibration(
                data_.emg.baseline, data_.emg.peakNorm);
        }
    }

    void CalibrationManager::captureFromTargetsLocked()
    {
        // IMU: no exponemos getter de offsets en MPU6050Driver para no
        // romper encapsulación. Asumimos que ya se llamó loadAndApply()
        // antes y data_.imu refleja lo aplicado; o bien que el usuario
        // recién calibró y nosotros recibimos el evento → en ese caso
        // necesitamos leer los offsets. Por eso aquí simplemente
        // confiamos en lo que tengamos en data_. Si quieres capturar
        // tras un calibrate() en runtime, expón un getter en el driver
        // y léelo aquí. Para ahora, lo dejamos como lo que está en RAM.

        // EMG sí tiene getter:
        if (targets_.emgSvc != nullptr)
        {
            float b = 0.0f, p = 0.0f;
            targets_.emgSvc->getCalibration(b, p);
            data_.emg.baseline = b;
            data_.emg.peakNorm = p;
            data_.emgValid     = true;
        }
        
        if (targets_.imuDriver != nullptr && targets_.imuDriver->isCalibrated())
        {
            targets_.imuDriver->getOffsets(
                data_.imu.ax, data_.imu.ay, data_.imu.az,
                data_.imu.gx, data_.imu.gy, data_.imu.gz
            );
            data_.imuValid = true;
        }

    }

    bool CalibrationManager::loadAndApply()
    {
        if (mutex_ == nullptr || !storage_.isReady()) return false;

        Models::CalibrationData fromNvs {};
        const size_t got = storage_.readBlob(
            NVS_KEY, &fromNvs, sizeof(fromNvs));

        if (got != sizeof(fromNvs))
        {
            ESP_LOGI(TAG, "no calibration stored");
            return false;
        }
        if (!validate(fromNvs))
        {
            ESP_LOGW(TAG, "calibration corrupted → ignored");
            return false;
        }

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) != pdTRUE) return false;
        data_ = fromNvs;
        applyToTargetsLocked();
        xSemaphoreGive(mutex_);

        ESP_LOGI(TAG, "calibration loaded & applied (imu=%s emg=%s)",
                 data_.imuValid ? "Y" : "n",
                 data_.emgValid ? "Y" : "n");
        return data_.imuValid || data_.emgValid;
    }

    bool CalibrationManager::save()
    {
        if (mutex_ == nullptr || !storage_.isReady()) return false;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) != pdTRUE) return false;

        captureFromTargetsLocked();

        data_.magic   = Models::CALIBRATION_MAGIC;
        data_.version = Models::CALIBRATION_VERSION;
        data_.crc     = computeCRC(data_);

        const bool ok = storage_.writeBlob(NVS_KEY, &data_, sizeof(data_));
        xSemaphoreGive(mutex_);

        if (ok) ESP_LOGI(TAG, "calibration saved");
        else    ESP_LOGE(TAG, "calibration save FAILED");
        return ok;
    }

    bool CalibrationManager::reset()
    {
        if (mutex_ == nullptr || !storage_.isReady()) return false;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) != pdTRUE) return false;

        const bool erased = storage_.erase(NVS_KEY);
        data_ = {};
        xSemaphoreGive(mutex_);

        ESP_LOGI(TAG, "calibration reset (erased=%s)", erased ? "yes" : "no");
        return erased;
    }

    Models::CalibrationData CalibrationManager::snapshot() const
    {
        Models::CalibrationData copy {};
        if (mutex_ == nullptr) return copy;

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            copy = data_;
            xSemaphoreGive(mutex_);
        }
        return copy;
    }

    void CalibrationManager::onEvent(const Models::EventMessage& event)
    {
        if (!autoSave_) return;
        if (event.type != Models::EventType::CALIBRATION_COMPLETE) return;

        ESP_LOGI(TAG, "CALIBRATION_COMPLETE received → auto-saving");
        save();
    }
}