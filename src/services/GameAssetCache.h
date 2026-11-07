#pragma once
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include "core/TaskMutex.h"
#include <freertos/task.h>
#include "drivers/storage/IFile.h"

class StorageSystem;

// Read-only source bytes, shared by Lua/storage and LVGL. Decoded audio keeps
// its own existing budget. Open readers pin data across scene changes.
class GameAssetCache
{
public:
    using Token = uint32_t;
    static constexpr size_t BUDGET = 1024*1024, MAX_FILE = 512*1024;
    static constexpr size_t RESERVE = 1024*1024, MAX_ENTRIES = 128;
    struct Status
    {
        size_t files=0, bytes=0, skipped=0, failed=0, hits=0;
        bool done=true;
    };
    static GameAssetCache &instance();
    Token begin(StorageSystem *storage,const std::string &directory);
    void release(Token token);
    bool prepared(Token token);
    static size_t residentBytes() { return liveBytes.load(); }
    Status status() const;
    std::vector<std::string> audioPaths(Token token) const;
    IFile *open(const char *path);
    void invalidate(const char *path);
    static std::string normalize(const char *path);
private:
    struct Blob;
    class Reader;
    struct Entry { std::string path; std::shared_ptr<Blob> data; };
    struct Candidate { std::string path; size_t size; };
    static std::atomic<size_t> liveBytes;
    mutable TaskMutex mutex;
    TaskHandle_t worker=nullptr;
    StorageSystem *storage=nullptr;
    Token generation=0;
    uint32_t revision=0;
    TickType_t startedAt=0;
    std::string root;
    Status progress;
    std::vector<Entry> entries;
    std::vector<std::string> wavs;
    bool current(Token token,uint32_t version) const;
    static void task(void *context);
    void prepare(Token token,uint32_t version,StorageSystem *source,const std::string &directory);
    void scan(Token token,uint32_t version,StorageSystem *source,const std::string &directory,
              unsigned depth,size_t &visited,std::vector<Candidate> &files);
    void load(Token token,uint32_t version,StorageSystem *source,const std::string &path);
};
