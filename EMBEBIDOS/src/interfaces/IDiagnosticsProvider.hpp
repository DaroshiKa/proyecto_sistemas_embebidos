#pragma once

#include "models/DiagnosticsSnapshot.hpp"

namespace Interfaces
{
    class IDiagnosticsProvider
    {
    public:
        virtual ~IDiagnosticsProvider() = default;

        virtual Models::DiagnosticsSnapshot snapshot() const = 0;
    };
}