idf_component_register(
    SRCS
        "main.cpp"

        "core/EventBus.cpp"
        "core/CommandDispatcher.cpp"
        "core/QueueManager.cpp"
        "core/SystemContext.cpp"
        "core/BasicSafetyValidator.cpp"

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

        "communication/CommandParser.cpp"
        "communication/UARTConsole.cpp"

        "app/DemoMode.cpp"

        "tasks/TaskIMU.cpp"
        "tasks/TaskEMG.cpp"
        "tasks/TaskMotion.cpp"
        "tasks/TaskCLI.cpp"

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