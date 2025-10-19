#ifndef UI_C
#define UI_C
#include <string.h>
#include "thirdparty/clay.h"
#include "colors.c"

#ifdef NO_SCALING
const float scale = 1.0;
#define S(x) (x)
#else
float scale = 1.0;
#define S(x) ((int) (x) * scale)
#endif // NO_SCALING

const uint8_t FONT_ID_BODY_16 = 0;
const uint8_t FONT_ID_SIDEBAR = 1;
const uint8_t FONT_ID_BUTTONS = 2;
const uint8_t FONT_ID_DOCUMNT = 3;
const uint8_t FONT_ID_WELCOME = 4;
const uint8_t FONTS_IDS = 5;
const uint8_t FONT_LOAD_SIZE = 40;
const uint8_t button_font_size = 20;
const uint8_t sidebar_font_size = 23;
const uint8_t document_font_size = 30;
const uint8_t title_font_size = 40;
const uint8_t welcome_font_size = 80;

#define CLAY_DYNAMIC_STRING(string) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = strlen(string), .chars = (string) })
#define CLAY_SB_STRING(sb) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = (sb).count, .chars = (sb).items })
#define BUTTON_RADIUS CLAY_CORNER_RADIUS(S(10))
#define LAYOUT_RADIUS CLAY_CORNER_RADIUS(S(10))
#define JUST_TEXT_CONFIG(font_size, font_id, color) CLAY_TEXT_CONFIG({ .fontId = (font_id), .fontSize = S(font_size), .textColor = (color) })
#define SANE_TEXT_CONFIG(font_size, font_id) JUST_TEXT_CONFIG((font_size), (font_id), c10n(COLOR_TEXT))
#define DEFAULT_TEXT  SANE_TEXT_CONFIG(document_font_size, FONT_ID_DOCUMNT)
#define DISABLED_TEXT JUST_TEXT_CONFIG(document_font_size, FONT_ID_DOCUMNT, c10n(COLOR_OVERLAY0))
#define BUTTON_TEXT   SANE_TEXT_CONFIG(button_font_size,   FONT_ID_BUTTONS)

Clay_Padding defaultPadding;

static inline void RenderHeaderButton(Clay_String text) {
    CLAY({
        .layout = {
            .padding = defaultPadding,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
        .backgroundColor = BUTTON_COLOR,
        .id = CLAY_SID(text),
        .cornerRadius = BUTTON_RADIUS,
    }) {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BUTTONS,
            .fontSize = S(button_font_size),
            .textColor = colors[colorscheme][COLOR_TEXT],
            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
            .wrapMode = CLAY_TEXT_WRAP_NONE,
        }));
    }
}

static inline void RenderDropdownMenuItem(Clay_String text) {
    CLAY({
        .backgroundColor = colors[colorscheme][COLOR_SURFACE0],
        .cornerRadius = BUTTON_RADIUS,
        .id = CLAY_SID(text),
        .layout = {
            .padding = CLAY_PADDING_ALL(S(16)),
            .sizing = {
                .width = CLAY_SIZING_GROW(0)
            },
    }}) {
        CLAY_TEXT(text, BUTTON_TEXT);
    }
}

static inline void RenderNumberPicker(char* value) {
    CLAY({
        .layout = { .padding = defaultPadding},
        .backgroundColor = BUTTON_COLOR,
        // .id = CLAY_SID(name),
        .cornerRadius = BUTTON_RADIUS,
    }) {
        CLAY_TEXT(CLAY_DYNAMIC_STRING(value), BUTTON_TEXT);
    }
}

static inline void RenderColorChannel(Clay_String text, Clay_Color color, char *value) {
    CLAY({
        .layout = {
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
        .id = CLAY_SID(text),
    }) {
        CLAY({ .layout = { .sizing = { .width = S(25) }}}) {
            CLAY_TEXT(text, CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_BODY_16,
                .fontSize = S(document_font_size),
                .textColor = color,
            }));
            CLAY_TEXT(CLAY_STRING(":"), DEFAULT_TEXT);
        }
        RenderNumberPicker(value);
    }
}

static inline void RenderColor(Clay_Color color) {
    uint8_t border_width = 5;
    uint8_t radius = 5;
    uint8_t size = 50;
    CLAY({
        .cornerRadius = CLAY_CORNER_RADIUS(S(radius)),
        .backgroundColor = color,
        .border = {
            .color = TERNARY_COLOR(Clay_Hovered(), colors[colorscheme][COLOR_SURFACE1], colors[colorscheme][COLOR_SURFACE0]),
            .width = CLAY_BORDER_OUTSIDE(S(border_width))
        },
        .layout = {
            .sizing = {
                .height = CLAY_SIZING_FIXED(S(size)),
                .width = CLAY_SIZING_FIXED(S(size))
            },
        },
    }) {}
}

static inline void StatedText(Clay_String text, bool enabled) {
    if (enabled)
        CLAY_TEXT(text, DEFAULT_TEXT);
    else
        CLAY_TEXT(text, DISABLED_TEXT);
}

static inline void HorizontalSeparator(void) {
    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }}}) {}
}

#endif // UI_C
