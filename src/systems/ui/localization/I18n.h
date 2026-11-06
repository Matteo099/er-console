#pragma once
#include <cstdint>
#include <string>

enum class Text : uint16_t
{
#define X(key, italian, english) key,
#include "Translations.def"
#undef X
    Count
};

class I18n
{
public:
    enum class Language : uint8_t { Italian, English };
    static Language language();
    static void select(Language language);
    static void select(const char *code);
    static const char *code();
    static const char *text(Text key);
    static Text identify(const char *text);
    static uint32_t revision();
    static std::string format(Text key, ...);
};
inline const char *tr(Text key) { return I18n::text(key); }
