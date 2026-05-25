#include "utils/GestureFSM.hpp"

namespace Utils
{
    GestureFSM::GestureFSM() = default;

    void GestureFSM::configure(const Models::EMGConfig& cfg)
    {
        doublePulseWindowMs_ = cfg.doublePulseWindowMs;
        singlePulseMinMs_    = cfg.singlePulseMinMs;
        longHoldMs_          = cfg.longHoldMs;
        relaxMs_             = cfg.relaxMs;

        reset();
    }

    void GestureFSM::reset()
    {
        state_ = State::IDLE;
        stateSince_ = 0;
        lastRelaxFiredAt_ = 0;
    }

    Models::EMGGesture GestureFSM::update(
        bool active,
        uint32_t nowMs
    )
    {
        const uint32_t elapsed = nowMs - stateSince_;

        switch (state_)
        {
            case State::IDLE:
            {
                if (active)
                {
                    state_ = State::CONTRACTION;
                    stateSince_ = nowMs;
                }
                else if (elapsed >= relaxMs_ &&
                         (nowMs - lastRelaxFiredAt_) >= relaxMs_)
                {
                    // RELAX se publica una vez tras periodo largo en idle
                    lastRelaxFiredAt_ = nowMs;
                    return Models::EMGGesture::RELAX;
                }
                break;
            }

            case State::CONTRACTION:
            {
                if (!active)
                {
                    if (elapsed >= singlePulseMinMs_)
                    {
                        state_ = State::AFTER_CONTRACTION;
                        stateSince_ = nowMs;
                    }
                    else
                    {
                        // glitch, ignorar
                        state_ = State::IDLE;
                        stateSince_ = nowMs;
                    }
                }
                else if (elapsed >= longHoldMs_)
                {
                    state_ = State::LONG_HOLD_FIRED;
                    stateSince_ = nowMs;
                    return Models::EMGGesture::LONG_HOLD;
                }
                break;
            }

            case State::AFTER_CONTRACTION:
            {
                if (active)
                {
                    // Segunda contracción dentro de la ventana → DOUBLE
                    if (elapsed <= doublePulseWindowMs_)
                    {
                        state_ = State::CONTRACTION;
                        stateSince_ = nowMs;
                        return Models::EMGGesture::DOUBLE_CONTRACTION;
                    }
                    else
                    {
                        // Demasiado tarde: trátalo como nueva contracción simple
                        state_ = State::CONTRACTION;
                        stateSince_ = nowMs;
                    }
                }
                else if (elapsed >= doublePulseWindowMs_)
                {
                    // No vino la segunda → fue contracción simple
                    state_ = State::IDLE;
                    stateSince_ = nowMs;
                    return Models::EMGGesture::SINGLE_CONTRACTION;
                }
                break;
            }

            case State::LONG_HOLD_FIRED:
            {
                if (!active)
                {
                    state_ = State::IDLE;
                    stateSince_ = nowMs;
                }
                break;
            }
        }

        return Models::EMGGesture::NONE;
    }
}