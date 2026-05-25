#include "services/EMGService.hpp"

#include "esp_log.h"
#include "esp_timer.h"

#include "models/MotionCommand.hpp"
#include "models/MotionTypes.hpp"

namespace Services
{
    static constexpr const char* TAG = "EMGService";

    // Tamaño máximo de batch leído del DMA por update()
    static constexpr size_t BATCH_MAX = 128;

    EMGService::EMGService(
        Drivers::EMGDriver& driver,
        Core::EventBus& eventBus,
        const Models::EMGConfig& config
    )
        : driver_(driver),
          eventBus_(eventBus),
          config_(config)
    {
    }

    void EMGService::attachCommandDispatcher(
        Interfaces::ICommandDispatcher* dispatcher
    )
    {
        dispatcher_ = dispatcher;
    }

    void EMGService::setState(Models::EMGState state)
    {
        status_.state = state;
    }

    bool EMGService::initialize()
    {
        mutex_ = xSemaphoreCreateMutex();
        if (mutex_ == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create mutex");
            return false;
        }

        setState(Models::EMGState::INITIALIZING);

        if (!driver_.initialize(config_))
        {
            setState(Models::EMGState::FAULT);
            return false;
        }

        processor_.configure(config_);
        gestureFsm_.configure(config_);
        detector_.setThresholds(config_.thresholdOn, config_.thresholdOff);
        detector_.setDebounce(config_.debounceMs);

        if (!driver_.start())
        {
            ESP_LOGE(TAG, "Driver start failed");
            setState(Models::EMGState::FAULT);
            return false;
        }

        setState(Models::EMGState::OK);
        ESP_LOGI(TAG, "EMGService ready");
        return true;
    }

    void EMGService::update()
    {
        if (status_.state != Models::EMGState::OK)
        {
            return;
        }

        uint16_t batch[BATCH_MAX];

        const size_t got =
            driver_.readSamples(batch, BATCH_MAX, 100);

        if (got == 0)
        {
            ++status_.consecutiveFails;
            if (status_.consecutiveFails >= config_.maxConsecutiveFails)
            {
                if (status_.state != Models::EMGState::TIMEOUT)
                {
                    ESP_LOGW(TAG, "EMG TIMEOUT (%lu fails)",
                             static_cast<unsigned long>(status_.consecutiveFails));
                    setState(Models::EMGState::TIMEOUT);

                    Models::EventMessage evt {};
                    evt.type = Models::EventType::SYSTEM_ERROR;
                    evt.timestampMs =
                        static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
                    eventBus_.publish(evt);
                }
            }
            return;
        }

        status_.consecutiveFails = 0;
        status_.totalSamples    += got;

        const uint32_t now =
            static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        for (size_t i = 0; i < got; ++i)
        {
            processOneSample(batch[i], now);
        }

        publishSensorUpdated();
    }

    void EMGService::processOneSample(uint16_t raw, uint32_t nowMs)
    {
        const Utils::EMGProcessedSample sample =
            processor_.process(raw);

        const bool active =
            detector_.update(sample.smoothed, nowMs);

        const Models::EMGGesture gesture =
            gestureFsm_.update(active, nowMs);

        // Sólo cacheamos: la publicación al bus se hace una vez por batch
        Models::EMGData data {};
        data.rawValue            = sample.raw;
        data.filteredValue       = sample.bandPassed;
        data.envelopeValue       = sample.envelope;
        data.smoothedValue       = sample.smoothed;
        data.contractionDetected = active;
        data.gesture             = gesture;
        data.timestampMs         = nowMs;
        data.valid               = true;

        if (sample.smoothed > status_.peakLevel)
        {
            status_.peakLevel = sample.smoothed;
        }

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(2)) == pdTRUE)
        {
            lastData_ = data;
            status_.lastUpdateMs = nowMs;
            xSemaphoreGive(mutex_);
        }

        if (gesture != Models::EMGGesture::NONE)
        {
            ++status_.totalGestures;
            emitMotionCommandIfGesture(gesture, nowMs);
        }
    }

    void EMGService::publishSensorUpdated()
    {
        Models::EventMessage evt {};
        evt.type        = Models::EventType::SENSOR_UPDATED;
        evt.timestampMs = status_.lastUpdateMs;
        evt.data        = static_cast<void*>(&lastData_);

        eventBus_.publish(evt);
    }

    void EMGService::emitMotionCommandIfGesture(
        Models::EMGGesture gesture,
        uint32_t nowMs
    )
    {
        if (dispatcher_ == nullptr)
        {
            return;
        }

        Models::MotionCommand cmd {};
        cmd.source      = Models::CommandSource::EMG;
        cmd.priority    = Models::CommandPriority::HIGH;
        cmd.timestampMs = nowMs;
        cmd.requiresAck = false;

        switch (gesture)
        {
            case Models::EMGGesture::SINGLE_CONTRACTION:
                cmd.type = Models::MotionType::HAND_CLOSE;
                break;
            case Models::EMGGesture::DOUBLE_CONTRACTION:
                cmd.type = Models::MotionType::HAND_OPEN;
                break;
            case Models::EMGGesture::LONG_HOLD:
                // Reservado: en Etapa 10 entrará a modo de configuración.
                // Por ahora no emite comando de motion.
                return;
            case Models::EMGGesture::RELAX:
                // Telemetría únicamente; no genera movimiento.
                return;
            default:
                return;
        }

        Models::EventMessage evt {};
        evt.type        = Models::EventType::MOTION_COMMAND_RECEIVED;
        evt.timestampMs = nowMs;
        eventBus_.publish(evt);

        dispatcher_->dispatch(cmd);
    }

    bool EMGService::getLatestData(Models::EMGData& out) const
    {
        if (mutex_ == nullptr) return false;

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(5)) != pdTRUE) return false;

        out = lastData_;
        xSemaphoreGive(mutex_);

        return out.valid;
    }

    Models::EMGStatus EMGService::getStatus() const
    {
        Models::EMGStatus copy {};

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

    bool EMGService::isCalibrated() const
    {
        return driver_.isCalibrated();
    }

    bool EMGService::startCalibration()
    {
        if (status_.state == Models::EMGState::UNINITIALIZED ||
            status_.state == Models::EMGState::FAULT)
        {
            return false;
        }

        const Models::EMGState previous = status_.state;
        setState(Models::EMGState::CALIBRATING);

        ESP_LOGI(TAG, "Calibration requested - keep muscle RELAXED");

        float baseline = 0.5f;
        float peak     = 0.5f;

        const bool ok =
            driver_.calibrate(config_.calibrationSamples, baseline, peak);

        if (ok)
        {
            processor_.setBaseline(baseline);
            processor_.setPeakNormalization(peak);
            processor_.reset();
            detector_.reset();
            gestureFsm_.reset();

            status_.calibrated    = true;
            status_.baselineLevel = baseline;
            status_.peakLevel     = peak;

            setState(Models::EMGState::OK);

            Models::EventMessage evt {};
            evt.type        = Models::EventType::CALIBRATION_COMPLETE;
            evt.timestampMs = static_cast<uint32_t>(
                esp_timer_get_time() / 1000ULL);
            eventBus_.publish(evt);
        }
        else
        {
            setState(previous);
        }

        return ok;
    }

    void EMGService::setThresholds(float onLevel, float offLevel)
    {
        detector_.setThresholds(onLevel, offLevel);
        config_.thresholdOn  = onLevel;
        config_.thresholdOff = offLevel;
    }
}