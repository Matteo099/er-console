#include "GameStatistics.h"
#include <algorithm>
#include <limits>

namespace
{
    void startTotals(GameTotals &totals,uint16_t players)
    {
        ++totals.played; totals.players+=players; totals.lastPlayers=players;
    }
    void finishTotals(GameTotals &totals,GameOutcome outcome,uint64_t duration)
    {
        if (outcome == GameOutcome::Won)
        {
            if (!totals.wins) totals.bestMs=totals.worstMs=duration;
            else { totals.bestMs=std::min(totals.bestMs,duration); totals.worstMs=std::max(totals.worstMs,duration); }
            ++totals.wins; totals.durationMs+=duration;
        }
        else if (outcome == GameOutcome::Lost) { ++totals.losses; totals.durationMs+=duration; }
        else ++totals.abandoned;
    }
}
uint64_t GameStatistics::start(const std::string &id,uint16_t players)
{
    if (id.empty() || id.size()>255 || !players || active.id || nextId==std::numeric_limits<uint64_t>::max()) return 0;
    auto found=std::find_if(games.begin(),games.end(),[&](const GameStatisticsEntry &entry) { return entry.gameId==id; });
    if (found==games.end()) { games.push_back({id,{}}); found=games.end()-1; }
    startTotals(global,players); startTotals(found->totals,players);
    active={nextId++,id,players,GameOutcome::Active,0}; return active.id;
}
bool GameStatistics::finish(uint64_t id,GameOutcome outcome,uint64_t duration)
{
    if (!id || active.id!=id || outcome==GameOutcome::Active || outcome>GameOutcome::Interrupted) return false;
    auto found=std::find_if(games.begin(),games.end(),[&](const GameStatisticsEntry &entry) { return entry.gameId==active.gameId; });
    if (found==games.end()) return false;
    finishTotals(global,outcome,duration); finishTotals(found->totals,outcome,duration);
    active.outcome=outcome; active.durationMs=duration;
    if (history.size()==HISTORY_LIMIT) history.erase(history.begin());
    history.push_back(active); active={}; return true;
}
const GameTotals &GameStatistics::forGame(const std::string &id) const
{
    for (const auto &entry:games) if (entry.gameId==id) return entry.totals;
    static const GameTotals empty; return empty;
}
bool GameStatistics::valid() const
{
    if (!nextId || history.size()>HISTORY_LIMIT) return false;
    if (global.played!=nextId-1 || history.size()!=std::min<uint64_t>(HISTORY_LIMIT,global.played-(active.id ? 1:0))) return false;
    if (global.played && !global.lastPlayers) return false;
    GameTotals sum;
    for (size_t i=0;i<games.size();++i)
    {
        const auto &entry=games[i]; const auto &s=entry.totals;
        if (entry.gameId.empty() || entry.gameId.size()>255 || !s.played || !s.lastPlayers || s.players<s.played ||
            s.played!=s.wins+s.losses+s.abandoned+(active.id && active.gameId==entry.gameId ? 1:0) ||
            (s.wins ? s.bestMs>s.worstMs : (s.bestMs || s.worstMs))) return false;
        for (size_t j=0;j<i;++j) if (games[j].gameId==entry.gameId) return false;
        sum.played+=s.played; sum.wins+=s.wins; sum.losses+=s.losses; sum.abandoned+=s.abandoned;
        sum.players+=s.players; sum.durationMs+=s.durationMs;
        if (s.wins) { if (!sum.lastPlayers || s.bestMs<sum.bestMs) sum.bestMs=s.bestMs; sum.worstMs=std::max(sum.worstMs,s.worstMs); sum.lastPlayers=1; }
    }
    if (sum.played!=global.played || sum.wins!=global.wins || sum.losses!=global.losses || sum.abandoned!=global.abandoned ||
        sum.players!=global.players || sum.durationMs!=global.durationMs || sum.bestMs!=global.bestMs || sum.worstMs!=global.worstMs ||
        global.played!=global.finished()+global.abandoned+(active.id ? 1:0)) return false;
    if (active.id && (active.id>=nextId || !active.players || active.outcome!=GameOutcome::Active || !forGame(active.gameId).played)) return false;
    uint64_t previous=0;
    for (const auto &record:history)
    {
        if (!record.id || record.id<=previous || record.id>=nextId || !record.players || !forGame(record.gameId).played ||
            record.outcome==GameOutcome::Active || record.outcome>GameOutcome::Interrupted) return false;
        previous=record.id;
    }
    return !active.id || previous<active.id;
}
