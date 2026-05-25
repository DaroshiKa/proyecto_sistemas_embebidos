#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "models/MotionCommand.hpp"

namespace Core
{
    class QueueManager
    {
    public:
        static bool initialize();

        static QueueHandle_t motionCommandQueue();

    private:
        static QueueHandle_t motionQueue_;
    };
}