#include "StatisticsRepository.h"
#include <ArduinoJson.h>
#include <nvs.h>
#include "log/Logger.h"

namespace {
constexpr size_t MAX_BYTES = 6000;

class NvsStatistics : public StatisticsStorage {
  public:
    explicit NvsStatistics(const char* space = "game-stats") : space(space) {
    }

  private:
    const char* space;

    bool read(std::string& value) override {
        nvs_handle_t handle;
        if (nvs_open(space, NVS_READONLY, &handle) != ESP_OK)
            return false;
        size_t size = 0;
        bool okay = nvs_get_blob(handle, "snapshot", nullptr, &size) == ESP_OK && size > 0 &&
                    size <= MAX_BYTES;
        if (okay) {
            value.resize(size);
            okay = nvs_get_blob(handle, "snapshot", &value[0], &size) == ESP_OK;
        }
        nvs_close(handle);
        return okay;
    }

    bool write(const std::string& value) override {
        if (value.size() > MAX_BYTES)
            return false;
        nvs_handle_t handle;
        if (nvs_open(space, NVS_READWRITE, &handle) != ESP_OK)
            return false;
        const bool okay = nvs_set_blob(handle, "snapshot", value.data(), value.size()) == ESP_OK &&
                          nvs_commit(handle) == ESP_OK;
        nvs_close(handle);
        return okay;
    }
};
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
StatisticsRepository* diagnosticRepository = nullptr;
#endif
void writeTotals(JsonArray array, const GameTotals& s) {
    array.add(s.played);
    array.add(s.wins);
    array.add(s.losses);
    array.add(s.abandoned);
    array.add(s.players);
    array.add(s.durationMs);
    array.add(s.bestMs);
    array.add(s.worstMs);
    array.add(s.lastPlayers);
}

bool readTotals(JsonArrayConst a, GameTotals& s) {
    if (a.size() != 9)
        return false;
    for (auto value : a)
        if (!value.is<uint64_t>())
            return false;
    if (a[8].as<uint64_t>() > UINT16_MAX)
        return false;
    s.played = a[0];
    s.wins = a[1];
    s.losses = a[2];
    s.abandoned = a[3];
    s.players = a[4];
    s.durationMs = a[5];
    s.bestMs = a[6];
    s.worstMs = a[7];
    s.lastPlayers = a[8];
    return true;
}

void writeSession(JsonArray a, const GameSessionRecord& s) {
    a.add(s.id);
    a.add(s.gameId);
    a.add(s.players);
    a.add(unsigned(s.outcome));
    a.add(s.durationMs);
}

bool readSession(JsonArrayConst a, GameSessionRecord& s) {
    if (a.size() != 5 || !a[0].is<uint64_t>() || !a[1].is<const char*>() || !a[2].is<uint16_t>() ||
        !a[2].as<uint16_t>() || !a[3].is<uint8_t>() ||
        a[3].as<uint8_t>() > unsigned(GameOutcome::Interrupted) || !a[4].is<uint64_t>())
        return false;
    s = {a[0], a[1].as<std::string>(), a[2].as<uint16_t>(), GameOutcome(a[3].as<uint8_t>()), a[4]};
    return true;
}
}  // namespace

StatisticsRepository& StatisticsRepository::instance() {
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    if (diagnosticRepository)
        return *diagnosticRepository;
#endif
    static NvsStatistics storage;
    static StatisticsRepository repository(storage);
    return repository;
}

std::string StatisticsRepository::encode(const GameStatistics& value) {
    JsonDocument doc;
    doc["v"] = 1;
    doc["n"] = value.nextId;
    writeTotals(doc["s"].to<JsonArray>(), value.global);
    auto games = doc["g"].to<JsonArray>();
    for (const auto& entry : value.games) {
        auto row = games.add<JsonArray>();
        row.add(entry.gameId);
        writeTotals(row.add<JsonArray>(), entry.totals);
    }
    if (value.active.id)
        writeSession(doc["a"].to<JsonArray>(), value.active);
    auto history = doc["h"].to<JsonArray>();
    for (const auto& record : value.history)
        writeSession(history.add<JsonArray>(), record);
    std::string output;
    serializeJson(doc, output);
    return output;
}

bool StatisticsRepository::decode(const std::string& source, GameStatistics& value) {
    JsonDocument doc;
    if (source.size() > MAX_BYTES || deserializeJson(doc, source) || !doc["v"].is<uint32_t>() ||
        doc["v"].as<uint32_t>() != 1 || !doc["n"].is<uint64_t>() || !doc["g"].is<JsonArray>() ||
        !doc["h"].is<JsonArray>())
        return false;
    GameStatistics parsed;
    parsed.nextId = doc["n"].as<uint64_t>();
    if (!readTotals(doc["s"].as<JsonArrayConst>(), parsed.global))
        return false;
    for (JsonArrayConst row : doc["g"].as<JsonArrayConst>()) {
        GameStatisticsEntry entry;
        if (row.size() != 2 || !row[0].is<const char*>() ||
            !readTotals(row[1].as<JsonArrayConst>(), entry.totals))
            return false;
        entry.gameId = row[0].as<std::string>();
        parsed.games.push_back(std::move(entry));
    }
    if (!doc["a"].isNull() && !readSession(doc["a"].as<JsonArrayConst>(), parsed.active))
        return false;
    for (JsonArrayConst row : doc["h"].as<JsonArrayConst>()) {
        GameSessionRecord record;
        if (!readSession(row, record))
            return false;
        parsed.history.push_back(std::move(record));
    }
    if (!parsed.valid())
        return false;
    value = std::move(parsed);
    return true;
}

void StatisticsRepository::load() {
    if (loaded)
        return;
    loaded = true;
    std::string serialized;
    if (storage.read(serialized) && !decode(serialized, data)) {
        persistent = false;
        Log::warn(Log::GAME_TAG,
                  "Statistics snapshot is invalid; starting an empty in-memory model");
    }
    if (data.active.id) {
        data.finish(data.active.id, GameOutcome::Interrupted, 0);
        save();
    }
}

void StatisticsRepository::save() {
    const auto value = encode(data);
    persistent = value.size() <= MAX_BYTES && storage.write(value);
    if (!persistent)
        Log::warn(Log::GAME_TAG,
                  "Statistics are temporary: NVS write failed or snapshot exceeds %u bytes",
                  unsigned(MAX_BYTES));
}

uint64_t StatisticsRepository::start(const std::string& id, uint16_t players) {
    TaskMutex::Guard lock(mutex);
    load();
    const auto session = data.start(id, players);
    if (session)
        save();
    return session;
}

bool StatisticsRepository::finish(uint64_t id, GameOutcome outcome, uint64_t duration) {
    TaskMutex::Guard lock(mutex);
    load();
    const bool changed = data.finish(id, outcome, duration);
    if (changed)
        save();
    return changed;
}

StatisticsSnapshot StatisticsRepository::snapshot(const std::string& id) {
    TaskMutex::Guard lock(mutex);
    load();
    return {id.empty() ? data.global : data.forGame(id),
            data.active.id && (id.empty() || data.active.gameId == id) ? 1u : 0u, persistent};
}

GameStatistics StatisticsRepository::records() {
    TaskMutex::Guard lock(mutex);
    load();
    return data;
}
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
bool StatisticsRepository::useForDiagnostics(StatisticsRepository* repository) {
    if (instance().records().active.id)
        return false;
    diagnosticRepository = repository;
    return true;
}

bool StatisticsRepository::testNvsRoundTrip() {
    NvsStatistics storage("stats-check");
    const auto erase = []() {
        nvs_handle_t handle;
        if (nvs_open("stats-check", NVS_READWRITE, &handle) == ESP_OK) {
            nvs_erase_key(handle, "snapshot");
            nvs_commit(handle);
            nvs_close(handle);
        }
    };
    erase();
    bool okay = false;
    {
        StatisticsRepository first(storage);
        const auto id = first.start("test-only", 3);
        okay = id && first.finish(id, GameOutcome::Won, 42000) && first.snapshot().persistent;
    }
    {
        StatisticsRepository second(storage);
        const auto copy = second.records();
        okay &= copy.global.wins == 1 && copy.history.size() == 1 && copy.history[0].players == 3 &&
                copy.global.bestMs == 42000;
    }
    erase();
    return okay;
}
#endif
