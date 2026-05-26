#pragma once

#include "communication/CommandParser.hpp"
#include "communication/UARTConsole.hpp"

#include "interfaces/ICommandDispatcher.hpp"
#include "interfaces/IIMUSource.hpp"
#include "interfaces/IEMGSource.hpp"
#include "interfaces/IMotionExecutor.hpp"

#include "core/CommandDispatcher.hpp"

#include "models/MotionCommand.hpp"
#include "models/ServoState.hpp"

// Forward declaration para evitar incluir SafetyService aquí
// (rompería ciclos si en algún momento Safety incluye algo del CLI).
namespace Services { class SafetyService; }

namespace Services
{
    // Dependencias necesarias para que el CLI pueda diagnosticar
    // y emitir comandos. Inyección opcional (nullptr = capacidad ausente).
    struct CLIDependencies
    {
        Interfaces::ICommandDispatcher*  dispatcher       { nullptr };
        Interfaces::IIMUSource*          imu              { nullptr };
        Interfaces::IEMGSource*          emg              { nullptr };
        Interfaces::IMotionExecutor*     executor         { nullptr };
        Core::CommandDispatcher*         dispatcherStats  { nullptr };
        Services::SafetyService*         safety           { nullptr };  // NUEVO en Etapa 9
    };

    class CLIService
    {
    public:
        CLIService(
            Communication::UARTConsole& console,
            const CLIDependencies& deps
        );

        // Procesa una línea recibida del usuario.
        void handleLine(const char* line);

        // Imprime el prompt
        void printPrompt();

        // Imprime banner inicial
        void printBanner();

    private:
        // Handlers
        void cmdHelp();
        void cmdSystem(const Communication::ParsedCommand& p);
        void cmdHand(const Communication::ParsedCommand& p);
        void cmdWrist(const Communication::ParsedCommand& p);
        void cmdElbow(const Communication::ParsedCommand& p);
        void cmdHome();
        void cmdReturnHome();
        void cmdServo(const Communication::ParsedCommand& p);
        void cmdImu(const Communication::ParsedCommand& p);
        void cmdEmg(const Communication::ParsedCommand& p);
        void cmdSafety(const Communication::ParsedCommand& p);
        void cmdEmergencyStop();
        void cmdDemo(const Communication::ParsedCommand& p);

        // Helpers de dispatch
        bool dispatchMotion(
            Models::MotionType type,
            Models::CommandPriority priority = Models::CommandPriority::NORMAL
        );

        bool dispatchCustomServo(uint8_t servoId, float angle);

        // Helpers de impresión
        void printServoStatusTable();
        void printImuStatus();
        void printEmgStatus();
        void printSystemStatus();
        void printSafetyStats();

        const char* motionTypeStr(Models::MotionType t) const;
        const char* planeStr(Models::OrientationPlane p) const;
        const char* gestureStr(Models::EMGGesture g) const;
        const char* imuStateStr(Models::IMUState s) const;
        const char* emgStateStr(Models::EMGState s) const;
        const char* systemStateStr(Models::SystemState s) const;  // NUEVO en Etapa 9

        Communication::UARTConsole& console_;
        CLIDependencies             deps_;
    };
}