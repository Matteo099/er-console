#pragma once
#include "subviews/GameObjectSubview.h"
#include <algorithm>

// Owns authored discoveries; only landed entries are exposed to the viewer.
class GameObjectInventory {
  public:
    using Object = GameObjectSubview::ObjectData;

    static std::string key(const Object& object) {
        return object.id.empty() ? "title:" + object.title : "id:" + object.id;
    }

    void set(const std::vector<Object>& objects) {
        auto previous = std::move(entries);
        entries.clear();
        for (const auto& object : objects) {
            const auto id = key(object);
            const auto old = std::find_if(previous.begin(), previous.end(), [&](const Entry& e) {
                return key(e.object) == id;
            });
            discover(object);
            if (old != previous.end())
                find(id)->collected = old->collected;
        }
    }

    void discover(const Object& object) {
        if (auto* entry = find(key(object)))
            entry->object = object;
        else {
            Entry created;
            created.object = object;
            entries.push_back(std::move(created));
        }
    }

    const Object* next() const {
        for (const auto& entry : entries)
            if (!entry.collected)
                return &entry.object;
        return nullptr;
    }

    bool contains(const std::string& id) const {
        return std::any_of(entries.begin(), entries.end(), [&](const Entry& e) {
            return key(e.object) == id;
        });
    }

    void collect(const std::string& id) {
        if (auto* entry = find(id))
            entry->collected = true;
    }

    std::vector<Object> collected() const {
        std::vector<Object> result;
        for (const auto& entry : entries)
            if (entry.collected)
                result.push_back(entry.object);
        return result;
    }

    size_t count() const {
        return std::count_if(entries.begin(), entries.end(), [](const Entry& e) {
            return e.collected;
        });
    }

    void clear() {
        entries.clear();
    }

  private:
    struct Entry {
        Object object;
        bool collected = false;
    };

    std::vector<Entry> entries;

    Entry* find(const std::string& id) {
        for (auto& entry : entries)
            if (key(entry.object) == id)
                return &entry;
        return nullptr;
    }
};
