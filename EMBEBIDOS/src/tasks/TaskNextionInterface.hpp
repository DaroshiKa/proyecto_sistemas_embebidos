#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/uart.h"

#include "hal/UARTHal.hpp"
#include "communication/NextionInterface.hpp"
#include "protocols/PacketParser.hpp"

namespace Tasks
{
    struct TaskNextionConfig
    {
        uart_port_t  uartPort    { UART_NUM_2 };
        gpio_num_t   txPin       { GPIO_NUM_17 };  // ojo: choca con ELBOW_Y default
        gpio_num_t   rxPin       { GPIO_NUM_16 };  // ojo: choca con ELBOW_Z default
        int          baudRate    { 115200 };

        uint32_t   stackSize   { 6144 };
        UBaseType_t priority   { 4 };          // por encima de CLI(3), debajo de IMU(5)
        BaseType_t  coreId     { 0 };          // core 0: io lenta
        const char* name       { "TaskNextion" };

        uint32_t    rxTimeoutMs { 20 };        // timeout corto entre lecturas
    };

    class TaskNextionInterface
    {
    public:
        TaskNextionInterface(
            HAL::UARTHal& uart,
            Communication::NextionInterface& iface,
            QueueHandle_t txQueue,
            const TaskNextionConfig& cfg = TaskNextionConfig{}
        );

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }

    private:
        static void taskEntry(void* arg);
        void run();

        bool readAndParse();
        void drainTxQueue();

        HAL::UARTHal&                    uart_;
        Communication::NextionInterface& iface_;
        QueueHandle_t                    txQueue_;
        TaskNextionConfig                config_;
        Protocols::PacketParser          parser_;
        TaskHandle_t                     handle_   { nullptr };
        volatile bool                    running_  { false };
    };
}