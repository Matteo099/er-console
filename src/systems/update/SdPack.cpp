#include "SdPack.h"
#include "config/Updates.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>
#include <mbedtls/sha256.h>

namespace {
struct Digest {
    mbedtls_sha256_context value;

    Digest() {
        mbedtls_sha256_init(&value);
        mbedtls_sha256_starts_ret(&value, 0);
    }

    ~Digest() {
        mbedtls_sha256_free(&value);
    }

    void add(const uint8_t* bytes, size_t size) {
        mbedtls_sha256_update_ret(&value, bytes, size);
    }

    void finish(uint8_t* out) {
        mbedtls_sha256_finish_ret(&value, out);
    }
};

std::string hex(const uint8_t* data) {
    constexpr char digits[] = "0123456789abcdef";
    std::string out(64, '0');
    for (unsigned i = 0; i < 32; ++i) {
        out[i * 2] = digits[data[i] >> 4];
        out[i * 2 + 1] = digits[data[i] & 15];
    }
    return out;
}

uint32_t u32(const uint8_t* p) {
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}

std::string folded(std::string value) {
    for (auto& c : value)
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
    return value;
}
}  // namespace

bool SdPack::validDigest(const std::string& value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}

bool SdPack::validPath(const std::string& path) {
    if (path.empty() || path.size() > 160 || path.find_first_of("\\:*?\"<>|") != std::string::npos)
        return false;
    for (unsigned char c : path)
        if (c < 32 || c >= 127)
            return false;
    size_t begin = 0;
    unsigned segments = 0;
    while (begin < path.size()) {
        if (++segments > 12)
            return false;
        const auto end = path.find('/', begin);
        const auto part = path.substr(begin, end == std::string::npos ? end : end - begin);
        if (part.empty() || part == "." || part == ".." || part.back() == '.' || part.back() == ' ')
            return false;
        if (end == std::string::npos)
            break;
        begin = end + 1;
        if (begin == path.size())
            return false;
    }
    return path == "build-info.json" || path.compare(0, 12, ".core/audio/") == 0 ||
           path.compare(0, 13, ".core/images/") == 0 || path.compare(0, 13, ".core/portal/") == 0 ||
           path.compare(0, 6, "games/") == 0;
}

std::string SdPack::sha256(const uint8_t* data, size_t size) {
    Digest hash;
    hash.add(data, size);
    uint8_t bytes[32];
    hash.finish(bytes);
    return hex(bytes);
}

UpdateError SdPack::install(UpdateInput& input, SdPackSink& sink, const UpdateRelease& release,
                            UpdateEngine::Progress progress, UpdateEngine::Cancel cancelled) {
    struct Cleanup {
        SdPackSink& sink;
        bool done = false;

        explicit Cleanup(SdPackSink& value) : sink(value) {
        }

        ~Cleanup() {
            if (!done)
                sink.abort();
        }
    } cleanup{sink};

    if (!validDigest(release.resourcesSha256) || release.resourcesSize < 12 ||
        release.resourcesSize > UpdateConfig::SDPACK_LIMIT)
        return UpdateError::Metadata;
    if (input.contentLength() >= 0 && uint64_t(input.contentLength()) != release.resourcesSize)
        return UpdateError::Size;
    Digest pack;
    size_t received = 0;
    const auto read = [&](uint8_t* data, size_t size) {
        if (size > release.resourcesSize - received)
            return false;
        while (size) {
            if (cancelled())
                return false;
            const int count = input.read(data, size);
            if (count <= 0 || size_t(count) > size)
                return false;
            pack.add(data, count);
            received += count;
            data += count;
            size -= count;
        }
        return true;
    };
    const auto readError = [&]() {
        return cancelled() ? UpdateError::Cancelled : UpdateError::Size;
    };
    uint8_t header[38];
    if (!read(header, 12))
        return readError();
    const unsigned files = u32(header + 8);
    if (std::memcmp(header, "ERSDPK01", 8) || !files || files > 1024)
        return UpdateError::Metadata;
    if (!sink.begin(release.resourcesSize, files))
        return sink.error();
    vTaskDelay(1);
    std::string previous;
    uint8_t buffer[4096];
    for (unsigned i = 0; i < files; ++i) {
        if (!read(header, sizeof(header)))
            return readError();
        const unsigned length = unsigned(header[0]) | unsigned(header[1]) << 8;
        const size_t size = u32(header + 2);
        if (!length || length > 160 || size > release.resourcesSize - received)
            return UpdateError::Size;
        std::string path(length, '\0');
        if (!read(reinterpret_cast<uint8_t*>(&path[0]), length))
            return readError();
        const auto key = folded(path);
        if (!validPath(path) || key <= previous)
            return UpdateError::Metadata;
        previous = key;
        if (!sink.open(path, size))
            return sink.error();
        Digest file;
        size_t remaining = size;
        while (remaining) {
            const size_t chunk = std::min(remaining, sizeof(buffer));
            if (!read(buffer, chunk))
                return readError();
            file.add(buffer, chunk);
            if (!sink.write(buffer, chunk))
                return sink.error();
            remaining -= chunk;
            progress(UpdateState::Resources, received, received);
            vTaskDelay(1);
        }
        uint8_t digest[32];
        file.finish(digest);
        if (std::memcmp(digest, header + 6, 32))
            return UpdateError::Integrity;
        if (!sink.finishFile(digest))
            return cancelled() ? UpdateError::Cancelled : sink.error();
        vTaskDelay(1);
    }
    if (received != release.resourcesSize)
        return UpdateError::Size;
    uint8_t extra;
    if (input.read(&extra, 1) != 0 || !input.complete())
        return UpdateError::Size;
    uint8_t digest[32];
    pack.finish(digest);
    if (hex(digest) != release.resourcesSha256)
        return UpdateError::Integrity;
    if (cancelled())
        return UpdateError::Cancelled;
    if (!sink.commit())
        return sink.error();
    cleanup.done = true;
    progress(UpdateState::Resources, received, received);
    return UpdateError::None;
}
