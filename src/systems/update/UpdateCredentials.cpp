#include "UpdateCredentials.h"
#include "UpdateMetadata.h"
#include "config/Updates.h"
#include <nvs.h>

std::string UpdateCredentials::load() const
{
    nvs_handle_t handle;
    if (nvs_open(name, NVS_READONLY, &handle) != ESP_OK)
        return {};
    size_t size = 0;
    char value[UpdateConfig::TOKEN_LIMIT + 1]{};
    bool valid = nvs_get_str(handle, "token", nullptr, &size) == ESP_OK && size && size <= sizeof(value);
    if (valid)
        valid = nvs_get_str(handle, "token", value, &size) == ESP_OK;
    nvs_close(handle);
    std::string result = valid ? value : "";
    return UpdateMetadata::validToken(result) ? result : std::string{};
}
bool UpdateCredentials::save(const std::string &value) const
{
    if (!UpdateMetadata::validToken(value))
        return false;
    nvs_handle_t handle;
    if (nvs_open(name, NVS_READWRITE, &handle) != ESP_OK)
        return false;
    auto result = value.empty() ? nvs_erase_key(handle, "token") : nvs_set_str(handle, "token", value.c_str());
    if (value.empty() && result == ESP_ERR_NVS_NOT_FOUND)
        result = ESP_OK;
    const bool saved = result == ESP_OK && nvs_commit(handle) == ESP_OK;
    nvs_close(handle);
    return saved;
}
