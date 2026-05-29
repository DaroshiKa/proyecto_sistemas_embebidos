#include "communication/TelemetryPublisher.hpp"

#include "esp_log.h"
#include "esp_timer.h"

namespace Communication
{
    static constexpr const char* TAG = "TelemetryPub";

    TelemetryPublisher::TelemetryPublisher(
        QueueHandle_t outputQueue,
        Interfaces::IDataProvider* dataProvider
    )
        : outQueue_(outputQueue),
          dataProvider_(dataProvider)
    {
    }

    bool TelemetryPublisher::enqueue(const Models::TelemetryFrame& frame)
    {
        if (outQueue_ == nullptr) return false;

        if (xQueueSend(outQueue_, &frame, 0) != pdTRUE)
        {
            ++totalDropped_;
            return false;
        }

        ++totalPushed_;
        return true;
    }

    void TelemetryPublisher::onEvent(const Models::EventMessage& event)
    {
        // Sólo eventos asíncronos relevantes para el display.
        // La telemetría periódica la genera publishPeriodic().
        Models::TelemetryFrame f {};
        f.timestampMs = event.timestampMs;

        switch (event.type)
        {
            case Models::EventType::EMERGENCY_TRIGGERED:
                f.type = Models::TelemetryType::ALARM;
                f.data.alarm.level = 3;  // CRITICAL
                f.data.alarm.code  = 1;  // EMERGENCY
                enqueue(f);
                break;

            case Models::EventType::SYSTEM_ERROR:
                f.type = Models::TelemetryType::ALARM;
                f.data.alarm.level = 2;  // ERROR
                f.data.alarm.code  = 2;  // SYSTEM_ERROR
                enqueue(f);
                break;

            case Models::EventType::CALIBRATION_COMPLETE:
                f.type = Models::TelemetryType::ALARM;
                f.data.alarm.level = 0;  // INFO
                f.data.alarm.code  = 3;  // CALIBRATION_OK
                enqueue(f);
                break;

            default:
                // El resto lo cubre publishPeriodic
                break;
        }
    }

    void TelemetryPublisher::notify(const Models::TelemetryFrame& frame)
    {
        enqueue(frame);
    }

    void TelemetryPublisher::publishPeriodic(uint32_t nowMs)
    {
        if (dataProvider_ == nullptr) return;

        // IMU
        if (nowMs - lastImuMs_ >= IMU_INTERVAL_MS)
        {
            Models::IMUData d {};
            if (dataProvider_->getIMUData(d))
            {
                const auto status = dataProvider_->getIMUStatus();

                Models::TelemetryFrame f {};
                f.type = Models::TelemetryType::IMU;
                f.timestampMs = nowMs;
                f.data.imu.pitch = d.pitch;
                f.data.imu.roll  = d.roll;
                f.data.imu.yaw   = d.yaw;
                f.data.imu.plane = static_cast<uint8_t>(d.plane);
                f.data.imu.state = static_cast<uint8_t>(status.state);
                f.data.imu.calibrated = status.calibrated ? 1 : 0;
                enqueue(f);
            }
            lastImuMs_ = nowMs;
        }

        // EMG
        if (nowMs - lastEmgMs_ >= EMG_INTERVAL_MS)
        {
            Models::EMGData d {};
            if (dataProvider_->getEMGData(d))
            {
                const auto status = dataProvider_->getEMGStatus();

                Models::TelemetryFrame f {};
                f.type = Models::TelemetryType::EMG;
                f.timestampMs = nowMs;
                f.data.emg.envelope   = d.envelopeValue;
                f.data.emg.smoothed   = d.smoothedValue;
                f.data.emg.gesture    = static_cast<uint8_t>(d.gesture);
                f.data.emg.active     = d.contractionDetected ? 1 : 0;
                f.data.emg.state      = static_cast<uint8_t>(status.state);
                f.data.emg.calibrated = status.calibrated ? 1 : 0;
                enqueue(f);
            }
            lastEmgMs_ = nowMs;
        }

        // Servos
        if (nowMs - lastServosMs_ >= SERVOS_INTERVAL_MS)
        {
            Models::TelemetryFrame f {};
            f.type = Models::TelemetryType::SERVOS;
            f.timestampMs = nowMs;
            uint8_t movingMask = 0;

            for (uint8_t i = 0;
                 i < static_cast<uint8_t>(Models::JointId::COUNT);
                 ++i)
            {
                Models::ServoState st {};
                if (dataProvider_->getServoState(
                        static_cast<Models::JointId>(i), st))
                {
                    f.data.servos.angles[i] =
                        static_cast<uint8_t>(st.currentAngle);
                    if (st.moving) movingMask |= (1U << i);
                }
            }
            f.data.servos.moving = movingMask;
            enqueue(f);
            lastServosMs_ = nowMs;
        }

        // System
        if (nowMs - lastSystemMs_ >= SYSTEM_INTERVAL_MS)
        {
            Models::TelemetryFrame f {};
            f.type = Models::TelemetryType::SYSTEM;
            f.timestampMs = nowMs;
            f.data.system.uptimeMs = dataProvider_->getSystemUptimeMs();
            f.data.system.freeHeap = dataProvider_->getFreeHeap();
            f.data.system.systemState = 0;
            f.data.system.emergencyActive = 0;
            enqueue(f);
            lastSystemMs_ = nowMs;
        }
    }
}