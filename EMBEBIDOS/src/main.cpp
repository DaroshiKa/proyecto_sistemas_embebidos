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
#include "core/AdvancedSafetyValidator.hpp"

#include "services/IMUService.hpp"
#include "services/EMGService.hpp"
#include "services/MotionService.hpp"
#include "services/CLIService.hpp"
#include "services/SafetyService.hpp"
#include "services/WatchdogManager.hpp"

#include "adapters/IMUHealthAdapter.hpp"
#include "adapters/EMGHealthAdapter.hpp"

#include "communication/UARTConsole.hpp"

#include "app/DemoMode.hpp"

#include "tasks/TaskIMU.hpp"
#include "tasks/TaskEMG.hpp"
#include "tasks/TaskMotion.hpp"
#include "tasks/TaskCLI.hpp"
#include "tasks/TaskSafety.hpp"
#include "tasks/TaskSystemMonitor.hpp"

#include "models/IMUConfig.hpp"
#include "models/EMGConfig.hpp"
#include "models/MotionConfig.hpp"
#include "models/SafetyConfig.hpp"

static const char* TAG = "MAIN";

// ----------------------------------------------------------------------------
// Callback estático del watchdog. NO podemos usar lambdas con captura
// porque el TWDT de IDF requiere un function pointer estilo C.
// ----------------------------------------------------------------------------
static void watchdogCallback(void* arg)
{
    auto* safety = static_cast<Services::SafetyService*>(arg);
    if (safety != nullptr) safety->onWatchdogTrigger();
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Boot - Stage 9 Safety");

    // =========================================================
    // 1) Core: EventBus + Queues
    // =========================================================
    static Core::EventBus eventBus;

    if (!Core::QueueManager::initialize())
    {
        ESP_LOGE(TAG, "QueueManager init failed");
        return;
    }

    // =========================================================
    // 2) HAL
    // =========================================================
    static HAL::I2CHal  i2c;
    static HAL::ADCHal  adc;
    static HAL::PWMHal  pwm;
    static HAL::UARTHal uart;

    Models::IMUConfig    imuCfg {};
    Models::EMGConfig    emgCfg {};
    Models::MotionConfig motionCfg {};
    Models::SafetyConfig safetyCfg {};

    i2c.initialize(imuCfg.sdaPin, imuCfg.sclPin, imuCfg.busFrequencyHz);

    static Communication::UARTConsole console(uart, UART_NUM_0);
    console.bind(115200, GPIO_NUM_1, GPIO_NUM_3);

    // =========================================================
    // 3) Drivers (hardware-facing)
    // =========================================================
    static Drivers::MPU6050Driver mpu(i2c);
    static Drivers::EMGDriver     emgDriver(adc);
    static Drivers::ServoManager  servoManager(pwm, motionCfg);

    if (!servoManager.initialize())
    {
        ESP_LOGE(TAG, "ServoManager init FAILED");
        // No tenemos actuadores: el sistema se queda en STARTUP y reporta INIT_FAILURE.
        // Sí podemos seguir para CLI/diagnóstico.
    }

    // =========================================================
    // 4) Services de sensor + Motion
    // =========================================================
    static Services::MotionService motionService(
        servoManager, eventBus, motionCfg);
    motionService.initialize();

    static Services::IMUService imuService(mpu, eventBus, imuCfg);
    static Services::EMGService emgService(emgDriver, eventBus, emgCfg);

    const bool imuOk = imuService.initialize();
    const bool emgOk = emgService.initialize();

    // =========================================================
    // 5) SafetyService + Adapters + Validator
    //    Orden CRÍTICO: safety necesita ServoManager (IMotionExecutor).
    // =========================================================
    static Services::SafetyService safetyService(
        servoManager, eventBus, safetyCfg);

    if (!safetyService.initialize())
    {
        ESP_LOGE(TAG, "SafetyService init FAILED");
        return;   // sin safety, no operamos
    }

    static Adapters::IMUHealthAdapter imuHealth(imuOk ? &imuService : nullptr);
    static Adapters::EMGHealthAdapter emgHealth(emgOk ? &emgService : nullptr);

    safetyService.registerHealthProvider(&imuHealth);
    safetyService.registerHealthProvider(&emgHealth);

    static Core::AdvancedSafetyValidator validator(safetyService);

    static Core::CommandDispatcher dispatcher(
        validator,
        Core::QueueManager::motionCommandQueue(),
        &eventBus
    );
    dispatcher.attachSafetyService(&safetyService);

    // Inyecciones tardías para resolver dependencias cíclicas conceptuales
    motionService.attachSafetyMonitor(&safetyService);

    // Conectar dispatchers a los servicios sensor
    if (imuOk) imuService.attachCommandDispatcher(&dispatcher);
    if (emgOk) emgService.attachCommandDispatcher(&dispatcher);

    // =========================================================
    // 6) Watchdog
    // =========================================================
    static Services::WatchdogManager watchdog;
    const bool wdtOk = watchdog.initialize(
        safetyCfg.watchdogTimeoutMs,
        safetyCfg.watchdogPanicOnTrigger,
        watchdogCallback,
        &safetyService
    );

    if (!wdtOk)
    {
        ESP_LOGW(TAG, "Watchdog disabled (init failed)");
    }

    // =========================================================
    // 7) CLI
    // =========================================================
    Services::CLIDependencies cliDeps {};
    cliDeps.dispatcher      = &dispatcher;
    cliDeps.imu             = imuOk ? &imuService : nullptr;
    cliDeps.emg             = emgOk ? &emgService : nullptr;
    cliDeps.executor        = &servoManager;
    cliDeps.dispatcherStats = &dispatcher;
    cliDeps.safetyMonitor   = &safetyService;

    static Services::CLIService cliService(console, cliDeps);
    static App::DemoMode        demoMode(dispatcher);

    // =========================================================
    // 8) Tasks
    // =========================================================
    static Tasks::TaskMotion taskMotion(
        motionService, servoManager,
        Core::QueueManager::motionCommandQueue()
    );
    taskMotion.attachWatchdog(&watchdog);

    static Tasks::TaskSafety        taskSafety(safetyService, watchdog);
    static Tasks::TaskCLI           taskCli(console, cliService, demoMode);
    static Tasks::TaskIMU           taskImu(imuService);
    static Tasks::TaskEMG           taskEmg(emgService);
    static Tasks::TaskSystemMonitor taskSysMon(safetyService);

    // Iniciar Safety primero — debe estar listo antes que nadie mueva nada
    taskSafety.start();
    taskMotion.start();
    taskCli.start();

    if (imuOk) taskImu.start();
    if (emgOk) taskEmg.start();

    // Registrar tasks críticas en el monitor
    taskSysMon.registerWatchedTask(taskMotion.handle(), "Motion");
    taskSysMon.start();

    ESP_LOGI(TAG, "Boot complete. Safety active.");

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}