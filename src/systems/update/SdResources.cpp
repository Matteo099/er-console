#include "SdResources.h"
#include "config/Environment.h"
#include "config/Updates.h"
#include "drivers/bus/SPIBusLock.h"
#include "log/Logger.h"
#include <Arduino.h>
#include <SD.h>
#include <algorithm>
#include <cstring>
#include <mbedtls/sha256.h>

namespace {
bool cardAvailable = false;
std::string active;
bool healthy = false;
uint32_t checkedAt = 0;
constexpr const char* staging = "/.releases/staging";

std::string directory(const std::string& digest) {
    return "/.releases/" + digest;
}

std::string marker(const std::string& digest, const std::string& version) {
    return version + "\n" + digest + "\n" + OTA_HARDWARE_ID + "\n";
}

bool parents(const std::string& path) {
    for (size_t p = path.find('/', 1); p != std::string::npos; p = path.find('/', p + 1)) {
        const auto part = path.substr(0, p);
        if (!SD.exists(part.c_str()) && !SD.mkdir(part.c_str()))
            return false;
        vTaskDelay(1);
    }
    return true;
}

bool readEquals(const std::string& path, const std::string& expected) {
    auto file = SD.open(path.c_str(), FILE_READ);
    if (!file || file.isDirectory() || file.size() != expected.size())
        return false;
    for (unsigned char c : expected)
        if (file.read() != c)
            return false;
    return true;
}

bool writeText(const std::string& path, const std::string& text) {
    auto file = SD.open(path.c_str(), FILE_WRITE);
    if (!file ||
        file.write(reinterpret_cast<const uint8_t*>(text.data()), text.size()) != text.size())
        return false;
    file.flush();
    file.close();
    return readEquals(path, text);
}

bool inventoryValid(const std::string& prefix) {
    auto inventory = SD.open((prefix + "/.inventory").c_str());
    if (!inventory || inventory.size() > 200 * 1024)
        return false;
    const auto line = [&](std::string& out) {
        out.clear();
        while (inventory.available() && out.size() <= 180) {
            const int c = inventory.read();
            if (c == '\n')
                return true;
            if (c < 32 && c != '\t')
                return false;
            out += char(c);
        }
        return false;
    };
    std::string record;
    if (!line(record))
        return false;
    const auto number = [](const std::string& value, uint32_t& out) {
        out = 0;
        if (value.empty() || value.size() > 10)
            return false;
        for (char c : value) {
            if (c < '0' || c > '9' || out > (UINT32_MAX - unsigned(c - '0')) / 10)
                return false;
            out = out * 10 + c - '0';
        }
        return true;
    };
    uint32_t count;
    if (!number(record, count) || !count || count > 1024)
        return false;
    for (uint32_t i = 0; i < count; ++i) {
        if (!line(record))
            return false;
        const auto tab = record.find('\t');
        if (tab == std::string::npos || !SdPack::validPath(record.substr(0, tab)))
            return false;
        uint32_t size;
        if (!number(record.substr(tab + 1), size))
            return false;
        auto file = SD.open((prefix + "/" + record.substr(0, tab)).c_str());
        if (!file || file.isDirectory() || file.size() != size)
            return false;
    }
    return !inventory.available();
}

bool removeStaging(const std::string& path, unsigned depth = 0) {
    // This helper is used exclusively inside the owned, reserved staging tree.
    if (depth > 16 || (path != staging && path.compare(0, 19, "/.releases/staging/") != 0))
        return false;
    auto file = SD.open(path.c_str());
    if (!file)
        return true;
    if (!file.isDirectory()) {
        file.close();
        return SD.remove(path.c_str());
    }
    for (;;) {
        std::string name;
        for (;;) {
            auto child = file.openNextFile();
            if (!child)
                break;
            name = child.name();
            const auto slash = name.find_last_of('/');
            if (slash != std::string::npos)
                name.erase(0, slash + 1);
            child.close();
            if (path != staging || name != ".owner")
                break;
            name.clear();
        }
        // Close parents before recursing: the SD VFS has only five file slots.
        file.close();
        if (name.empty())
            break;
        if (!removeStaging(path + "/" + name, depth + 1))
            return false;
        file = SD.open(path.c_str());
        if (!file)
            return false;
    }
    if (path == staging && !SD.remove((path + "/.owner").c_str()))
        return false;
    return SD.rmdir(path.c_str());
}

class Store : public SdPackSink {
  public:
    Store(const UpdateRelease& value, UpdateEngine::Cancel cancel)
        : release(value), cancelled(std::move(cancel)) {
    }

    bool begin(size_t bytes, unsigned files) override {
        SPIBusLock::Guard lock;
        if (!cardAvailable)
            return false;
        if (SD.exists(staging)) {
            if (!readEquals(std::string(staging) + "/.owner", "ERSDPK01\n") ||
                !removeStaging(staging))
                return false;
        }
        const uint64_t total = SD.totalBytes(), used = SD.usedBytes();
        if (!total || used > total ||
            total - used < uint64_t(bytes) + uint64_t(files) * 32768 + 1024 * 1024) {
            failure = UpdateError::StorageSpace;
            return false;
        }
        if (!parents(std::string(staging) + "/.owner") ||
            !writeText(std::string(staging) + "/.owner", "ERSDPK01\n"))
            return false;
        owned = true;
        return writeText(std::string(staging) + "/.inventory", std::to_string(files) + "\n");
    }

    bool open(const std::string& path, size_t size) override {
        SPIBusLock::Guard lock;
        filename = std::string(staging) + "/" + path;
        logical = path;
        expectedSize = size;
        if (!parents(filename) || SD.exists(filename.c_str()))
            return false;
        file = SD.open(filename.c_str(), FILE_WRITE);
        return bool(file);
    }

    bool write(const uint8_t* data, size_t size) override {
        SPIBusLock::Guard lock;
        return file && file.write(data, size) == size;
    }

    bool finishFile(const uint8_t* expected) override {
        {
            SPIBusLock::Guard lock;
            file.flush();
            file.close();
            file = SD.open(filename.c_str(), FILE_READ);
            if (!file || file.size() != expectedSize)
                return false;
        }
        mbedtls_sha256_context hash;
        mbedtls_sha256_init(&hash);
        mbedtls_sha256_starts_ret(&hash, 0);
        uint8_t buffer[4096], digest[32];
        size_t remaining = expectedSize;
        bool okay = true;
        while (remaining && !cancelled()) {
            int count;
            {
                SPIBusLock::Guard lock;
                count = file.read(buffer, std::min(remaining, sizeof(buffer)));
            }
            if (count <= 0 || size_t(count) > remaining) {
                okay = false;
                break;
            }
            mbedtls_sha256_update_ret(&hash, buffer, count);
            remaining -= count;
            vTaskDelay(1);
        }
        mbedtls_sha256_finish_ret(&hash, digest);
        mbedtls_sha256_free(&hash);
        {
            SPIBusLock::Guard lock;
            file.close();
        }
        if (!okay || remaining || std::memcmp(expected, digest, 32))
            return false;
        SPIBusLock::Guard lock;
        auto inventory = SD.open((std::string(staging) + "/.inventory").c_str(), FILE_APPEND);
        const auto record = logical + "\t" + std::to_string(expectedSize) + "\n";
        if (!inventory || inventory.write(reinterpret_cast<const uint8_t*>(record.data()),
                                          record.size()) != record.size())
            return false;
        inventory.flush();
        return true;
    }

    bool commit() override {
        SPIBusLock::Guard lock;
        const auto target = directory(release.resourcesSha256);
        if (!writeText(std::string(staging) + "/.complete",
                       marker(release.resourcesSha256, release.version)))
            return false;
        if (!inventoryValid(staging))
            return false;
        std::string quarantine;
        if (SD.exists(target.c_str())) {
            quarantine = target + "-replaced-" + std::to_string(millis());
            if (SD.exists(quarantine.c_str()) || !SD.rename(target.c_str(), quarantine.c_str()))
                return false;
        }
        if (!SD.rename(staging, target.c_str())) {
            if (!quarantine.empty())
                SD.rename(quarantine.c_str(), target.c_str());
            return false;
        }
        owned = false;
        return SdResources::ready(release.resourcesSha256, release.version);
    }

    void abort() override {
        SPIBusLock::Guard lock;
        file.close();
        if (owned)
            removeStaging(staging);
        owned = false;
    }

    UpdateError error() const override {
        return failure;
    }

  private:
    const UpdateRelease& release;
    UpdateEngine::Cancel cancelled;
    File file;
    std::string filename, logical;
    size_t expectedSize = 0;
    bool owned = false;
    UpdateError failure = UpdateError::Storage;
};
}  // namespace

bool SdResources::required() {
    return SD_CONTENT_SHA256[0] != '\0';
}

bool SdResources::reserved(const char* path) {
    if (!path)
        return false;
    std::string value(path);
    for (auto& c : value)
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
    return value == "/.releases" || value.compare(0, 11, "/.releases/") == 0;
}

bool SdResources::ready(const std::string& digest, const std::string& version) {
    SPIBusLock::Guard lock;
    if (!cardAvailable || !SdPack::validDigest(digest))
        return false;
    const auto prefix = directory(digest);
    if (!readEquals(prefix + "/.complete", marker(digest, version)))
        return false;
    if (!inventoryValid(prefix))
        return false;
    for (const char* path :
         {"/.core/portal/index.html", "/.core/audio/lose.wav", "/.core/audio/win.wav",
          "/.core/audio/tickingclock.wav", "/.core/images/logo.png", "/games"})
        if (!SD.exists((prefix + path).c_str()))
            return false;
    return true;
}

void SdResources::mounted(bool available) {
    SPIBusLock::Guard lock;
    cardAvailable = available;
    active.clear();
    if (required() && ready(SD_CONTENT_SHA256, FW_VERSION))
        active = directory(SD_CONTENT_SHA256);
    healthy = !required() || !active.empty();
    checkedAt = millis();
    if (required()) {
        if (healthy)
            Log::info("OTA", "Selected SD resources: %s", SD_CONTENT_SHA256);
        else
            Log::warn("OTA", "Missing or incomplete SD resources: %s", SD_CONTENT_SHA256);
    }
}

bool SdResources::current() {
    SPIBusLock::Guard lock;
    if (!required())
        return true;
    if (active.empty())
        return false;
    if (uint32_t(millis() - checkedAt) >= 1000) {
        healthy = ready(SD_CONTENT_SHA256, FW_VERSION);
        checkedAt = millis();
    }
    return healthy;
}

std::string SdResources::root() {
    SPIBusLock::Guard lock;
    return active;
}

std::string SdResources::resolve(const char* logical, bool writing) {
    SPIBusLock::Guard lock;
    if (!logical || reserved(logical))
        return "";
    std::string path(logical);
    if (path.empty() || path[0] != '/' || path.find('\\') != std::string::npos)
        return "";
    size_t begin = 1;
    while (begin < path.size()) {
        const auto end = path.find('/', begin);
        const auto part = path.substr(begin, end == std::string::npos ? end : end - begin);
        if (part.empty() || part == "." || part == ".." || part.back() == '.' || part.back() == ' ')
            return "";
        if (end == std::string::npos)
            break;
        begin = end + 1;
    }
    if (active.empty()) {
        if (required() && (path.compare(0, 6, "/.core") == 0 || path.compare(0, 6, "/games") == 0))
            return "";
        return path;
    }
    const auto target = active + path;
    const bool core =
        path.compare(0, 7, "/.core/") == 0 || path == "/.core" || path == "/build-info.json";
    const bool bundled = SD.exists(target.c_str());
    if (writing)
        return core || bundled ? "" : path;
    if (core || bundled)
        return target;
    if (path.compare(0, 7, "/games/") == 0 &&
        (path.size() >= 9 && (path.substr(path.find_last_of('/') + 1) == "game.lua" ||
                              path.substr(path.find_last_of('/') + 1) == "config.json"))) {
        const auto parent = path.substr(0, path.find_last_of('/'));
        if (SD.exists((active + parent).c_str()))
            return target;
    }
    return path;
}

bool SdResources::managed(const char* logical) {
    SPIBusLock::Guard lock;
    return !logical || resolve(logical, true).empty();
}

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
bool SdResources::withGenerationForDiagnostics(const std::string& digest,
                                               const std::string& version,
                                               const std::function<void()>& inspect) {
    SPIBusLock::Guard lock;
    if (!ready(digest, version))
        return false;
    const auto previous = active;
    active = directory(digest);
    inspect();
    active = previous;
    return true;
}
#endif

UpdateError SdResources::install(UpdateInput& input, const UpdateRelease& release,
                                 UpdateEngine::Progress progress, UpdateEngine::Cancel cancelled) {
    Store store(release, cancelled);
    return SdPack::install(input, store, release, progress, cancelled);
}
