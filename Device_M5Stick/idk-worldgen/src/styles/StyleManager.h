#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace idk {

struct Style {
    const char* name;
    const char* glyphs;
    bool useColor;
};

static const Style kStyles[] = {
    // Basic & Detailed
    {"Detailed", " .:-=+*#%@", true},
    {"Standard", " .oO@", true},
    {"Minimal", " .:", true},
    {"Blocks", " \xB0\xB1\xB2\xDB", true},
    {"Dots", " .·°", true},
    {"Maximal", " .'`^ \",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczMW&8%B@", true},
    {"Extended", " .:-=+*#%@&8BMW", true},
    {"Dense", "@@@@@@@@", true},
    {"High Contrast", " #", true},
    {"Artistic", " .~*#", true},
    {"Terminal", " >_", true},

    // Gradients
    {"Gradient 9", " .:-=+*#%@", true},
    {"Gradient 70", " .'`^ \",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczMW&8%B@", true},

    // Symbols & Data
    {"Numbers", "0123456789", true},
    {"Letters", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", true},
    {"Binary", "01", true},
    {"Hex", "0123456789ABCDEF", true},
    {"Shadows", " \xB0\xB1\xB2", true},

    // Retro & Gaming
    {"Retro", " \xFE\xFD\xFC\xFB", true},
    {"Binary Blocks", "\xDB ", true},
    {"Retro Gaming", "\x01\x02\x03\x04\x05\x06", true},
    {"Texture", "\xB0\xB1\xB2\xB3", true},
    {"Teletext", " \xA1\xA2\xA3\xA4", true},
    {"Industrial", "#%&+", true},

    // Specialized Blocks
    {"Block HD", " \xDB\xB2\xB1\xB0", true},
    {"Braille", ".:!|", true}, // Limited to ASCII for speed
    {"Braille HD", "\x11\x12\x13\x14", true},
    {"Dots Braille", "\x07\x08\x09", true},
    {"Gradient Smooth", " .:-=+*", true},

    // Geometric
    {"Geometric", "+-x|", true},
    {"Geometric Full", "\x01\x02\x03\x04\x05\x06", true},
    {"Shapes", "\x0F\x10\x11\x12", true},
    {"Triangles", "\x1E\x1F", true},
    {"Circles", "oO", true},
    {"Linus", "---", true},
    {"Double Lines", "\xCD\xBA\xC8\xBC\xC9\xBB", true},
    {"Box Drawing Full", "\xDB\xB2\xB1\xB0", true},
    {"Box Drawing Heavy", "\x13\x14\x15", true},

    // Thematic
    {"Cyberpunk", "01#!?", true},
    {"Matrix", "01\x10\x11", true},
    {"Psychedelic", "@#$%&", true},
    {"Art Deco", "|||||", true},
    {"Gothic", "+++++", true},
    {"Street Art", "####", true},
    {"Electronic", "0101", true},
    {"Portrait", "....", true},
    {"Landscape", "====", true},
    {"Architecture", "||||", true},

    // Nature & Seasons
    {"Forest", "fFtT", true},
    {"Winter", "* .·", true},
    {"Flame", "vVwW", true},
    {"Zen", " - ", true},
    {"Calm", " ~ ", true},
    {"Energy", "! ! !", true},

    // Cultural/Script
    {"Nordic", "I I I", true},
    {"Celtic", "O O O", true},
    {"Greek", "a b g", true},
    {"Cyrillic", "\xA0\xA1\xA2", true},
    {"Roman", "I V X L", true},

    // Abstract
    {"Stars", "* * *", true},
    {"Arrows", "< > ^ v", true},
    {"Math", "+ - / * = %", true},
    {"Dashes", "- - -", true},
    {"Alchemical", "o + x", true}
};

static const int kStyleCount = sizeof(kStyles) / sizeof(kStyles[0]);

} // namespace idk
