#include "VolumeFeedbackDiagnostics.h"
#include "config/Console.h"

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include <algorithm>
#include <cstdlib>
#include "VolumeFeedback.h"
#include "systems/audio/AudioSystem.h"
#include "scenes/settings/AudioPreferenceSubview.h"

namespace {
lv_obj_t* find(lv_obj_t* root, const lv_obj_class_t* type) {
    if (lv_obj_check_type(root, type))
        return root;
    for (unsigned i = 0; i < lv_obj_get_child_count(root); ++i)
        if (auto* object = find(lv_obj_get_child(root, i), type))
            return object;
    return nullptr;
}

int peak(AudioChannel channel, uint8_t volume) {
    AudioMixer mixer(22050);
    AudioPreferences preferences;
    preferences.music = {channel == AudioChannel::Music, volume};
    preferences.effects = {channel == AudioChannel::Effects, volume};
    mixer.configure(preferences);
    mixer.tone(VolumeFeedback::FREQUENCY_HZ, VolumeFeedback::DURATION_MS, 0,
               VolumeFeedback::SOURCE_VOLUME, channel);
    uint8_t data[1764]{};
    const int length = mixer.read(data, sizeof(data));
    int value = 0;
    for (int i = 0; i + 1 < length; i += 2)
        value =
            std::max(value, std::abs(int(int16_t(uint16_t(data[i]) | uint16_t(data[i + 1]) << 8))));
    return value;
}
}  // namespace

void runVolumeFeedbackDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool okay, const char* name) {
        okay ? ++passed : ++failed;
        Serial.printf("@VOLUMECHECK %s %s\n", okay ? "PASS" : "FAIL", name);
    };
    auto queue = xQueueCreate(8, sizeof(AudioEvent));
    {
        AudioSystem audio(queue);
        VolumeFeedback feedback(&audio);
        AudioEvent event{};
        feedback.request(AudioChannel::Music, {true, 20}, 0);
        const bool first = xQueueReceive(queue, &event, 0) == pdPASS;
        uint8_t gain = 0;
        check(first && event.type == AudioEventType::BEEP && event.channel == AudioChannel::Music,
              "first slider change immediately queues the selected music bus");
        check(event.beep.ms == 40 && event.beep.freq == 750 &&
                  audio.getSourceVolume(event.source, gain) && gain == 50,
              "preview is short, quiet and generated without SD access");
        const auto original = event.source;
        for (unsigned i = 1; i <= 24; ++i)
            feedback.request(AudioChannel::Music, {true, uint8_t(20 + i)}, i * 10);
        check(uxQueueMessagesWaiting(queue) == 0,
              "24 rapid drag changes are coalesced without flooding the queue");
        feedback.update(249);
        check(!uxQueueMessagesWaiting(queue), "cooldown does not fire early");
        feedback.update(250);
        check(xQueueReceive(queue, &event, 0) == pdPASS && event.channel == AudioChannel::Music,
              "final drag value gets trailing feedback at the deadline");
        check(!audio.isPlaying(original), "new preview releases only its previous source");
        feedback.update(1000);
        check(!uxQueueMessagesWaiting(queue), "idle slider never repeats previews");
        feedback.request(AudioChannel::Music, {true, 44}, 1100);
        check(!uxQueueMessagesWaiting(queue),
              "unchanged value does not repeat an accepted preview");
        feedback.request(AudioChannel::Effects, {true, 44}, 1100);
        check(xQueueReceive(queue, &event, 0) == pdPASS && event.channel == AudioChannel::Effects,
              "effects slider uses its own bus independently of music");
        feedback.request(AudioChannel::Music, {true, 60}, 1120);
        check(!uxQueueMessagesWaiting(queue), "both sliders share the same frequency limit");
        feedback.update(1350);
        check(xQueueReceive(queue, &event, 0) == pdPASS && event.channel == AudioChannel::Music,
              "latest channel wins over a pending preview");
        const auto beforeMute = event.source;
        feedback.request(AudioChannel::Music, {true, 0}, 1360);
        feedback.update(1700);
        check(!audio.isPlaying(beforeMute) && !uxQueueMessagesWaiting(queue),
              "zero volume cancels current and pending feedback");
        feedback.request(AudioChannel::Effects, {false, 80}, 1800);
        feedback.update(2100);
        check(!uxQueueMessagesWaiting(queue),
              "disabled audio stays silent while its slider changes");
        const auto background = audio.play("background.wav", true);
        xQueueReceive(queue, &event, 0);
        feedback.request(AudioChannel::Music, {true, 55}, 2200);
        xQueueReceive(queue, &event, 0);
        feedback.request(AudioChannel::Music, {true, 65}, 2210);
        feedback.stop();
        feedback.update(2600);
        check(!audio.isPlaying(event.source) && !uxQueueMessagesWaiting(queue),
              "leaving the section cancels trailing and active previews");
        check(audio.isPlaying(background),
              "preview cancellation preserves unrelated background audio");
        audio.stop();
        feedback.bind(&audio);
        feedback.request(AudioChannel::Music, {true, 25}, 0xfffffff0u);
        xQueueReceive(queue, &event, 0);
        feedback.request(AudioChannel::Music, {true, 35}, 0x20u);
        feedback.update(0xe9u);
        check(!uxQueueMessagesWaiting(queue), "rate limit survives millis wraparound");
        feedback.update(0xeau);
        check(xQueueReceive(queue, &event, 0) == pdPASS,
              "trailing feedback resumes at the wrapped deadline");
        feedback.stop();
        audio.stop();
        feedback.bind(&audio);
        AudioEvent blocker{};
        blocker.type = AudioEventType::PRELOAD;
        for (unsigned i = 0; i < 8; ++i)
            xQueueSend(queue, &blocker, 0);
        feedback.request(AudioChannel::Effects, {true, 35}, 0);
        feedback.update(249);
        check(uxQueueMessagesWaiting(queue) == 8,
              "full queue does not block UI or enqueue repeated previews");
        while (xQueueReceive(queue, &event, 0)) {
        }
        feedback.update(250);
        check(xQueueReceive(queue, &event, 0) == pdPASS && event.type == AudioEventType::BEEP,
              "preview retries after queue pressure without losing source slots");
    }
    vQueueDelete(queue);
    const int music100 = peak(AudioChannel::Music, 100), music25 = peak(AudioChannel::Music, 25);
    check(music100 > 7000 && std::abs(music25 * 4 - music100) <= 4,
          "rendered PCM amplitude reflects music volume even with effects disabled");
    check(peak(AudioChannel::Effects, 25) == music25,
          "effects preview applies the same gain independently");
    check(peak(AudioChannel::Music, 0) == 0 && peak(AudioChannel::Effects, 0) == 0,
          "zero settings produce silent PCM");
    {
        auto* root = lv_obj_create(nullptr);
        AudioPreferenceSubview view;
        unsigned changes = 0;
        bool preview = false;
        AudioChannelPreferences value;
        view.create(root, Text::Music, [&](AudioChannelPreferences changed, bool fromSlider) {
            ++changes;
            preview = fromSlider;
            value = changed;
        });
        view.set({true, 40});
        check(changes == 0, "loading or refreshing preferences never requests a sound");
        auto *slider = find(root, &lv_slider_class), *toggle = find(root, &lv_switch_class);
        if (slider && toggle) {
            lv_slider_set_value(slider, 65, LV_ANIM_OFF);
            lv_obj_send_event(slider, LV_EVENT_VALUE_CHANGED, nullptr);
            check(changes == 1 && preview && value.volume == 65,
                  "actual slider event publishes preview intent and current level");
            lv_obj_remove_state(toggle, LV_STATE_CHECKED);
            lv_obj_send_event(toggle, LV_EVENT_VALUE_CHANGED, nullptr);
            check(changes == 2 && !preview && !value.enabled,
                  "toggle event updates preferences without requesting a preview");
        } else
            check(false, "audio subview controls found");
        view.clear();
        lv_obj_delete(root);
    }
    Serial.printf("@VOLUMETEST passed=%u failed=%u\n", passed, failed);
}
#endif
