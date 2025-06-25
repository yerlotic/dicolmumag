#include "clay.h"

// Catppuccin
#ifdef LATTE
    Clay_Color COLOR_ROSEWATER = {220, 138, 120, 255};  // #dc8a78
    Clay_Color COLOR_FLAMINGO  = {221, 120, 120, 255};  // #dd7878
    Clay_Color COLOR_PINK      = {234, 118, 203, 255};  // #ea76cb
    Clay_Color COLOR_MAUVE     = {136, 57,  239, 255};  // #8839ef
    Clay_Color COLOR_RED       = {210, 15,  57,  255};  // #d20f39
    Clay_Color COLOR_MAROON    = {230, 69,  83,  255};  // #e64553
    Clay_Color COLOR_PEACH     = {254, 100, 11,  255};  // #fe640b
    Clay_Color COLOR_YELLOW    = {223, 142, 29,  255};  // #df8e1d
    Clay_Color COLOR_GREEN     = {64,  160, 43,  255};  // #40a02b
    Clay_Color COLOR_TEAL      = {23,  146, 153, 255};  // #179299
    Clay_Color COLOR_SKY       = {4,   165, 229, 255};  // #04a5e5
    Clay_Color COLOR_SAPPHIRE  = {32,  159, 181, 255};  // #209fb5
    Clay_Color COLOR_BLUE      = {30,  102, 245, 255};  // #1e66f5
    Clay_Color COLOR_LAVENDER  = {114, 135, 253, 255};  // #7287fd
    Clay_Color COLOR_TEXT      = {76,  79,  105, 255};  // #4c4f69
    Clay_Color COLOR_SUBTEXT1  = {92,  95,  119, 255};  // #5c5f77
    Clay_Color COLOR_SUBTEXT0  = {108, 111, 133, 255};  // #6c6f85
    Clay_Color COLOR_OVERLAY2  = {124, 127, 147, 255};  // #7c7f93
    Clay_Color COLOR_OVERLAY1  = {140, 143, 161, 255};  // #8c8fa1
    Clay_Color COLOR_OVERLAY0  = {156, 160, 176, 255};  // #9ca0b0
    Clay_Color COLOR_SURFACE2  = {172, 176, 190, 255};  // #acb0be
    Clay_Color COLOR_SURFACE1  = {188, 192, 204, 255};  // #bcc0cc
    Clay_Color COLOR_SURFACE0  = {204, 208, 218, 255};  // #ccd0da
    Clay_Color COLOR_BASE      = {239, 241, 245, 255};  // #eff1f5
    Clay_Color COLOR_MANTLE    = {230, 233, 239, 255};  // #e6e9ef
    Clay_Color COLOR_CRUST     = {220, 224, 232, 255};  // #dce0e8
#else
    #define FRAPPE
    Clay_Color COLOR_ROSEWATER = {242, 213, 207, 255};  // #f2d5cf
    Clay_Color COLOR_FLAMINGO  = {238, 190, 190, 255};  // #eebebe
    Clay_Color COLOR_PINK      = {244, 184, 228, 255};  // #f4b8e4
    Clay_Color COLOR_MAUVE     = {202, 158, 230, 255};  // #ca9ee6
    Clay_Color COLOR_RED       = {231, 130, 132, 255};  // #e78284
    Clay_Color COLOR_MAROON    = {234, 153, 156, 255};  // #ea999c
    Clay_Color COLOR_PEACH     = {239, 159, 118, 255};  // #ef9f76
    Clay_Color COLOR_YELLOW    = {229, 200, 144, 255};  // #e5c890
    Clay_Color COLOR_GREEN     = {166, 209, 137, 255};  // #a6d189
    Clay_Color COLOR_TEAL      = {129, 200, 190, 255};  // #81c8be
    Clay_Color COLOR_SKY       = {153, 209, 219, 255};  // #99d1db
    Clay_Color COLOR_SAPPHIRE  = {133, 193, 220, 255};  // #85c1dc
    Clay_Color COLOR_BLUE      = {140, 170, 238, 255};  // #8caaee
    Clay_Color COLOR_LAVENDER  = {186, 187, 241, 255};  // #babbf1
    Clay_Color COLOR_TEXT      = {198, 208, 245, 255};  // #c6d0f5
    Clay_Color COLOR_SUBTEXT1  = {181, 191, 226, 255};  // #b5bfe2
    Clay_Color COLOR_SUBTEXT0  = {165, 173, 206, 255};  // #a5adce
    Clay_Color COLOR_OVERLAY2  = {148, 156, 187, 255};  // #949cbb
    Clay_Color COLOR_OVERLAY1  = {131, 139, 167, 255};  // #838ba7
    Clay_Color COLOR_OVERLAY0  = {115, 121, 148, 255};  // #737994
    Clay_Color COLOR_SURFACE2  = {98,  104, 128, 255};  // #626880
    Clay_Color COLOR_SURFACE1  = {81,  87,  109, 255};  // #51576d
    Clay_Color COLOR_SURFACE0  = {65,  69,  89,  255};  // #414559
    Clay_Color COLOR_BASE      = {48,  52,  70,  255};  // #303446
    Clay_Color COLOR_MANTLE    = {41,  44,  60,  255};  // #292c3c
    Clay_Color COLOR_CRUST     = {35,  38,  52,  255};  // #232634
#endif // LATTE

#define TERNARY_COLOR(condition, first, second) {\
        condition ? ((first).r) : ((second).r), \
        condition ? ((first).g) : ((second).g), \
        condition ? ((first).b) : ((second).b), \
        condition ? ((first).a) : ((second).a), \
}
#define OPACITY(color, opacity) (Clay_Color) {(color).r, (color).g, (color).b, (opacity)*255.0/100.0}

#define ACTIVE_PERCENT 20

#define BUTTON_COLOR TERNARY_COLOR(Clay_Hovered(), OPACITY(COLOR_SURFACE1, ACTIVE_PERCENT), OPACITY(COLOR_SURFACE1, 15))
#define TOGGLE_COLOR TERNARY_COLOR(Clay_Hovered(), OPACITY(COLOR_GREEN,    ACTIVE_PERCENT), OPACITY(COLOR_SURFACE1, 15))
#define COLOR_ACTIVE OPACITY(COLOR_OVERLAY0, ACTIVE_PERCENT)

