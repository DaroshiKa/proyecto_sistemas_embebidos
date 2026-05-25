#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hal/I2CHal.hpp"
#include "hal/ADCHal.hpp"
#include "hal/PWMHal.hpp"
#include "hal/UARTHal.hpp"

#include "drivers/MPU6050Driver.hpp"
#include "drivers/EMGDriver.hpp"
#include "drivers/ServoManager.hpp"

#include "core/EventBus.hpp"
#include "core/QueueManager.hpp"
#include "core/CommandDispatcher.hpp"
#include "core/BasicSafetyValidator.hpp"

#include "services/IMUService.hpp"
#include "services/EMGService.hpp"
#include "services/MotionService.hpp"
#include "services/CLIService.hpp"

#include "communication/UARTConsole.hpp"

#include "app/DemoMode.hpp"

#include "tasks/TaskIMU.hpp"
#include "tasks/TaskEMG.hpp"
#include "tasks/TaskMotion.hpp"
#include "tasks/TaskCLI.hpp"

#include "models/IMUConfig.hpp"
#include "models/EMGConfig.hpp"
#include "models/MotionConfig.hpp"

static const char* TAG = "MAIN";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Boot - Stage 7 CLI Integration");

    // ============ Core ============
    static Core::EventBus eventBus;

    if (!Core::QueueManager::initialize())
    {
        ESP_LOGE(TAG, "QueueManager init failed");
        return;
    }

    static Core::BasicSafetyValidator validator;

    static Core::CommandDispatcher dispatcher(
        validator,
        Core::QueueManager::motionCommandQueue(),
        &eventBus
    );

    // ============ HAL ============
    static HAL::I2CHal  i2c;
    static HAL::ADCHal  adc;
    static HAL::PWMHal  pwm;
    static HAL::UARTHal uart;

    Models::IMUConfig    imuCfg {};
    Models::EMGConfig    emgCfg {};
    Models::MotionConfig motionCfg {};

    i2c.initialize(imuCfg.sdaPin, imuCfg.sclPin, imuCfg.busFrequencyHz);

    // UART0: ya está configurado por ESP-IDF para logs. Para CLI usamos UART0
    // también, lo cual coexiste con los logs (acceptable en debug).
    // Pines TX/RX default de UART0: GPIO1/GPIO3.
    static Communication::UARTConsole console(uart, UART_NUM_0);
    console.bind(115200, GPIO_NUM_1, GPIO_NUM_3);

    // ============ Drivers ============
    static Drivers::MPU6050Driver mpu(i2c);
    static Drivers::EMGDriver     emgDriver(adc);
    static Drivers::ServoManager  servoManager(pwm, motionCfg);

    if (!servoManager.initialize())
    {
        ESP_LOGE(TAG, "ServoManager init FAILED");
        return;
    }

    // ============ Services ============
    static Services::MotionService motionService(
        servoManager, eventBus, motionCfg);
    motionService.initialize();

    static Services::IMUService imuService(mpu, eventBus, imuCfg);
    static Services::EMGService emgService(emgDriver, eventBus, emgCfg);

    const bool imuOk = imuService.initialize();
    if (imuOk)
    {
        imuService.attachCommandDispatcher(&dispatcher);
    }

    const bool emgOk = emgService.initialize();
    if (emgOk)
    {
        emgService.attachCommandDispatcher(&dispatcher);
    }

    // ============ CLI ============
    Services::CLIDependencies cliDeps {};
    cliDeps.dispatcher      = &dispatcher;
    cliDeps.imu             = imuOk ? &imuService : nullptr;
    cliDeps.emg             = emgOk ? &emgService : nullptr;
    cliDeps.executor        = &servoManager;
    cliDeps.dispatcherStats = &dispatcher;

    static Services::CLIService cliService(console, cliDeps);
    static App::DemoMode        demoMode(dispatcher);

    // ============ Tasks ============
    static Tasks::TaskMotion taskMotion(
        motionService, servoManager,
        Core::QueueManager::motionCommandQueue()
    );
    static Tasks::TaskCLI taskCli(console, cliService, demoMode);

    static Tasks::TaskIMU taskImu(imuService);
    static Tasks::TaskEMG taskEmg(emgService);

    taskMotion.start();
    taskCli.start();

    if (imuOk) taskImu.start();
    if (emgOk) taskEmg.start();

    // Loop de app_main vacío: todo corre en sus tasks.
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}