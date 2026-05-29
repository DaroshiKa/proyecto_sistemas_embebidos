# 1. Buscar recursivamente todos los archivos .cpp y .c
file(GLOB_RECURSE app_sources "*.cpp" "*.c")

<<<<<<< HEAD
# 2. Registrar el componente usando la lista generada
idf_component_register(SRCS ${app_sources}
                       INCLUDE_DIRS ".")
=======
        "core/EventBus.cpp"
        "core/CommandDispatcher.cpp"
        "core/QueueManager.cpp"
        "core/SystemContext.cpp"
        "core/BasicSafetyValidator.cpp"
        "core/SystemStateMachine.cpp"          # ← NUEVO Etapa 9

        "hal/GPIOHal.cpp"
        "hal/ADCHal.cpp"
        "hal/PWMHal.cpp"
        "hal/UARTHal.cpp"
        "hal/I2CHal.cpp"

        "drivers/ServoDriver.cpp"
        "drivers/MPU6050Driver.cpp"
        "drivers/EMGDriver.cpp"
        "drivers/ServoManager.cpp"

        "services/IMUService.cpp"
        "services/EMGService.cpp"
        "services/MotionService.cpp"
        "services/CLIService.cpp"
        "services/SafetyService.cpp"           # ← NUEVO Etapa 9

        "communication/CommandParser.cpp"
        "communication/UARTConsole.cpp"
        "communication/NextionInterface.cpp"
        "communication/TelemetryPublisher.cpp"
        "communication/SystemDataProvider.cpp"

        "protocols/CRC16.cpp"
        "protocols/PacketSerializer.cpp"
        "protocols/PacketParser.cpp"

        "app/DemoMode.cpp"

        "tasks/TaskIMU.cpp"
        "tasks/TaskEMG.cpp"
        "tasks/TaskMotion.cpp"
        "tasks/TaskCLI.cpp"
        "tasks/TaskNextionInterface.cpp"
        "tasks/TaskSafety.cpp"                 # ← NUEVO Etapa 9

        "utils/ComplementaryFilter.cpp"
        "utils/PlaneDetector.cpp"
        "utils/BiquadFilter.cpp"
        "utils/DCRemoveFilter.cpp"
        "utils/MovingAverage.cpp"
        "utils/ThresholdDetector.cpp"
        "utils/EMGProcessor.cpp"
        "utils/GestureFSM.cpp"
        "utils/JointInterpolator.cpp"

    INCLUDE_DIRS
        "."
)
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
