#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include "GameAssetDiagnostics.h"
#include "services/GameAssetCache.h"
#include "systems/storage/StorageSystem.h"
#include "core/SystemEventBus.h"
#include "drivers/bus/SPIBusLock.h"
#include "systems/lua/LuaAssetLoader.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <Arduino.h>
#include <lvgl.h>
#include <src/draw/lv_image_decoder_private.h>
#include <memory>
#include <cstring>
#include <algorithm>

namespace
{
    class SourceFile : public IFile
    {
    public:
        int read(uint8_t *buffer,size_t count) override
        { count=std::min(count,length()-position); std::memcpy(buffer,text+position,count); position+=count; return count; }
        bool seek(size_t value) override { if (value>length()) return false; position=value; return true; }
        bool available() override { return position<length(); }
        size_t length() const override { return std::strlen(text); }
        void close() override {} void flush() override {}
        size_t write(const uint8_t *,size_t) override { return 0; }
        bool writeLine(const char *,bool) override { return false; }
        bool isMemoryBacked() const override { return true; }
    private:
        const char *text="return {answer=42}";
        size_t position=0;
    };
    class ScriptStorage : public StorageSystem
    {
    public:
        IFile *open(const char *path,FileMode mode=FileMode::Read) override
        { return mode==FileMode::Read && !std::strcmp(path,"/games/test/helpers/util.lua") ? new SourceFile : nullptr; }
    };
}

void runGameAssetDiagnostics()
{
    auto &cache=GameAssetCache::instance();
    unsigned passed=0,failed=0;
    const auto check=[&](bool ok,const char *message) {
        ok ? ++passed : ++failed; Serial.printf("@ASSETCHECK %s %s\n",ok ? "PASS" : "FAIL",message);
    };
    auto *storage=static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    if (!storage || !storage->isReady()) { check(false,"SD ready"); return; }
    check(GameAssetCache::normalize("S:/games/5-t_gameobject/images/../game.lua")=="/games/5-t_gameobject/game.lua",
          "cache keys normalize LVGL prefixes and relative segments");
    check(GameAssetCache::normalize("/../../invalid").empty() && GameAssetCache::normalize("relative").empty(),
          "invalid root traversal and relative keys are rejected");
    const auto wait=[&](GameAssetCache::Token token) { while (!cache.prepared(token)) vTaskDelay(1); };
    const auto token=cache.begin(storage,"/games/5-t_gameobject"); wait(token);
    const auto ready=cache.status();
    check(ready.files==4 && !ready.failed,"nested game images, configuration and script are all prefetched");
    check(ready.bytes<=GameAssetCache::BUDGET && GameAssetCache::residentBytes()<=GameAssetCache::BUDGET,
          "source buffers stay within their independent memory budget");
    std::unique_ptr<IFile> script(storage->open("/games/5-t_gameobject/game.lua"));
    check(script && script->isMemoryBacked() && script->length()>0,"StorageSystem reads the main Lua script from RAM");
    std::string source;
    const auto hits=cache.status().hits;
    check(storage->readTextFile("/games/5-t_gameobject/game.lua",source,true) && !source.empty() && cache.status().hits>hits,
          "normal Lua text-loading path consumes prefetched bytes");
    const auto priorHealth=storage->health(); storage->noteIOFailure();
    storage->readTextFile("/games/5-t_gameobject/game.lua",source,true);
    check(storage->health()!=StorageSystem::Health::Ready,"cached reads cannot hide an SD health error");
    if (priorHealth==StorageSystem::Health::Ready) storage->noteIOSuccess();
    std::unique_ptr<IFile> image(cache.open("/games/5-t_gameobject/images/map.png"));
    uint8_t bytes[8]{};
    check(image && image->read(bytes,8)==8 && bytes[0]==0x89 && !std::memcmp(bytes+1,"PNG",3),"nested image preserves its binary signature");
    check(image && !image->write(bytes,8) && !image->writeLine("no",false),"cached assets are read-only");
    check(image && image->seek(image->length()) && image->read(bytes,8)==0 && !image->available() && !image->seek(image->length()+1),
          "memory readers handle seek, EOF and out-of-bounds positions");
    lv_fs_file_t file{};
    const auto imageHits=cache.status().hits;
    bool opened=lv_fs_open(&file,"S:/games/5-t_gameobject/images/map.png",LV_FS_MODE_RD)==LV_FS_RES_OK;
    uint32_t read=0,position=0;
    bool fs=opened && lv_fs_read(&file,bytes,8,&read)==LV_FS_RES_OK && read==8 &&
        lv_fs_seek(&file,0,LV_FS_SEEK_SET)==LV_FS_RES_OK && lv_fs_tell(&file,&position)==LV_FS_RES_OK && position==0;
    if (opened) lv_fs_close(&file);
    check(fs && cache.status().hits>imageHits,"LVGL filesystem uses the same cached image bytes");
    lv_image_decoder_dsc_t decoder{};
    const bool decoded=lv_image_decoder_open(&decoder,"S:/games/5-t_gameobject/images/map.png",nullptr)==LV_RESULT_OK;
    check(decoded,"LVGL can decode a prefetched PNG through its normal decoder");
    if (decoded) lv_image_decoder_close(&decoder);
    cache.invalidate("/games/5-t_gameobject/images/map.png");
    std::unique_ptr<IFile> invalid(cache.open("/games/5-t_gameobject/images/map.png"));
    check(!invalid && image && image->seek(0) && image->read(bytes,8)==8,"invalidation drops future hits while open readers retain safe snapshots");
    const auto newer=cache.begin(storage,"/games/3-t_loop"); cache.release(token); wait(newer);
    std::unique_ptr<IFile> next(cache.open("/games/3-t_loop/game.lua"));
    check(next && next->isMemoryBacked(),"releasing an old scene cannot clear the next game's cache");
    cache.release(newer);
    check(script && script->seek(0) && script->read(bytes,8)==8,"scene release cannot free data still held by a Lua/image reader");
    script.reset(); image.reset(); next.reset();
    check(GameAssetCache::residentBytes()==0,"all source bytes are reclaimed after the last reader closes");
    const auto cancelled=cache.begin(storage,"/games/5-t_gameobject");
    const auto replacement=cache.begin(storage,"/games/3-t_loop"); wait(replacement); cache.release(cancelled);
    check(cache.status().files==2,"a superseded worker cannot publish assets into the replacement game");
    cache.release(replacement);
    {
        ScriptStorage source; auto *L=luaL_newstate();
        if (L)
        {
            luaL_openlibs(L); installGameAssetLoaders(L,&source,"/games/test/game.lua");
            const char *test="assert(require('helpers.util').answer==42); "
                "assert(assert(loadfile('helpers/util.lua'))().answer==42); "
                "assert(dofile('helpers/util.lua').answer==42); "
                "assert(dofile('/sd/games/test/helpers/util.lua').answer==42); "
                "assert(dofile('S:/games/test/helpers/util.lua').answer==42); "
                "assert(loadfile('missing.lua')==nil)";
            check(luaL_loadstring(L,test)==LUA_OK && lua_pcall(L,0,0,0)==LUA_OK,
                  "require, loadfile and dofile resolve game scripts through the shared storage adapter");
            lua_close(L);
        }
        else check(false,"Lua VM available for loader check");
    }
    Serial.printf("@ASSETTEST passed=%u failed=%u\n",passed,failed);
}
#endif
