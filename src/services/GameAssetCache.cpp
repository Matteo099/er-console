#include "GameAssetCache.h"
#include "systems/storage/StorageSystem.h"
#include "drivers/bus/SPIBusLock.h"
#include "log/Logger.h"
#include <esp_heap_caps.h>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <cerrno>

std::atomic<size_t> GameAssetCache::liveBytes{0};

struct GameAssetCache::Blob {
    uint8_t* bytes = nullptr;
    size_t size = 0;

    ~Blob() {
        if (bytes) {
            heap_caps_free(bytes);
            liveBytes.fetch_sub(size);
        }
    }
};

class GameAssetCache::Reader : public IFile {
  public:
    explicit Reader(std::shared_ptr<Blob> value) : data(std::move(value)) {
    }

    bool isMemoryBacked() const override {
        return true;
    }

    int read(uint8_t* buffer, size_t length) override {
        if (!data || !buffer)
            return 0;
        const auto count = std::min(length, data->size - position);
        if (count)
            std::memcpy(buffer, data->bytes + position, count);
        position += count;
        return count;
    }

    bool seek(size_t value) override {
        if (!data || value > data->size)
            return false;
        position = value;
        return true;
    }

    size_t length() const override {
        return data ? data->size : 0;
    }

    bool available() override {
        return data && position < data->size;
    }

    size_t write(const uint8_t*, size_t) override {
        return 0;
    }

    bool writeLine(const char*, bool) override {
        return false;
    }

    void flush() override {
    }

    void close() override {
        data.reset();
        position = 0;
    }

  private:
    std::shared_ptr<Blob> data;
    size_t position = 0;
};

namespace {
bool inside(const std::string& path, const std::string& directory) {
    return path == directory ||
           (path.size() > directory.size() && path.compare(0, directory.size(), directory) == 0 &&
            path[directory.size()] == '/');
}

std::string extension(const std::string& path) {
    const auto dot = path.find_last_of('.');
    std::string result = dot == std::string::npos ? "" : path.substr(dot);
    for (auto& ch : result)
        ch = std::tolower(static_cast<unsigned char>(ch));
    return result;
}
}  // namespace

GameAssetCache& GameAssetCache::instance() {
    static GameAssetCache cache;
    return cache;
}

std::string GameAssetCache::normalize(const char* path) {
    if (!path || std::strlen(path) > 512)
        return {};
    if (path[0] == 'S' && path[1] == ':')
        path += 2;
    if (*path != '/')
        return {};
    std::string result, part;
    for (size_t i = 1;; ++i) {
        const char ch = path[i];
        if (ch == '/' || !ch) {
            if (part == "..") {
                if (result.empty())
                    return {};
                result.erase(result.find_last_of('/'));
            } else if (!part.empty() && part != ".")
                result += "/" + part;
            part.clear();
            if (!ch)
                break;
        } else
            part += ch;
    }
    return result.empty() ? "/" : result;
}

GameAssetCache::Token GameAssetCache::begin(StorageSystem* source, const std::string& directory) {
    TaskMutex::Guard lock(mutex);
    if (!++generation)
        ++generation;
    ++revision;
    entries.clear();
    wavs.clear();
    progress = Status{};
    root = normalize(directory.c_str());
    storage = source;
    if (!source || !source->isReady() || root.compare(0, 7, "/games/") != 0) {
        progress.failed = 1;
        return generation;
    }
    progress.done = false;
    startedAt = xTaskGetTickCount();
    if (!worker && xTaskCreate(task, "gameAssets", 6144, this, 1, &worker) != pdPASS) {
        progress.done = true;
        progress.failed = 1;
        worker = nullptr;
    }
    if (worker)
        xTaskNotifyGive(worker);
    return generation;
}

void GameAssetCache::release(Token token) {
    TaskMutex::Guard lock(mutex);
    if (token != generation)
        return;
    ++revision;
    root.clear();
    entries.clear();
    wavs.clear();
    progress = Status{};
}

bool GameAssetCache::current(Token token, uint32_t version) const {
    TaskMutex::Guard lock(mutex);
    return token == generation && version == revision && !root.empty();
}

bool GameAssetCache::prepared(Token token) {
    TaskMutex::Guard lock(mutex);
    if (token == generation && !progress.done &&
        (xTaskGetTickCount() - startedAt) * portTICK_PERIOD_MS >= 15000) {
        ++revision;
        progress.done = true;
        ++progress.skipped;
        Log::warn(Log::GAME_TAG, "Game assets: preload timeout; remaining files will use SD");
    }
    return token != generation || progress.done;
}

GameAssetCache::Status GameAssetCache::status() const {
    TaskMutex::Guard lock(mutex);
    return progress;
}

std::vector<std::string> GameAssetCache::audioPaths(Token token) const {
    TaskMutex::Guard lock(mutex);
    return token == generation ? wavs : std::vector<std::string>{};
}

IFile* GameAssetCache::open(const char* path) {
    const auto key = normalize(path);
    TaskMutex::Guard lock(mutex);
    for (const auto& entry : entries)
        if (entry.path == key) {
            ++progress.hits;
            return new Reader(entry.data);
        }
    return nullptr;
}

void GameAssetCache::invalidate(const char* path) {
    const auto key = normalize(path);
    TaskMutex::Guard lock(mutex);
    if (root.empty() || key.empty() || (key != "/" && !inside(key, root) && !inside(root, key)))
        return;
    // Stop publication from an in-flight read as well as invalidate old bytes.
    ++revision;
    entries.clear();
    wavs.clear();
    progress.bytes = progress.files = 0;
    progress.done = true;
}

void GameAssetCache::task(void* context) {
    auto& cache = *static_cast<GameAssetCache*>(context);
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Token token;
        uint32_t version;
        StorageSystem* source;
        std::string directory;
        {
            TaskMutex::Guard lock(cache.mutex);
            if (cache.progress.done)
                continue;
            token = cache.generation;
            version = cache.revision;
            source = cache.storage;
            directory = cache.root;
        }
        cache.prepare(token, version, source, directory);
    }
}

void GameAssetCache::scan(Token token, uint32_t version, StorageSystem* source,
                          const std::string& directory, unsigned depth, size_t& visited,
                          std::vector<Candidate>& files) {
    File dir;
    {
        SPIBusLock::Guard bus;
        dir = source->openNative(directory.c_str());
    }
    if (!dir) {
        TaskMutex::Guard lock(mutex);
        if (current(token, version))
            ++progress.failed;
        return;
    }
    while (current(token, version) && visited < MAX_ENTRIES) {
        std::string name;
        bool isDirectory = false;
        size_t size = 0;
        bool end = false, error = false;
        {
            SPIBusLock::Guard bus;
            errno = 0;
            File entry = dir.openNextFile();
            end = !entry;
            error = end && errno && errno != ENOENT;
            if (entry) {
                name = entry.name();
                isDirectory = entry.isDirectory();
                size = entry.size();
                entry.close();
            }
        }
        if (end) {
            if (error) {
                source->noteIOFailure();
                TaskMutex::Guard lock(mutex);
                if (current(token, version))
                    ++progress.failed;
            }
            break;
        }
        ++visited;
        const auto slash = name.find_last_of('/');
        if (slash != std::string::npos)
            name.erase(0, slash + 1);
        if (name.empty() || name[0] == '.')
            continue;
        const auto path = directory + "/" + name;
        if (isDirectory && depth < 8)
            scan(token, version, source, path, depth + 1, visited, files);
        else if (!isDirectory && path.size() < 128)
            files.push_back(Candidate{path, size});
        else {
            TaskMutex::Guard lock(mutex);
            if (current(token, version))
                ++progress.skipped;
        }
        vTaskDelay(1);
    }
    {
        SPIBusLock::Guard bus;
        dir.close();
    }
}

void GameAssetCache::load(Token token, uint32_t version, StorageSystem* source,
                          const std::string& path) {
    if (!current(token, version))
        return;
    File file;
    size_t size = 0;
    bool readable = false;
    {
        SPIBusLock::Guard bus;
        file = source->openNative(path.c_str());
        readable = file && !file.isDirectory();
        if (readable)
            size = file.size();
    }
    std::shared_ptr<Blob> blob;
    {
        TaskMutex::Guard lock(mutex);
        if (current(token, version) && readable && size <= MAX_FILE &&
            liveBytes.load() + size <= BUDGET &&
            heap_caps_get_free_size(MALLOC_CAP_SPIRAM) >= size + RESERVE &&
            heap_caps_get_free_size(MALLOC_CAP_INTERNAL) >= 32 * 1024) {
            blob = std::make_shared<Blob>();
            blob->bytes = static_cast<uint8_t*>(
                heap_caps_malloc(std::max<size_t>(1, size), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            if (blob->bytes) {
                blob->size = size;
                liveBytes.fetch_add(size);
            } else
                blob.reset();
        }
    }
    size_t total = 0;
    while (blob && total < size && current(token, version)) {
        const size_t requested = std::min<size_t>(4096, size - total);
        size_t count;
        {
            SPIBusLock::Guard bus;
            count = file.read(blob->bytes + total, requested);
        }
        if (!count || count > requested)
            break;
        total += count;
        if (total % (32 * 1024) == 0)
            vTaskDelay(1);
    }
    {
        SPIBusLock::Guard bus;
        file.close();
    }
    if (blob && total != size && current(token, version))
        source->noteIOFailure();
    TaskMutex::Guard lock(mutex);
    if (!current(token, version))
        return;
    if (blob && total == size) {
        entries.push_back(Entry{path, blob});
        ++progress.files;
        progress.bytes += size;
        Log::debug(Log::GAME_TAG, "Asset cached: %s (%u bytes)", path.c_str(), unsigned(size));
    } else if (blob || !readable)
        ++progress.failed;
    else
        ++progress.skipped;
}

void GameAssetCache::prepare(Token token, uint32_t version, StorageSystem* source,
                             const std::string& directory) {
    const auto started = xTaskGetTickCount();
    const auto script = directory + "/game.lua";
    load(token, version, source, script);  // Reserve the entry point before images/data.
    std::vector<Candidate> files;
    size_t visited = 0;
    scan(token, version, source, directory, 0, visited, files);
    std::stable_sort(files.begin(), files.end(), [](const Candidate& a, const Candidate& b) {
        const bool as = extension(a.path) == ".lua", bs = extension(b.path) == ".lua";
        return as != bs ? as : a.size < b.size;
    });
    for (const auto& file : files) {
        if (!current(token, version))
            return;
        if (file.path == script)
            continue;
        if (extension(file.path) == ".wav") {
            TaskMutex::Guard lock(mutex);
            if (current(token, version))
                wavs.push_back(file.path);
        } else
            load(token, version, source, file.path);
        vTaskDelay(1);
    }
    TaskMutex::Guard lock(mutex);
    if (!current(token, version))
        return;
    if (visited == MAX_ENTRIES)
        ++progress.skipped;
    progress.done = true;
    Log::info(Log::GAME_TAG,
              "Game assets prepared: files=%u bytes=%u/%u skipped=%u failed=%u wavs=%u time=%lu ms",
              unsigned(progress.files), unsigned(progress.bytes), unsigned(BUDGET),
              unsigned(progress.skipped), unsigned(progress.failed), unsigned(wavs.size()),
              static_cast<unsigned long>((xTaskGetTickCount() - started) * portTICK_PERIOD_MS));
}
