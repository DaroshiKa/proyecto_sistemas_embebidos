#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace Core
{
    class SystemContext
    {
    public:
        static bool initialize();

        static EventGroupHandle_t eventGroup();

        static constexpr EventBits_t EMG_CONNECTED =
            (1 << 0);

        static constexpr EventBits_t IMU_CONNECTED =
            (1 << 1);

        static constexpr EventBits_t NEXTION_CONNECTED =
            (1 << 2);

        static constexpr EventBits_t EMERGENCY_STOP =
            (1 << 3);

    private:
        static EventGroupHandle_t eventGroup_;
    };
}