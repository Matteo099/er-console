#pragma once
#include <string>
struct lua_State;
class StorageSystem;
void installGameAssetLoaders(lua_State* L, StorageSystem* storage, const std::string& entryPoint);
