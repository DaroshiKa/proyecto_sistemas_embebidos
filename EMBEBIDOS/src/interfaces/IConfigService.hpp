#pragma once

#include "models/PersistentConfig.hpp"

namespace Interfaces
{
    class IConfigService
    {
    public:
        virtual ~IConfigService() = default;

        // Snapshot de la config actual en memoria.
        virtual Models::PersistentConfig snapshot() const = 0;

        // Sobrescribe la config en memoria (no persiste).
        // Aplica los cambios a los servicios suscritos.
        virtual bool apply(const Models::PersistentConfig& cfg) = 0;

        // Persiste la config actual al storage.
        virtual bool save() = 0;

        // Recarga desde storage. Si no hay o está corrupta, no toca nada
        // y retorna false.
        virtual bool reload() = 0;

        // Restaura defaults en memoria. NO persiste hasta save().
        virtual void resetToDefaults() = 0;
    };
}