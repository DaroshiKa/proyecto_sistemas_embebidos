#include "tasks/TaskNextionInterface.hpp"

#include "esp_log.h"
#include "esp_timer.h"

#include "protocols/PacketSerializer.hpp"
#include "models/TelemetryFrame.hpp"

namespace Tasks
{
    static constexpr const char* TAG = "TaskNextion";

    TaskNextionInterface::TaskNextionInterface(
        HAL::UARTHal& uart,
        Communication::NextionInterface& iface,
        QueueHandle_t txQueue,
        const TaskNextionConfig& cfg
    )
        : uart_(uart),
          iface_(iface),
          txQueue_(txQueue),
          config_(cfg)
    {
    }

    bool TaskNextionInterface::start()
    {
        if (handle_ != nullptr) return true;

        // Inicializar UART2
        if (!uart_.initialize(
                config_.uartPort,
                config_.baudRate,
                config_.txPin,
                config_.rxPin))
        {
            ESP_LOGE(TAG, "UART init failed");
            return false;
        }

        running_ = true;

        const BaseType_t res = xTaskCreatePinnedToCore(
            &TaskNextionInterface::taskEntry,
            config_.name,
            config_.stackSize,
            this,
            config_.priority,
            &handle_,
            config_.coreId
        );

        if (res != pdPASS)
        {
            ESP_LOGE(TAG, "task create failed");
            running_ = false;
            handle_  = nullptr;
            return false;
        }

        ESP_LOGI(
            TAG,
            "Started on core %d, UART%d @ %d baud (tx=%d rx=%d)",
            static_cast<int>(config_.coreId),
            static_cast<int>(config_.uartPort),
            config_.baudRate,
            static_cast<int>(config_.txPin),
            static_cast<int>(config_.rxPin)
        );
        return true;
    }

    void TaskNextionInterface::stop()
    {
        running_ = false;
    }

    void TaskNextionInterface::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskNextionInterface*>(arg);
        self->run();

        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
    }

    bool TaskNextionInterface::readAndParse()
    {
        uint8_t buf[64];

        const int got = uart_.read(
            config_.uartPort,
            buf,
            sizeof(buf),
            pdMS_TO_TICKS(config_.rxTimeoutMs)
        );

        if (got <= 0) return false;

        bool anyFrame = false;

        parser_.feedBuffer(
            buf,
            static_cast<size_t>(got),
            [this, &anyFrame](const Protocols::ProtocolFrame& f)
            {
                Models::DisplayEvent ev {};
                ev.timestampMs =
                    static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

                if (iface_.handleIncomingFrame(f, ev))
                {
                    anyFrame = true;

                    // Marcar como conectado: hemos recibido al menos un frame válido
                    iface_.setConnected(true);
                }
            }
        );

        return anyFrame;
    }

    void TaskNextionInterface::drainTxQueue()
    {
        if (txQueue_ == nullptr) return;

        Models::TelemetryFrame frame {};
        uint8_t outBuf[Protocols::PROTO_MAX_FRAME];

        // Drenar hasta 4 frames por iteración para ceder CPU
        for (int i = 0; i < 4; ++i)
        {
            if (xQueueReceive(txQueue_, &frame, 0) != pdTRUE) break;

            Protocols::ProtocolFrame pkt {};
            static uint8_t txSeq = 0;
            pkt.seq = txSeq++;

            // Serializar payload según tipo
            switch (frame.type)
            {
                case Models::TelemetryType::IMU:
                {
                    pkt.type   = Protocols::MessageType::TLM_IMU;
                    pkt.length = sizeof(Models::TelemetryIMU);
                    memcpy(pkt.payload, &frame.data.imu, pkt.length);
                    break;
                }
                case Models::TelemetryType::EMG:
                {
                    pkt.type   = Protocols::MessageType::TLM_EMG;
                    pkt.length = sizeof(Models::TelemetryEMG);
                    memcpy(pkt.payload, &frame.data.emg, pkt.length);
                    break;
                }
                case Models::TelemetryType::SERVOS:
                {
                    pkt.type   = Protocols::MessageType::TLM_SERVOS;
                    pkt.length = sizeof(Models::TelemetryServos);
                    memcpy(pkt.payload, &frame.data.servos, pkt.length);
                    break;
                }
                case Models::TelemetryType::SYSTEM:
                {
                    pkt.type   = Protocols::MessageType::TLM_SYSTEM;
                    pkt.length = sizeof(Models::TelemetrySystem);
                    memcpy(pkt.payload, &frame.data.system, pkt.length);
                    break;
                }
                case Models::TelemetryType::ALARM:
                {
                    pkt.type   = Protocols::MessageType::EVT_ALARM;
                    pkt.length = sizeof(Models::TelemetryAlarm);
                    memcpy(pkt.payload, &frame.data.alarm, pkt.length);
                    break;
                }
                default:
                    continue;
            }

            const size_t n = Protocols::PacketSerializer::serialize(
                pkt, outBuf, sizeof(outBuf)
            );

            if (n > 0)
            {
                uart_.write(config_.uartPort, outBuf, n);
            }
        }
    }

    void TaskNextionInterface::run()
    {
        while (running_)
        {
            // 1) Leer y parsear (bloquea hasta rxTimeoutMs)
            readAndParse();

            // 2) Enviar telemetría pendiente
            drainTxQueue();
        }
    }
}