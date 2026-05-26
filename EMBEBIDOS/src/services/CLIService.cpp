#include "services/CLIService.hpp"

#include <cstring>

#include "esp_system.h"
#include "esp_timer.h"

#include "services/SafetyService.hpp"   // NUEVO en Etapa 9: necesario para usar el puntero

namespace Services
{
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

    void CLIService::handleLine(const char* line)
    {
        Communication::ParsedCommand p {};

        if (!Communication::CommandParser::parse(line, p))
        {
            return;
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

    // ---------- Handlers ----------

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
        console_.writeLine("  safety status | clear | safemode");   // ACTUALIZADO en Etapa 9
        console_.writeLine("  emergency_stop");
        console_.writeLine("  demo start | stop");
    }

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

    void CLIService::cmdHome()
    {
        dispatchMotion(Models::MotionType::HOME);
    }

    void CLIService::cmdReturnHome()
    {
        dispatchMotion(Models::MotionType::RETURN_HOME);
    }

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

    // ---------- AMPLIADO en Etapa 9 ----------
    void CLIService::cmdSafety(const Communication::ParsedCommand& p)
    {
        if (Communication::CommandParser::equals(p.subverb, "status"))
        {
            printSafetyStats();
        }
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
        }
    }

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

    void CLIService::cmdDemo(const Communication::ParsedCommand& p)
    {
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

    // ---------- Helpers de dispatch ----------

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

    // ---------- Printers ----------

    void CLIService::printServoStatusTable()
    {
        if (deps_.executor == nullptr)
        {
            console_.writeLine("ERR: executor not available");
            return;
        }

        console_.writeLine("");
        console_.writeLine("ID  Name     cur     tgt     moving");
        console_.writeLine("----------------------------------------");

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
                    "%u   %-7s  %6.1f  %6.1f  %s\r\n",
                    static_cast<unsigned>(i),
                    names[i],
                    st.currentAngle,
                    st.targetAngle,
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
        }
    }

    // ---------- String helpers ----------

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
        }
    }
}