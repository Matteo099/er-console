#pragma once
#include "scenes/game/GameInfo.h"
#include <string>
#include <vector>

// Application-owned cache: its worker never holds a Scene or an LVGL pointer.
class GameCatalog
{
public:
    struct Snapshot
    {
        std::vector<GameInfo> games;
        std::string error;
        uint32_t version = 1;
        bool loading = false, initialized = false;
    };
    static void request(bool force = false);
    static bool snapshot(uint32_t seenVersion, Snapshot &result);
private:
    static Snapshot readGames();
    static void loadTask(void *);
};
