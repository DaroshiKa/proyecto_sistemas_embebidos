#include "drivers/ServoDriver.hpp"

namespace Drivers
{
    static constexpr uint32_t MIN_PULSE = 1638;   // ~1.0 ms a 50 Hz, 16 bits
    static constexpr uint32_t MAX_PULSE = 8192;   // ~2.0 ms

    ServoDriver::ServoDriver(HAL::PWMHal& pwmHal)
        : pwmHal_(pwmHal),
          channel_(LEDC_CHANNEL_0)
    {
    }

    bool ServoDriver::attach(gpio_num_t pin, ledc_channel_t channel)
    {
        channel_ = channel;
        attached_ = pwmHal_.configureChannel(pin, channel);
        return attached_;
    }

    bool ServoDriver::setAngle(float angle)
    {
        if (!attached_) return false;

        if (angle < 0.0f)   angle = 0.0f;
        if (angle > 180.0f) angle = 180.0f;

        const uint32_t duty =
            MIN_PULSE +
            static_cast<uint32_t>(
                (angle / 180.0f) * (MAX_PULSE - MIN_PULSE));

        if (!pwmHal_.setDuty(channel_, duty))
        {
            return false;
        }

        lastAngle_ = angle;
        return true;
    }
}