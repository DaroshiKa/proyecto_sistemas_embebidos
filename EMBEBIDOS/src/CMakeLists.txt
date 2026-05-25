idf_component_register(
    SRCS
        "main.cpp"

        "core/EventBus.cpp"
        "core/CommandDispatcher.cpp"
        "core/QueueManager.cpp"
        "core/SystemContext.cpp"

        "hal/GPIOHal.cpp"
        "hal/ADCHal.cpp"
        "hal/PWMHal.cpp"
        "hal/UARTHal.cpp"
        "hal/I2CHal.cpp"

        "drivers/ServoDriver.cpp"

    INCLUDE_DIRS
        "."
)