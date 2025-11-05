#ifndef COLORS_C
#define COLORS_C
#include "thirdparty/clay.h"

int8_t app_colorscheme = 1;

CLAY_PACKED_ENUM {
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

#define APP_COLORSCHEMES 5
#define APP_COLORSCHEME_CAPACITY 26
Clay_Color colors[APP_COLORSCHEMES][APP_COLORSCHEME_CAPACITY] = {
{   // Latte
    [COLOR_ROSEWATER] = {220, 138, 120, 255},  // #dc8a78
    [COLOR_FLAMINGO]  = {221, 120, 120, 255},  // #dd7878
    [COLOR_PINK]      = {234, 118, 203, 255},  // #ea76cb
    [COLOR_MAUVE]     = {136, 57,  239, 255},  // #8839ef
    [COLOR_RED]       = {210, 15,  57,  255},  // #d20f39
    [COLOR_MAROON]    = {230, 69,  83,  255},  // #e64553
    [COLOR_PEACH]     = {254, 100, 11,  255},  // #fe640b
    [COLOR_YELLOW]    = {223, 142, 29,  255},  // #df8e1d
    [COLOR_GREEN]     = {64,  160, 43,  255},  // #40a02b
    [COLOR_TEAL]      = {23,  146, 153, 255},  // #179299
    [COLOR_SKY]       = {4,   165, 229, 255},  // #04a5e5
    [COLOR_SAPPHIRE]  = {32,  159, 181, 255},  // #209fb5
    [COLOR_BLUE]      = {30,  102, 245, 255},  // #1e66f5
    [COLOR_LAVENDER]  = {114, 135, 253, 255},  // #7287fd
    [COLOR_TEXT]      = {76,  79,  105, 255},  // #4c4f69
    [COLOR_SUBTEXT1]  = {92,  95,  119, 255},  // #5c5f77
    [COLOR_SUBTEXT0]  = {108, 111, 133, 255},  // #6c6f85
    [COLOR_OVERLAY2]  = {124, 127, 147, 255},  // #7c7f93
    [COLOR_OVERLAY1]  = {140, 143, 161, 255},  // #8c8fa1
    [COLOR_OVERLAY0]  = {156, 160, 176, 255},  // #9ca0b0
    [COLOR_SURFACE2]  = {172, 176, 190, 255},  // #acb0be
    [COLOR_SURFACE1]  = {188, 192, 204, 255},  // #bcc0cc
    [COLOR_SURFACE0]  = {204, 208, 218, 255},  // #ccd0da
    [COLOR_BASE]      = {239, 241, 245, 255},  // #eff1f5
    [COLOR_MANTLE]    = {230, 233, 239, 255},  // #e6e9ef
    [COLOR_CRUST]     = {220, 224, 232, 255},  // #dce0e8
},
{   // Frappe
    [COLOR_ROSEWATER] = {242, 213, 207, 255},  // #f2d5cf
    [COLOR_FLAMINGO]  = {238, 190, 190, 255},  // #eebebe
    [COLOR_PINK]      = {244, 184, 228, 255},  // #f4b8e4
    [COLOR_MAUVE]     = {202, 158, 230, 255},  // #ca9ee6
    [COLOR_RED]       = {231, 130, 132, 255},  // #e78284
    [COLOR_MAROON]    = {234, 153, 156, 255},  // #ea999c
    [COLOR_PEACH]     = {239, 159, 118, 255},  // #ef9f76
    [COLOR_YELLOW]    = {229, 200, 144, 255},  // #e5c890
    [COLOR_GREEN]     = {166, 209, 137, 255},  // #a6d189
    [COLOR_TEAL]      = {129, 200, 190, 255},  // #81c8be
    [COLOR_SKY]       = {153, 209, 219, 255},  // #99d1db
    [COLOR_SAPPHIRE]  = {133, 193, 220, 255},  // #85c1dc
    [COLOR_BLUE]      = {140, 170, 238, 255},  // #8caaee
    [COLOR_LAVENDER]  = {186, 187, 241, 255},  // #babbf1
    [COLOR_TEXT]      = {198, 208, 245, 255},  // #c6d0f5
    [COLOR_SUBTEXT1]  = {181, 191, 226, 255},  // #b5bfe2
    [COLOR_SUBTEXT0]  = {165, 173, 206, 255},  // #a5adce
    [COLOR_OVERLAY2]  = {148, 156, 187, 255},  // #949cbb
    [COLOR_OVERLAY1]  = {131, 139, 167, 255},  // #838ba7
    [COLOR_OVERLAY0]  = {115, 121, 148, 255},  // #737994
    [COLOR_SURFACE2]  = {98,  104, 128, 255},  // #626880
    [COLOR_SURFACE1]  = {81,  87,  109, 255},  // #51576d
    [COLOR_SURFACE0]  = {65,  69,  89,  255},  // #414559
    [COLOR_BASE]      = {48,  52,  70,  255},  // #303446
    [COLOR_MANTLE]    = {41,  44,  60,  255},  // #292c3c
    [COLOR_CRUST]     = {35,  38,  52,  255},  // #232634
},
{   // Macchiato
    [COLOR_ROSEWATER] = {244, 219, 214, 255},  // #f4dbd6
    [COLOR_FLAMINGO]  = {240, 198, 198, 255},  // #f0c6c6
    [COLOR_PINK]      = {245, 189, 230, 255},  // #f5bde6
    [COLOR_MAUVE]     = {198, 160, 246, 255},  // #c6a0f6
    [COLOR_RED]       = {237, 135, 150, 255},  // #ed8796
    [COLOR_MAROON]    = {238, 153, 160, 255},  // #ee99a0
    [COLOR_PEACH]     = {245, 169, 127, 255},  // #f5a97f
    [COLOR_YELLOW]    = {238, 212, 159, 255},  // #eed49f
    [COLOR_GREEN]     = {166, 218, 149, 255},  // #a6da95
    [COLOR_TEAL]      = {139, 213, 202, 255},  // #8bd5ca
    [COLOR_SKY]       = {145, 215, 227, 255},  // #91d7e3
    [COLOR_SAPPHIRE]  = {125, 196, 228, 255},  // #7dc4e4
    [COLOR_BLUE]      = {138, 173, 244, 255},  // #8aadf4
    [COLOR_LAVENDER]  = {183, 189, 248, 255},  // #b7bdf8
    [COLOR_TEXT]      = {202, 211, 245, 255},  // #cad3f5
    [COLOR_SUBTEXT1]  = {184, 192, 224, 255},  // #b8c0e0
    [COLOR_SUBTEXT0]  = {165, 173, 203, 255},  // #a5adcb
    [COLOR_OVERLAY2]  = {147, 154, 183, 255},  // #939ab7
    [COLOR_OVERLAY1]  = {128, 135, 162, 255},  // #8087a2
    [COLOR_OVERLAY0]  = {110, 115, 141, 255},  // #6e738d
    [COLOR_SURFACE2]  = {91,  96,  120, 255},  // #5b6078
    [COLOR_SURFACE1]  = {73,  77,  100, 255},  // #494d64
    [COLOR_SURFACE0]  = {54,  58,  79,  255},  // #363a4f
    [COLOR_BASE]      = {36,  39,  58,  255},  // #24273a
    [COLOR_MANTLE]    = {30,  32,  48,  255},  // #1e2030
    [COLOR_CRUST]     = {24,  25,  38,  255},  // #181926
},
{   // Mocha
    [COLOR_ROSEWATER] = {245, 224, 220, 255},  // #f5e0dc
    [COLOR_FLAMINGO]  = {242, 205, 205, 255},  // #f2cdcd
    [COLOR_PINK]      = {245, 194, 231, 255},  // #f5c2e7
    [COLOR_MAUVE]     = {203, 166, 247, 255},  // #cba6f7
    [COLOR_RED]       = {243, 139, 168, 255},  // #f38ba8
    [COLOR_MAROON]    = {235, 160, 172, 255},  // #eba0ac
    [COLOR_PEACH]     = {250, 179, 135, 255},  // #fab387
    [COLOR_YELLOW]    = {249, 226, 175, 255},  // #f9e2af
    [COLOR_GREEN]     = {166, 227, 161, 255},  // #a6e3a1
    [COLOR_TEAL]      = {148, 226, 213, 255},  // #94e2d5
    [COLOR_SKY]       = {137, 220, 235, 255},  // #89dceb
    [COLOR_SAPPHIRE]  = {116, 199, 236, 255},  // #74c7ec
    [COLOR_BLUE]      = {137, 180, 250, 255},  // #89b4fa
    [COLOR_LAVENDER]  = {180, 190, 254, 255},  // #b4befe
    [COLOR_TEXT]      = {205, 214, 244, 255},  // #cdd6f4
    [COLOR_SUBTEXT1]  = {186, 194, 222, 255},  // #bac2de
    [COLOR_SUBTEXT0]  = {166, 173, 200, 255},  // #a6adc8
    [COLOR_OVERLAY2]  = {147, 153, 178, 255},  // #9399b2
    [COLOR_OVERLAY1]  = {127, 132, 156, 255},  // #7f849c
    [COLOR_OVERLAY0]  = {108, 112, 134, 255},  // #6c7086
    [COLOR_SURFACE2]  = {88,  91,  112, 255},  // #585b70
    [COLOR_SURFACE1]  = {69,  71,  90,  255},  // #45475a
    [COLOR_SURFACE0]  = {49,  50,  68,  255},  // #313244
    [COLOR_BASE]      = {30,  30,  46,  255},  // #1e1e2e
    [COLOR_MANTLE]    = {24,  24,  37,  255},  // #181825
    [COLOR_CRUST]     = {17,  17,  27,  255},  // #11111b
},
{   // Breeze Light
    [COLOR_ROSEWATER] = {220, 138, 120, 255},  // #??????
    [COLOR_FLAMINGO]  = {221, 120, 120, 255},  // #??????
    [COLOR_PINK]      = {234, 118, 203, 255},  // #??????
    [COLOR_MAUVE]     = {155, 89,  182, 255},  // #9b59b6
    [COLOR_RED]       = {218, 68,  83,  255},  // #da4453
    [COLOR_MAROON]    = {230, 69,  83,  255},  // #??????
    [COLOR_PEACH]     = {246, 116, 0,   255},  // #f67400
    [COLOR_YELLOW]    = {223, 142, 29,  255},  // #??????
    [COLOR_GREEN]     = {39,  174, 96,  255},  // #27ae60
    [COLOR_TEAL]      = {23,  146, 153, 255},  // #??????
    [COLOR_SKY]       = {61,  174, 233, 255},  // #3daee9
    [COLOR_SAPPHIRE]  = {32,  159, 181, 255},  // #??????
    [COLOR_BLUE]      = {41,  128, 185, 255},  // #2980b9
    [COLOR_LAVENDER]  = {114, 135, 253, 255},  // #??????
    [COLOR_TEXT]      = {0,   0,   0,   255},  // #000000
    [COLOR_SUBTEXT1]  = {92,  95,  119, 255},  // #??????
    [COLOR_SUBTEXT0]  = {108, 111, 133, 255},  // #??????
    [COLOR_OVERLAY2]  = {124, 127, 147, 255},  // #??????
    [COLOR_OVERLAY1]  = {140, 143, 161, 255},  // #??????
    [COLOR_OVERLAY0]  = {156, 160, 176, 255},  // #??????
    [COLOR_SURFACE2]  = {172, 176, 190, 255},  // #??????
    [COLOR_SURFACE1]  = {188, 192, 204, 255},  // #??????
    [COLOR_SURFACE0]  = {204, 208, 218, 255},  // #eff0f1
    [COLOR_BASE]      = {255, 255, 255, 255},  // #ffffff
    [COLOR_MANTLE]    = {230, 233, 239, 255},  // #??????
    [COLOR_CRUST]     = {220, 224, 232, 255},  // #??????
},
};

#define TERNARY_COLOR(condition, first, second) {\
        condition ? ((first).r) : ((second).r), \
        condition ? ((first).g) : ((second).g), \
        condition ? ((first).b) : ((second).b), \
        condition ? ((first).a) : ((second).a), \
}
#define OPACITY(color, opacity) (Clay_Color) {(color).r, (color).g, (color).b, (opacity)*255.0/100.0}

#define ACTIVE_PERCENT 20

// Colorization
#define c10n(COLOR) colors[app_colorscheme][COLOR]

#define BUTTON_COLOR TERNARY_COLOR(Clay_Hovered(), OPACITY(c10n(COLOR_SURFACE1), ACTIVE_PERCENT), OPACITY(c10n(COLOR_SURFACE1), 15))
#define TOGGLE_COLOR TERNARY_COLOR(Clay_Hovered(), OPACITY(c10n(COLOR_GREEN),    ACTIVE_PERCENT), OPACITY(c10n(COLOR_SURFACE1), 15))
#define COLOR_ACTIVE OPACITY(c10n(COLOR_OVERLAY0), ACTIVE_PERCENT)

#endif // COLORS_C
