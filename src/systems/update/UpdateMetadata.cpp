#include "UpdateMetadata.h"
#include "config/Updates.h"
#include <ArduinoJson.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <sstream>
#include <esp_heap_caps.h>
#include <esp32-hal-psram.h>

namespace {
class JsonAllocator : public ArduinoJson::Allocator {
    uint32_t caps() const {
        return MALLOC_CAP_8BIT | (psramFound() ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL);
    }

  public:
    void* allocate(size_t size) override {
        if (!psramFound() && heap_caps_get_free_size(MALLOC_CAP_INTERNAL) < size + 32 * 1024)
            return nullptr;
        return heap_caps_malloc(size, caps());
    }

    void deallocate(void* value) override {
        heap_caps_free(value);
    }

    void* reallocate(void* value, size_t size) override {
        if (!psramFound() && heap_caps_get_free_size(MALLOC_CAP_INTERNAL) < size + 32 * 1024)
            return nullptr;
        return heap_caps_realloc(value, size, caps());
    }
};

struct Version {
    uint32_t parts[3]{};
    std::vector<std::string> pre;
};

bool number(const std::string& value, uint32_t& out) {
    if (value.empty() || (value.size() > 1 && value[0] == '0'))
        return false;
    out = 0;
    for (unsigned char ch : value) {
        if (!std::isdigit(ch) || out > (UINT32_MAX - uint32_t(ch - '0')) / 10)
            return false;
        out = out * 10 + ch - '0';
    }
    return true;
}

bool identifiers(const std::string& value, std::vector<std::string>& parts, bool numericRule) {
    if (value.empty() || value.back() == '.')
        return false;
    std::istringstream stream(value);
    std::string part;
    while (std::getline(stream, part, '.')) {
        if (part.empty())
            return false;
        bool numeric = true;
        for (unsigned char ch : part) {
            if (!std::isalnum(ch) && ch != '-')
                return false;
            numeric &= std::isdigit(ch) != 0;
        }
        if (numericRule && numeric && part.size() > 1 && part[0] == '0')
            return false;
        parts.push_back(part);
    }
    return !parts.empty();
}

bool parseVersion(std::string value, Version& out) {
    if (value.empty() || value.size() > 128)
        return false;
    if (value[0] == 'v')
        value.erase(0, 1);
    auto plus = value.find('+');
    if (plus != std::string::npos) {
        std::vector<std::string> build;
        if (!identifiers(value.substr(plus + 1), build, false))
            return false;
        value.resize(plus);
    }
    auto dash = value.find('-');
    if (dash != std::string::npos) {
        if (!identifiers(value.substr(dash + 1), out.pre, true))
            return false;
        value.resize(dash);
    }
    std::istringstream stream(value);
    std::string part;
    unsigned count = 0;
    while (std::getline(stream, part, '.')) {
        if (count == 3 || !number(part, out.parts[count++]))
            return false;
    }
    return count == 3 && !value.empty() && value.back() != '.';
}

bool hexDigest(const std::string& value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](unsigned char ch) {
               return std::isxdigit(ch) != 0;
           });
}

std::string lower(std::string value) {
    for (auto& ch : value)
        ch = std::tolower(static_cast<unsigned char>(ch));
    return value;
}

std::string host(const std::string& url) {
    if (url.compare(0, 8, "https://") != 0 || url.size() > 4096)
        return {};
    auto end = url.find('/', 8);
    const auto name = url.substr(8, end == std::string::npos ? std::string::npos : end - 8);
    if (url.find_first_of("\r\n\\\t ") != std::string::npos)
        return {};
    return name;
}
}  // namespace

bool UpdateMetadata::versionValid(const std::string& value) {
    Version version;
    return parseVersion(value, version);
}

int UpdateMetadata::compareVersions(const std::string& left, const std::string& right) {
    Version a, b;
    if (!parseVersion(left, a) || !parseVersion(right, b))
        return 0;
    for (unsigned i = 0; i < 3; ++i)
        if (a.parts[i] != b.parts[i])
            return a.parts[i] < b.parts[i] ? -1 : 1;
    if (a.pre.empty() != b.pre.empty())
        return a.pre.empty() ? 1 : -1;
    for (size_t i = 0; i < std::min(a.pre.size(), b.pre.size()); ++i) {
        const auto& x = a.pre[i];
        const auto& y = b.pre[i];
        if (x == y)
            continue;
        const auto numeric = [](const std::string& value) {
            return std::all_of(value.begin(), value.end(), [](unsigned char ch) {
                return std::isdigit(ch) != 0;
            });
        };
        bool xn = numeric(x), yn = numeric(y);
        if (xn != yn)
            return xn ? -1 : 1;
        if (xn && x.size() != y.size())
            return x.size() < y.size() ? -1 : 1;
        return x < y ? -1 : 1;
    }
    return a.pre.size() == b.pre.size() ? 0 : a.pre.size() < b.pre.size() ? -1 : 1;
}

bool UpdateMetadata::trustedDownloadUrl(const std::string& url) {
    const auto name = host(url);
    return name == "api.github.com" || name == "github.com" ||
           name == "release-assets.githubusercontent.com" ||
           name == "objects.githubusercontent.com" ||
           name == "github-releases.githubusercontent.com";
}

std::string UpdateMetadata::readable(const char* text, size_t limit, bool& truncated) {
    std::string result;
    truncated = false;
    if (!text)
        return result;
    const size_t length = std::strlen(text);
    for (size_t i = 0; i < length;) {
        unsigned char ch = text[i];
        size_t count = 1;
        uint32_t code = ch;
        if (ch >= 0xC2 && ch <= 0xDF) {
            count = 2;
            code = ch & 31;
        } else if (ch >= 0xE0 && ch <= 0xEF) {
            count = 3;
            code = ch & 15;
        } else if (ch >= 0xF0 && ch <= 0xF4) {
            count = 4;
            code = ch & 7;
        } else if (ch >= 128) {
            ++i;
            continue;
        }
        if (i + count > length)
            break;
        bool valid = true;
        for (size_t j = 1; j < count; ++j) {
            const unsigned char next = text[i + j];
            valid &= (next & 0xC0) == 0x80;
            code = (code << 6) | (next & 63);
        }
        valid &= !(count == 3 && code < 0x800) && !(count == 4 && code < 0x10000) &&
                 code <= 0x10ffff && !(code >= 0xd800 && code <= 0xdfff);
        if (!valid) {
            ++i;
            continue;
        }
        std::string part;
        if (code == '\n' || (code >= 32 && code <= 255))
            part.assign(text + i, count);
        else if (code == 0x2018 || code == 0x2019)
            part = "'";
        else if (code == 0x2013 || code == 0x2014)
            part = "-";
        else if (code == 0x2026)
            part = "...";
        else if (code > 255)
            part = " ";
        if (result.size() + part.size() > limit) {
            truncated = true;
            break;
        }
        result += part;
        i += count;
    }
    return result;
}

UpdateError UpdateMetadata::parseRelease(char* json, size_t length, UpdateRelease& out) {
    out = UpdateRelease{};
    if (!json || !length || length > UpdateConfig::JSON_LIMIT)
        return UpdateError::Metadata;
    JsonAllocator allocator;
    JsonDocument filter(&allocator);
    for (const char* key : {"tag_name", "name", "body", "published_at", "draft", "prerelease"})
        filter[key] = true;
    for (const char* key : {"id", "name", "size", "digest", "state"})
        filter["assets"][0][key] = true;
    JsonDocument doc(&allocator);
    const auto parsed = deserializeJson(doc, json, length, DeserializationOption::Filter(filter));
    if (parsed)
        return parsed == DeserializationError::NoMemory ? UpdateError::Memory
                                                        : UpdateError::Metadata;
    if (!doc["tag_name"].is<const char*>() || !doc["draft"].is<bool>() ||
        !doc["prerelease"].is<bool>() || doc["draft"].as<bool>() || doc["prerelease"].as<bool>())
        return UpdateError::Metadata;
    const char* tag = doc["tag_name"].as<const char*>();
    if (std::strlen(tag) > 128)
        return UpdateError::Metadata;
    out.version = tag;
    if (!out.version.empty() && out.version[0] == 'v')
        out.version.erase(0, 1);
    Version version;
    if (!parseVersion(out.version, version) || !version.pre.empty())
        return UpdateError::Metadata;
    bool cut = false;
    out.title = readable(doc["name"] | "", 128, cut);
    out.notes = readable(doc["body"] | "", 2048, out.notesTruncated);
    out.published = readable(doc["published_at"] | "", 32, cut);
    if (!doc["assets"].is<JsonArray>() || doc["assets"].size() > 32)
        return UpdateError::Metadata;
    for (JsonObject value : doc["assets"].as<JsonArray>()) {
        if (!value["id"].is<uint64_t>() || !value["size"].is<size_t>() ||
            !value["name"].is<const char*>() || std::strcmp(value["state"] | "", "uploaded"))
            return UpdateError::Metadata;
        ReleaseAsset asset;
        asset.id = value["id"].as<uint64_t>();
        asset.size = value["size"].as<size_t>();
        const char *name = value["name"].as<const char*>(), *digest = value["digest"] | "";
        if (std::strlen(name) > 192 || std::strlen(digest) > 71)
            return UpdateError::Metadata;
        asset.name = name;
        asset.digest = digest;
        if (!asset.id || asset.name.empty() || asset.name.size() > 192 ||
            asset.name.find_first_of("/\\\r\n") != std::string::npos)
            return UpdateError::Metadata;
        for (const auto& prior : out.assets)
            if (prior.id == asset.id || prior.name == asset.name)
                return UpdateError::Metadata;
        if (asset.name == "manifest.json") {
            if (!asset.size || asset.size > UpdateConfig::MANIFEST_LIMIT)
                return UpdateError::Metadata;
            out.manifestId = asset.id;
        }
        out.assets.push_back(std::move(asset));
    }
    return out.manifestId ? UpdateError::None : UpdateError::Metadata;
}

UpdateError UpdateMetadata::parseManifest(char* json, size_t length, UpdateRelease& out,
                                          const std::string& hardware, const std::string& layout,
                                          size_t slotSize) {
    out.firmwareId = out.resourcesId = 0;
    if (!json || !length || length > UpdateConfig::MANIFEST_LIMIT)
        return UpdateError::Metadata;
    JsonAllocator allocator;
    JsonDocument doc(&allocator);
    const auto parsed = deserializeJson(doc, json, length);
    if (parsed)
        return parsed == DeserializationError::NoMemory ? UpdateError::Memory
                                                        : UpdateError::Metadata;
    if (!doc["schema_version"].is<unsigned>() || doc["schema_version"].as<unsigned>() != 2 ||
        !doc["version"].is<const char*>() || std::strcmp(doc["channel"] | "", "stable") ||
        !doc["dirty"].is<bool>() || doc["dirty"].as<bool>() ||
        out.version != doc["version"].as<const char*>())
        return UpdateError::Metadata;
    const char* tableHash = doc["partition_table_sha256"] | "";
    if (hardware != (doc["hardware"] | "") || hardware != (doc["platformio_environment"] | "") ||
        std::strcmp(doc["firmware_type"] | "", "application") || !hexDigest(layout) ||
        std::strlen(tableHash) != 64 || lower(layout) != lower(tableHash) ||
        !doc["ota_slot_size"].is<size_t>() || doc["ota_slot_size"].as<size_t>() != slotSize)
        return UpdateError::Incompatible;
    for (unsigned kind = 0; kind < 2; ++kind) {
        JsonVariant value = doc["files"][kind ? "resources" : "firmware"];
        if (!value["file"].is<const char*>() || !value["sha256"].is<const char*>() ||
            !value["size"].is<size_t>())
            return UpdateError::Metadata;
        if (std::strlen(value["file"].as<const char*>()) > 192 ||
            std::strlen(value["sha256"].as<const char*>()) != 64)
            return UpdateError::Metadata;
        const std::string name = value["file"].as<const char*>(),
                          digest = value["sha256"].as<const char*>();
        const size_t size = value["size"].as<size_t>();
        if (!hexDigest(digest))
            return UpdateError::Integrity;
        if (size < (kind ? 12u : 288u) || size > (kind ? UpdateConfig::SDPACK_LIMIT : slotSize))
            return UpdateError::Size;
        for (const auto& asset : out.assets)
            if (asset.name == name) {
                const std::string suffix = kind ? ".sdpack" : ".bin";
                if (asset.size != size || asset.name.size() < suffix.size() ||
                    asset.name.substr(asset.name.size() - suffix.size()) != suffix)
                    return UpdateError::Size;
                if (!asset.digest.empty() && asset.digest != "sha256:" + lower(digest))
                    return UpdateError::Integrity;
                if (kind) {
                    out.resourcesId = asset.id;
                    out.resourcesSize = size;
                    out.resourcesSha256 = lower(digest);
                } else {
                    out.firmwareId = asset.id;
                    out.size = size;
                    out.sha256 = lower(digest);
                    out.filename = name;
                }
                break;
            }
        if (kind ? !out.resourcesId : !out.firmwareId)
            return UpdateError::Metadata;
    }
    return UpdateError::None;
}
