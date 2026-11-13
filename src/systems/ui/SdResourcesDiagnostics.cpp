#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include "SdResourcesDiagnostics.h"
#include "systems/update/SdResources.h"
#include "systems/update/SdPack.h"
#include "drivers/bus/SPIBusLock.h"
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include <SD.h>
#include <esp_ota_ops.h>
#include <mbedtls/sha256.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <freertos/semphr.h>
#include <freertos/task.h>

namespace {
struct Input : UpdateInput {
    const std::vector<uint8_t>& bytes;
    size_t pos = 0, limit = SIZE_MAX;

    explicit Input(const std::vector<uint8_t>& data) : bytes(data) {
    }

    int read(uint8_t* out, size_t capacity) override {
        if (pos >= limit)
            return -1;
        const size_t size = std::min<size_t>(17, std::min(capacity, bytes.size() - pos));
        if (size)
            std::memcpy(out, bytes.data() + pos, size);
        pos += size;
        return size;
    }

    bool complete() const override {
        return pos == bytes.size();
    }

    int64_t contentLength() const override {
        return -1;
    }
};

struct Sink : SdPackSink {
    unsigned commits = 0, aborts = 0;
    bool failWrite = false;

    bool begin(size_t, unsigned) override {
        return true;
    }

    bool open(const std::string&, size_t) override {
        return true;
    }

    bool write(const uint8_t*, size_t) override {
        return !failWrite;
    }

    bool finishFile(const uint8_t*) override {
        return true;
    }

    bool commit() override {
        ++commits;
        return true;
    }

    void abort() override {
        ++aborts;
    }
};

std::vector<uint8_t> fixture() {
    const std::map<std::string, std::string> files = {
        {".core/audio/lose.wav", "test-lose"},
        {".core/audio/tickingclock.wav", "test-clock"},
        {".core/audio/win.wav", "test-win"},
        {".core/images/logo.png", "test-logo"},
        {".core/portal/a/b/c/d/e/diagnostic.txt", "deep"},
        {".core/portal/index.html", "test-portal"},
        {"build-info.json", "{}"},
        {"games/sd-ota-diagnostic/config.json", "{}"},
        {"games/sd-ota-diagnostic/game.lua", "function init() end"}};
    std::vector<uint8_t> out{'E', 'R', 'S', 'D', 'P', 'K', '0', '1'};
    const auto append = [&](uint32_t number, unsigned length) {
        for (unsigned i = 0; i < length; ++i)
            out.push_back(number >> (8 * i));
    };
    append(files.size(), 4);
    for (const auto& item : files) {
        append(item.first.size(), 2);
        append(item.second.size(), 4);
        uint8_t digest[32];
        mbedtls_sha256_ret(reinterpret_cast<const uint8_t*>(item.second.data()), item.second.size(),
                           digest, 0);
        out.insert(out.end(), digest, digest + 32);
        out.insert(out.end(), item.first.begin(), item.first.end());
        out.insert(out.end(), item.second.begin(), item.second.end());
    }
    return out;
}
}  // namespace

static void runChecks(const std::function<void(bool, const char*)>& check) {
    auto bytes = fixture();
    const auto originalRoot = SdResources::root();
    UpdateRelease release;
    release.version = "0.0.0-sd-diagnostic";
    release.resourcesSize = bytes.size();
    release.resourcesSha256 = SdPack::sha256(bytes.data(), bytes.size());
    const auto quiet = [](UpdateState, size_t, size_t) {
    };
    const auto never = []() {
        return false;
    };
    {
        Input input(bytes);
        Sink sink;
        check(SdPack::install(input, sink, release, quiet, never) == UpdateError::None &&
                  sink.commits == 1 && sink.aborts == 0,
              "SD stream accepts fragmented complete verified package");
    }
    for (unsigned kind = 0; kind < 5; ++kind) {
        auto altered = bytes;
        auto candidate = release;
        if (kind == 0)
            altered.back() ^= 1;
        if (kind == 1)
            candidate.resourcesSha256[0] = candidate.resourcesSha256[0] == '0' ? '1' : '0';
        if (kind == 2)
            altered.push_back(1);
        Input input(altered);
        Sink sink;
        if (kind == 3)
            input.limit = 50;
        if (kind == 4)
            sink.failWrite = true;
        const auto error = SdPack::install(input, sink, candidate, quiet, never);
        check(error != UpdateError::None && sink.aborts == 1 && !sink.commits,
              "SD corrupt, truncated, appended or failed writes cannot publish resources");
    }
    {
        bool stop = false;
        Input input(bytes);
        Sink sink;
        check(SdPack::install(
                  input, sink, release,
                  [&](UpdateState, size_t, size_t) {
                      stop = true;
                  },
                  [&]() {
                      return stop;
                  }) == UpdateError::Cancelled &&
                  sink.aborts == 1 && !sink.commits,
              "SD cancellation removes staged data before publication");
    }
    check(!SdPack::validPath("games/../save") && !SdPack::validPath(".core/config/wifi") &&
              !SdPack::validPath("/games/a") &&
              SdResources::resolve("/games/../../.releases/test").empty() &&
              SdResources::resolve("/.RELEASES/test").empty(),
          "SD paths cannot escape managed resource directories");
    const auto root = "/.releases/" + release.resourcesSha256;
    const auto* before = esp_ota_get_boot_partition();
    const auto address = before ? before->address : 0;
    {
        unsigned chunks = 0;
        bool stop = false;
        Input input(bytes);
        const auto error = SdResources::install(
            input, release,
            [&](UpdateState, size_t, size_t) {
                stop = ++chunks >= 5;
            },
            [&]() {
                return stop;
            });
        SPIBusLock::Guard guard;
        check(error == UpdateError::Cancelled && !SD.exists("/.releases/staging"),
              "native cancellation cleans deep staging without exhausting five VFS slots");
    }
    // Never replace an existing directory: the fixture occupies its own content hash.
    if (!SdResources::ready(release.resourcesSha256, release.version)) {
        Input input(bytes);
        check(SdResources::install(input, release, quiet, never) == UpdateError::None,
              "native SD installation flushes and rereads each file before commit");
    }
    check(SdResources::ready(release.resourcesSha256, release.version) &&
              !SdResources::ready(release.resourcesSha256, "wrong-version"),
          "native SD marker binds installed version and content");
    auto* storage = static_cast<StorageSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    if (storage) {
        const char* save = "/games/sd-ota-diagnostic/runtime-save.txt";
        const bool existed = storage->exists(save);
        check(SdResources::withGenerationForDiagnostics(
                  release.resourcesSha256, release.version,
                  [&]() {
                      std::string text;
                      check(storage->readTextFile("/.core/portal/index.html", text, true) &&
                                text == "test-portal",
                            "logical portal reads select the release generation");
                      check(!storage->writeTextFile("/.core/portal/index.html", "overwrite",
                                                    true) &&
                                !storage->removePath("/.core/portal/index.html") &&
                                !storage->movePath("/.core/portal/index.html", "/portal-replaced",
                                                   false),
                            "ordinary writes, deletes and moves cannot modify managed files");
                      check(existed || storage->writeTextFile(save, "user-save", true),
                            "runtime saves create flat parents even when the game is managed");
                      std::vector<StorageEntry> entries;
                      const bool listed = storage->listDirectory("/", entries, false);
                      check(listed && std::none_of(entries.begin(), entries.end(),
                                                   [](const StorageEntry& entry) {
                                                       return entry.name == ".releases" ||
                                                              entry.name == ".complete" ||
                                                              entry.name == ".inventory";
                                                   }),
                            "logical directory browsing hides reserved generation metadata");
                  }),
              "diagnostic resource selection is scoped and restored");
        if (!existed) {
            std::string text;
            check(storage->readTextFile(save, text, true) && text == "user-save" &&
                      storage->removePath(save),
                  "user save survives generation selection and returns to the flat namespace");
            SPIBusLock::Guard guard;
            SD.rmdir("/games/sd-ota-diagnostic");  // Remove only the empty diagnostic directory.
        }
    } else
        check(false, "storage service available for SD diagnostics");
    {
        SPIBusLock::Guard guard;
        const auto path = root + "/.core/portal/index.html";
        const auto hidden = path + ".diagnostic-missing";
        const bool renamed = !SD.exists(hidden.c_str()) && SD.rename(path.c_str(), hidden.c_str());
        const bool rejected =
            renamed && !SdResources::ready(release.resourcesSha256, release.version);
        const bool restored = renamed && SD.rename(hidden.c_str(), path.c_str());
        check(rejected && restored && SdResources::ready(release.resourcesSha256, release.version),
              "native inventory detects a missing file and accepts its restoration");
    }
    {
        SPIBusLock::Guard guard;
        auto file = SD.open((root + "/.core/portal/index.html").c_str());
        char data[12]{};
        check(file && file.readBytes(data, 11) == 11 && !std::strcmp(data, "test-portal"),
              "native SD committed content matches the downloaded file");
    }
    const auto* after = esp_ota_get_boot_partition();
    check(after && after->address == address && SdResources::root() == originalRoot,
          "SD staging leaves boot selection and development resource selection unchanged");
}

void runSdResourcesDiagnostics(const std::function<void(bool, const char*)>& check) {
    struct Context {
        const std::function<void(bool, const char*)>& check;
        SemaphoreHandle_t done;
    } context{check, xSemaphoreCreateBinary()};

    if (!context.done) {
        check(false, "SD diagnostic worker allocation");
        return;
    }
    const auto worker = [](void* value) {
        auto& context = *static_cast<Context*>(value);
        runChecks(context.check);
        Serial.printf("@SDSTACK free=%u\n", unsigned(uxTaskGetStackHighWaterMark(nullptr)));
        xSemaphoreGive(context.done);
        vTaskDelete(nullptr);
    };
    if (xTaskCreatePinnedToCore(worker, "sdPackTest", 16384, &context, 1, nullptr, 0) == pdPASS)
        xSemaphoreTake(context.done, portMAX_DELAY);
    else
        check(false, "SD diagnostic worker creation");
    vSemaphoreDelete(context.done);
}
#endif
