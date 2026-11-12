#pragma once
#include <ArduinoJson.h>
#include "scenes/game/GameInfo.h"

namespace GameMetadata {
// Keep both languages in the catalog cache: switching language needs no SD I/O.
inline void read(JsonObjectConst document, GameInfo& info) {
    info.title = document["name"] | info.id;
    info.description = document["description"] | std::string();
    info.author = document["author"] | std::string();
    const auto italian = document["translations"]["it"];
    const auto english = document["translations"]["en"];
    info.title = italian["name"] | info.title;
    info.description = italian["description"] | info.description;
    info.englishTitle = english["name"] | std::string();
    info.englishDescription = english["description"] | std::string();
    info.difficulty = document["difficulty"] | 1;
    info.durationMin = document["durationMin"] | 0;
}
}  // namespace GameMetadata
