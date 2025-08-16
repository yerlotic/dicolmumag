#include "clay.h"

int8_t colorscheme = 1;

enum {
    COLOR_ROSEWATER,
    COLOR_FLAMINGO,
    COLOR_PINK,
    COLOR_MAUVE,
    COLOR_RED,
    COLOR_MAROON,
    COLOR_PEACH,
    COLOR_YELLOW,
    COLOR_GREEN,
    COLOR_TEAL,
    COLOR_SKY,
    COLOR_SAPPHIRE,
    COLOR_BLUE,
    COLOR_LAVENDER,
    COLOR_TEXT,
    COLOR_SUBTEXT1,
    COLOR_SUBTEXT0,
    COLOR_OVERLAY2,
    COLOR_OVERLAY1,
    COLOR_OVERLAY0,
    COLOR_SURFACE2,
    COLOR_SURFACE1,
    COLOR_SURFACE0,
    COLOR_BASE,
    COLOR_MANTLE,
    COLOR_CRUST,
};

#define COLORSCHEMES 2
#define COLORSCHEME_CAPACITY 26
Clay_Color colors[COLORSCHEMES][COLORSCHEME_CAPACITY] = {
{
    {220, 138, 120, 255},  // #dc8a78 - COLOR_ROSEWATER
    {221, 120, 120, 255},  // #dd7878 - COLOR_FLAMINGO
    {234, 118, 203, 255},  // #ea76cb - COLOR_PINK
    {136, 57,  239, 255},  // #8839ef - COLOR_MAUVE
    {210, 15,  57,  255},  // #d20f39 - COLOR_RED
    {230, 69,  83,  255},  // #e64553 - COLOR_MAROON
    {254, 100, 11,  255},  // #fe640b - COLOR_PEACH
    {223, 142, 29,  255},  // #df8e1d - COLOR_YELLOW
    {64,  160, 43,  255},  // #40a02b - COLOR_GREEN
    {23,  146, 153, 255},  // #179299 - COLOR_TEAL
    {4,   165, 229, 255},  // #04a5e5 - COLOR_SKY
    {32,  159, 181, 255},  // #209fb5 - COLOR_SAPPHIRE
    {30,  102, 245, 255},  // #1e66f5 - COLOR_BLUE
    {114, 135, 253, 255},  // #7287fd - COLOR_LAVENDER
    {76,  79,  105, 255},  // #4c4f69 - COLOR_TEXT
    {92,  95,  119, 255},  // #5c5f77 - COLOR_SUBTEXT1
    {108, 111, 133, 255},  // #6c6f85 - COLOR_SUBTEXT0
    {124, 127, 147, 255},  // #7c7f93 - COLOR_OVERLAY2
    {140, 143, 161, 255},  // #8c8fa1 - COLOR_OVERLAY1
    {156, 160, 176, 255},  // #9ca0b0 - COLOR_OVERLAY0
    {172, 176, 190, 255},  // #acb0be - COLOR_SURFACE2
    {188, 192, 204, 255},  // #bcc0cc - COLOR_SURFACE1
    {204, 208, 218, 255},  // #ccd0da - COLOR_SURFACE0
    {239, 241, 245, 255},  // #eff1f5 - COLOR_BASE
    {230, 233, 239, 255},  // #e6e9ef - COLOR_MANTLE
    {220, 224, 232, 255},  // #dce0e8 - COLOR_CRUST
},
{
    {242, 213, 207, 255},  // #f2d5cf - COLOR_ROSEWATER
    {238, 190, 190, 255},  // #eebebe - COLOR_FLAMINGO
    {244, 184, 228, 255},  // #f4b8e4 - COLOR_PINK
    {202, 158, 230, 255},  // #ca9ee6 - COLOR_MAUVE
    {231, 130, 132, 255},  // #e78284 - COLOR_RED
    {234, 153, 156, 255},  // #ea999c - COLOR_MAROON
    {239, 159, 118, 255},  // #ef9f76 - COLOR_PEACH
    {229, 200, 144, 255},  // #e5c890 - COLOR_YELLOW
    {166, 209, 137, 255},  // #a6d189 - COLOR_GREEN
    {129, 200, 190, 255},  // #81c8be - COLOR_TEAL
    {153, 209, 219, 255},  // #99d1db - COLOR_SKY
    {133, 193, 220, 255},  // #85c1dc - COLOR_SAPPHIRE
    {140, 170, 238, 255},  // #8caaee - COLOR_BLUE
    {186, 187, 241, 255},  // #babbf1 - COLOR_LAVENDER
    {198, 208, 245, 255},  // #c6d0f5 - COLOR_TEXT
    {181, 191, 226, 255},  // #b5bfe2 - COLOR_SUBTEXT1
    {165, 173, 206, 255},  // #a5adce - COLOR_SUBTEXT0
    {148, 156, 187, 255},  // #949cbb - COLOR_OVERLAY2
    {131, 139, 167, 255},  // #838ba7 - COLOR_OVERLAY1
    {115, 121, 148, 255},  // #737994 - COLOR_OVERLAY0
    {98,  104, 128, 255},  // #626880 - COLOR_SURFACE2
    {81,  87,  109, 255},  // #51576d - COLOR_SURFACE1
    {65,  69,  89,  255},  // #414559 - COLOR_SURFACE0
    {48,  52,  70,  255},  // #303446 - COLOR_BASE
    {41,  44,  60,  255},  // #292c3c - COLOR_MANTLE
    {35,  38,  52,  255},  // #232634 - COLOR_CRUST
}
};

#define TERNARY_COLOR(condition, first, second) {\
        condition ? ((first).r) : ((second).r), \
        condition ? ((first).g) : ((second).g), \
        condition ? ((first).b) : ((second).b), \
        condition ? ((first).a) : ((second).a), \
}
#define OPACITY(color, opacity) (Clay_Color) {(color).r, (color).g, (color).b, (opacity)*255.0/100.0}

#define ACTIVE_PERCENT 20

#define BUTTON_COLOR TERNARY_COLOR(Clay_Hovered(), OPACITY(colors[colorscheme][COLOR_SURFACE1], ACTIVE_PERCENT), OPACITY(colors[colorscheme][COLOR_SURFACE1], 15))
#define TOGGLE_COLOR TERNARY_COLOR(Clay_Hovered(), OPACITY(colors[colorscheme][COLOR_GREEN],    ACTIVE_PERCENT), OPACITY(colors[colorscheme][COLOR_SURFACE1], 15))
#define COLOR_ACTIVE OPACITY(colors[colorscheme][COLOR_OVERLAY0], ACTIVE_PERCENT)

