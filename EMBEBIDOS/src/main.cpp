// ============================================================
// EMBEBIDOS — Firmware mano robótica EMG + IMU
// main.cpp — Etapa 9: Safety + FSM + Watchdog integrados
// ============================================================

#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
<<<<<<< HEAD
#include "tasks/TaskDiagnostics.hpp"
=======

// ------------ Core ------------
#include "core/EventBus.hpp"
#include "core/CommandDispatcher.hpp"
#include "core/QueueManager.hpp"
#include "core/SystemContext.hpp"
#include "core/SystemStateMachine.hpp"

// ------------ HAL ------------
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
#include "hal/I2CHal.hpp"
#include "hal/PWMHal.hpp"
#include "hal/UARTHal.hpp"

// ------------ Drivers ------------
#include "drivers/ServoDriver.hpp"
#include "drivers/ServoManager.hpp"
#include "drivers/MPU6050Driver.hpp"
#include "drivers/EMGDriver.hpp"
<<<<<<< HEAD
#include "drivers/ServoManager.hpp"

#include "core/EventBus.hpp"
#include "core/QueueManager.hpp"
#include "core/CommandDispatcher.hpp"
#include "core/AdvancedSafetyValidator.hpp"
=======
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

// ------------ Services ------------
#include "services/IMUService.hpp"
#include "services/EMGService.hpp"
#include "services/MotionService.hpp"
#include "services/CLIService.hpp"
#include "services/SafetyService.hpp"
<<<<<<< HEAD
#include "services/WatchdogManager.hpp"

#include "adapters/IMUHealthAdapter.hpp"
#include "adapters/EMGHealthAdapter.hpp"
=======
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

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
<<<<<<< HEAD
#include "tasks/TaskSafety.hpp"
#include "tasks/TaskSystemMonitor.hpp"
=======
#include "tasks/TaskNextionInterface.hpp"
#include "tasks/TaskSafety.hpp"
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

// ------------ Interfaces ------------
#include "interfaces/ISensorHealthSource.hpp"

// ------------ Models ------------
#include "models/IMUConfig.hpp"
#include "models/EMGConfig.hpp"
#include "models/MotionConfig.hpp"
#include "models/SafetyConfig.hpp"
<<<<<<< HEAD
#include "services/NVSStorage.hpp"
#include "services/ConfigService.hpp"
#include "services/CalibrationManager.hpp"
=======
#include "models/ServoConfig.hpp"
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

static constexpr const char* TAG = "MAIN";

<<<<<<< HEAD
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
    // 0) Persistencia NVS — Etapa 10
    //    Debe ser lo PRIMERO para que ConfigService pueda alimentar
    //    los Configs antes de pasarlos a HAL/Services.
    // =========================================================
    if (!Services::NVSStorage::initBackend())
    {
        ESP_LOGE(TAG, "NVS backend init failed -> running with defaults");
    }
    static Services::NVSStorage configStore("cfg");
    static Services::NVSStorage calStore("cal");
    configStore.open();
    calStore.open();

    // =========================================================
    // 1) Core: EventBus + Queues
    // =========================================================
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    static Core::EventBus eventBus;

    if (!Core::QueueManager::initialize())
    {
        ESP_LOGE(TAG, "QueueManager init failed");
        return;
    }

<<<<<<< HEAD
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

    // ConfigService aplica la persistencia ANTES de inicializar HW.
    // Así los Configs que recibe el HAL ya traen los valores correctos.
    Services::ConfigService::Targets cfgTargets {};
    cfgTargets.imuCfg    = &imuCfg;
    cfgTargets.emgCfg    = &emgCfg;
    cfgTargets.motionCfg = &motionCfg;
    cfgTargets.imuSvc    = nullptr;     // todavía no existen
    cfgTargets.emgSvc    = nullptr;

    static Services::ConfigService configService(configStore, cfgTargets);
    configService.initialize();

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
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    static Services::MotionService motionService(
        servoManager,
        eventBus,
        Models::MotionConfig{}
    );
    motionService.initialize();

<<<<<<< HEAD
    static Services::IMUService imuService(mpu, eventBus, imuCfg);
    static Services::EMGService emgService(emgDriver, eventBus, emgCfg);

    const bool imuOk = imuService.initialize();
    const bool emgOk = emgService.initialize();

    // Ahora que los servicios viven, le damos sus referencias al ConfigService
    // para reconfiguración runtime (ej. emg setThresholds en caliente).
    // No tenemos setter pero podemos reasignar via apply() futuro; por ahora,
    // construimos el CalibrationManager y le pasamos los punteros vivos.
    Services::CalibrationManager::Targets calTargets {};
    calTargets.imuDriver = &mpu;
    calTargets.emgSvc    = emgOk ? &emgService : nullptr;

    static Services::CalibrationManager calibrationMgr(
        calStore, calTargets, /*autoSaveOnComplete=*/ true);
    calibrationMgr.initialize();

    // Suscribir al EventBus para auto-save tras CALIBRATION_COMPLETE
    eventBus.subscribe(&calibrationMgr);

    // Cargar offsets persistidos (si existen) y aplicarlos a IMU+EMG
    calibrationMgr.loadAndApply();

    // =========================================================
    // 5) SafetyService + Adapters + Validator
    //    Orden CRÍTICO: safety necesita ServoManager (IMotionExecutor).
    // =========================================================
    static Services::SafetyService safetyService(
        servoManager, eventBus, safetyCfg);

    if (!safetyService.initialize())
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    {
        ESP_LOGE(TAG, "SafetyService init FAILED");
        return;   // sin safety, no operamos
    }

<<<<<<< HEAD
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
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
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
<<<<<<< HEAD
    cliDeps.safetyMonitor   = &safetyService;
    cliDeps.configService   = &configService;
    cliDeps.calibrationMgr  = &calibrationMgr;

    static Services::CLIService cliService(console, cliDeps);
    static App::DemoMode        demoMode(dispatcher);
// =========================================================
    // 7.5) Diagnostics (creado ANTES del CLI para que CLI lo vea)
    // =========================================================
    Tasks::TaskDiagnostics::Dependencies diagDeps {};
    diagDeps.safetyMonitor   = &safetyService;
    diagDeps.dispatcherStats = &dispatcher;
    diagDeps.imuSvc          = imuOk ? &imuService : nullptr;
    diagDeps.emgSvc          = emgOk ? &emgService : nullptr;

    static Tasks::TaskDiagnostics taskDiag(diagDeps);

    // =========================================================
    // 8) CLI (con todas sus dependencias listas)
    // =========================================================
    cliDeps.diagnostics = &taskDiag;


    // =========================================================
    // 9) Tasks
    // =========================================================
=======
    cliDeps.safety          = &safety;

    static Services::CLIService cliService(console, cliDeps);
    cliService.initialize();

    static App::DemoMode demoMode(dispatcher);

    // ============================================================
    // 11) Tasks RTOS
    // ============================================================
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    static Tasks::TaskMotion taskMotion(
        motionService,
        servoManager,
        Core::QueueManager::motionCommandQueue()
    );
<<<<<<< HEAD
    taskMotion.attachWatchdog(&watchdog);

    static Tasks::TaskSafety        taskSafety(safetyService, watchdog);
    static Tasks::TaskCLI           taskCli(console, cliService, demoMode);
    static Tasks::TaskIMU           taskImu(imuService);
    static Tasks::TaskEMG           taskEmg(emgService);
    static Tasks::TaskSystemMonitor taskSysMon(safetyService);

    // ---- Arrancar Safety primero ----
    if (!taskSafety.start())
    {
        ESP_LOGE(TAG, "TaskSafety start failed → aborting boot");
        safetyService.triggerEmergencyStop(
            Models::SafetyFault::INIT_FAILURE);
    }
=======

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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

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

<<<<<<< HEAD
    // ---- Registrar TODAS las tasks críticas en SystemMonitor ----
    taskSysMon.registerWatchedTask(taskMotion.handle(),  "Motion");
    taskSysMon.registerWatchedTask(taskSafety.handle(),  "Safety");
    taskSysMon.registerWatchedTask(taskCli.handle(),     "CLI");
    if (imuOk) taskSysMon.registerWatchedTask(taskImu.handle(), "IMU");
    if (emgOk) taskSysMon.registerWatchedTask(taskEmg.handle(), "EMG");
    taskSysMon.start();

    // ---- Mismas tasks en Diagnostics ----
    taskDiag.addWatchedTask(taskMotion.handle(), "Motion");
    taskDiag.addWatchedTask(taskSafety.handle(), "Safety");
    taskDiag.addWatchedTask(taskCli.handle(),    "CLI");
    if (imuOk) taskDiag.addWatchedTask(taskImu.handle(), "IMU");
    if (emgOk) taskDiag.addWatchedTask(taskEmg.handle(), "EMG");

    taskDiag.start();

    // ---- Boot final ----
    if (!servoManager.initialize() ? false : true)
    {
        // (placeholder: servos ya inicializaron arriba; aquí sólo logueamos)
    }

    if (!imuOk || !emgOk)
    {
        ESP_LOGW(TAG, "Boot complete with DEGRADED sensors (imu=%s emg=%s)",
                 imuOk ? "OK" : "down",
                 emgOk ? "OK" : "down");
    }
    else
    {
        ESP_LOGI(TAG, "Boot complete. All systems nominal.");
    }

    // =========================================================
    // 10) Main loop: solo idle. Todo el trabajo real está en tasks.
    //     Verificamos esporádicamente la salud del bootstrap.
    // =========================================================
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(30000));   // 30 s — no hace nada útil

        // Sanidad mínima: si Safety está FATAL, no hay nada que hacer.
        const auto st = safetyService.getStatus();
        if (st.state == Models::SafetyState::FATAL)
        {
            ESP_LOGE(TAG, "FATAL safety state → halting main loop");
            // Mantenemos la task viva para que el watchdog del bootloader
            // siga teniendo este símbolo; las tasks RTOS siguen corriendo.
        }
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    }
}