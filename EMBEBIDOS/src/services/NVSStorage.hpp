#pragma once

#include "interfaces/IPersistentStorage.hpp"
#include "nvs_flash.h"
#include "nvs.h"

namespace Services
{
    // Implementación concreta de IPersistentStorage sobre nvs_flash.
    // Un único namespace lógico por instancia (default: "robohand").
    class NVSStorage final : public Interfaces::IPersistentStorage
    {
    public:
        // Inicializa nvs_flash() de manera segura (re-formatea si está
        // corrupto). DEBE llamarse una sola vez en el boot, ANTES de
        // abrir cualquier instancia de NVSStorage.
        static bool initBackend();

        explicit NVSStorage(const char* nsName = "robohand");
        ~NVSStorage() override;

        bool open();
        void close();

        bool isReady() const override { return open_; }

        size_t readBlob(
            const char* key,
            void* outBuffer,
            size_t outBufferSize
        ) override;

        bool writeBlob(
            const char* key,
            const void* data,
            size_t length
        ) override;

        bool erase(const char* key) override;
        bool eraseAll() override;

    private:
        const char*       nsName_;
        nvs_handle_t      handle_ { 0 };
        bool              open_   { false };
    };
}