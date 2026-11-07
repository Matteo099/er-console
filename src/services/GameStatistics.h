#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum class GameOutcome : uint8_t { Active, Won, Lost, Abandoned, Interrupted };
struct GameTotals
{
    uint64_t played=0,wins=0,losses=0,abandoned=0,players=0;
    uint64_t durationMs=0,bestMs=0,worstMs=0;
    uint16_t lastPlayers=0;
    uint64_t finished() const { return wins+losses; }
    uint64_t averageMs() const { return finished() ? durationMs/finished() : 0; }
};
struct GameSessionRecord
{
    GameSessionRecord(uint64_t id=0,const std::string &gameId="",uint16_t players=1,GameOutcome outcome=GameOutcome::Active,uint64_t durationMs=0)
        : id(id),gameId(gameId),players(players),outcome(outcome),durationMs(durationMs) {}
    uint64_t id=0;
    std::string gameId;
    uint16_t players=1;
    GameOutcome outcome=GameOutcome::Active;
    uint64_t durationMs=0;
};
struct GameStatisticsEntry { std::string gameId; GameTotals totals; };

// Pure model, independently testable without NVS, SD, Lua or a clock source.
class GameStatistics
{
public:
    static constexpr size_t HISTORY_LIMIT=32;
    GameTotals global;
    std::vector<GameStatisticsEntry> games;
    std::vector<GameSessionRecord> history;
    GameSessionRecord active;
    uint64_t nextId=1;
    uint64_t start(const std::string &gameId,uint16_t players);
    bool finish(uint64_t id,GameOutcome outcome,uint64_t durationMs);
    const GameTotals &forGame(const std::string &id) const;
    bool valid() const;
};

class GameSessionClock
{
public:
    void start(uint32_t now) { activeMs=pausedMs=0; last=now; started=true; running=true; finished=false; }
    void tick(uint32_t now)
    {
        if (!started || finished) return;
        (running ? activeMs : pausedMs) += uint32_t(now-last); last=now;
    }
    void pause(uint32_t now) { tick(now); running=false; }
    void resume(uint32_t now) { tick(now); if (!finished) running=true; }
    void stop(uint32_t now) { tick(now); running=false; finished=true; }
    bool isStarted() const { return started; }
    uint64_t elapsed(uint32_t now) const { return activeMs+(started && running && !finished ? uint32_t(now-last) : 0); }
    uint64_t paused(uint32_t now) const { return pausedMs+(started && !running && !finished ? uint32_t(now-last) : 0); }
private:
    uint64_t activeMs=0,pausedMs=0;
    uint32_t last=0;
    bool started=false,running=false,finished=false;
};
