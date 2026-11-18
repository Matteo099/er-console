#include "UpdateHttp.h"
#include "UpdateMetadata.h"
#include "config/Updates.h"
#include "log/Logger.h"
#include <Arduino.h>
#include <WiFi.h>
// Arduino 2 ships a same-named header with the same guard but different symbols.
// Use the SDK bundle API, which includes its compiled default root store.
extern "C" esp_err_t esp_crt_bundle_attach(void* configuration);
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_tls.h>
#include <mbedtls/platform.h>
#include <mbedtls/ssl.h>
#include <cstring>
#include <cerrno>
#include <algorithm>

namespace {
void* tlsCalloc(size_t count, size_t size) {
    // The prebuilt core allocates TLS records only in internal RAM. Scene
    // changes can fragment that heap below the required contiguous 16 KB.
    const auto caps = MALLOC_CAP_8BIT | (psramFound() ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL);
    return heap_caps_calloc(count, size, caps);
}
}  // namespace

void UpdateBufferFree::operator()(char* value) const {
    heap_caps_free(value);
}

UpdateHttp::UpdateHttp(UpdateEngine::Cancel cancel) : cancelled(std::move(cancel)) {
    // Configure once, without temporarily switching allocators underneath
    // other TLS users. heap_caps_free handles allocations from either heap.
    static const int allocator = mbedtls_platform_set_calloc_free(tlsCalloc, heap_caps_free);
    (void)allocator;
    // Keep transfer logging from flooding the serial port.
    esp_log_level_set("HTTP_CLIENT", ESP_LOG_WARN);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_WARN);
    esp_log_level_set("esp-tls", ESP_LOG_WARN);
}

UpdateHttp::~UpdateHttp() {
    close();
}

UpdateError UpdateHttp::transportError(const char* operation, int code) const {
    const auto wifi = WiFi.status();
    Log::warn("OTA", "HTTP %s failed: code=%d, Wi-Fi=%u", operation, code, unsigned(wifi));
    if (tlsMemoryFailure)
        return UpdateError::Memory;
    return wifi == WL_CONNECTED ? UpdateError::Network : UpdateError::WifiDisconnected;
}

void UpdateHttp::close() {
    if (client) {
        esp_http_client_cleanup(client);
        client = nullptr;
    }
}

esp_err_t UpdateHttp::event(esp_http_client_event_t* value) {
    auto& self = *static_cast<UpdateHttp*>(value->user_data);
    if (value->event_id == HTTP_EVENT_ERROR && value->data) {
        int tlsCode = 0;
        const auto error =
            esp_tls_get_and_clear_last_error(static_cast<esp_tls_error_handle_t>(value->data),
                                             &tlsCode, nullptr);
        self.tlsMemoryFailure =
            tlsCode == MBEDTLS_ERR_SSL_ALLOC_FAILED || tlsCode == -MBEDTLS_ERR_SSL_ALLOC_FAILED;
        Log::warn("OTA", "TLS failed: code=%d, detail=%d", int(error), tlsCode);
        return ESP_OK;
    }
    if (value->event_id != HTTP_EVENT_ON_HEADER || !value->header_key || !value->header_value)
        return ESP_OK;
    if (!strcasecmp(value->header_key, "Location") && std::strlen(value->header_value) <= 4096)
        self.location = value->header_value;
    if (!strcasecmp(value->header_key, "X-RateLimit-Remaining") &&
        !std::strcmp(value->header_value, "0"))
        self.rateLimited = true;
    return ESP_OK;
}

bool UpdateHttp::open(const std::string& initial, bool binary) {
    std::string url = initial;
    for (unsigned redirects = 0; redirects < 6; ++redirects) {
        close();
        location.clear();
        rateLimited = false;
        tlsMemoryFailure = false;
        statusCode = 0;
        length = -1;
        problem = UpdateError::None;
        if (cancelled()) {
            problem = UpdateError::Cancelled;
            return false;
        }
        if (!UpdateMetadata::trustedDownloadUrl(url)) {
            problem = UpdateError::Metadata;
            return false;
        }
        if (WiFi.status() != WL_CONNECTED) {
            problem = UpdateError::WifiDisconnected;
            return false;
        }
        esp_http_client_config_t config{};
        config.url = url.c_str();
        config.timeout_ms = 4000;
        config.disable_auto_redirect = true;
        config.buffer_size = 4096;
        config.buffer_size_tx = 1024;
        config.crt_bundle_attach = esp_crt_bundle_attach;
        config.event_handler = event;
        config.user_data = this;
        client = esp_http_client_init(&config);
        if (!client) {
            problem = UpdateError::Memory;
            return false;
        }
        esp_http_client_set_header(client, "User-Agent", "Escaperiom-OTA");
        esp_http_client_set_header(client, "Accept",
                                   binary ? "application/octet-stream"
                                          : "application/vnd.github+json");
        esp_http_client_set_header(client, "Accept-Encoding", "identity");
        if (url.compare(0, 23, "https://api.github.com/") == 0)
            esp_http_client_set_header(client, "X-GitHub-Api-Version", "2022-11-28");
        const auto opened = esp_http_client_open(client, 0);
        if (opened != ESP_OK) {
            problem =
                opened == ESP_ERR_NO_MEM ? UpdateError::Memory : transportError("connect", opened);
            return false;
        }
        if (esp_http_client_fetch_headers(client) < 0) {
            problem = transportError("headers", esp_http_client_get_errno(client));
            return false;
        }
        statusCode = esp_http_client_get_status_code(client);
        length = esp_http_client_get_content_length(client);
        if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 307 ||
            statusCode == 308) {
            if (location.empty() || !UpdateMetadata::trustedDownloadUrl(location)) {
                problem = UpdateError::Metadata;
                return false;
            }
            url = location;
            continue;
        }
        if (statusCode != 200) {
            problem = statusCode == 404 ? UpdateError::NotFound
                      : statusCode == 429 || (statusCode == 403 && rateLimited)
                          ? UpdateError::RateLimit
                          : UpdateError::Http;
            return false;
        }
        lastData = millis();
        return true;
    }
    problem = UpdateError::Http;
    return false;
}

bool UpdateHttp::complete() const {
    return client && esp_http_client_is_complete_data_received(client);
}

int UpdateHttp::read(uint8_t* data, size_t capacity) {
    while (client) {
        if (cancelled()) {
            problem = UpdateError::Cancelled;
            return -1;
        }
        errno = 0;
        const int count = esp_http_client_read(client, reinterpret_cast<char*>(data), capacity);
        if (count > 0) {
            lastData = millis();
            return count;
        }
        if (count == 0 && complete())
            return 0;
        if (WiFi.status() != WL_CONNECTED) {
            problem = UpdateError::WifiDisconnected;
            return -1;
        }
        if (uint32_t(millis() - lastData) >= UpdateConfig::IO_TIMEOUT_MS) {
            problem = UpdateError::Timeout;
            return -1;
        }
        if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK && count != -ESP_ERR_HTTP_EAGAIN) {
            problem = transportError("read", count);
            return -1;
        }
        vTaskDelay(1);
    }
    return -1;
}

UpdateError UpdateHttp::json(const std::string& url, size_t maximum, UpdateBuffer& buffer,
                             size_t& size, bool asset) {
    size = 0;
    if (!open(url, asset))
        return problem;
    if (length > int64_t(maximum))
        return UpdateError::Metadata;
    uint32_t caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
    if (!psramFound()) {
        if (heap_caps_get_free_size(MALLOC_CAP_INTERNAL) < maximum + 64 * 1024)
            return UpdateError::Memory;
        caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
    }
    buffer.reset(static_cast<char*>(heap_caps_malloc(maximum + 1, caps)));
    if (!buffer)
        return UpdateError::Memory;
    while (size < maximum) {
        const auto count = read(reinterpret_cast<uint8_t*>(buffer.get() + size),
                                std::min<size_t>(4096, maximum - size));
        if (count < 0)
            return problem;
        if (!count)
            break;
        size += count;
    }
    uint8_t extra;
    if (size == maximum && read(&extra, 1) != 0)
        return UpdateError::Metadata;
    if (!complete())
        return transportError("body", ESP_FAIL);
    buffer.get()[size] = 0;
    close();
    return UpdateError::None;
}
