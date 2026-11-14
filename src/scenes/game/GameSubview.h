#pragma once

#include <functional>

using EventCallback = std::function<void(const char*)>;

enum class GameSubview { Loading, Main, Victory, Fail, GameObject, Hardware };
