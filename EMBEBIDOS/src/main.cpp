// ============================================================
// EMBEBIDOS — Firmware mano robótica EMG + IMU
// main.cpp — Etapa 9: Safety + FSM + Watchdog integrados
// ============================================================

#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ------------ Core ------------
#include "core/EventBus.hpp"
#include "core/CommandDispatcher.hpp"
#include "core/QueueManager.hpp"
#include "core/SystemContext.hpp"
#include "core/SystemStateMachine.hpp"

// ------------ HAL ------------
#include "hal/I2CHal.hpp"
#include "hal/PWMHal.hpp"
#include "hal/UARTHal.hpp"

// ------------ Drivers ------------
#include "drivers/ServoDriver.hpp"
#include "drivers/ServoManager.hpp"
#include "drivers/MPU6050Driver.hpp"
#include "drivers/EMGDriver.hpp"

// ------------ Services ------------
#include "services/IMUService.hpp"
#include "services/EMGService.hpp"
#include "services/MotionService.hpp"
#include "services/CLIService.hpp"
#include "services/SafetyService.hpp"

// ------------ Communication ------------
#include "communication/UARTConsole.hpp"
#include "communication/NextionInterface.hpp"
#include "communication/TelemetryPublisher.hpp"
#include "communication/SystemDataProvider.hpp"

// ------------ App ------------
#include "app/DemoMode.hpp"

// ------------ Tasks ------------
#include "tasks/TaskIMU.hpp"
#include "tasks/TaskEMG.hpp"
#include "tasks/TaskMotion.hpp"
#include "tasks/TaskCLI.hpp"
#include "tasks/TaskNextionInterface.hpp"
#include "tasks/TaskSafety.hpp"

// ------------ Interfaces ------------
#include "interfaces/ISensorHealthSource.hpp"

// ------------ Models ------------
#include "models/IMUConfig.hpp"
#include "models/EMGConfig.hpp"
#include "models/MotionConfig.hpp"
#include "models/SafetyConfig.hpp"
#include "models/ServoConfig.hpp"

static constexpr const char* TAG = "MAIN";

// ============================================================
// Adapters ISensorHealthSource — viven aquí porque su única
// razón de existir es enchufar IMUService y EMGService al
// SafetyService sin tocar esos servicios. Patrón Adapter.
// ============================================================
class ImuHealthAdapter final : public Interfaces::ISensorHealthSource
{
public:
    explicit ImuHealthAdapter(Services::IMUService& s) : svc_(s) {}

    uint32_t lastSampleTimestampMs() const override
    {
        return svc_.status().lastUpdateMs;
    }

    bool isSensorOk() const override
    {
        return svc_.status().state == Models::IMUState::OK;
    }

private:
    Services::IMUService& svc_;
};

class EmgHealthAdapter final : public Interfaces::ISensorHealthSource
{
public:
    explicit EmgHealthAdapter(Services::EMGService& s) : svc_(s) {}

    uint32_t lastSampleTimestampMs() const override
    {
        return svc_.status().lastUpdateMs;
    }

    bool isSensorOk() const override
    {
        return svc_.status().state == Models::EMGState::OK;
    }

private:
    Services::EMGService& svc_;
};

// ============================================================
// Queue de TX para Nextion (declarada global estática para que
// TelemetryPublisher y TaskNextionInterface compartan handle).
// ============================================================
static QueueHandle_t s_nextionTxQueue = nullptr;

// ============================================================
// Entry point ESP-IDF
// ============================================================
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "=========================================");
    ESP_LOGI(TAG, "  EMBEBIDOS - Hand Robot Firmware");
    ESP_LOGI(TAG, "  Stage 9: Safety + FSM + Watchdog");
    ESP_LOGI(TAG, "=========================================");

    // ============================================================
    // 1) Infraestructura base: EventBus, Queues, EventGroups
    // ============================================================
    static Core::EventBus eventBus;

    if (!Core::QueueManager::initialize())
    {
        ESP_LOGE(TAG, "QueueManager init failed");
        return;
    }

    if (!Core::SystemContext::initialize())
    {
        ESP_LOGE(TAG, "SystemContext init failed");
        return;
    }

    s_nextionTxQueue = xQueueCreate(
        16,
        sizeof(Models::TelemetryFrame)
    );

    if (s_nextionTxQueue == nullptr)
    {
        ESP_LOGE(TAG, "Nextion TX queue creation failed");
        return;
    }

    // ============================================================
    // 2) FSM global del sistema (debe existir ANTES de SafetyService)
    // ============================================================
    static Core::SystemStateMachine fsm(eventBus);
    if (!fsm.initialize())
    {
        ESP_LOGE(TAG, "FSM init failed");
        return;
    }

    // ============================================================
    // 3) SafetyService — implementa ISafetyValidator. Sustituye al
    //    BasicSafetyValidator usado en etapas previas.
    // ============================================================
    static Services::SafetyService safety(
        fsm,
        eventBus,
        Models::SafetyConfig{}
    );

    if (!safety.initialize())
    {
        ESP_LOGE(TAG, "SafetyService init failed");
        return;
    }

    // ============================================================
    // 4) CommandDispatcher con SafetyService como validador
    // ============================================================
    static Core::CommandDispatcher dispatcher(
        safety,
        Core::QueueManager::motionCommandQueue(),
        &eventBus
    );

    // ============================================================
    // 5) HAL: I2C, PWM, UART (consola y Nextion)
    // ============================================================
    static HAL::I2CHal i2cBus;
    const bool i2cOk = i2cBus.initialize(
        GPIO_NUM_21,   // SDA
        GPIO_NUM_22,   // SCL
        400000
    );
    if (!i2cOk)
    {
        ESP_LOGW(TAG, "I2C bus init failed - IMU will be disabled");
    }

    static HAL::PWMHal pwm;
    if (!pwm.initialize())
    {
        ESP_LOGE(TAG, "PWM init failed");
        return;
    }

    static HAL::UARTHal uartConsoleHal;
    if (!uartConsoleHal.initialize(
            UART_NUM_0,
            115200,
            GPIO_NUM_1,    // TX
            GPIO_NUM_3))   // RX
    {
        ESP_LOGE(TAG, "UART0 (console) init failed");
        return;
    }

    static HAL::UARTHal uartNextionHal;
    if (!uartNextionHal.initialize(
            UART_NUM_2,
            115200,
            GPIO_NUM_17,   // TX
            GPIO_NUM_16))  // RX
    {
        ESP_LOGW(TAG, "UART2 (Nextion) init failed - HMI disabled");
    }

    // ============================================================
    // 6) Drivers
    // ============================================================
    static Drivers::ServoManager servoManager(pwm);
    if (!servoManager.initialize(Models::ServoConfig{}))
    {
        ESP_LOGE(TAG, "ServoManager init failed");
        return;
    }

    static Drivers::MPU6050Driver mpu(i2cBus);
    const bool imuOk = i2cOk && mpu.initialize(Models::IMUConfig{});
    if (!imuOk)
    {
        ESP_LOGW(TAG, "MPU6050 init failed - IMU service disabled");
    }

    static Drivers::EMGDriver emgDriver;
    const bool emgDriverOk = emgDriver.initialize(Models::EMGConfig{});
    if (!emgDriverOk)
    {
        ESP_LOGW(TAG, "EMG driver init failed - EMG service disabled");
    }

    // ============================================================
    // 7) Services
    // ============================================================
    static Services::MotionService motionService(
        servoManager,
        eventBus,
        Models::MotionConfig{}
    );
    motionService.initialize();

    static Services::IMUService imuService(
        mpu,
        eventBus,
        Models::IMUConfig{}
    );
    const bool imuSvcOk = imuOk && imuService.initialize();
    if (imuSvcOk)
    {
        imuService.attachCommandDispatcher(&dispatcher);
    }

    static Services::EMGService emgService(
        emgDriver,
        eventBus,
        Models::EMGConfig{}
    );
    const bool emgSvcOk = emgDriverOk && emgService.initialize();
    if (emgSvcOk)
    {
        emgService.attachCommandDispatcher(&dispatcher);
    }

    // ============================================================
    // 8) Adapters de salud — enchufar al SafetyService SOLO si el
    //    servicio correspondiente arrancó OK.
    // ============================================================
    static ImuHealthAdapter imuHealthAdapter(imuService);
    static EmgHealthAdapter emgHealthAdapter(emgService);

    if (imuSvcOk) safety.attachImuHealth(&imuHealthAdapter);
    if (emgSvcOk) safety.attachEmgHealth(&emgHealthAdapter);

    // ============================================================
    // 9) Communication: consola, Nextion, telemetría
    // ============================================================
    static Communication::UARTConsole console(uartConsoleHal, UART_NUM_0);
    console.initialize();

    static Communication::NextionInterface nextion(s_nextionTxQueue);
    nextion.attachCommandDispatcher(&dispatcher);

    static Communication::SystemDataProvider dataProvider;
    dataProvider.attachImu(imuSvcOk ? &imuService : nullptr);
    dataProvider.attachEmg(emgSvcOk ? &emgService : nullptr);
    dataProvider.attachServos(&servoManager);

    static Communication::TelemetryPublisher telemetryPub(
        dataProvider,
        s_nextionTxQueue
    );
    eventBus.subscribe(&telemetryPub);

    // ============================================================
    // 10) CLI: ahora con safety inyectado
    // ============================================================
    Services::CLIDependencies cliDeps {};
    cliDeps.console         = &console;
    cliDeps.dispatcher      = &dispatcher;
    cliDeps.imu             = imuSvcOk
        ? static_cast<Interfaces::IIMUSource*>(&imuService)
        : nullptr;
    cliDeps.emg             = emgSvcOk
        ? static_cast<Interfaces::IEMGSource*>(&emgService)
        : nullptr;
    cliDeps.executor        = &servoManager;
    cliDeps.dispatcherStats = &dispatcher;
    cliDeps.safety          = &safety;

    static Services::CLIService cliService(console, cliDeps);
    cliService.initialize();

    static App::DemoMode demoMode(dispatcher);

    // ============================================================
    // 11) Tasks RTOS
    // ============================================================
    static Tasks::TaskMotion taskMotion(
        motionService,
        servoManager,
        Core::QueueManager::motionCommandQueue()
    );

    static Tasks::TaskCLI taskCli(console, cliService, demoMode);

    Tasks::TaskNextionConfig nxConfig {};
    static Tasks::TaskNextionInterface taskNextion(
        uartNextionHal,
        nextion,
        s_nextionTxQueue,
        nxConfig
    );

    static Tasks::TaskIMU taskImu(imuService);
    static Tasks::TaskEMG taskEmg(emgService);

    if (!taskMotion.start())
    {
        ESP_LOGE(TAG, "TaskMotion failed to start");
    }
    if (!taskCli.start())
    {
        ESP_LOGE(TAG, "TaskCLI failed to start");
    }
    if (!taskNextion.start())
    {
        ESP_LOGW(TAG, "TaskNextion failed to start (HMI disabled)");
    }
    if (imuSvcOk && !taskImu.start())
    {
        ESP_LOGE(TAG, "TaskIMU failed to start");
    }
    if (emgSvcOk && !taskEmg.start())
    {
        ESP_LOGE(TAG, "TaskEMG failed to start");
    }

    // ============================================================
    // 12) Boot complete + TaskSafety
    //
    //    El orden es deliberado:
    //    - Todas las tasks de trabajo ya corren.
    //    - Pasamos la FSM a IDLE.
    //    - Levantamos TaskSafety, que comenzará a vigilar.
    // ============================================================
    fsm.bootComplete();

    static Tasks::TaskSafety taskSafety(safety);
    if (!taskSafety.start())
    {
        ESP_LOGE(TAG, "TaskSafety failed to start");
    }

    ESP_LOGI(TAG, "System ready. IMU=%d EMG=%d Nextion=%d",
        imuSvcOk ? 1 : 0,
        emgSvcOk ? 1 : 0,
        s_nextionTxQueue != nullptr ? 1 : 0);

    // ============================================================
    // 13) Loop principal: solo publica telemetría periódica.
    //     Todo el trabajo real lo hacen las tasks RTOS.
    // ============================================================
    while (true)
    {
        const uint32_t nowMs =
            static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        telemetryPub.publishPeriodic(nowMs);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}