#include "services/NVSStorage.hpp"

#include "esp_log.h"
#include <cstring>

namespace Services
{
    static constexpr const char* TAG = "NVSStore";

    bool NVSStorage::initBackend()
    {
        esp_err_t err = nvs_flash_init();

        if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
            err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_LOGW(TAG, "nvs corrupted/outdated → erasing");
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
            return false;
        }

        ESP_LOGI(TAG, "NVS backend ready");
        return true;
    }

    NVSStorage::NVSStorage(const char* nsName)
        : nsName_(nsName)
    {
    }

    NVSStorage::~NVSStorage()
    {
        close();
    }

    bool NVSStorage::open()
    {
        if (open_) return true;

        const esp_err_t err = nvs_open(nsName_, NVS_READWRITE, &handle_);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "nvs_open(%s) failed: %s",
                     nsName_, esp_err_to_name(err));
            return false;
        }
        open_ = true;
        ESP_LOGI(TAG, "ns=%s opened", nsName_);
        return true;
    }

    void NVSStorage::close()
    {
        if (!open_) return;
        nvs_close(handle_);
        handle_ = 0;
        open_   = false;
    }

    size_t NVSStorage::readBlob(
        const char* key,
        void* outBuffer,
        size_t outBufferSize
    )
    {
        if (!open_ || key == nullptr || outBuffer == nullptr) return 0;

        size_t required = 0;
        esp_err_t err = nvs_get_blob(handle_, key, nullptr, &required);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            return 0;
        }
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "size query '%s' err=%s", key, esp_err_to_name(err));
            return 0;
        }

        if (required > outBufferSize)
        {
            ESP_LOGW(TAG, "blob '%s' too big: stored=%u buf=%u",
                     key, static_cast<unsigned>(required),
                     static_cast<unsigned>(outBufferSize));
            return 0;
        }

        err = nvs_get_blob(handle_, key, outBuffer, &required);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "read '%s' err=%s", key, esp_err_to_name(err));
            return 0;
        }
        return required;
    }

    bool NVSStorage::writeBlob(
        const char* key,
        const void* data,
        size_t length
    )
    {
        if (!open_ || key == nullptr || data == nullptr || length == 0)
            return false;

        esp_err_t err = nvs_set_blob(handle_, key, data, length);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "set_blob '%s' err=%s", key, esp_err_to_name(err));
            return false;
        }

        err = nvs_commit(handle_);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "commit '%s' err=%s", key, esp_err_to_name(err));
            return false;
        }
        ESP_LOGI(TAG, "wrote '%s' (%u bytes)", key,
                 static_cast<unsigned>(length));
        return true;
    }

    bool NVSStorage::erase(const char* key)
    {
        if (!open_ || key == nullptr) return false;

        const esp_err_t err = nvs_erase_key(handle_, key);
        if (err == ESP_ERR_NVS_NOT_FOUND) return false;
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "erase '%s' err=%s", key, esp_err_to_name(err));
            return false;
        }

        return nvs_commit(handle_) == ESP_OK;
    }

    bool NVSStorage::eraseAll()
    {
        if (!open_) return false;

        if (nvs_erase_all(handle_) != ESP_OK) return false;
        return nvs_commit(handle_) == ESP_OK;
    }
}