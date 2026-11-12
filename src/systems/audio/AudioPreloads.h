#pragma once
#include <array>
#include <cstdint>
#include <cstring>

using AudioAssetId = uint32_t;
enum class AudioAssetState : uint8_t { Missing, Pending, Ready, Failed };

// AudioSystem protects this small reservation table with its control mutex.
// Reserving a path pins its cache entry even while it is not playing.
class AudioPreloads {
  public:
    static constexpr size_t CAPACITY = 8, PATH_LENGTH = 128;

    struct Asset {
        AudioAssetId id = 0;
        char path[PATH_LENGTH]{};
        uint16_t references = 0;
        AudioAssetState state = AudioAssetState::Pending;
    };

    AudioAssetId retain(const char* path, bool& created) {
        created = false;
        if (!path || !*path || std::strlen(path) >= PATH_LENGTH)
            return 0;
        for (auto& asset : assets)
            if (asset.id && !std::strcmp(asset.path, path)) {
                if (asset.references == UINT16_MAX)
                    return 0;
                ++asset.references;
                return asset.id;
            }
        for (size_t i = 0; i < assets.size(); ++i)
            if (!assets[i].id) {
                generation = (generation + 1) & 0x07ffffff;
                if (!generation)
                    ++generation;
                auto& asset = assets[i];
                asset.id = (generation << 4) | uint32_t(i + 1);
                asset.references = 1;
                std::strcpy(asset.path, path);
                created = true;
                return asset.id;
            }
        return 0;
    }

    Asset* find(AudioAssetId id) {
        const auto slot = id & 15;
        if (!slot || slot > assets.size())
            return nullptr;
        auto& asset = assets[slot - 1];
        return asset.id == id ? &asset : nullptr;
    }

    AudioAssetState state(AudioAssetId id) {
        const auto* asset = find(id);
        return asset ? asset->state : AudioAssetState::Missing;
    }

    void finish(AudioAssetId id, bool ready) {
        if (auto* asset = find(id))
            asset->state = ready ? AudioAssetState::Ready : AudioAssetState::Failed;
    }

    bool release(AudioAssetId id) {
        if (auto* asset = find(id)) {
            if (!--asset->references)
                *asset = Asset{};
            return true;
        }
        return false;
    }

    bool retained(const char* path) const {
        for (const auto& asset : assets)
            if (asset.id && !std::strcmp(asset.path, path))
                return true;
        return false;
    }

  private:
    std::array<Asset, CAPACITY> assets{};
    uint32_t generation = 0;
};
