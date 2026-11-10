#include "UpdateEngine.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>
#include <mbedtls/sha256.h>
#include <esp_app_format.h>

namespace
{
    struct Hash
    {
        mbedtls_sha256_context context;
        Hash() { mbedtls_sha256_init(&context); }
        ~Hash() { mbedtls_sha256_free(&context); }
    };
    struct Transaction
    {
        UpdateFlash &flash;
        bool active = false;
        explicit Transaction(UpdateFlash &value) : flash(value) {}
        ~Transaction()
        {
            if (active)
                flash.abort();
        }
    };
}
bool UpdateEngine::compatibleHeader(const uint8_t *data, size_t size)
{
    const size_t offset = sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t);
    if (!data || size < offset + sizeof(esp_app_desc_t))
        return false;
    esp_image_header_t header;
    esp_app_desc_t app;
    std::memcpy(&header, data, sizeof(header));
    std::memcpy(&app, data + offset, sizeof(app));
    return header.magic == ESP_IMAGE_HEADER_MAGIC && header.chip_id == ESP_CHIP_ID_ESP32S3 &&
           header.segment_count > 0 && header.segment_count <= ESP_IMAGE_MAX_SEGMENTS &&
           app.magic_word == ESP_APP_DESC_MAGIC_WORD;
}
UpdateError UpdateEngine::install(UpdateInput &input, UpdateFlash &flash, const UpdateRelease &release,
                                  Progress progress, Cancel cancelled)
{
    if (release.size < 288 || release.sha256.size() != 64)
        return UpdateError::Metadata;
    if (input.contentLength() > 0 && uint64_t(input.contentLength()) != release.size)
        return UpdateError::Size;
    uint8_t buffer[4096];
    size_t downloaded = 0, written = 0, buffered = 0;
    Hash hash;
    Transaction transaction(flash);
    if (mbedtls_sha256_starts_ret(&hash.context, 0))
        return UpdateError::Integrity;
    // Check the real chip/image header before erasing the inactive slot.
    while (buffered < 288)
    {
        if (cancelled())
            return UpdateError::Cancelled;
        const int count = input.read(buffer + buffered, 288 - buffered);
        if (cancelled())
            return UpdateError::Cancelled;
        if (count <= 0 || size_t(count) > 288 - buffered)
            return UpdateError::Network;
        buffered += count;
    }
    if (!compatibleHeader(buffer, buffered))
        return UpdateError::Image;
    if (cancelled())
        return UpdateError::Cancelled;
    if (!flash.begin(release.size))
        return UpdateError::Flash;
    transaction.active = true;
    while (downloaded < release.size)
    {
        if (cancelled())
            return UpdateError::Cancelled;
        const int count = buffered ? int(buffered) : input.read(buffer, std::min(sizeof(buffer), release.size - downloaded));
        buffered = 0;
        if (cancelled())
            return UpdateError::Cancelled;
        if (count <= 0 || size_t(count) > sizeof(buffer) || size_t(count) > release.size - downloaded)
            return UpdateError::Network;
        downloaded += count;
        if (mbedtls_sha256_update_ret(&hash.context, buffer, count))
            return UpdateError::Integrity;
        progress(UpdateState::Downloading, downloaded, written);
        if (!flash.write(buffer, count))
            return UpdateError::Flash;
        written += count;
        progress(UpdateState::Downloading, downloaded, written);
        vTaskDelay(1);
    }
    // Complete framing is checked too; do not accept truncated or appended data.
    const int tail = input.read(buffer, 1);
    if (cancelled())
        return UpdateError::Cancelled;
    if (tail != 0 || !input.complete())
        return UpdateError::Size;
    progress(UpdateState::Verifying, downloaded, written);
    uint8_t digest[32];
    char actual[65]{};
    if (mbedtls_sha256_finish_ret(&hash.context, digest))
        return UpdateError::Integrity;
    const char digits[] = "0123456789abcdef";
    for (unsigned i = 0; i < 32; ++i)
    {
        actual[i * 2] = digits[digest[i] >> 4];
        actual[i * 2 + 1] = digits[digest[i] & 15];
    }
    unsigned difference = 0;
    for (unsigned i = 0; i < 64; ++i)
        difference |= uint8_t(actual[i]) ^ uint8_t(release.sha256[i]);
    if (difference)
        return UpdateError::Integrity;
    if (cancelled())
        return UpdateError::Cancelled;
    progress(UpdateState::Installing, downloaded, written);
    if (cancelled())
        return UpdateError::Cancelled;
    if (!flash.finish())
        return UpdateError::Image; // esp_ota_end verifies the entire ESP image.
    transaction.active = false;
    if (!flash.activate(release.version))
        return UpdateError::Persistence;
    return UpdateError::None;
}
