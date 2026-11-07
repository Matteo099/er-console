#include "StatisticsDiagnostics.h"
#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include "services/StatisticsRepository.h"
#include "systems/ui/components/PlayerSelectorDialog.h"
#include "systems/ui/UIFeedback.h"

namespace
{
    struct MemoryStorage : StatisticsStorage
    {
        std::string value; bool fail=false;
        bool read(std::string &output) override { output=value; return !value.empty(); }
        bool write(const std::string &input) override { if (fail) return false; value=input; return true; }
    };
    MemoryStorage *sandboxStorage=nullptr;
    StatisticsRepository *sandboxRepository=nullptr;
    lv_obj_t *button(lv_obj_t *root,const char *text)
    {
        if (lv_obj_check_type(root,&lv_label_class) && !std::strcmp(lv_label_get_text(root),text) && lv_obj_check_type(lv_obj_get_parent(root),&lv_button_class)) return lv_obj_get_parent(root);
        for (unsigned i=0;i<lv_obj_get_child_count(root);++i) if (auto *found=button(lv_obj_get_child(root,i),text)) return found;
        return nullptr;
    }
}
bool statisticsSandbox(bool enabled)
{
    if (!StatisticsRepository::useForDiagnostics(nullptr)) return false;
    delete sandboxRepository; sandboxRepository=nullptr; delete sandboxStorage; sandboxStorage=nullptr;
    if (enabled) { sandboxStorage=new MemoryStorage; sandboxRepository=new StatisticsRepository(*sandboxStorage); return StatisticsRepository::useForDiagnostics(sandboxRepository); }
    return true;
}
void runStatisticsDiagnostics()
{
    unsigned passed=0,failed=0;
    const auto check=[&](bool okay,const char *name) { okay ? ++passed : ++failed; Serial.printf("@STATSCHECK %s %s\n",okay ? "PASS":"FAIL",name); };
    GameStatistics model;
    check(model.valid() && !model.global.played,"empty statistics are valid");
    check(!model.start("",1) && !model.start("room",0),"invalid game or zero players cannot start a session");
    auto id=model.start("room",3);
    check(id && model.global.played==1 && model.global.players==3 && model.active.players==3,"players belong to the started session");
    check(!model.start("other",1) && model.global.played==1,"duplicate start cannot count twice");
    check(model.finish(id,GameOutcome::Won,120000) && !model.finish(id,GameOutcome::Lost,1),"terminal outcome is recorded exactly once");
    check(model.global.wins==1 && model.global.bestMs==120000 && model.history[0].players==3,"first victory creates completion records and session history");
    id=model.start("room",2); model.finish(id,GameOutcome::Lost,60000);
    check(model.global.averageMs()==90000 && model.global.worstMs==120000 && model.global.players==5,"losses affect average but never victory records");
    id=model.start("other",4); model.finish(id,GameOutcome::Won,30000);
    check(model.forGame("room").averageMs()==90000 && model.forGame("other").bestMs==30000 && model.global.averageMs()==70000,"per-game and weighted global summaries agree");
    id=model.start("other",1); model.finish(id,GameOutcome::Won,240000);
    check(model.global.bestMs==30000 && model.global.worstMs==240000,"best and worst victory times span all games");
    id=model.start("room",1); model.finish(id,GameOutcome::Abandoned,5000);
    check(model.global.abandoned==1 && model.global.averageMs()==112500,"abandonment is separate from losses and completed-game averages");
    check(model.valid(),"aggregate and session invariants remain balanced");
    const auto serialized=StatisticsRepository::encode(model); GameStatistics decoded;
    check(StatisticsRepository::decode(serialized,decoded) && StatisticsRepository::encode(decoded)==serialized,"complete snapshot round-trips without loss");
    check(!StatisticsRepository::decode("{\"v\":99}",decoded) && !StatisticsRepository::decode(serialized.substr(0,serialized.size()/2),decoded),"unsupported or truncated snapshots rejected");
    auto invalid=model; ++invalid.global.wins;
    check(!StatisticsRepository::decode(StatisticsRepository::encode(invalid),decoded),"inconsistent aggregate data rejected");
    MemoryStorage storage;
    {
        StatisticsRepository repo(storage); id=repo.start("reboot",7);
        check(id && repo.snapshot().active==1 && repo.snapshot().persistent,"active session is durable before gameplay");
    }
    {
        StatisticsRepository recovered(storage); const auto data=recovered.records();
        check(!data.active.id && data.global.played==1 && data.global.abandoned==1 && data.history.back().outcome==GameOutcome::Interrupted && data.history.back().players==7,"reboot recovers interrupted session once with its players");
    }
    {
        StatisticsRepository again(storage); check(again.snapshot().totals.abandoned==1,"reopening recovered data does not double-count interruption");
    }
    storage.fail=true;
    {
        StatisticsRepository repo(storage); id=repo.start("temporary",1);
        check(id && !repo.snapshot().persistent && repo.snapshot().totals.played==2,"storage failure preserves live counts and reports temporary data");
    }
    for (unsigned i=0;i<40;++i) { id=model.start("history",1); model.finish(id,GameOutcome::Won,i); }
    check(model.history.size()==32 && model.global.played==45 && model.valid(),"bounded history retains complete lifetime aggregates");
    GameSessionClock clock; clock.start(0); clock.tick(1500); clock.pause(1750); clock.resume(5000); clock.stop(6500);
    check(clock.elapsed(9000)==3250 && clock.paused(9000)==3250,"duration excludes pauses including fractional seconds");
    clock.start(0xfffffff0u); clock.tick(0x10u); clock.stop(0x20u);
    check(clock.elapsed(123)==48,"session clock handles millis wraparound");
    check(StatisticsRepository::testNvsRoundTrip(),"isolated NVS snapshot survives repository recreation");
    const auto sound=UIFeedback::soundEnabled(); UIFeedback::setSoundEnabled(false);
    {
        auto *root=lv_obj_create(nullptr); PlayerSelectorDialog selector;
        bool confirmed=false; uint16_t count=0;
        selector.create(root,[&](bool yes,uint16_t value) { confirmed=yes; count=value; });
        selector.show(1); auto *down=button(root,LV_SYMBOL_DOWN),*up=button(root,LV_SYMBOL_UP);
        check(down && lv_obj_has_state(down,LV_STATE_DISABLED),"player selector disables decrement at one");
        if (up && down)
        {
            lv_obj_send_event(down,LV_EVENT_CLICKED,nullptr);
            lv_obj_send_event(up,LV_EVENT_CLICKED,nullptr);
            lv_obj_send_event(button(root,tr(Text::Start)),LV_EVENT_CLICKED,nullptr);
            check(confirmed && count==2,"arrow increment and confirmation publish the selected count");
            selector.show(UINT16_MAX); lv_obj_send_event(up,LV_EVENT_CLICKED,nullptr);
            lv_obj_send_event(button(root,tr(Text::Start)),LV_EVENT_CLICKED,nullptr);
            check(count==UINT16_MAX && lv_obj_has_state(up,LV_STATE_DISABLED),"selector cannot overflow its stored player count");
            lv_obj_send_event(button(root,tr(Text::Cancel)),LV_EVENT_CLICKED,nullptr);
            check(!confirmed,"cancel is distinct from confirming a session");
        }
        selector.clear(); lv_obj_delete(root);
    }
    UIFeedback::setSoundEnabled(sound);
    Serial.printf("@STATSTEST passed=%u failed=%u\n",passed,failed);
}
#endif
