#include "I18n.h"
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace {
std::atomic<I18n::Language> active{I18n::Language::Italian};
std::atomic<uint32_t> generation{0};

struct Translation {
    const char *italian, *english;
};

const Translation messages[] = {
#define X(key, italian, english) {italian, english},
#include "Translations.def"
#undef X
};
static_assert(sizeof(messages) / sizeof(messages[0]) == size_t(Text::Count),
              "Complete language catalog");
}  // namespace

I18n::Language I18n::language() {
    return active.load(std::memory_order_relaxed);
}

void I18n::select(Language language) {
    language = language == Language::English ? language : Language::Italian;
    if (active.exchange(language) != language)
        generation.fetch_add(1);
}

uint32_t I18n::revision() {
    return generation.load();
}

Text I18n::identify(const char* value) {
    for (size_t i = 1; i < size_t(Text::Count); ++i)
        if (value == messages[i].italian || value == messages[i].english)
            return static_cast<Text>(i);
    return Text::None;
}

void I18n::select(const char* code) {
    select(code && (!std::strcmp(code, "en") || !std::strncmp(code, "en-", 3)) ? Language::English
                                                                               : Language::Italian);
}

const char* I18n::code() {
    return language() == Language::English ? "en-GB" : "it-IT";
}

const char* I18n::text(Text key) {
    if (size_t(key) >= size_t(Text::Count))
        key = Text::None;
    const auto& entry = messages[size_t(key)];
    return language() == Language::English ? entry.english : entry.italian;
}

std::string I18n::format(Text key, ...) {
    char buffer[512];
    va_list arguments;
    va_start(arguments, key);
    std::vsnprintf(buffer, sizeof(buffer), text(key), arguments);
    va_end(arguments);
    return buffer;
}
