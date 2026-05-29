#pragma once

#include <cstdint>
#include <cstddef>

namespace Interfaces
{
    // Abstracción sobre el almacenamiento no-volátil.
    // Implementación concreta: NVSStorage (nvs_flash de ESP-IDF).
    // Mañana se puede sustituir por SPIFFS, LittleFS, FRAM, etc.
    class IPersistentStorage
    {
    public:
        virtual ~IPersistentStorage() = default;

        virtual bool isReady() const = 0;

        // Lee un blob binario asociado a `key`.
        // Retorna número de bytes leídos, 0 si no existe o no cabe.
        virtual size_t readBlob(
            const char* key,
            void* outBuffer,
            size_t outBufferSize
        ) = 0;

        // Escribe un blob binario. Sobrescribe si existe.
        // Retorna true si commit OK.
        virtual bool writeBlob(
            const char* key,
            const void* data,
            size_t length
        ) = 0;

        // Borra una key. true si existía y se borró.
        virtual bool erase(const char* key) = 0;

        // Borra todo el namespace.
        virtual bool eraseAll() = 0;
    };
}