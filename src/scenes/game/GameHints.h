#pragma once
#include "subviews/GameObjectSubview.h"
#include <algorithm>

struct HintDefinition {
    GameObjectSubview::ObjectData data;
    std::string step;
    uint32_t afterMs = 0;
};

// Deadlines use the session's active-play clock, never wall-clock time.
class GameHints {
  public:
    using Data = GameObjectSubview::ObjectData;

    void configure(std::vector<HintDefinition> value) {
        rules = std::move(value);
        std::stable_sort(rules.begin(), rules.end(),
                         [](const HintDefinition& a, const HintDefinition& b) {
                             return a.afterMs < b.afterMs;
                         });
        for (const auto& rule : rules)
            if (find(rule.data.id))
                unlock(rule.data);
    }

    void setStep(const std::string& value, uint64_t activeMs) {
        if (step == value)
            return;
        step = value;
        stepStarted = activeMs;
    }

    bool update(uint64_t activeMs) {
        bool changed = false;
        if (step.empty() || activeMs < stepStarted)
            return false;
        for (const auto& rule : rules)
            if (rule.step == step && activeMs - stepStarted >= rule.afterMs &&
                !find(rule.data.id)) {
                unlock(rule.data);
                changed = true;
            }
        return changed;
    }

    void unlock(const Data& data) {
        if (data.id.empty())
            return;
        if (auto* existing = find(data.id))
            *existing = data;
        else
            unlocked.push_back(data);
    }

    bool markViewed(const std::string& id) {
        if (!find(id) || std::find(viewed.begin(), viewed.end(), id) != viewed.end())
            return false;
        viewed.push_back(id);
        return true;
    }

    size_t viewedCount() const {
        return viewed.size();
    }

    const std::vector<Data>& available() const {
        return unlocked;
    }

    void clear() {
        rules.clear();
        unlocked.clear();
        viewed.clear();
        step.clear();
        stepStarted = 0;
    }

  private:
    std::vector<HintDefinition> rules;
    std::vector<Data> unlocked;
    std::vector<std::string> viewed;
    std::string step;
    uint64_t stepStarted = 0;

    Data* find(const std::string& id) {
        for (auto& data : unlocked)
            if (data.id == id)
                return &data;
        return nullptr;
    }
};
