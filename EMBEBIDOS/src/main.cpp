#include "esp_log.h"

#include "drivers/ServoDriver.hpp"
#include "hal/PWMHal.hpp"

extern "C" void app_main(void)
{
    ESP_LOGI("MAIN", "HAL TEST");

    HAL::PWMHal pwm;

    if (!pwm.initialize())
    {
        ESP_LOGE("MAIN", "PWM init failed");

        return;
    }

    Drivers::ServoDriver servo(pwm);

    if (!servo.attach(
            GPIO_NUM_18,
            LEDC_CHANNEL_0
        ))
    {
        ESP_LOGE("MAIN", "Servo attach failed");

        return;
    }

    ESP_LOGI("MAIN", "Moving servo");

    servo.setAngle(90.0f);
}