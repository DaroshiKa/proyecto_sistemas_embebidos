#include "core/QueueManager.hpp"

namespace Core
{
    QueueHandle_t QueueManager::motionQueue_ = nullptr;

    bool QueueManager::initialize()
    {
        motionQueue_ = xQueueCreate(
            16,
            sizeof(Models::MotionCommand)
        );

        return motionQueue_ != nullptr;
    }

    QueueHandle_t QueueManager::motionCommandQueue()
    {
        return motionQueue_;
    }
}