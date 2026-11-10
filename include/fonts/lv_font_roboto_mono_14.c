/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --bpp 1 --size 14 --no-compress --stride 1 --align 1 --font RobotoMono-VariableFont_wght.ttf --range 32-127,160-255 --format lvgl -o lv_font_roboto_mono_14.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef LV_FONT_ROBOTO_MONO_14
#define LV_FONT_ROBOTO_MONO_14 1
#endif

#if LV_FONT_ROBOTO_MONO_14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfe, 0x20,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x12, 0x12, 0x12, 0x7f, 0x14, 0x24, 0x24, 0xff,
    0x24, 0x24, 0x28,

    /* U+0024 "$" */
    0x10, 0xe4, 0xd1, 0x41, 0x83, 0x83, 0x6, 0x1c,
    0xde, 0x10,

    /* U+0025 "%" */
    0x60, 0x90, 0x94, 0x94, 0x68, 0x8, 0x16, 0x19,
    0x29, 0x9, 0x6,

    /* U+0026 "&" */
    0x38, 0x91, 0x22, 0x47, 0xe, 0x34, 0xcd, 0x8f,
    0x99, 0xe8,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x5, 0x29, 0x24, 0x92, 0x24, 0x88,

    /* U+0029 ")" */
    0x11, 0x22, 0x49, 0x24, 0xa4, 0xa0,

    /* U+002A "*" */
    0x10, 0x23, 0x5b, 0xe3, 0x85, 0x11, 0x0,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0x56,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x8, 0x84, 0x22, 0x10, 0x88, 0x46, 0x21, 0x0,

    /* U+0030 "0" */
    0x79, 0x28, 0x61, 0x9e, 0xde, 0x61, 0x85, 0x27,
    0x80,

    /* U+0031 "1" */
    0x3, 0xd1, 0x11, 0x11, 0x11, 0x10,

    /* U+0032 "2" */
    0x79, 0x9a, 0x10, 0x20, 0x41, 0x4, 0x18, 0x60,
    0x83, 0xf8,

    /* U+0033 "3" */
    0x7b, 0x18, 0x41, 0xc, 0xe0, 0xc1, 0x87, 0x37,
    0x80,

    /* U+0034 "4" */
    0x4, 0x18, 0x70, 0xa2, 0x4c, 0x91, 0x7f, 0x4,
    0x8, 0x10,

    /* U+0035 "5" */
    0x7d, 0x4, 0x10, 0xf9, 0x30, 0x41, 0x87, 0x37,
    0x80,

    /* U+0036 "6" */
    0x39, 0x84, 0x20, 0xbb, 0x38, 0x61, 0x85, 0x33,
    0x80,

    /* U+0037 "7" */
    0xfe, 0xc, 0x10, 0x20, 0x81, 0x6, 0x8, 0x30,
    0x40, 0x80,

    /* U+0038 "8" */
    0x7b, 0x38, 0x61, 0xcd, 0xec, 0xe1, 0x87, 0x37,
    0x80,

    /* U+0039 "9" */
    0x7b, 0x28, 0x61, 0x87, 0x37, 0xc1, 0x8, 0x67,
    0x0,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0xf0, 0xa, 0xa0,

    /* U+003C "<" */
    0x4, 0x76, 0x30, 0x70, 0x30, 0x0,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0x83, 0x81, 0x83, 0x3b, 0x0, 0x0,

    /* U+003F "?" */
    0x7b, 0x18, 0x41, 0xc, 0x63, 0x8, 0x0, 0x2,
    0x0,

    /* U+0040 "@" */
    0x3c, 0x62, 0x5f, 0x95, 0xa5, 0xa5, 0xa9, 0xa9,
    0x96, 0x40, 0x3c,

    /* U+0041 "A" */
    0x10, 0x60, 0xe1, 0x46, 0x89, 0x13, 0x3e, 0x85,
    0xe, 0x8,

    /* U+0042 "B" */
    0xfa, 0x38, 0x61, 0x8f, 0xe8, 0xe1, 0x86, 0x3f,
    0x80,

    /* U+0043 "C" */
    0x38, 0x8a, 0x14, 0x8, 0x10, 0x20, 0x40, 0x84,
    0x88, 0xe0,

    /* U+0044 "D" */
    0xf9, 0x1a, 0x14, 0x18, 0x30, 0x60, 0xc1, 0x85,
    0x1b, 0xe0,

    /* U+0045 "E" */
    0xfe, 0x8, 0x20, 0x83, 0xe8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0x83, 0xe8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0047 "G" */
    0x38, 0x8a, 0xc, 0x8, 0x10, 0x23, 0xc1, 0xc2,
    0x84, 0xf0,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x18,
    0x40,

    /* U+0049 "I" */
    0xfc, 0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x8f,
    0xc0,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x10, 0x41, 0x87, 0x27,
    0x80,

    /* U+004B "K" */
    0x85, 0x12, 0x65, 0x8a, 0x1c, 0x3c, 0x4c, 0x89,
    0x1a, 0x10,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+004D "M" */
    0xcf, 0x3c, 0xf3, 0xb6, 0xdb, 0x69, 0x86, 0x18,
    0x40,

    /* U+004E "N" */
    0x87, 0x1c, 0x79, 0xa6, 0xd9, 0x67, 0x8e, 0x38,
    0x40,

    /* U+004F "O" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0050 "P" */
    0xfa, 0x38, 0x61, 0x86, 0x3f, 0xa0, 0x82, 0x8,
    0x0,

    /* U+0051 "Q" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x82,
    0x88, 0xf0, 0x30, 0x20,

    /* U+0052 "R" */
    0xfa, 0x38, 0x61, 0x86, 0x3f, 0xa6, 0x8a, 0x38,
    0x40,

    /* U+0053 "S" */
    0x7b, 0x38, 0x60, 0xc1, 0xe0, 0xc1, 0x87, 0x37,
    0x80,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x61, 0x85, 0x27,
    0x80,

    /* U+0056 "V" */
    0x43, 0x42, 0x62, 0x62, 0x26, 0x24, 0x34, 0x14,
    0x18, 0x18, 0x18,

    /* U+0057 "W" */
    0x49, 0x59, 0x59, 0x59, 0x57, 0x56, 0x56, 0x56,
    0x66, 0x26, 0x26,

    /* U+0058 "X" */
    0x87, 0x89, 0x31, 0x43, 0x82, 0xe, 0x34, 0x4d,
    0x8a, 0x18,

    /* U+0059 "Y" */
    0xc6, 0x89, 0x13, 0x62, 0x87, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+005A "Z" */
    0xfc, 0x30, 0x86, 0x10, 0xc2, 0x18, 0x43, 0xf,
    0xc0,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x49, 0xc0,

    /* U+005C "\\" */
    0x84, 0x30, 0x84, 0x10, 0x84, 0x10, 0x86, 0x10,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x93, 0xc0,

    /* U+005E "^" */
    0x23, 0x18, 0xa9, 0x4c,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0xc8,

    /* U+0061 "a" */
    0x7b, 0x10, 0x5f, 0xc6, 0x18, 0xdf,

    /* U+0062 "b" */
    0x82, 0x8, 0x3e, 0xca, 0x18, 0x61, 0x87, 0x2f,
    0x80,

    /* U+0063 "c" */
    0x38, 0x8a, 0x1c, 0x8, 0x10, 0x91, 0x1c,

    /* U+0064 "d" */
    0x4, 0x10, 0x5f, 0x4e, 0x18, 0x61, 0x85, 0x37,
    0xc0,

    /* U+0065 "e" */
    0x38, 0x8a, 0x17, 0xf8, 0x18, 0x11, 0x1c,

    /* U+0066 "f" */
    0x1c, 0x40, 0x87, 0xe2, 0x4, 0x8, 0x10, 0x20,
    0x40, 0x80,

    /* U+0067 "g" */
    0x7d, 0x38, 0x61, 0x86, 0x14, 0xdf, 0x6, 0x37,
    0x80,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+0069 "i" */
    0x10, 0x0, 0x3c, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+006A "j" */
    0x10, 0x7, 0x11, 0x11, 0x11, 0x11, 0x1e,

    /* U+006B "k" */
    0x82, 0x8, 0x22, 0x92, 0xce, 0x3c, 0x92, 0x28,
    0xc0,

    /* U+006C "l" */
    0xf0, 0x41, 0x4, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+006D "m" */
    0xff, 0x26, 0x4c, 0x99, 0x32, 0x64, 0xc9,

    /* U+006E "n" */
    0xbb, 0x18, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x51, 0x1c,

    /* U+0070 "p" */
    0xfa, 0x28, 0x61, 0x86, 0x18, 0xbe, 0x82, 0x8,
    0x0,

    /* U+0071 "q" */
    0x7d, 0x18, 0x61, 0x86, 0x14, 0x5f, 0x4, 0x10,
    0x40,

    /* U+0072 "r" */
    0xbe, 0x21, 0x8, 0x42, 0x10,

    /* U+0073 "s" */
    0x7b, 0x38, 0x3c, 0x1c, 0x1c, 0xde,

    /* U+0074 "t" */
    0x20, 0x8f, 0xc8, 0x20, 0x82, 0x8, 0x20, 0x70,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0xdd,

    /* U+0076 "v" */
    0x87, 0x89, 0x12, 0x46, 0x85, 0xc, 0x8,

    /* U+0077 "w" */
    0xc9, 0x59, 0x59, 0x5a, 0x56, 0x66, 0x26, 0x26,

    /* U+0078 "x" */
    0xc4, 0x98, 0xa1, 0x83, 0x5, 0x13, 0x62,

    /* U+0079 "y" */
    0x43, 0x42, 0x62, 0x24, 0x34, 0x1c, 0x18, 0x8,
    0x10, 0x10, 0x60,

    /* U+007A "z" */
    0xfc, 0x31, 0x84, 0x21, 0x8c, 0x3f,

    /* U+007B "{" */
    0x12, 0x22, 0x22, 0xc6, 0x22, 0x22, 0x31,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0x84, 0x44, 0x44, 0x36, 0x44, 0x44, 0xc8,

    /* U+007E "~" */
    0x71, 0x59, 0x86,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0x8f, 0xe0,

    /* U+00A2 "¢" */
    0x10, 0x71, 0x14, 0x28, 0x10, 0x20, 0x42, 0x44,
    0x70, 0x40,

    /* U+00A3 "£" */
    0x3c, 0xcd, 0xa, 0x4, 0x1f, 0x10, 0x20, 0x40,
    0x83, 0xf8,

    /* U+00A4 "¤" */
    0xff, 0x8e, 0xc, 0x18, 0x30, 0x71, 0xfd,

    /* U+00A5 "¥" */
    0x83, 0x8d, 0x13, 0x62, 0x85, 0x3f, 0x88, 0xfe,
    0x20, 0x40,

    /* U+00A6 "¦" */
    0xf9, 0xf0,

    /* U+00A7 "§" */
    0x7d, 0x8e, 0xc, 0x7, 0x13, 0xa1, 0xc1, 0x72,
    0x38, 0x1c, 0x1c, 0x6f, 0x80,

    /* U+00A8 "¨" */
    0x90,

    /* U+00A9 "©" */
    0x38, 0xba, 0xad, 0x1a, 0xb3, 0x51, 0x1c,

    /* U+00AA "ª" */
    0x74, 0x9f, 0x3f, 0x80,

    /* U+00AB "«" */
    0x29, 0xa5, 0x14, 0x68, 0xa0,

    /* U+00AC "¬" */
    0xf8, 0x42,

    /* U+00AD "­" */
    0xf8,

    /* U+00AE "®" */
    0x38, 0xfa, 0xad, 0x9b, 0x35, 0x51, 0x1c,

    /* U+00AF "¯" */
    0xf8,

    /* U+00B0 "°" */
    0x6a, 0xa6,

    /* U+00B1 "±" */
    0x10, 0x41, 0x3f, 0x10, 0x41, 0x0, 0xfc,

    /* U+00B2 "²" */
    0xf9, 0x24, 0xf0,

    /* U+00B3 "³" */
    0xf1, 0x61, 0xf0,

    /* U+00B4 "´" */
    0x50,

    /* U+00B5 "µ" */
    0x86, 0x18, 0x61, 0x86, 0x1c, 0x7f, 0x82, 0x8,
    0x0,

    /* U+00B6 "¶" */
    0x7f, 0xff, 0xff, 0xfd, 0xe1, 0x8, 0x42,

    /* U+00B7 "·" */
    0x80,

    /* U+00B8 "¸" */
    0x9c,

    /* U+00B9 "¹" */
    0xe4, 0x92,

    /* U+00BA "º" */
    0x74, 0x63, 0x17, 0x0,

    /* U+00BB "»" */
    0xa2, 0x96, 0xb5, 0x50,

    /* U+00BC "¼" */
    0x40, 0xc0, 0x44, 0x48, 0x48, 0x50, 0x12, 0x26,
    0x2a, 0x1f, 0x2,

    /* U+00BD "½" */
    0x40, 0xc0, 0x48, 0x48, 0x50, 0x50, 0x2e, 0x2a,
    0x42, 0x4, 0xf,

    /* U+00BE "¾" */
    0xe0, 0x20, 0x64, 0x28, 0xa8, 0xe8, 0x12, 0x16,
    0x2a, 0xf, 0x2,

    /* U+00BF "¿" */
    0x20, 0x0, 0x8, 0x21, 0x8, 0x20, 0x8a, 0x67,
    0x0,

    /* U+00C0 "À" */
    0x60, 0x40, 0x40, 0x83, 0x7, 0xa, 0x34, 0x48,
    0x99, 0xf4, 0x28, 0x70, 0x40,

    /* U+00C1 "Á" */
    0xc, 0x30, 0x40, 0x83, 0x7, 0xa, 0x34, 0x48,
    0x99, 0xf4, 0x28, 0x70, 0x40,

    /* U+00C2 "Â" */
    0x0, 0x18, 0x24, 0x18, 0x18, 0x18, 0x3c, 0x24,
    0x24, 0x24, 0x7e, 0x42, 0x42, 0xc2,

    /* U+00C3 "Ã" */
    0x78, 0x0, 0x41, 0x83, 0x85, 0x1a, 0x24, 0x4c,
    0xfa, 0x14, 0x38, 0x20,

    /* U+00C4 "Ä" */
    0x24, 0x0, 0x60, 0xc1, 0x87, 0x89, 0x12, 0x24,
    0xfd, 0xa, 0x14, 0x20,

    /* U+00C5 "Å" */
    0x38, 0x50, 0xe0, 0x83, 0x85, 0xa, 0x14, 0x6c,
    0x89, 0xf6, 0x38, 0x30, 0x40,

    /* U+00C6 "Æ" */
    0x1f, 0x18, 0x18, 0x28, 0x28, 0x2e, 0x48, 0x78,
    0x48, 0x88, 0x8f,

    /* U+00C7 "Ç" */
    0x38, 0x8a, 0x1c, 0x8, 0x10, 0x20, 0x40, 0x82,
    0x88, 0xe0, 0xc0, 0x83, 0x0,

    /* U+00C8 "È" */
    0x41, 0x82, 0x3f, 0x82, 0x8, 0x20, 0xfa, 0x8,
    0x20, 0x83, 0xf0,

    /* U+00C9 "É" */
    0x18, 0x42, 0x3f, 0x82, 0x8, 0x20, 0xfa, 0x8,
    0x20, 0x83, 0xf0,

    /* U+00CA "Ê" */
    0x20, 0xc4, 0xbf, 0x82, 0x8, 0x20, 0xfa, 0x8,
    0x20, 0x83, 0xf0,

    /* U+00CB "Ë" */
    0x48, 0xf, 0xe0, 0x82, 0x8, 0x3e, 0x82, 0x8,
    0x20, 0xfc,

    /* U+00CC "Ì" */
    0xc1, 0x2, 0x3f, 0x20, 0x82, 0x8, 0x20, 0x82,
    0x8, 0x23, 0xf0,

    /* U+00CD "Í" */
    0x18, 0xc2, 0x3f, 0x20, 0x82, 0x8, 0x20, 0x82,
    0x8, 0x23, 0xf0,

    /* U+00CE "Î" */
    0x21, 0x89, 0x3f, 0x20, 0x82, 0x8, 0x20, 0x82,
    0x8, 0x23, 0xf0,

    /* U+00CF "Ï" */
    0x48, 0xf, 0xc8, 0x20, 0x82, 0x8, 0x20, 0x82,
    0x8, 0xfc,

    /* U+00D0 "Ð" */
    0x3c, 0x22, 0x23, 0x21, 0x21, 0xf1, 0x21, 0x21,
    0x23, 0x22, 0x3c,

    /* U+00D1 "Ñ" */
    0x78, 0x8, 0x71, 0xc7, 0x9a, 0x6d, 0x96, 0x78,
    0xe3, 0x84,

    /* U+00D2 "Ò" */
    0x60, 0x60, 0x41, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x6, 0xc, 0x14, 0x47, 0x0,

    /* U+00D3 "Ó" */
    0xc, 0x10, 0x41, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x6, 0xc, 0x14, 0x47, 0x0,

    /* U+00D4 "Ô" */
    0x18, 0x48, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x6, 0xc, 0x14, 0x47, 0x0,

    /* U+00D5 "Õ" */
    0x34, 0x98, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x83,
    0x6, 0xc, 0x14, 0x47, 0x0,

    /* U+00D6 "Ö" */
    0x24, 0x0, 0xe2, 0x28, 0x30, 0x60, 0xc1, 0x83,
    0x6, 0xa, 0x23, 0x80,

    /* U+00D7 "×" */
    0xc5, 0xb3, 0x8c, 0x6b, 0x10, 0x0,

    /* U+00D8 "Ø" */
    0x3e, 0x8a, 0x1c, 0x58, 0xb2, 0x64, 0xd1, 0xc2,
    0x8b, 0xe4, 0x0,

    /* U+00D9 "Ù" */
    0x60, 0x81, 0x21, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x49, 0xe0,

    /* U+00DA "Ú" */
    0x8, 0x61, 0x21, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x49, 0xe0,

    /* U+00DB "Û" */
    0x10, 0x62, 0xe1, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x49, 0xe0,

    /* U+00DC "Ü" */
    0x24, 0x8, 0x61, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x52, 0x78,

    /* U+00DD "Ý" */
    0x8, 0x23, 0x1a, 0x24, 0x4d, 0x8a, 0x1c, 0x10,
    0x20, 0x40, 0x81, 0x0,

    /* U+00DE "Þ" */
    0x82, 0x8, 0x3e, 0x8e, 0x18, 0x63, 0xfa, 0x8,
    0x0,

    /* U+00DF "ß" */
    0x73, 0x28, 0xa2, 0x92, 0x49, 0xa3, 0x86, 0x1b,
    0x80,

    /* U+00E0 "à" */
    0x60, 0xc0, 0x1e, 0x84, 0x17, 0xf1, 0x86, 0x37,
    0xc0,

    /* U+00E1 "á" */
    0x8, 0x40, 0x1e, 0xc4, 0x17, 0xf1, 0x86, 0x37,
    0xc0,

    /* U+00E2 "â" */
    0x31, 0x20, 0x1e, 0xc4, 0x17, 0xf1, 0x86, 0x37,
    0xc0,

    /* U+00E3 "ã" */
    0x78, 0x0, 0x1e, 0xc4, 0x17, 0xf1, 0x86, 0x37,
    0xc0,

    /* U+00E4 "ä" */
    0x48, 0x0, 0x1e, 0x84, 0x17, 0xf1, 0x86, 0x37,
    0xc0,

    /* U+00E5 "å" */
    0x30, 0xa3, 0x0, 0x7b, 0x10, 0x5f, 0xc6, 0x18,
    0xdf,

    /* U+00E6 "æ" */
    0x6e, 0x91, 0x11, 0x7f, 0x90, 0x90, 0x98, 0x6f,

    /* U+00E7 "ç" */
    0x38, 0x8a, 0x1c, 0x8, 0x10, 0x91, 0x1c, 0x18,
    0x10, 0x60,

    /* U+00E8 "è" */
    0x20, 0x60, 0x1, 0xc4, 0x50, 0xbf, 0xc0, 0xc0,
    0x88, 0xe0,

    /* U+00E9 "é" */
    0x8, 0x20, 0x1, 0xc4, 0x50, 0xbf, 0xc0, 0xc0,
    0x88, 0xe0,

    /* U+00EA "ê" */
    0x18, 0x24, 0x0, 0x3c, 0x66, 0x42, 0x7f, 0xc0,
    0x40, 0x62, 0x3c,

    /* U+00EB "ë" */
    0x48, 0x0, 0x1, 0xc4, 0xd0, 0xbf, 0xc0, 0x80,
    0x88, 0xe0,

    /* U+00EC "ì" */
    0x20, 0x40, 0x3c, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+00ED "í" */
    0x8, 0x40, 0x3c, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+00EE "î" */
    0x11, 0xa0, 0x3c, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+00EF "ï" */
    0x48, 0x0, 0x3c, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+00F0 "ð" */
    0x32, 0x38, 0xf0, 0x33, 0xa8, 0xe0, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+00F1 "ñ" */
    0x78, 0x0, 0x2e, 0xc6, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+00F2 "ò" */
    0x60, 0x60, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+00F3 "ó" */
    0xc, 0x20, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+00F4 "ô" */
    0x30, 0x90, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+00F5 "õ" */
    0x7c, 0x0, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+00F6 "ö" */
    0x48, 0x0, 0x1, 0xc4, 0x50, 0x60, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+00F7 "÷" */
    0x10, 0x0, 0x7, 0xf0, 0x0, 0x4, 0x0,

    /* U+00F8 "ø" */
    0x4, 0x79, 0x3c, 0x59, 0x32, 0x68, 0xf2, 0x78,
    0x80,

    /* U+00F9 "ù" */
    0x60, 0x80, 0x21, 0x86, 0x18, 0x61, 0x86, 0x37,
    0x40,

    /* U+00FA "ú" */
    0x18, 0x40, 0x21, 0x86, 0x18, 0x61, 0x86, 0x37,
    0x40,

    /* U+00FB "û" */
    0x31, 0x20, 0x21, 0x86, 0x18, 0x61, 0x86, 0x37,
    0x40,

    /* U+00FC "ü" */
    0x48, 0x0, 0x21, 0x86, 0x18, 0x61, 0x86, 0x37,
    0x40,

    /* U+00FD "ý" */
    0x4, 0x8, 0x0, 0x43, 0x42, 0x62, 0x24, 0x34,
    0x1c, 0x18, 0x8, 0x10, 0x10, 0x60,

    /* U+00FE "þ" */
    0x82, 0x8, 0x20, 0xfa, 0x28, 0x61, 0x86, 0x1c,
    0xbe, 0x82, 0x8, 0x0,

    /* U+00FF "ÿ" */
    0x24, 0x0, 0x2, 0x14, 0x6c, 0x89, 0x16, 0x38,
    0x30, 0x40, 0x83, 0xc, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 134, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 134, .box_w = 1, .box_h = 11, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 134, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 5, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 134, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 26, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 134, .box_w = 1, .box_h = 3, .ofs_x = 3, .ofs_y = 8},
    {.bitmap_index = 48, .adv_w = 134, .box_w = 3, .box_h = 15, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 54, .adv_w = 134, .box_w = 3, .box_h = 15, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 60, .adv_w = 134, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 67, .adv_w = 134, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 74, .adv_w = 134, .box_w = 2, .box_h = 4, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 75, .adv_w = 134, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 76, .adv_w = 134, .box_w = 2, .box_h = 2, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 134, .box_w = 5, .box_h = 12, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 85, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 134, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 134, .box_w = 2, .box_h = 8, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 134, .box_w = 2, .box_h = 11, .ofs_x = 4, .ofs_y = -3},
    {.bitmap_index = 180, .adv_w = 134, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 186, .adv_w = 134, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 189, .adv_w = 134, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 195, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 337, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 134, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 377, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 395, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 414, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 134, .box_w = 3, .box_h = 14, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 471, .adv_w = 134, .box_w = 5, .box_h = 12, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 479, .adv_w = 134, .box_w = 3, .box_h = 14, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 485, .adv_w = 134, .box_w = 5, .box_h = 6, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 489, .adv_w = 134, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 490, .adv_w = 134, .box_w = 3, .box_h = 2, .ofs_x = 3, .ofs_y = 9},
    {.bitmap_index = 491, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 497, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 513, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 529, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 548, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 557, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 566, .adv_w = 134, .box_w = 4, .box_h = 14, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 573, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 582, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 591, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 598, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 611, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 620, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 629, .adv_w = 134, .box_w = 5, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 634, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 640, .adv_w = 134, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 648, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 654, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 661, .adv_w = 134, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 669, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 676, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 687, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 693, .adv_w = 134, .box_w = 4, .box_h = 14, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 700, .adv_w = 134, .box_w = 1, .box_h = 14, .ofs_x = 4, .ofs_y = -3},
    {.bitmap_index = 702, .adv_w = 134, .box_w = 4, .box_h = 14, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 709, .adv_w = 134, .box_w = 8, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 712, .adv_w = 134, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 713, .adv_w = 134, .box_w = 1, .box_h = 11, .ofs_x = 4, .ofs_y = -3},
    {.bitmap_index = 715, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 725, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 735, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 742, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 752, .adv_w = 134, .box_w = 1, .box_h = 12, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 754, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 767, .adv_w = 134, .box_w = 4, .box_h = 1, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 768, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 775, .adv_w = 134, .box_w = 5, .box_h = 5, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 779, .adv_w = 134, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 784, .adv_w = 134, .box_w = 5, .box_h = 3, .ofs_x = 2, .ofs_y = 3},
    {.bitmap_index = 786, .adv_w = 134, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 787, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 794, .adv_w = 134, .box_w = 5, .box_h = 1, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 795, .adv_w = 134, .box_w = 4, .box_h = 4, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 797, .adv_w = 134, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 804, .adv_w = 134, .box_w = 4, .box_h = 5, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 807, .adv_w = 134, .box_w = 4, .box_h = 5, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 810, .adv_w = 134, .box_w = 3, .box_h = 2, .ofs_x = 3, .ofs_y = 9},
    {.bitmap_index = 811, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 820, .adv_w = 134, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 827, .adv_w = 134, .box_w = 1, .box_h = 1, .ofs_x = 3, .ofs_y = 5},
    {.bitmap_index = 828, .adv_w = 134, .box_w = 2, .box_h = 3, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 829, .adv_w = 134, .box_w = 3, .box_h = 5, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 831, .adv_w = 134, .box_w = 5, .box_h = 5, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 835, .adv_w = 134, .box_w = 5, .box_h = 6, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 839, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 850, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 861, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 872, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 881, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 894, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 907, .adv_w = 134, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 921, .adv_w = 134, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 933, .adv_w = 134, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 945, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 958, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 969, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 982, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 993, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1004, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1015, .adv_w = 134, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1025, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1036, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1047, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1058, .adv_w = 134, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1068, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1079, .adv_w = 134, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1089, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1102, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1115, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1128, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1141, .adv_w = 134, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1153, .adv_w = 134, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 1159, .adv_w = 134, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1170, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1181, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1192, .adv_w = 134, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1203, .adv_w = 134, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1213, .adv_w = 134, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1225, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1234, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1243, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1252, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1261, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1270, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1279, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1288, .adv_w = 134, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1297, .adv_w = 134, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1305, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1315, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1325, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1335, .adv_w = 134, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1346, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1356, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1365, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1374, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1383, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1392, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1402, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1411, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1421, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1431, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1441, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1451, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1461, .adv_w = 134, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 1468, .adv_w = 134, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1477, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1486, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1495, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1504, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1513, .adv_w = 134, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1527, .adv_w = 134, .box_w = 6, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1539, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 0, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 160, .range_length = 96, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};

extern const lv_font_t lv_font_montserrat_14;


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_font_roboto_mono_14 = {
#else
lv_font_t lv_font_roboto_mono_14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .static_bitmap = 0,
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = &lv_font_montserrat_14,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_ROBOTO_MONO_14*/
