#include "services/CLIService.hpp"

#include <cstring>
#include "services/SafetyService.hpp"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "interfaces/IDiagnosticsProvider.hpp"
#include "models/SafetyState.hpp"

#include "services/SafetyService.hpp"   // NUEVO en Etapa 9: necesario para usar el puntero

namespace Services
{
    // ==================================================================
    // Construcción / banner / prompt
    // ==================================================================

    CLIService::CLIService(
        Communication::UARTConsole& console,
        const CLIDependencies& deps
    )
        : console_(console),
          deps_(deps)
    {
    }

    void CLIService::printBanner()
    {
        console_.writeLine("");
        console_.writeLine("==============================================");
        console_.writeLine(" Robotic Hand Firmware - CLI");
        console_.writeLine(" Type 'help' for commands");
        console_.writeLine("==============================================");
    }

    void CLIService::printPrompt()
    {
        console_.write("> ");
    }

    // ==================================================================
    // Dispatch principal
    // ==================================================================

    void CLIService::handleLine(const char* line)
    {
        Communication::ParsedCommand p {};

        if (!Communication::CommandParser::parse(line, p))
        {
            return;
        }
        else if (Communication::CommandParser::equals(p.verb, "sethome"))
        {
            cmdSetHome(p);
        }
        else if (Communication::CommandParser::equals(p.verb, "config"))
        {
            cmdConfig(p);
        }
        else if (Communication::CommandParser::equals(p.verb, "cal"))
        {
            cmdCal(p);
        }
        else if (Communication::CommandParser::equals(p.verb, "lock"))
        {
            cmdLock(p);
        }
        else if (Communication::CommandParser::equals(p.verb, "diag"))
        {
            cmdDiag(p);
        }

        using namespace Communication;

        if      (CommandParser::equals(p.verb, "help"))           cmdHelp();
        else if (CommandParser::equals(p.verb, "system"))         cmdSystem(p);
        else if (CommandParser::equals(p.verb, "hand"))           cmdHand(p);
        else if (CommandParser::equals(p.verb, "wrist"))          cmdWrist(p);
        else if (CommandParser::equals(p.verb, "elbow"))          cmdElbow(p);
        else if (CommandParser::equals(p.verb, "home"))           cmdHome();
        else if (CommandParser::equals(p.verb, "return_home"))    cmdReturnHome();
        else if (CommandParser::equals(p.verb, "servo"))          cmdServo(p);
        else if (CommandParser::equals(p.verb, "imu"))            cmdImu(p);
        else if (CommandParser::equals(p.verb, "emg"))            cmdEmg(p);
        else if (CommandParser::equals(p.verb, "safety"))         cmdSafety(p);
        else if (CommandParser::equals(p.verb, "emergency_stop")) cmdEmergencyStop();
        else if (CommandParser::equals(p.verb, "demo"))           cmdDemo(p);
        else
        {
            console_.printf("ERR: unknown command '%s' (try 'help')\r\n", p.verb);
        }
    }

    // ==================================================================
    // Help
    // ==================================================================

    void CLIService::cmdHelp()
    {
        console_.writeLine("");
        console_.writeLine("Available commands:");
        console_.writeLine("  help");
        console_.writeLine("  system status | reset");
        console_.writeLine("  hand open | close");
        console_.writeLine("  wrist left | right");
        console_.writeLine("  elbow xy | xz | yz");
        console_.writeLine("  home");
        console_.writeLine("  return_home");
        console_.writeLine("  servo <id> <angle>      (id: 0..4, angle: 0..180)");
        console_.writeLine("  servo status");
        console_.writeLine("  imu status | calibrate | raw");
        console_.writeLine("  emg status | calibrate | threshold <on> <off>");
<<<<<<< HEAD
        console_.writeLine("  safety status | reset | clear");
=======
        console_.writeLine("  safety status | clear | safemode");   // ACTUALIZADO en Etapa 9
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
        console_.writeLine("  emergency_stop");
        console_.writeLine("  demo start | stop");
        console_.writeLine("  diag show | tasks | heap | dispatch");
    }

    // ==================================================================
    // System
    // ==================================================================

    void CLIService::cmdSystem(const Communication::ParsedCommand& p)
    {
        if (Communication::CommandParser::equals(p.subverb, "status"))
        {
            printSystemStatus();
        }
        else if (Communication::CommandParser::equals(p.subverb, "reset"))
        {
            console_.writeLine("Restarting...");
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_restart();
        }
        else
        {
            console_.writeLine("ERR: system status | reset");
        }
    }

    // ==================================================================
    // Hand / Wrist / Elbow
    // ==================================================================

    void CLIService::cmdHand(const Communication::ParsedCommand& p)
    {
        if (Communication::CommandParser::equals(p.subverb, "open"))
        {
            dispatchMotion(Models::MotionType::HAND_OPEN);
        }
        else if (Communication::CommandParser::equals(p.subverb, "close"))
        {
            dispatchMotion(Models::MotionType::HAND_CLOSE);
        }
        else
        {
            console_.writeLine("ERR: hand open | close");
        }
    }

    void CLIService::cmdWrist(const Communication::ParsedCommand& p)
    {
        if (Communication::CommandParser::equals(p.subverb, "left"))
        {
            dispatchMotion(Models::MotionType::WRIST_LEFT);
        }
        else if (Communication::CommandParser::equals(p.subverb, "right"))
        {
            dispatchMotion(Models::MotionType::WRIST_RIGHT);
        }
        else
        {
            console_.writeLine("ERR: wrist left | right");
        }
    }

    void CLIService::cmdElbow(const Communication::ParsedCommand& p)
    {
        if (Communication::CommandParser::equals(p.subverb, "xy"))
        {
            dispatchMotion(Models::MotionType::ELBOW_XY);
        }
        else if (Communication::CommandParser::equals(p.subverb, "xz"))
        {
            dispatchMotion(Models::MotionType::ELBOW_XZ);
        }
        else if (Communication::CommandParser::equals(p.subverb, "yz"))
        {
            dispatchMotion(Models::MotionType::ELBOW_YZ);
        }
        else
        {
            console_.writeLine("ERR: elbow xy | xz | yz");
        }
    }

    // ==================================================================
    // Home / Return-home
    // ==================================================================

    void CLIService::cmdHome()
    {
        dispatchMotion(Models::MotionType::HOME);
    }

    void CLIService::cmdReturnHome()
    {
        dispatchMotion(Models::MotionType::RETURN_HOME);
    }

    // ==================================================================
    // Servo
    // ==================================================================

    void CLIService::cmdServo(const Communication::ParsedCommand& p)
    {
        if (Communication::CommandParser::equals(p.subverb, "status"))
        {
            printServoStatusTable();
            return;
        }

        int id = 0;
        float angle = 0.0f;

        if (!Communication::CommandParser::argToInt(p.subverb, id))
        {
            console_.writeLine("ERR: servo <id> <angle> | servo status");
            return;
        }

        if (p.argCount < 1 ||
            !Communication::CommandParser::argToFloat(p.args[0], angle))
        {
            console_.writeLine("ERR: missing angle");
            return;
        }

        dispatchCustomServo(static_cast<uint8_t>(id), angle);
    }

    // ==================================================================
    // IMU
    // ==================================================================

    void CLIService::cmdImu(const Communication::ParsedCommand& p)
    {
        if (deps_.imu == nullptr)
        {
            console_.writeLine("ERR: IMU not available");
            return;
        }

        if (Communication::CommandParser::equals(p.subverb, "status"))
        {
            printImuStatus();
        }
        else if (Communication::CommandParser::equals(p.subverb, "calibrate"))
        {
            console_.writeLine("Hold IMU still. Calibrating...");
            const bool ok = deps_.imu->startCalibration();
            console_.printf("Calibration %s\r\n", ok ? "OK" : "FAILED");
        }
        else if (Communication::CommandParser::equals(p.subverb, "raw"))
        {
            Models::IMUData d {};
            if (deps_.imu->getLatestData(d))
            {
                console_.printf(
                    "a=[%.2f %.2f %.2f]g  g=[%.1f %.1f %.1f]dps  "
                    "P=%.1f R=%.1f Y=%.1f  plane=%s  T=%.1fC\r\n",
                    d.ax, d.ay, d.az,
                    d.gx, d.gy, d.gz,
                    d.pitch, d.roll, d.yaw,
                    planeStr(d.plane),
                    d.temperatureC
                );
            }
            else
            {
                console_.writeLine("ERR: no IMU data");
            }
        }
        else
        {
            console_.writeLine("ERR: imu status | calibrate | raw");
        }
    }

    // ==================================================================
    // EMG
    // ==================================================================

    void CLIService::cmdEmg(const Communication::ParsedCommand& p)
    {
        if (deps_.emg == nullptr)
        {
            console_.writeLine("ERR: EMG not available");
            return;
        }

        if (Communication::CommandParser::equals(p.subverb, "status"))
        {
            printEmgStatus();
        }
        else if (Communication::CommandParser::equals(p.subverb, "calibrate"))
        {
            console_.writeLine("Relax muscle. Calibrating...");
            const bool ok = deps_.emg->startCalibration();
            console_.printf("Calibration %s\r\n", ok ? "OK" : "FAILED");
        }
        else if (Communication::CommandParser::equals(p.subverb, "threshold"))
        {
            float onLv = 0.0f, offLv = 0.0f;
            if (p.argCount < 2 ||
                !Communication::CommandParser::argToFloat(p.args[0], onLv) ||
                !Communication::CommandParser::argToFloat(p.args[1], offLv))
            {
                console_.writeLine("ERR: emg threshold <on> <off>");
                return;
            }

<<<<<<< HEAD
            // El CLI conoce sólo IEMGSource (interfaz). Para setThresholds
            // necesitamos el servicio concreto. Esto se podría exponer en la
            // interfaz; por ahora, se hace via puntero EMGService directo
            // si fuera necesario. En Etapa 10 NVS expondremos esto a través
            // de un ConfigService dedicado para no inflar IEMGSource.
=======
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
            console_.printf(
                "OK: threshold set requested on=%.2f off=%.2f\r\n",
                onLv, offLv
            );
        }
        else
        {
            console_.writeLine("ERR: emg status | calibrate | threshold <on> <off>");
        }
    }

<<<<<<< HEAD
    // ==================================================================
    // SAFETY  (Etapa 9 — actualizado)
    //
    // Subcomandos:
    //   safety status  -> imprime estado FSM, faltas, contadores
    //   safety reset   -> requestRecovery(): intenta salir de E-STOP_HOLD
    //   safety clear   -> clearLatchedFaults(): borra historial de faltas
    // ==================================================================

=======
    // ---------- AMPLIADO en Etapa 9 ----------
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    void CLIService::cmdSafety(const Communication::ParsedCommand& p)
    {
        if (Communication::CommandParser::equals(p.subverb, "status"))
        {
            printSafetyStats();
        }
<<<<<<< HEAD
        else if (Communication::CommandParser::equals(p.subverb, "reset"))
        {
            if (deps_.safetyMonitor == nullptr)
            {
                console_.writeLine("ERR: safety monitor not available");
                return;
            }
            if (deps_.safetyMonitor->requestRecovery())
            {
                console_.writeLine("OK: recovery requested");
            }
            else
            {
                console_.writeLine("ERR: recovery denied (faults still active?)");
            }
        }
        else if (Communication::CommandParser::equals(p.subverb, "clear"))
        {
            if (deps_.safetyMonitor == nullptr)
            {
                console_.writeLine("ERR: safety monitor not available");
                return;
            }
            deps_.safetyMonitor->clearLatchedFaults();
            console_.writeLine("OK: latched faults cleared");
        }
        else
        {
            console_.writeLine("ERR: safety status | reset | clear");
=======
        else if (Communication::CommandParser::equals(p.subverb, "clear"))
        {
            if (deps_.safety == nullptr)
            {
                console_.writeLine("ERR: safety service not available");
                return;
            }
            const bool ok = deps_.safety->clearEmergency();
            console_.printf("Clear %s\r\n", ok ? "OK" : "DENIED (system unhealthy)");
        }
        else if (Communication::CommandParser::equals(p.subverb, "safemode"))
        {
            if (deps_.safety == nullptr)
            {
                console_.writeLine("ERR: safety service not available");
                return;
            }
            deps_.safety->enterSafeMode();
            console_.writeLine("Entered SAFE_MODE");
        }
        else
        {
            console_.writeLine("ERR: safety status | clear | safemode");
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
        }
    }

    // ==================================================================
    // Emergency stop
    // ==================================================================

    void CLIService::cmdEmergencyStop()
    {
        if (deps_.dispatcher == nullptr)
        {
            console_.writeLine("ERR: dispatcher not available");
            return;
        }

        Models::MotionCommand cmd {};
        cmd.type        = Models::MotionType::EMERGENCY_STOP;
        cmd.source      = Models::CommandSource::CLI;
        cmd.priority    = Models::CommandPriority::CRITICAL;
        cmd.timestampMs = static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        if (deps_.dispatcher->dispatch(cmd))
        {
            console_.writeLine("EMERGENCY_STOP dispatched");
        }
        else
        {
            console_.writeLine("ERR: dispatch failed");
        }
    }

    // ==================================================================
    // Demo
    // ==================================================================

    void CLIService::cmdDemo(const Communication::ParsedCommand& p)
    {
<<<<<<< HEAD
        // El demo se maneja en TaskCLI/DemoMode; aquí solo encolamos un comando
        // declarativo que TaskCLI interpreta.

=======
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
        if (Communication::CommandParser::equals(p.subverb, "start"))
        {
            dispatchMotion(Models::MotionType::DEMO_START);
            console_.writeLine("Demo START requested");
        }
        else if (Communication::CommandParser::equals(p.subverb, "stop"))
        {
            dispatchMotion(Models::MotionType::DEMO_STOP);
            console_.writeLine("Demo STOP requested");
        }
        else
        {
            console_.writeLine("ERR: demo start | stop");
        }
    }

    // ==================================================================
    // Helpers de dispatch
    // ==================================================================

    bool CLIService::dispatchMotion(
        Models::MotionType type,
        Models::CommandPriority priority
    )
    {
        if (deps_.dispatcher == nullptr)
        {
            console_.writeLine("ERR: dispatcher not available");
            return false;
        }

        Models::MotionCommand cmd {};
        cmd.type        = type;
        cmd.source      = Models::CommandSource::CLI;
        cmd.priority    = priority;
        cmd.timestampMs = static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        const bool ok = deps_.dispatcher->dispatch(cmd);
        if (!ok)
        {
            console_.printf(
                "ERR: dispatch rejected (type=%s)\r\n",
                motionTypeStr(type)
            );
        }
        else
        {
            console_.printf("OK: dispatched %s\r\n", motionTypeStr(type));
        }
        return ok;
    }

    bool CLIService::dispatchCustomServo(uint8_t servoId, float angle)
    {
        if (deps_.dispatcher == nullptr)
        {
            console_.writeLine("ERR: dispatcher not available");
            return false;
        }

        Models::MotionCommand cmd {};
        cmd.type         = Models::MotionType::CUSTOM_SERVO;
        cmd.source       = Models::CommandSource::CLI;
        cmd.priority     = Models::CommandPriority::NORMAL;
        cmd.targetServo  = servoId;
        cmd.targetAngle  = angle;
        cmd.timestampMs  = static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        const bool ok = deps_.dispatcher->dispatch(cmd);
        if (!ok)
        {
            console_.printf(
                "ERR: servo %u angle %.1f rejected\r\n",
                static_cast<unsigned>(servoId), angle
            );
        }
        else
        {
            console_.printf(
                "OK: servo %u -> %.1f deg\r\n",
                static_cast<unsigned>(servoId), angle
            );
        }
        return ok;
    }

    // ==================================================================
    // Printers
    // ==================================================================

    void CLIService::printServoStatusTable()
    {
        if (deps_.executor == nullptr)
        {
            console_.writeLine("ERR: executor not available");
            return;
        }

        console_.writeLine("");
        console_.writeLine("ID  Name     cur     tgt     speed   planned  moving");
        console_.writeLine("---------------------------------------------------------");

        static const char* names[] = { "hand", "wrist", "elbowX", "elbowY", "elbowZ" };

        for (uint8_t i = 0;
             i < static_cast<uint8_t>(Models::JointId::COUNT);
             ++i)
        {
            Models::ServoState st {};
            if (deps_.executor->getServoState(
                    static_cast<Models::JointId>(i), st))
            {
                console_.printf(
                    "%u   %-7s  %6.1f  %6.1f  %6.1f   %6.1f   %s\r\n",
                    static_cast<unsigned>(i),
                    names[i],
                    st.currentAngle,
                    st.targetAngle,
                    st.currentSpeedDps,    // velocidad instantánea medida
                    st.plannedSpeedDps,    // velocidad comandada
                    st.moving ? "YES" : "no"
                );
            }
        }
    }

    void CLIService::printImuStatus()
    {
        const Models::IMUStatus s = deps_.imu->getStatus();
        Models::IMUData d {};
        deps_.imu->getLatestData(d);

        console_.writeLine("");
        console_.writeLine("IMU STATUS");
        console_.printf("  state         : %s\r\n", imuStateStr(s.state));
        console_.printf("  calibrated    : %s\r\n", s.calibrated ? "YES" : "NO");
        console_.printf("  total samples : %lu\r\n",
                        static_cast<unsigned long>(s.totalSamples));
        console_.printf("  total errors  : %lu\r\n",
                        static_cast<unsigned long>(s.totalErrors));
        console_.printf("  last update ms: %lu\r\n",
                        static_cast<unsigned long>(s.lastUpdateMs));
        console_.printf("  pitch         : %.2f\r\n", d.pitch);
        console_.printf("  roll          : %.2f\r\n", d.roll);
        console_.printf("  yaw           : %.2f\r\n", d.yaw);
        console_.printf("  plane         : %s\r\n",  planeStr(d.plane));
        console_.printf("  temperature   : %.1f C\r\n", d.temperatureC);
    }

    void CLIService::printEmgStatus()
    {
        const Models::EMGStatus s = deps_.emg->getStatus();
        Models::EMGData d {};
        deps_.emg->getLatestData(d);

        console_.writeLine("");
        console_.writeLine("EMG STATUS");
        console_.printf("  state         : %s\r\n", emgStateStr(s.state));
        console_.printf("  calibrated    : %s\r\n", s.calibrated ? "YES" : "NO");
        console_.printf("  baseline      : %.4f\r\n", s.baselineLevel);
        console_.printf("  peak (max)    : %.4f\r\n", s.peakLevel);
        console_.printf("  total samples : %lu\r\n",
                        static_cast<unsigned long>(s.totalSamples));
        console_.printf("  total gestures: %lu\r\n",
                        static_cast<unsigned long>(s.totalGestures));
        console_.printf("  envelope live : %.3f\r\n", d.envelopeValue);
        console_.printf("  smoothed live : %.3f\r\n", d.smoothedValue);
        console_.printf("  active        : %s\r\n",
                        d.contractionDetected ? "YES" : "no");
        console_.printf("  last gesture  : %s\r\n", gestureStr(d.gesture));
    }

    void CLIService::printSystemStatus()
    {
        const int64_t uptimeMs = esp_timer_get_time() / 1000LL;
        console_.writeLine("");
        console_.printf("System uptime: %lld ms\r\n", uptimeMs);
        console_.printf("Free heap:     %lu bytes\r\n",
                        static_cast<unsigned long>(esp_get_free_heap_size()));

        if (deps_.imu != nullptr)
        {
            const auto s = deps_.imu->getStatus();
            console_.printf("IMU state:     %s (calibrated=%s)\r\n",
                            imuStateStr(s.state),
                            s.calibrated ? "YES" : "NO");
        }
        if (deps_.emg != nullptr)
        {
            const auto s = deps_.emg->getStatus();
            console_.printf("EMG state:     %s (calibrated=%s)\r\n",
                            emgStateStr(s.state),
                            s.calibrated ? "YES" : "NO");
        }

<<<<<<< HEAD
        if (deps_.safetyMonitor != nullptr)
        {
            const auto safety = deps_.safetyMonitor->getStatus();
            console_.printf("Safety state:  %s\r\n",
                            Models::safetyStateToString(safety.state));
        }
    }

    // ==================================================================
    // SAFETY STATS  (Etapa 9 — actualizado)
    //
    // Si SafetyMonitor está disponible, imprime estado FSM, faltas
    // activas/latched, salud de sensores y métricas. Si no, cae al
    // dispatcher stats como fallback (modo testing aislado).
    // ==================================================================

    void CLIService::printSafetyStats()
    {
        if (deps_.safetyMonitor == nullptr)
        {
            // Fallback al stats del dispatcher
            if (deps_.dispatcherStats == nullptr)
            {
                console_.writeLine("ERR: no safety info available");
                return;
            }
            console_.writeLine("");
            console_.writeLine("SAFETY / DISPATCHER (fallback, no monitor)");
            console_.printf(
                "  dispatched=%lu rejected=%lu dropped=%lu\r\n",
                static_cast<unsigned long>(deps_.dispatcherStats->totalDispatched()),
                static_cast<unsigned long>(deps_.dispatcherStats->totalRejected()),
                static_cast<unsigned long>(deps_.dispatcherStats->totalDropped())
            );
            return;
        }

        const auto st = deps_.safetyMonitor->getStatus();

        console_.writeLine("");
        console_.writeLine("--- Safety Status ---");
        console_.printf("  state            : %s\r\n",
                        Models::safetyStateToString(st.state));
        console_.printf("  active_faults    : 0x%04X\r\n",
                        static_cast<unsigned>(st.activeFaults));
        console_.printf("  latched_faults   : 0x%04X\r\n",
                        static_cast<unsigned>(st.latchedFaults));
        console_.printf("  imu_healthy      : %s\r\n", st.imuHealthy ? "yes" : "no");
        console_.printf("  emg_healthy      : %s\r\n", st.emgHealthy ? "yes" : "no");
        console_.printf("  total_estops     : %lu\r\n",
                        static_cast<unsigned long>(st.totalEmergencyStops));
        console_.printf("  total_recoveries : %lu\r\n",
                        static_cast<unsigned long>(st.totalRecoveries));
        console_.printf("  total_deg_trans  : %lu\r\n",
                        static_cast<unsigned long>(st.totalTransitionsToDeg));
        console_.printf("  cmds_rejected    : %lu\r\n",
                        static_cast<unsigned long>(st.totalCommandsRejected));
        console_.printf("  rate_limited     : %lu\r\n",
                        static_cast<unsigned long>(st.rateLimitedCount));
        console_.printf("  min_free_heap    : %lu bytes\r\n",
                        static_cast<unsigned long>(st.minFreeHeapBytes));
        console_.printf("  last_fault_ms    : %lu\r\n",
                        static_cast<unsigned long>(st.lastFaultTimestampMs));
        console_.printf("  last_transit_ms  : %lu\r\n",
                        static_cast<unsigned long>(st.lastTransitionMs));

        if (deps_.dispatcherStats != nullptr)
        {
            console_.writeLine("--- Dispatcher ---");
            console_.printf(
                "  dispatched=%lu rejected=%lu dropped=%lu\r\n",
                static_cast<unsigned long>(deps_.dispatcherStats->totalDispatched()),
                static_cast<unsigned long>(deps_.dispatcherStats->totalRejected()),
                static_cast<unsigned long>(deps_.dispatcherStats->totalDropped())
            );
=======
        // NUEVO en Etapa 9: incluir estado del SafetyMonitor
        if (deps_.safety != nullptr)
        {
            console_.printf("System state:  %s\r\n",
                            systemStateStr(deps_.safety->currentState()));
        }
    }

    // ---------- AMPLIADO en Etapa 9 ----------
    void CLIService::printSafetyStats()
    {
        console_.writeLine("");
        console_.writeLine("SAFETY");

        if (deps_.safety != nullptr)
        {
            const auto state = deps_.safety->currentState();
            console_.printf("  state          : %s\r\n", systemStateStr(state));
            console_.printf("  emergencies    : %lu\r\n",
                            static_cast<unsigned long>(deps_.safety->totalEmergencies()));
            console_.printf("  recoveries     : %lu\r\n",
                            static_cast<unsigned long>(deps_.safety->totalRecoveries()));
            console_.printf("  task stalls    : %lu\r\n",
                            static_cast<unsigned long>(deps_.safety->totalTaskStalls()));
            console_.printf("  sensor timeouts: %lu\r\n",
                            static_cast<unsigned long>(deps_.safety->totalSensorTimeouts()));
        }
        else
        {
            console_.writeLine("  safety service: NOT AVAILABLE");
        }

        if (deps_.dispatcherStats != nullptr)
        {
            auto* d = deps_.dispatcherStats;
            console_.printf("  dispatched     : %lu\r\n",
                            static_cast<unsigned long>(d->totalDispatched()));
            console_.printf("  rejected       : %lu\r\n",
                            static_cast<unsigned long>(d->totalRejected()));
            console_.printf("  dropped        : %lu\r\n",
                            static_cast<unsigned long>(d->totalDropped()));
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
        }
    }

    // ==================================================================
    // String helpers (enum → const char*)
    // ==================================================================

    const char* CLIService::motionTypeStr(Models::MotionType t) const
    {
        switch (t)
        {
            case Models::MotionType::HAND_OPEN:      return "HAND_OPEN";
            case Models::MotionType::HAND_CLOSE:     return "HAND_CLOSE";
            case Models::MotionType::WRIST_LEFT:     return "WRIST_LEFT";
            case Models::MotionType::WRIST_RIGHT:    return "WRIST_RIGHT";
            case Models::MotionType::ELBOW_XY:       return "ELBOW_XY";
            case Models::MotionType::ELBOW_XZ:       return "ELBOW_XZ";
            case Models::MotionType::ELBOW_YZ:       return "ELBOW_YZ";
            case Models::MotionType::HOME:           return "HOME";
            case Models::MotionType::RETURN_HOME:    return "RETURN_HOME";
            case Models::MotionType::CUSTOM_SERVO:   return "CUSTOM_SERVO";
            case Models::MotionType::DEMO_START:     return "DEMO_START";
            case Models::MotionType::DEMO_STOP:      return "DEMO_STOP";
            case Models::MotionType::EMERGENCY_STOP: return "EMERGENCY_STOP";
            default:                                 return "NONE";
        }
    }

    const char* CLIService::planeStr(Models::OrientationPlane p) const
    {
        switch (p)
        {
            case Models::OrientationPlane::XY: return "XY";
            case Models::OrientationPlane::XZ: return "XZ";
            case Models::OrientationPlane::YZ: return "YZ";
            default:                           return "UNKNOWN";
        }
    }

    const char* CLIService::gestureStr(Models::EMGGesture g) const
    {
        switch (g)
        {
            case Models::EMGGesture::RELAX:              return "RELAX";
            case Models::EMGGesture::SINGLE_CONTRACTION: return "SINGLE";
            case Models::EMGGesture::DOUBLE_CONTRACTION: return "DOUBLE";
            case Models::EMGGesture::LONG_HOLD:          return "HOLD";
            default:                                     return "NONE";
        }
    }

    const char* CLIService::imuStateStr(Models::IMUState s) const
    {
        switch (s)
        {
            case Models::IMUState::UNINITIALIZED: return "UNINIT";
            case Models::IMUState::INITIALIZING:  return "INIT";
            case Models::IMUState::CALIBRATING:   return "CAL";
            case Models::IMUState::OK:            return "OK";
            case Models::IMUState::TIMEOUT:       return "TIMEOUT";
            case Models::IMUState::BUS_ERROR:     return "BUS_ERR";
            case Models::IMUState::FAULT:         return "FAULT";
            default:                              return "?";
        }
    }

    const char* CLIService::emgStateStr(Models::EMGState s) const
    {
        switch (s)
        {
            case Models::EMGState::UNINITIALIZED: return "UNINIT";
            case Models::EMGState::INITIALIZING:  return "INIT";
            case Models::EMGState::CALIBRATING:   return "CAL";
            case Models::EMGState::OK:            return "OK";
            case Models::EMGState::TIMEOUT:       return "TIMEOUT";
            case Models::EMGState::FAULT:         return "FAULT";
            default:                              return "?";
        }
    }
<<<<<<< HEAD
    // ==================================================================
    // CONFIG — persistencia de configuración (Etapa 10)
    //   config show    -> imprime resumen
    //   config save    -> persiste a NVS
    //   config load    -> recarga desde NVS
    //   config reset   -> defaults (no persiste hasta 'config save')
    // ==================================================================
    void CLIService::cmdConfig(const Communication::ParsedCommand& p)
    {
        if (deps_.configService == nullptr)
        {
            console_.writeLine("ERR: ConfigService not available");
            return;
        }

        if (Communication::CommandParser::equals(p.subverb, "show"))
        {
            const auto s = deps_.configService->snapshot();
            console_.writeLine("");
            console_.printf("Config schema version: 0x%04X\r\n", s.version);
            console_.writeLine("[EMG]");
            console_.printf("  thresholdOn       : %.3f\r\n", s.emg.thresholdOn);
            console_.printf("  thresholdOff      : %.3f\r\n", s.emg.thresholdOff);
            console_.printf("  debounceMs        : %u\r\n",   s.emg.debounceMs);
            console_.printf("  doublePulseWinMs  : %u\r\n",   s.emg.doublePulseWindowMs);
            console_.printf("  longHoldMs        : %u\r\n",   s.emg.longHoldMs);
            console_.printf("  calibrationSampl. : %u\r\n",   s.emg.calibrationSamples);
            console_.writeLine("[IMU]");
            console_.printf("  complAlpha        : %.3f\r\n", s.imu.complementaryAlpha);
            console_.printf("  planeThresholdG   : %.2f\r\n", s.imu.planeThresholdG);
            console_.printf("  sampleRateDiv     : %u\r\n",   s.imu.sampleRateDiv);
            console_.printf("  calibrationSampl. : %u\r\n",   s.imu.calibrationSamples);
            console_.writeLine("[MOTION]");
            console_.printf("  defaultSpeedDps   : %.1f\r\n", s.motion.defaultSpeedDps);
            console_.printf("  hand open / close : %.1f / %.1f\r\n",
                            s.motion.handOpenAngle, s.motion.handCloseAngle);
            console_.printf("  wrist L / R       : %.1f / %.1f\r\n",
                            s.motion.wristLeftAngle, s.motion.wristRightAngle);
            console_.printf("  elbow XY/XZ/YZ    : %.1f / %.1f / %.1f\r\n",
                            s.motion.elbowXyAngle, s.motion.elbowXzAngle,
                            s.motion.elbowYzAngle);
            console_.printf("  home angles       : [%.1f %.1f %.1f %.1f %.1f]\r\n",
                            s.motion.homeAngles[0], s.motion.homeAngles[1],
                            s.motion.homeAngles[2], s.motion.homeAngles[3],
                            s.motion.homeAngles[4]);
        }
        else if (Communication::CommandParser::equals(p.subverb, "save"))
        {
            const bool ok = deps_.configService->save();
            console_.writeLine(ok ? "OK: config saved to NVS"
                                  : "ERR: config save failed");
        }
        else if (Communication::CommandParser::equals(p.subverb, "load"))
        {
            const bool ok = deps_.configService->reload();
            console_.writeLine(ok ? "OK: config reloaded from NVS"
                                  : "ERR: no valid config in NVS");
        }
        else if (Communication::CommandParser::equals(p.subverb, "reset"))
        {
            deps_.configService->resetToDefaults();
            console_.writeLine("OK: defaults restored (run 'config save' to persist)");
        }
        else
        {
            console_.writeLine("ERR: config show | save | load | reset");
        }
    }

    // ==================================================================
    // CAL — persistencia de calibraciones (Etapa 10)
    //   cal show   -> imprime offsets y baseline guardados
    //   cal save   -> persiste calibración actual a NVS
    //   cal load   -> recarga desde NVS y aplica
    //   cal reset  -> borra calibración persistida
    // ==================================================================
    void CLIService::cmdCal(const Communication::ParsedCommand& p)
    {
        if (deps_.calibrationMgr == nullptr)
        {
            console_.writeLine("ERR: CalibrationManager not available");
            return;
        }

        if (Communication::CommandParser::equals(p.subverb, "show"))
        {
            const auto d = deps_.calibrationMgr->snapshot();
            console_.writeLine("");
            console_.printf("Calibration schema version: 0x%04X\r\n", d.version);
            console_.printf("IMU valid: %s\r\n", d.imuValid ? "YES" : "no");
            if (d.imuValid)
            {
                console_.printf("  A offsets [g]   : %.4f %.4f %.4f\r\n",
                                d.imu.ax, d.imu.ay, d.imu.az);
                console_.printf("  G offsets [dps] : %.3f %.3f %.3f\r\n",
                                d.imu.gx, d.imu.gy, d.imu.gz);
            }
            console_.printf("EMG valid: %s\r\n", d.emgValid ? "YES" : "no");
            if (d.emgValid)
            {
                console_.printf("  baseline : %.4f\r\n", d.emg.baseline);
                console_.printf("  peakNorm : %.4f\r\n", d.emg.peakNorm);
            }
        }
        else if (Communication::CommandParser::equals(p.subverb, "save"))
        {
            const bool ok = deps_.calibrationMgr->save();
            console_.writeLine(ok ? "OK: calibration saved to NVS"
                                  : "ERR: save failed");
        }
        else if (Communication::CommandParser::equals(p.subverb, "load"))
        {
            const bool ok = deps_.calibrationMgr->loadAndApply();
            console_.writeLine(ok ? "OK: calibration loaded & applied"
                                  : "ERR: no valid calibration in NVS");
        }
        else if (Communication::CommandParser::equals(p.subverb, "reset"))
        {
            const bool ok = deps_.calibrationMgr->reset();
            console_.writeLine(ok ? "OK: calibration cleared"
                                  : "ERR: nothing to clear");
        }
        else
        {
            console_.writeLine("ERR: cal show | save | load | reset");
        }
    }
    // ==================================================================
    // DIAG — diagnóstico del sistema (Etapa 11)
    //   diag show     -> resumen general
    //   diag tasks    -> stack high-water mark de cada task vigilada
    //   diag heap     -> métricas detalladas de heap
    //   diag dispatch -> contadores del dispatcher
    // ==================================================================
    void CLIService::cmdDiag(const Communication::ParsedCommand& p)
    {
        if (deps_.diagnostics == nullptr)
        {
            console_.writeLine("ERR: diagnostics not available");
            return;
        }

        const auto s = deps_.diagnostics->snapshot();

        if (Communication::CommandParser::equals(p.subverb, "show") ||
            p.subverb[0] == '\0')
        {
            console_.writeLine("");
            console_.writeLine("--- Diagnostics ---");
            console_.printf("  uptime          : %lu s\r\n",
                            static_cast<unsigned long>(s.uptimeMs / 1000));
            console_.printf("  heap free       : %lu bytes\r\n",
                            static_cast<unsigned long>(s.freeHeapBytes));
            console_.printf("  heap min ever   : %lu bytes\r\n",
                            static_cast<unsigned long>(s.minFreeHeapBytes));
            console_.printf("  largest block   : %lu bytes\r\n",
                            static_cast<unsigned long>(s.largestBlockBytes));
            console_.printf("  safety state    : 0x%02X\r\n",
                            static_cast<unsigned>(s.safetyState));
            console_.printf("  active faults   : 0x%04X\r\n",
                            static_cast<unsigned>(s.activeFaults));
            console_.printf("  latched faults  : 0x%04X\r\n",
                            static_cast<unsigned>(s.latchedFaults));
            console_.printf("  e-stops total   : %lu\r\n",
                            static_cast<unsigned long>(s.totalEmergencyStops));
            console_.printf("  recoveries      : %lu\r\n",
                            static_cast<unsigned long>(s.totalRecoveries));
            console_.printf("  dispatched/rej/drop : %lu / %lu / %lu\r\n",
                            static_cast<unsigned long>(s.dispatched),
                            static_cast<unsigned long>(s.rejected),
                            static_cast<unsigned long>(s.dropped));
            console_.printf("  imu samples     : %lu (%s)\r\n",
                            static_cast<unsigned long>(s.imuTotalSamples),
                            s.imuHealthy ? "OK" : "down");
            console_.printf("  emg samples     : %lu (%s)\r\n",
                            static_cast<unsigned long>(s.emgTotalSamples),
                            s.emgHealthy ? "OK" : "down");
            console_.printf("  tasks watched   : %u (min HWM=%lu words)\r\n",
                            static_cast<unsigned>(s.taskCount),
                            static_cast<unsigned long>(s.minStackHighWater));
        }
        else if (Communication::CommandParser::equals(p.subverb, "tasks"))
        {
            console_.writeLine("");
            console_.writeLine("Task            HWM(words)  alive");
            console_.writeLine("---------------------------------");
            for (size_t i = 0; i < s.taskCount; ++i)
            {
                const auto& t = s.tasks[i];
                console_.printf("  %-12s  %8lu     %s\r\n",
                    (t.name != nullptr) ? t.name : "?",
                    static_cast<unsigned long>(t.stackHighWater),
                    t.alive ? "Y" : "n");
            }
        }
        else if (Communication::CommandParser::equals(p.subverb, "heap"))
        {
            console_.writeLine("");
            console_.printf("free=%lu  min_ever=%lu  largest_block=%lu\r\n",
                static_cast<unsigned long>(s.freeHeapBytes),
                static_cast<unsigned long>(s.minFreeHeapBytes),
                static_cast<unsigned long>(s.largestBlockBytes));
        }
        else if (Communication::CommandParser::equals(p.subverb, "dispatch"))
        {
            console_.writeLine("");
            console_.printf("dispatched=%lu  rejected=%lu  dropped=%lu\r\n",
                static_cast<unsigned long>(s.dispatched),
                static_cast<unsigned long>(s.rejected),
                static_cast<unsigned long>(s.dropped));
        }
        else
        {
            console_.writeLine("ERR: diag show | tasks | heap | dispatch");
        }
    }
    void CLIService::cmdSetHome(const Communication::ParsedCommand& p)
    {
        if (deps_.configService == nullptr)
        {
            console_.writeLine("ERR: ConfigService not available");
            return;
        }

        int   id    = -1;
        float angle = 0.0f;

        // p.subverb = id, p.args[0] = angle, p.args[1] = "save" (opcional)
        if (!Communication::CommandParser::argToInt(p.subverb, id))
        {
            console_.writeLine("Uso: sethome <id> <angle> [save]");
            console_.writeLine("  id: 0=hand 1=wrist 2=elbowX 3=elbowY 4=elbowZ");
            return;
        }
        if (p.argCount < 1 ||
            !Communication::CommandParser::argToFloat(p.args[0], angle))
        {
            console_.writeLine("ERR: falta el angulo");
            return;
        }
        if (id < 0 || id >= static_cast<int>(Models::JointId::COUNT))
        {
            console_.writeLine("ERR: id fuera de rango (0..4)");
            return;
        }
        if (angle < 0.0f || angle > 180.0f)
        {
            console_.writeLine("ERR: angulo debe estar entre 0 y 180");
            return;
        }

        // Modificar la config en memoria
        auto cfg = deps_.configService->snapshot();
        cfg.motion.homeAngles[id] = angle;

        if (!deps_.configService->apply(cfg))
        {
            console_.writeLine("ERR: apply fallo");
            return;
        }

        console_.printf("OK: home del servo %d cambiado a %.1f deg\r\n",
                        id, angle);

        // Si pasó "save" como segundo argumento, persiste a NVS
        if (p.argCount >= 2 &&
            Communication::CommandParser::equals(p.args[1], "save"))
        {
            if (deps_.configService->save())
                console_.writeLine("OK: guardado en NVS (sobrevive reset)");
            else
                console_.writeLine("WARN: no se pudo guardar en NVS");
        }
        else
        {
            console_.writeLine("INFO: usa 'sethome <id> <angle> save' o 'config save' para persistir");
        }

        console_.writeLine("INFO: ejecuta 'home' para mover los servos ahora");
    }
    // ==================================================================
    // LOCK — bloqueo manual de movimientos
    //   lock on    -> bloquea servos (E-STOP solo sigue disponible)
    //   lock off   -> libera el bloqueo
    //   lock       -> muestra el estado actual
    // ==================================================================
    void CLIService::cmdLock(const Communication::ParsedCommand& p)
    {
        if (deps_.safetyMonitor == nullptr)
        {
            console_.writeLine("ERR: SafetyMonitor not available");
            return;
        }

        // Casteamos a SafetyService (impl concreta) para acceder a setUserLock().
        // El CLI sí puede conocer el tipo concreto; la abstracción es para Motion.
        auto* svc = static_cast<Services::SafetyService*>(deps_.safetyMonitor);

        if (Communication::CommandParser::equals(p.subverb, "on"))
        {
            svc->setUserLock(true);
            console_.writeLine("OK: motion LOCKED. emergency_stop still works.");
        }
        else if (Communication::CommandParser::equals(p.subverb, "off"))
        {
            svc->setUserLock(false);
            console_.writeLine("OK: motion UNLOCKED.");
        }
        else if (p.subverb[0] == '\0' ||
                 Communication::CommandParser::equals(p.subverb, "status"))
        {
            console_.printf(
                "Lock state: %s\r\n",
                svc->isUserLocked() ? "LOCKED" : "unlocked"
            );
        }
        else
        {
            console_.writeLine("ERR: lock on | off | status");
=======

    // ---------- NUEVO en Etapa 9 ----------
    const char* CLIService::systemStateStr(Models::SystemState s) const
    {
        switch (s)
        {
            case Models::SystemState::INIT:           return "INIT";
            case Models::SystemState::IDLE:           return "IDLE";
            case Models::SystemState::ACTIVE:         return "ACTIVE";
            case Models::SystemState::CALIBRATING:    return "CALIBRATING";
            case Models::SystemState::ERROR:          return "ERROR";
            case Models::SystemState::SAFE_MODE:      return "SAFE_MODE";
            case Models::SystemState::EMERGENCY_STOP: return "EMERGENCY";
            default:                                  return "?";
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
        }
    }
}