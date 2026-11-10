#pragma once
#include <Arduino.h>
#include "UpdateEngine.h"
#include <esp_http_client.h>
#include <memory>

struct UpdateBufferFree
{
    void operator()(char *value) const;
};
using UpdateBuffer = std::unique_ptr<char, UpdateBufferFree>;

class UpdateHttp : public UpdateInput
{
public:
    explicit UpdateHttp(UpdateEngine::Cancel cancel);
    ~UpdateHttp();
    bool open(const std::string &url, const std::string &token, bool binary);
    UpdateError json(const std::string &url, const std::string &token, size_t maximum, UpdateBuffer &buffer, size_t &size, bool asset = false);
    int read(uint8_t *data, size_t capacity) override;
    bool complete() const override;
    int64_t contentLength() const override { return length; }
    UpdateError error() const { return problem; }
    int status() const { return statusCode; }

private:
    static esp_err_t event(esp_http_client_event_t *value);
    void close();
    esp_http_client_handle_t client = nullptr;
    UpdateEngine::Cancel cancelled;
    std::string location;
    bool rateLimited = false;
    int statusCode = 0;
    int64_t length = -1;
    uint32_t lastData = 0;
    UpdateError problem = UpdateError::None;
};
