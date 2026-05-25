#pragma once

namespace Interfaces
{
    class IService
    {
    public:
        virtual ~IService() = default;

        virtual bool initialize() = 0;

        virtual void update() = 0;
    };
}