#include "AudioPreloadDiagnostics.h"
#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include "systems/audio/AudioSystem.h"
#include "scenes/game/GameAudioAssets.h"
#include "scenes/game/GameView.h"
#include "systems/ui/Scene.h"
#include "config/Assets.h"

namespace {
class PreloadPreview;
PreloadPreview* active = nullptr;

class PreloadPreview : public Scene {
    GameView view;
    GameAudioAssets assets;
    AudioSystem* audio = nullptr;
    uint8_t master = 100;
    uint32_t enteredAt = 0;
    bool begun = false, ready = false;

  public:
    void onEnter() override {
        audio = static_cast<AudioSystem*>(
            SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
        master = active ? active->master : audio->getVolume();
        active = this;
        audio->setVolume(1);
        audio->stop();
        view.create();
        view.setTime(60);
        view.setRunning(false);
        view.showLoading();
        view.setLoadingMessage(tr(Text::PrepareAudio));
        enteredAt = lv_tick_get();
    }

    void onUpdate() override {
        if (!begun && lv_tick_get() - enteredAt >= 160) {
            assets.begin(audio, lv_tick_get());
            begun = true;
        }
        if (begun && !ready && assets.prepared(lv_tick_get())) {
            ready = true;
            view.showMain();
            Serial.printf("@PRELOADREADY elapsed=%lu\n",
                          static_cast<unsigned long>(lv_tick_get() - enteredAt));
        }
        view.update();
    }

    void action(const char* name) {
        if (!ready) {
            Serial.println("@PRELOADERROR not ready");
            return;
        }
        Serial.printf("@PRELOADEVENT %s at=%lu\n", name, static_cast<unsigned long>(millis()));
        if (!std::strcmp(name, "pressure")) {
            audio->preload(AUDIO_MAIN_THEME_FILE);
            return;
        }
        if (!std::strcmp(name, "pause")) {
            view.setRunning(false);
            return;
        }
        if (!std::strcmp(name, "resume")) {
            view.setRunning(true);
            return;
        }
        view.setRunning(false);
        audio->stop();
        if (!std::strcmp(name, "clock")) {
            view.showMain();
            view.setTime(60);
            view.setRunning(true);
        } else if (!std::strcmp(name, "fail"))
            view.showFailure();
        else if (!std::strcmp(name, "victory")) {
            view.setVictoryStats({});
            view.showVictory();
        }
    }

    void onExit() override {
        view.setRunning(false);
        audio->stop();
        assets.release();
        view.destroy();
        if (active == this) {
            audio->setVolume(master);
            active = nullptr;
        }
    }

    View& getView() override {
        return view;
    }
};
}  // namespace

Scene* createAudioPreloadPreview() {
    return new PreloadPreview;
}

void audioPreloadAction(const char* action) {
    if (active)
        active->action(action);
}

void runAudioPreloadDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool okay, const char* name) {
        okay ? ++passed : ++failed;
        Serial.printf("@PRELOADCHECK %s %s\n", okay ? "PASS" : "FAIL", name);
    };
    AudioPreloads reservations;
    bool created = false;
    const auto first = reservations.retain("first.wav", created);
    check(first && created && reservations.state(first) == AudioAssetState::Pending,
          "new reservation is pending");
    const auto shared = reservations.retain("first.wav", created);
    check(shared == first && !created, "shared path deduplicates the queued preload");
    reservations.finish(first, true);
    check(reservations.state(first) == AudioAssetState::Ready && reservations.retained("first.wav"),
          "ready unused file stays pinned");
    reservations.release(shared);
    check(reservations.retained("first.wav"),
          "one owner cannot release another scene's reservation");
    reservations.release(first);
    check(!reservations.retained("first.wav") &&
              reservations.state(first) == AudioAssetState::Missing,
          "last owner unpins the cache entry");
    const auto replacement = reservations.retain("next.wav", created);
    reservations.finish(first, true);
    reservations.release(first);
    check(replacement != first && reservations.state(replacement) == AudioAssetState::Pending,
          "stale completion or release cannot affect a reused slot");
    reservations.finish(replacement, false);
    check(reservations.state(replacement) == AudioAssetState::Failed,
          "I/O or WAV validation failure is observable");
    reservations.release(replacement);
    check(!reservations.retain(nullptr, created) && !reservations.retain("", created) &&
              !reservations.retain(std::string(128, 'a').c_str(), created),
          "invalid and truncated asset paths rejected");
    std::array<AudioAssetId, 8> ids{};
    for (unsigned i = 0; i < ids.size(); ++i)
        ids[i] = reservations.retain(("asset" + std::to_string(i)).c_str(), created);
    check(ids.back() && !reservations.retain("overflow", created),
          "reservation count is bounded without replacing active pins");
    for (auto id : ids)
        reservations.release(id);
    auto queue = xQueueCreate(8, sizeof(AudioEvent));
    {
        AudioSystem audio(queue);
        AudioEvent event{};
        const auto one = audio.retainAsset("first.wav"), two = audio.retainAsset("first.wav");
        check(one == two && uxQueueMessagesWaiting(queue) == 1,
              "service enqueues shared asset only once");
        check(xQueueReceive(queue, &event, 0) && event.type == AudioEventType::PRELOAD &&
                  event.source == one,
              "queued load carries a generation-safe reservation id");
        audio.releaseAsset(one);
        check(audio.assetState(two) == AudioAssetState::Pending,
              "service retains another owner's pending load");
        audio.releaseAsset(two);
        check(audio.assetState(one) == AudioAssetState::Missing,
              "release cancels an unprocessed scene load");
        for (unsigned i = 0; i < 8; ++i)
            audio.preload("busy.wav");
        bool rejected = true;
        for (unsigned i = 0; i < 20; ++i)
            rejected &= !audio.retainAsset("retry.wav");
        check(rejected, "full queue rejects without leaking reservation slots");
        while (xQueueReceive(queue, &event, 0)) {
        }
        const auto retry = audio.retainAsset("retry.wav");
        check(retry && audio.assetState(retry) == AudioAssetState::Pending,
              "reservation succeeds after queue pressure clears");
        audio.releaseAsset(retry);
        xQueueReceive(queue, &event, 0);
        {
            GameAudioAssets game;
            game.begin(&audio, 0);
            check(uxQueueMessagesWaiting(queue) == 3 && !game.prepared(0),
                  "game waits for all three event sounds before enabling play");
            check(!game.prepared(14999) && game.prepared(15000),
                  "unavailable audio has a bounded best-effort timeout");
            game.release();
            bool released = true;
            while (xQueueReceive(queue, &event, 0))
                released &= audio.assetState(event.source) == AudioAssetState::Missing;
            check(released, "leaving game releases every event sound reservation");
        }
        GameAudioAssets noService;
        noService.begin(nullptr, 0);
        check(noService.prepared(0), "missing optional audio service does not prevent gameplay");
    }
    vQueueDelete(queue);
    Serial.printf("@PRELOADTEST passed=%u failed=%u\n", passed, failed);
}
#endif
