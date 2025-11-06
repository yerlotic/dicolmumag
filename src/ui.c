#ifndef UI_C
#define UI_C
#include <string.h>
#include "thirdparty/clay.h"
#include "colors.c"

#ifdef NO_SCALING
const float scale = 1.0;
#define S(x) (x)
#else
// idk why, but fonts look bad at exact 1.5 scale
#define APP_DEFAULT_SCALE 1.001
float app_scale = 1.001;
#define S(x) (int) ((x) * app_scale)
#endif // NO_SCALING

CLAY_PACKED_ENUM {
    FONT_ID_TITLE = 0,
    FONT_ID_SIDEBAR,
    FONT_ID_BUTTONS,
    FONT_ID_DOCUMENT,
    FONT_ID_WELCOME,
    FONTS_IDS,
};
const uint8_t title_font_size = 40;
const uint8_t sidebar_font_size = 23;
const uint8_t buttons_font_size = 20;
const uint8_t document_font_size = 30;
const uint8_t welcome_font_size = 80;

#define CLAY_DYNAMIC_STRING(string) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = strlen(string), .chars = (string) })
#define CLAY_SB_STRING(sb) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = (sb).count, .chars = (sb).items })
#define BUTTON_RADIUS CLAY_CORNER_RADIUS(S(10))
#define LAYOUT_RADIUS CLAY_CORNER_RADIUS(S(10))
#define JUST_TEXT_CONFIG(font_size, font_id, color) CLAY_TEXT_CONFIG({ .fontId = (font_id), .fontSize = S(font_size), .textColor = (color) })
#define SANE_TEXT_CONFIG(font_size, font_id) JUST_TEXT_CONFIG((font_size), (font_id), c10n(COLOR_TEXT))
#define DEFAULT_TEXT  SANE_TEXT_CONFIG(document_font_size, FONT_ID_DOCUMENT)
#define DISABLED_TEXT JUST_TEXT_CONFIG(document_font_size, FONT_ID_DOCUMENT, c10n(COLOR_OVERLAY0))
#define BUTTON_TEXT   SANE_TEXT_CONFIG(buttons_font_size,  FONT_ID_BUTTONS)

Clay_Padding defaultPadding;

static inline void HorizontalSeparator(void) {
    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }}}) {}
}

void RenderHeaderButton(Clay_String text) {
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
            .fontSize = S(buttons_font_size),
            .textColor = c10n(COLOR_TEXT),
            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
            .wrapMode = CLAY_TEXT_WRAP_NONE,
        }));
    }
}

#define RenderDropdownMenuItem(text, shortcut)                                 \
  do {                                                                         \
    CLAY({.backgroundColor = c10n(COLOR_SURFACE0),              \
          .cornerRadius = BUTTON_RADIUS,                                       \
          .id = CLAY_SID(text),                                                \
          .layout = {                                                          \
              .padding = CLAY_PADDING_ALL(S(16)),                              \
              .sizing = {.width = CLAY_SIZING_GROW(0),                         \
                         .height = CLAY_SIZING_GROW(0)},                       \
              .childGap = S(4),                                                \
              .childAlignment =                                                \
                  {                                                            \
                      .x = CLAY_ALIGN_X_LEFT,                                  \
                      .y = CLAY_ALIGN_Y_CENTER,                                \
                  },                                                           \
          }}) {                                                                \
      CLAY_TEXT(text, BUTTON_TEXT);                                            \
      HorizontalSeparator();                                                   \
      if (shortcut[0] != '\0') {                                               \
        CLAY_TEXT(CLAY_STRING(shortcut),                                       \
                  CLAY_TEXT_CONFIG({.fontId = FONT_ID_BUTTONS,                 \
                                    .fontSize = S(buttons_font_size),           \
                                    .textColor = c10n(COLOR_OVERLAY0)}));      \
      }                                                                        \
    }                                                                          \
  } while (0)

static inline void RenderNumberPicker(char* value) {
    CLAY({
        .layout = { .padding = defaultPadding},
        .backgroundColor = BUTTON_COLOR,
        // .id = CLAY_SID(name),
        .cornerRadius = BUTTON_RADIUS,
    }) {
        CLAY_TEXT(CLAY_DYNAMIC_STRING(value), BUTTON_TEXT);
    };
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
                .fontId = FONT_ID_DOCUMENT,
                .fontSize = S(document_font_size),
                .textColor = color,
            }));
            CLAY_TEXT(CLAY_STRING(":"), DEFAULT_TEXT);
        }
        RenderNumberPicker(value);
    };
}

static inline void RenderColor(Clay_Color color) {
    uint8_t border_width = 5;
    uint8_t radius = 5;
    uint8_t size = 50;
    CLAY({
        .cornerRadius = CLAY_CORNER_RADIUS((float)radius * app_scale),
        .backgroundColor = TERNARY_COLOR(Clay_Hovered(), c10n(COLOR_SURFACE1), c10n(COLOR_SURFACE0)),
        .layout = {
            .sizing = {
                .height = CLAY_SIZING_FIXED(S(size)),
                .width =  CLAY_SIZING_FIXED(S(size))
            },
            .padding = CLAY_PADDING_ALL(S(border_width)),
        },
    }) {
        CLAY({
            .backgroundColor = color,
            .cornerRadius = CLAY_CORNER_RADIUS(S(radius)),
            .layout = {
                .sizing = {
                    .height = CLAY_SIZING_GROW(0),
                    .width =  CLAY_SIZING_GROW(0)
                },
            },
        });
    };
}

static inline void StatedText(Clay_String text, bool enabled) {
    if (enabled)
        CLAY_TEXT(text, DEFAULT_TEXT);
    else
        CLAY_TEXT(text, DISABLED_TEXT);
}

#endif // UI_C
