#include "services/IMUService.hpp"

#include "esp_log.h"
#include "esp_timer.h"

#include "models/MotionCommand.hpp"
#include "models/MotionTypes.hpp"

namespace Services
{
    static constexpr const char* TAG = "IMUService";

    IMUService::IMUService(
        Drivers::MPU6050Driver& driver,
        Core::EventBus& eventBus,
        const Models::IMUConfig& config
    )
        : driver_(driver),
          eventBus_(eventBus),
          config_(config),
          filter_(config.complementaryAlpha),
          planeDetector_(config.planeThresholdG, config.planeHysteresisG)
    {
    }

    void IMUService::attachCommandDispatcher(
        Interfaces::ICommandDispatcher* dispatcher
    )
    {
        dispatcher_ = dispatcher;
    }

    void IMUService::setState(
        Models::IMUState state
    )
    {
        status_.state = state;
    }

    bool IMUService::initialize()
    {
        mutex_ = xSemaphoreCreateMutex();

        if (mutex_ == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create mutex");

            return false;
        }

        setState(Models::IMUState::INITIALIZING);

        if (!driver_.initialize(config_))
        {
            setState(Models::IMUState::BUS_ERROR);

            ESP_LOGE(TAG, "Driver initialize failed");

            return false;
        }

        filter_.reset();
        planeDetector_.reset();

        lastUpdateTickUs_ = static_cast<uint32_t>(esp_timer_get_time());

        setState(Models::IMUState::OK);

        ESP_LOGI(TAG, "IMUService ready");

        return true;
    }

    void IMUService::update()
    {
        if (status_.state == Models::IMUState::UNINITIALIZED ||
            status_.state == Models::IMUState::CALIBRATING)
        {
            // Nada que hacer; durante la calibración el control está en startCalibration()
            return;
        }

        Drivers::MPU6050Raw raw {};

        if (!driver_.readRaw(raw))
        {
            ++status_.consecutiveFails;
            ++status_.totalErrors;

            if (status_.consecutiveFails >= config_.maxConsecutiveFails)
            {
                if (status_.state != Models::IMUState::TIMEOUT)
                {
                    ESP_LOGW(TAG, "IMU TIMEOUT (%lu fails)",
                             static_cast<unsigned long>(status_.consecutiveFails));

                    setState(Models::IMUState::TIMEOUT);

                    Models::EventMessage evt {};
                    evt.type        = Models::EventType::SYSTEM_ERROR;
                    evt.timestampMs =
                        static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

                    eventBus_.publish(evt);
                }
            }

            return;
        }

        // Lectura OK
        status_.consecutiveFails = 0;
        ++status_.totalSamples;

        if (status_.state == Models::IMUState::TIMEOUT ||
            status_.state == Models::IMUState::BUS_ERROR ||
            status_.state == Models::IMUState::FAULT)
        {
            ESP_LOGI(TAG, "IMU recovered");
            setState(Models::IMUState::OK);
        }

        // Conversión física
        float ax, ay, az, gx, gy, gz, tC;

        driver_.convertToPhysical(raw, ax, ay, az, gx, gy, gz, tC);
        driver_.applyOffsets(ax, ay, az, gx, gy, gz);

        // dt real
        const uint32_t nowUs =
            static_cast<uint32_t>(esp_timer_get_time());

        float dt =
            static_cast<float>(nowUs - lastUpdateTickUs_) / 1.0e6f;

        lastUpdateTickUs_ = nowUs;

        // Clamp de dt: evita saltos enormes (p.ej. tras una pausa de debug)
        if (dt <= 0.0f || dt > 0.1f)
        {
            dt = 0.01f;
        }

        // Filtro complementario
        filter_.update(ax, ay, az, gx, gy, gz, dt);

        // Detección de plano (usa aceleraciones ya corregidas por offset)
        const Models::OrientationPlane plane =
            planeDetector_.detect(ax, ay, az);

        // Cachear data
        Models::IMUData data {};
        data.ax = ax;
        data.ay = ay;
        data.az = az;
        data.gx = gx;
        data.gy = gy;
        data.gz = gz;
        data.temperatureC = tC;
        data.pitch = filter_.pitch();
        data.roll  = filter_.roll();
        data.yaw   = filter_.yaw();
        data.plane = plane;
        data.timestampMs = static_cast<uint32_t>(nowUs / 1000ULL);
        data.valid = true;

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(2)) == pdTRUE)
        {
            lastData_ = data;
            status_.lastUpdateMs = data.timestampMs;
            status_.calibrated   = driver_.isCalibrated();

            xSemaphoreGive(mutex_);
        }

        // Publicar evento de sensor actualizado
        publishSensorUpdated();

        // Comando de movimiento opcional: sólo si cambia el plano dominante
        emitMotionCommandIfPlaneChanged(plane);
    }

    void IMUService::publishSensorUpdated()
    {
        Models::EventMessage evt {};
        evt.type        = Models::EventType::SENSOR_UPDATED;
        evt.timestampMs = status_.lastUpdateMs;

        // Nota: el puntero se mantiene válido porque lastData_ vive en el servicio
        // y el modelo de pub/sub es síncrono (los listeners reciben en el mismo
        // hilo y deben copiar lo que necesiten antes de retornar).
        evt.data = static_cast<void*>(&lastData_);

        eventBus_.publish(evt);
    }

    void IMUService::emitMotionCommandIfPlaneChanged(
        Models::OrientationPlane newPlane
    )
    {
        if (dispatcher_ == nullptr)
        {
            return;
        }

        if (newPlane == lastPublishedPlane_ ||
            newPlane == Models::OrientationPlane::UNKNOWN)
        {
            return;
        }

        Models::MotionCommand cmd {};
        cmd.source      = Models::CommandSource::IMU;
        cmd.priority    = Models::CommandPriority::NORMAL;
        cmd.timestampMs = status_.lastUpdateMs;
        cmd.requiresAck = false;

        switch (newPlane)
        {
            case Models::OrientationPlane::XY:
                cmd.type = Models::MotionType::ELBOW_XY;
                break;
            case Models::OrientationPlane::XZ:
                cmd.type = Models::MotionType::ELBOW_XZ;
                break;
            case Models::OrientationPlane::YZ:
                cmd.type = Models::MotionType::ELBOW_YZ;
                break;
            default:
                return;
        }

        // El dispatcher hará pasar el comando por el validador de seguridad.
        // El servicio NO toca servos.
        if (dispatcher_->dispatch(cmd))
        {
            lastPublishedPlane_ = newPlane;
        }
    }

    bool IMUService::getLatestData(
        Models::IMUData& outData
    ) const
    {
        if (mutex_ == nullptr)
        {
            return false;
        }

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(5)) != pdTRUE)
        {
            return false;
        }

        outData = lastData_;

        xSemaphoreGive(mutex_);

        return outData.valid;
    }

    Models::IMUStatus IMUService::getStatus() const
    {
        Models::IMUStatus copy {};

        if (mutex_ != nullptr &&
            xSemaphoreTake(mutex_, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            copy = status_;
            xSemaphoreGive(mutex_);
        }
        else
        {
            copy = status_;
        }

        return copy;
    }

    bool IMUService::isCalibrated() const
    {
        return driver_.isCalibrated();
    }

    bool IMUService::startCalibration()
    {
        if (status_.state == Models::IMUState::UNINITIALIZED ||
            status_.state == Models::IMUState::BUS_ERROR)
        {
            return false;
        }

        const Models::IMUState previous = status_.state;

        setState(Models::IMUState::CALIBRATING);

        ESP_LOGI(TAG, "Calibration requested");

        const bool ok = driver_.calibrate(config_.calibrationSamples, 5);

        if (ok)
        {
            filter_.reset();
            planeDetector_.reset();

            setState(Models::IMUState::OK);

            Models::EventMessage evt {};
            evt.type        = Models::EventType::CALIBRATION_COMPLETE;
            evt.timestampMs = static_cast<uint32_t>(
                esp_timer_get_time() / 1000ULL
            );

            eventBus_.publish(evt);
        }
        else
        {
            setState(previous);
        }

        return ok;
    }
}