#include "drivers/ServoDriver.hpp"

namespace Drivers
{
    static constexpr uint32_t MIN_PULSE =
        1638;

    static constexpr uint32_t MAX_PULSE =
        8192;

    ServoDriver::ServoDriver(
        HAL::PWMHal& pwmHal
    )
        : pwmHal_(pwmHal)
    {
    }

    bool ServoDriver::attach(
        gpio_num_t pin,
        ledc_channel_t channel
    )
    {
        channel_ = channel;

        return pwmHal_.configureChannel(
            pin,
            channel
        );
    }

    bool ServoDriver::setAngle(
        float angle
    )
    {
        if (angle < 0.0f)
        {
            angle = 0.0f;
        }

        if (angle > 180.0f)
        {
            angle = 180.0f;
        }

        const uint32_t duty =
            MIN_PULSE +
            static_cast<uint32_t>(
                (angle / 180.0f) *
                (MAX_PULSE - MIN_PULSE)
            );

        return pwmHal_.setDuty(
            channel_,
            duty
        );
    }
}