#pragma once
#include "GameStatistics.h"
#include "core/TaskMutex.h"
#include "config/Console.h"

class StatisticsStorage
{
public:
    virtual ~StatisticsStorage() = default;
    virtual bool read(std::string &value)=0;
    virtual bool write(const std::string &value)=0;
};
struct StatisticsSnapshot
{
    StatisticsSnapshot(const GameTotals &totals=GameTotals{},uint64_t active=0,bool persistent=true) : totals(totals),active(active),persistent(persistent) {}
    GameTotals totals; uint64_t active; bool persistent;
};

class StatisticsRepository
{
public:
    explicit StatisticsRepository(StatisticsStorage &storage) : storage(storage) {}
    static StatisticsRepository &instance();
    uint64_t start(const std::string &gameId,uint16_t players);
    bool finish(uint64_t id,GameOutcome outcome,uint64_t milliseconds);
    StatisticsSnapshot snapshot(const std::string &gameId="");
    GameStatistics records();
    static std::string encode(const GameStatistics &value);
    static bool decode(const std::string &source,GameStatistics &value);
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    static bool useForDiagnostics(StatisticsRepository *repository);
    static bool testNvsRoundTrip();
#endif
private:
    void load();
    void save();
    StatisticsStorage &storage;
    TaskMutex mutex;
    GameStatistics data;
    bool loaded=false,persistent=true;
};
