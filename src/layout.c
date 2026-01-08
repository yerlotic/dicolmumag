#include <stdint.h>
#include <stdlib.h>

#include "thirdparty/cthreads.h"
#include "thirdparty/raylib/raylib.h"
#include "thirdparty/tinyfiledialogs.h"
#include "thirdparty/clay.h"
#define NOB_IMPLEMENTATION
#include "thirdparty/nob.h"
#include "strings.c"
#include "ui.c"

#define MAGICK_RESIZE_IGNORE_RATIO    "!"
#define MAGICK_RESIZE_SHRINK_LARGER   ">"
#define MAGICK_RESIZE_ENLARGE_SMALLER "<"
#define MAGICK_RESIZE_FILL_AREA       "^"
#define MAGICK_RESIZE_PERCENTAGE      "%"
#define MAGICK_RESIZE_PIXEL_LIMIT     "@"

typedef struct {
    Clay_String title;
    Clay_String contents;
} Document;

typedef struct {
    Document *documents;
    uint8_t length;
} DocumentArray;

#define DOCUMENTS_LEN 6

Document documentsRaw[DOCUMENTS_LEN];

DocumentArray documents = {
    .length = DOCUMENTS_LEN,
    .documents = documentsRaw
};

typedef struct {
    intptr_t offset;
    intptr_t memory;
} AppArena;

typedef CLAY_PACKED_ENUM {
    MAGICK_BEST_FIT        = 1 << 0,
    MAGICK_TRANSPARENT_BG  = 1 << 1,
    MAGICK_OPEN_ON_DONE    = 1 << 2,
    MAGICK_RESIZE          = 1 << 3,
    MAGICK_IGNORE_RATIO    = 1 << 4,
    MAGICK_SHRINK_LARGER   = 1 << 5,
    MAGICK_ENLARGE_SMALLER = 1 << 6,
    MAGICK_FILL_AREA       = 1 << 7,
    MAGICK_PERCENTAGE      = 1 << 8,
    MAGICK_PIXEL_LIMIT     = 1 << 9,
    MAGICK_SET_RESOLUTION  = 1 << 10,
    MAGICK_GRAVITY         = 1 << 11,
} MagickState;

CLAY_PACKED_ENUM {
    // Indexes for `documents` array
    MAGICK_BEST_FIT_I = 0,
    MAGICK_TRANSPARENT_BG_I,
    MAGICK_OPEN_ON_DONE_I,
    MAGICK_RESIZE_I,
    MAGICK_SET_RESOLUTION_I,
    MAGICK_ADVANCED_SETTINGS_I,

    // Not in `documents` array
    // These should only be placed here to not confuse comparison
    MAGICK_WELCOME_PAGE_I,
    MAGICK_SETTINGS_I,
};

typedef struct rgba {
    // alpha is in [0-100]
    uint8_t r, g, b, a;
} rgba;

typedef struct rgba_str {
    // 3 for digits [0-255], 1 for \0
    char r[4], g[4], b[4], a[4];
} rgba_str;

typedef struct resize_t {
    uint16_t w, h;
} resize_t;

typedef struct resize_str_t {
    char w[6], h[6];  // max: 6 bytes for string repr of uint16
} resize_str_t;

typedef struct resize_element_t {
    Clay_String id;
    resize_t values;
    resize_str_t str;
    uint8_t step;
} resize_element_t;

CLAY_PACKED_ENUM {
    RESIZES_INPUT,
    RESIZES_OUTPUT_RES,
    RESIZES_OUTPUT_MARGIN,
};

#define gravity_len 10
typedef struct gravity_t {
    char *values[gravity_len];
    uint8_t selected;
} gravity_t;

#define resizes_len 3
typedef struct magick_params_t {
    gravity_t gravity;
    resize_element_t resizes[resizes_len];
    Nob_Cmd inputFiles;
    Nob_String_Builder outputFile;
    Nob_String_Builder tempDir;
    Nob_String_Builder magickBinary;
    Texture2D logo;
    rgba_str color_str;
    rgba color;
    Nob_Proc magickProc;


    uint16_t state;
    uint8_t tip;
    Nob_ProcStatus threadRunning;
} magick_params_t;

typedef struct {
    magick_params_t params;
    AppArena frameArena;
    struct cthreads_args threadArgs;
    struct cthreads_thread magickThread;
    uint16_t tabWidth;
    uint8_t selectedDocumentIndex;
    MagickStatus errorIndex;
    // These should be stored on the heap to prevent
    // race condition caused by stack being gone
    bool shouldClose;
} AppData;

typedef struct {
    uint16_t *state;
    uint16_t flag;
} FlagClickData;

typedef struct {
    uint16_t* state;
    uint8_t* selectedDocumentIndex;
    uint8_t requestedDocumentIndex;
} SidebarClickData;

static inline void HandleSidebarInteraction(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData
) {
    (void) elementId;
    SidebarClickData *clickData = (SidebarClickData*)userData;
    // If this button was clicked
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (clickData->requestedDocumentIndex < documents.length) {
            if (clickData->requestedDocumentIndex == MAGICK_BEST_FIT_I) {
                *clickData->state ^= MAGICK_BEST_FIT;
            } else if (clickData->requestedDocumentIndex == MAGICK_TRANSPARENT_BG_I) {
                *clickData->state ^= MAGICK_TRANSPARENT_BG;
            } else if (clickData->requestedDocumentIndex == MAGICK_OPEN_ON_DONE_I) {
                *clickData->state ^= MAGICK_OPEN_ON_DONE;
            } else if (clickData->requestedDocumentIndex == MAGICK_RESIZE_I) {
                *clickData->state ^= MAGICK_RESIZE;
            } else if (clickData->requestedDocumentIndex == MAGICK_SET_RESOLUTION_I) {
                *clickData->state ^= MAGICK_SET_RESOLUTION;
            }
            // Select the corresponding document
            *clickData->selectedDocumentIndex = clickData->requestedDocumentIndex;
        }
    }
}

static inline void HandleActiveColor(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData
) {
    (void) elementId;
    magick_params_t *params = (magick_params_t*)userData;
    rgba *color = &params->color;
    rgba_str *color_str = &params->color_str;
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        fprintf(stderr, "You pressed color, nice!\n");
        unsigned char aoResultRGB[3] = {color->r, color->g, color->b};
        fprintf(stderr, "old rgb: {%d, %d, %d}\n", color->r, color->g, color->b);
        if (tinyfd_colorChooser(NULL, NULL, aoResultRGB, aoResultRGB) == NULL)
            return;
        color->r = aoResultRGB[0]; sprintf(color_str->r, "%d", color->r);
        color->g = aoResultRGB[1]; sprintf(color_str->g, "%d", color->g);
        color->b = aoResultRGB[2]; sprintf(color_str->b, "%d", color->b);
        fprintf(stderr, "new rgb: {%d, %d, %d}\n", color->r, color->g, color->b);
    }
}

#define HANDLE_TOGGLE(name, flag)                                  \
static inline void Handle##name(Clay_ElementId elementId,          \
                                Clay_PointerData pointerData,      \
                                intptr_t userData) {               \
    (void)elementId;                                               \
    uint16_t *state = (uint16_t *)userData;                        \
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) \
      *state ^= (flag);                                            \
}

HANDLE_TOGGLE(OutputRes, MAGICK_SET_RESOLUTION)
HANDLE_TOGGLE(Resizes, MAGICK_RESIZE)
HANDLE_TOGGLE(Gravity, MAGICK_GRAVITY)

static inline void RenderMagickColor(const rgba *color, const uint16_t state) {
    if (state & MAGICK_TRANSPARENT_BG) {
        RenderColor((Clay_Color) { .r = 0, .g = 0, .b = 0, .a = 0 });
    } else {
        RenderColor((Clay_Color) { .r = color->r,
                                   .g = color->g,
                                   .b = color->b,
                                   .a = 255.0*color->a/100 });
    }
}

#define RenderResize(element, id_str) do { \
    CLAY({ \
        .backgroundColor = BUTTON_COLOR, \
        .id = CLAY_ID(id_str), \
        .cornerRadius = BUTTON_RADIUS, \
    }) { \
        CLAY({ .id = CLAY_ID(id_str "W"), .layout = { .padding = { .left = S(16), .top = S(8), .bottom = S(8) } } }) { \
            CLAY_TEXT(CLAY_DYNAMIC_STRING((element)->str.w), BUTTON_TEXT); \
        } \
        CLAY({ .id = CLAY_ID(id_str "Each"), .layout = { .padding = {.top = S(8), .bottom = S(8)} }}) { CLAY_TEXT(CLAY_STRING("x"), BUTTON_TEXT); } \
        CLAY({ .id = CLAY_ID(id_str "H"), .layout = { .padding = { .right = S(16), .top = S(8), .bottom = S(8) } } }) { \
            CLAY_TEXT(CLAY_DYNAMIC_STRING((element)->str.h), BUTTON_TEXT); \
        } \
    } \
} while (0)

static inline void HandleFlagInteraction(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData
) {
    (void) elementId;
    FlagClickData *clickData = (FlagClickData *)userData;
    // If this button was clicked
    if (pointerData.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME) {
        *clickData->state ^= clickData->flag;
    }
}

void RenderFlag(Clay_String text,
                uint16_t *state,
                uint16_t triggerFlag,
                uint16_t displayFlag,
                AppArena *arena) {
    Clay_Color background = BUTTON_COLOR;
    if (*state & displayFlag)
        background = OPACITY(c10n(COLOR_GREEN), 35);
    CLAY({
        .clip = {.horizontal = true},
        .layout = {
            .padding = defaultPadding,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
        .id = CLAY_SID(text),
        .backgroundColor = background,
        .cornerRadius = BUTTON_RADIUS,
    }) {
        FlagClickData *clickData = (FlagClickData *)(arena->memory + arena->offset);
        *clickData = (FlagClickData) { .flag = triggerFlag, .state = state };
        arena->offset += sizeof(FlagClickData);
        Clay_OnHover(HandleFlagInteraction, (intptr_t)clickData);

        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BUTTONS,
            .fontSize = S(buttons_font_size),
            .textColor = c10n(COLOR_TEXT),
            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
            .wrapMode = CLAY_TEXT_WRAP_NONE,
        }));
    }
}

void DocumentsUpdate() {
    // Update DocumentArray
    documents.documents[MAGICK_BEST_FIT_I] = (Document){ .title = i18n(AS_TEXT_BEST_FIT), .contents = i18n(AS_TEXT_BEST_FIT_EXPL) };
    documents.documents[MAGICK_TRANSPARENT_BG_I] = (Document){ .title = i18n(AS_TEXT_TRANSPARENT_BG), .contents = i18n(AS_TEXT_TRANSPARENT_BG_EXPL) };
    documents.documents[MAGICK_OPEN_ON_DONE_I] = (Document){ .title = i18n(AS_TEXT_OPEN_ON_DONE), .contents = i18n(AS_TEXT_OPEN_ON_DONE_EXPL) };
    documents.documents[MAGICK_RESIZE_I] = (Document){ .title = i18n(AS_TEXT_ENABLE_RESIZE), .contents = i18n(AS_TEXT_ENABLE_RESIZE_EXPL) };
    documents.documents[MAGICK_SET_RESOLUTION_I] = (Document){ .title = i18n(AS_TEXT_SET_OUTPUT_RES), .contents = i18n(AS_TEXT_SET_OUTPUT_RES_EXPL) };
    documents.documents[MAGICK_ADVANCED_SETTINGS_I] = (Document){ .title = i18n(AS_ADVANCED_SETTINGS_S), .contents = i18n(AS_TEXT_ADVANCED_SETTINGS_EXPL) };
}

AppData AppDataInit(void) {
    DocumentsUpdate();
    AppData data = {
        .frameArena = { .memory = (intptr_t)malloc(1024) },
        .shouldClose = false,
        // .selectedDocumentIndex = MAGICK_ADVANCED_SETTINGS,
        .selectedDocumentIndex = MAGICK_WELCOME_PAGE_I,
        .errorIndex = 0,
        .tabWidth = 260,
        .params = {
            .state = MAGICK_BEST_FIT | MAGICK_OPEN_ON_DONE | MAGICK_RESIZE,
            .tip = rand() % APP_TIPS,
            .outputFile = {0},
            .tempDir = {0},
            .magickBinary = {0},
            .inputFiles = {0},
            .resizes = {
                {
                    .id = CLAY_STRING(ID_RESIZE_INPUT),
                    .values = {.w =  1000,  .h =  1000 },
                    .str    = {.w = "1000", .h = "1000"},
                    .step = 50,
                },
                {
                    .id = CLAY_STRING(ID_RESIZE_OUTPUT),
                    .values = {.w =  4000,  .h =  4000 },
                    .str    = {.w = "4000", .h = "4000"},
                    .step = 50,
                },
                {
                    .id = CLAY_STRING(ID_RESIZE_OUTPUT_MARGIN),
                    .values = {.w =  4,  .h =  4 },
                    .str    = {.w = "4", .h = "4"},
                    .step = 1,
                },
            },
            .color =     { .r =  0,  .b =  0,  .g =  0,  .a =  0  },
            // This should be set because these strings are only updated when `color` is updated
            .color_str = { .r = "0", .b = "0", .g = "0", .a = "0" },
            .magickProc = NOB_INVALID_PROC,
            .threadRunning = false,
            .gravity = {
                .values = {
                    "None",
                    "Center",
                    "East",
                    // "Forget", // here is also this
                    "NorthEast",
                    "North",
                    "NorthWest",
                    "SouthEast",
                    "South",
                    "SouthWest",
                    "West",
                },
                .selected = 1,
            },
#ifdef INSTALLED
            .logo = LoadTexture("/usr/share/dicolmumag/banner.png"),
#endif // INSTALLED
        },
        .magickThread = {0},
    };
    nob_sb_append_cstr(&data.params.outputFile, "res.png");
    nob_sb_append_null(&data.params.outputFile);

#ifndef INSTALLED
    if (nob_file_exists("banner.png") == 1) {
        data.params.logo = LoadTexture("banner.png");
#ifndef _WIN32
    } else if (nob_file_exists("../resources/banner.png") == 1) {
        data.params.logo = LoadTexture("../resources/banner.png");
#endif // _WIN32
    }
#endif // INSTALLED

#ifdef _WIN32
    nob_sb_append_cstr(&data.params.magickBinary, "magick.exe");
#else
  #ifdef APPIMAGE
    fprintf(stderr, "Appdir: %s\n", getenv("APPDIR"));
    nob_sb_append_cstr(&data.params.magickBinary, getenv("APPDIR"));
    nob_sb_append_cstr(&data.params.magickBinary, "/usr/bin/magick");
  #else
    nob_sb_append_cstr(&data.params.magickBinary, "magick");
  #endif // APPIMAGE
#endif // _WIN32
    nob_sb_append_null(&data.params.magickBinary);

    return data;
}

Clay_RenderCommandArray AppCreateLayout(AppData *data) {
    data->frameArena.offset = 0;

    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };

    Clay_Color contentBackgroundColor = c10n(COLOR_BASE);
    defaultPadding = (Clay_Padding) { S(16), S(16), S(8), S(8) };
    // Build UI here
    CLAY({
        .backgroundColor = c10n(COLOR_MANTLE),
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = layoutExpand,
            .padding = CLAY_PADDING_ALL(S(16)),
            .childGap = S(16),
        }
    }) {
        // Child elements go inside braces
        CLAY({
            .layout = {
                .sizing = {
                    .height = CLAY_SIZING_FIXED(S(60)),
                    .width = CLAY_SIZING_GROW(0)
                },
                .padding = CLAY_PADDING_ALL(S(16)),
                .childGap = S(16),
                .childAlignment = {
                    .y = CLAY_ALIGN_Y_CENTER,
                }
            },
            .clip = { .horizontal = true, .childOffset = Clay_GetScrollOffset() },
            .cornerRadius = LAYOUT_RADIUS,
            .backgroundColor = contentBackgroundColor,
        }) {
            // Header buttons go here
            CLAY({
                .id = CLAY_ID(ID_FILE_BUTTON),
                .layout = { .padding = defaultPadding},
                .backgroundColor = BUTTON_COLOR,
                .cornerRadius = BUTTON_RADIUS,
            }) {
                CLAY_TEXT(i18n(AS_BUTTON_FILE), BUTTON_TEXT);

                bool fileMenuVisible =
                    Clay_PointerOver(Clay_GetElementId(CLAY_STRING(ID_FILE_BUTTON)))
                    ||
                    Clay_PointerOver(Clay_GetElementId(CLAY_STRING(ID_FILE_MENU)));

                if (fileMenuVisible) {
                    CLAY({
                        .id = CLAY_ID(ID_FILE_MENU),
                        .floating = {
                            .attachTo = CLAY_ATTACH_TO_PARENT,
                            .attachPoints = {
                                .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM
                            },
                        },
                        .layout = {
                            .padding = { 0, 0, S(8), S(8) }
                        }
                    }) {
                        CLAY({
                            .layout = {
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                .sizing = {
                                    .width = CLAY_SIZING_FIXED(S(200))
                                },
                            },
                            .backgroundColor = c10n(COLOR_SURFACE0),
                            .cornerRadius = BUTTON_RADIUS,
                        }) {
                            // Render dropdown items here
                            RenderDropdownMenuItem(i18n(AS_BUTTON_OPEN_RESULT),     "O");
                            RenderDropdownMenuItem(i18n(AS_BUTTON_CHANGE_UI_COLOR), "C");
                            RenderDropdownMenuItem(i18n(AS_BUTTON_CHANGE_LANGUAGE), "I");
                            RenderDropdownMenuItem(i18n(AS_BUTTON_SETTINGS),        "S");
                            CLAY({
                                .backgroundColor = c10n(COLOR_SURFACE0),
                                .cornerRadius = BUTTON_RADIUS,
                                .id = CLAY_ID(ID_QUIT),
                                .layout = {
                                    .padding = CLAY_PADDING_ALL(S(16)),
                                    .sizing = { .width = CLAY_SIZING_GROW(0) },
                                },
                            }) {
                                CLAY_TEXT(i18n(AS_BUTTON_QUIT), BUTTON_TEXT);
                                HorizontalSeparator();
                                CLAY_TEXT(CLAY_STRING("Ctrl-Q"),
                                        CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BUTTONS, .fontSize = S(buttons_font_size), .textColor = c10n(COLOR_OVERLAY0) }));
                            }
                        }
                    }
                }
            }
            RenderHeaderButton(i18n(AS_BUTTON_SELECT_IMAGES));
            if (data->params.magickProc == NOB_INVALID_PROC)
                RenderHeaderButton(i18n(AS_BUTTON_RUN));
            else
                RenderHeaderButton(i18n(AS_BUTTON_STOP));
            HorizontalSeparator();
            if (i18n(errors[data->errorIndex]).chars[0] == '\0') {
                CLAY({.id = CLAY_ID(ID_INPUT_FILE)}) {CLAY_TEXT(CLAY_SB_STRING(data->params.outputFile), DEFAULT_TEXT);}
            } else if (data->errorIndex == MAGICK_ERROR_RUNNING) {
                CLAY({.id = CLAY_ID(ID_ERROR)}) {
                  CLAY_TEXT(i18n(errors[data->errorIndex]),
                      JUST_TEXT_CONFIG(document_font_size, FONT_ID_DOCUMENT, c10n(COLOR_BLUE))
                  );
                }
            } else {
                CLAY({.id = CLAY_ID(ID_ERROR)}) {
                  CLAY_TEXT(i18n(errors[data->errorIndex]),
                      JUST_TEXT_CONFIG(document_font_size, FONT_ID_DOCUMENT, c10n(COLOR_RED))
                  );
                }
            }
            HorizontalSeparator();
            CLAY({
                .layout = { .padding = defaultPadding},
                .backgroundColor = BUTTON_COLOR,
                .cornerRadius = BUTTON_RADIUS,
            }) {
                CLAY_TEXT(i18n(AS_SECTION_RESIZE), BUTTON_TEXT);
                CLAY_TEXT(CLAY_DYNAMIC_STRING(data->params.resizes[RESIZES_INPUT].str.w), BUTTON_TEXT);
            }
            RenderHeaderButton(i18n(AS_BUTTON_SUPPORT));
        }

        CLAY({
            .layout = { .sizing = layoutExpand, .childGap = S(16) },
        }) {
            CLAY({
                .backgroundColor = contentBackgroundColor,
                .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() },
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .padding = CLAY_PADDING_ALL(S(16)),
                    .childGap = S(8),
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(S(data->tabWidth)),
                        .height = CLAY_SIZING_GROW(0)
                    },
                },
                .cornerRadius = LAYOUT_RADIUS,
            }) {
                for (uint8_t i = 0; i < documents.length; i++) {
                    Document document = documents.documents[i];
                    Clay_LayoutConfig sidebarButtonLayout = {
                        .sizing = { .width = CLAY_SIZING_GROW(0) },
                        .padding = CLAY_PADDING_ALL(S(16))
                    };

                    SidebarClickData *clickData = (SidebarClickData *)(data->frameArena.memory + data->frameArena.offset);
                    *clickData = (SidebarClickData) { .requestedDocumentIndex = i, .selectedDocumentIndex = &data->selectedDocumentIndex, .state = &data->params.state };
                    data->frameArena.offset += sizeof(SidebarClickData);
                    if (
                        (i == MAGICK_BEST_FIT_I && data->params.state & MAGICK_BEST_FIT) ||
                        (i == MAGICK_TRANSPARENT_BG_I && data->params.state & MAGICK_TRANSPARENT_BG) ||
                        (i == MAGICK_OPEN_ON_DONE_I && data->params.state & MAGICK_OPEN_ON_DONE) ||
                        (i == MAGICK_RESIZE_I && data->params.state & MAGICK_RESIZE) ||
                        (i == MAGICK_SET_RESOLUTION_I && data->params.state & MAGICK_SET_RESOLUTION) ||
                        false  // for easier feature addition
                       ) {
                        CLAY({
                            .layout = sidebarButtonLayout,
                            .backgroundColor = OPACITY(c10n(COLOR_GREEN), 35),
                            .cornerRadius = CLAY_CORNER_RADIUS(S(8))
                        }) {
                            Clay_OnHover(HandleSidebarInteraction, (intptr_t)clickData);
                            CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({
                                .fontId = FONT_ID_SIDEBAR,
                                .fontSize = S(sidebar_font_size),
                                .textColor = c10n(COLOR_TEXT),
                            }));
                        }
                    } else {
                        CLAY({ .layout = sidebarButtonLayout, .backgroundColor = BUTTON_COLOR, .cornerRadius = CLAY_CORNER_RADIUS(8) }) {
                            Clay_OnHover(HandleSidebarInteraction, (intptr_t)clickData);
                            CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({
                                .fontId = FONT_ID_SIDEBAR,
                                .fontSize = S(sidebar_font_size),
                                .textColor = c10n(COLOR_TEXT),
                            }));
                        }
                    }
                }
            }

            CLAY({
                .backgroundColor = contentBackgroundColor,
                .clip = { .vertical = true, /*.horizontal = true,*/ .childOffset = Clay_GetScrollOffset() },
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .childGap = S(16),
                    .padding = CLAY_PADDING_ALL(S(16)),
                    .sizing = {
                        .height = CLAY_SIZING_GROW(0),
                        .width = CLAY_SIZING_GROW(0),
                    }
                },
                .cornerRadius = LAYOUT_RADIUS,
            }) {

                if (data->selectedDocumentIndex < MAGICK_WELCOME_PAGE_I) {
                    Document selectedDocument = documents.documents[data->selectedDocumentIndex];
                    CLAY_TEXT(selectedDocument.title, CLAY_TEXT_CONFIG({
                        .fontId = FONT_ID_TITLE,
                        .fontSize = S(title_font_size),
                        .textColor = c10n(COLOR_TEXT)
                    }));
                    CLAY_TEXT(selectedDocument.contents, DEFAULT_TEXT);
                }


                if (data->selectedDocumentIndex == MAGICK_ADVANCED_SETTINGS_I) {
                    CLAY({
                        .layout = {
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    }) {
                        CLAY({.layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM }}) {
                            StatedText(i18n(AS_SECTION_BACKGROUND_COLOR), !(data->params.state & MAGICK_TRANSPARENT_BG));
                            RenderMagickColor(&data->params.color, data->params.state);
                            Clay_OnHover(HandleActiveColor, (intptr_t)&data->params);
                        }
                        CLAY({
                            .layout = {
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(105),
                                },
                                .padding = CLAY_PADDING_ALL(10),
                                .childGap = S(8),
                            },
                        }) {
                            RenderColorChannel(CLAY_STRING("r"), c10n(COLOR_RED),   data->params.color_str.r);
                            RenderColorChannel(CLAY_STRING("g"), c10n(COLOR_GREEN), data->params.color_str.g);
                            RenderColorChannel(CLAY_STRING("b"), c10n(COLOR_BLUE),  data->params.color_str.b);
                            RenderColorChannel(CLAY_STRING("a"), c10n(COLOR_TEXT),  data->params.color_str.a);
                        }
                        if (data->params.state & MAGICK_TRANSPARENT_BG)
                            CLAY_TEXT(i18n(AS_TEXT_TRANSPARENT_BG_WARNING), DEFAULT_TEXT);
                    }
                    CLAY({
                        .layout = {
                            .childGap = S(8),
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        }
                    }) {
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = S(8)}}) {
                            StatedText(i18n(AS_SECTION_GRAVITY), data->params.state & MAGICK_GRAVITY);
                            Clay_OnHover(HandleGravity, (intptr_t) &data->params.state);
                            CLAY({
                                .backgroundColor = BUTTON_COLOR,
                                .cornerRadius = BUTTON_RADIUS,
                            }) {
                                CLAY({ .id = CLAY_ID(ID_GRAVITY_SELECTION), .layout = { .padding = defaultPadding } }) {
                                    CLAY_TEXT(CLAY_DYNAMIC_STRING(data->params.gravity.values[data->params.gravity.selected]), BUTTON_TEXT);
                                }
                            }
                        }
                    }

                    CLAY({.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM, .childGap = 8}}) {
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = S(8)}}) {
                            StatedText(i18n(AS_SECTION_RESIZE_EACH), data->params.state & MAGICK_RESIZE);
                            RenderResize(&data->params.resizes[RESIZES_INPUT], ID_RESIZE_INPUT);
                            Clay_OnHover(HandleResizes, (intptr_t) &data->params.state);
                        };
                        // CLAY()
                        RenderFlag(i18n(AS_TEXT_IGNORE_ASPECT), &data->params.state, MAGICK_IGNORE_RATIO, MAGICK_IGNORE_RATIO, &data->frameArena);
                        // Well... this would only work for two
                        RenderFlag(i18n(AS_TEXT_SHRINK_LARGER), &data->params.state,
                                (data->params.state & MAGICK_ENLARGE_SMALLER) ? MAGICK_ENLARGE_SMALLER | MAGICK_SHRINK_LARGER : MAGICK_SHRINK_LARGER, MAGICK_SHRINK_LARGER, &data->frameArena);
                        RenderFlag(i18n(AS_TEXT_ENLARGE_SMALLER), &data->params.state,
                                (data->params.state & MAGICK_SHRINK_LARGER) ? MAGICK_SHRINK_LARGER | MAGICK_ENLARGE_SMALLER : MAGICK_ENLARGE_SMALLER, MAGICK_ENLARGE_SMALLER, &data->frameArena);
                        RenderFlag(i18n(AS_TEXT_FILL_AREA), &data->params.state, MAGICK_FILL_AREA, MAGICK_FILL_AREA, &data->frameArena);
                    }

                    CLAY({
                        .layout = {
                            .childGap = S(8),
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            .childAlignment = {
                                .x = CLAY_ALIGN_X_LEFT,
                                .y = CLAY_ALIGN_Y_CENTER,
                            },
                        }
                    }) {
                        CLAY() {
                            StatedText(i18n(AS_TEXT_OUTPUT_RES), data->params.state & MAGICK_SET_RESOLUTION);
                            Clay_OnHover(HandleOutputRes, (intptr_t) &data->params.state);
                        }
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = S(8), .padding = S(8), .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}}) {
                            CLAY_TEXT(i18n(AS_TEXT_DIMENSIONS), BUTTON_TEXT);
                            RenderResize(&data->params.resizes[RESIZES_OUTPUT_RES], ID_RESIZE_OUTPUT);
                        }
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = S(8), .padding = S(8), .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}}) {
                            CLAY_TEXT(i18n(AS_TEXT_MARGIN), BUTTON_TEXT);
                            RenderResize(&data->params.resizes[RESIZES_OUTPUT_MARGIN], ID_RESIZE_OUTPUT_MARGIN);
                        }
                    }

                    CLAY({
                        .layout = {
                            .childGap = S(8),
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        }
                    }) {
                        CLAY_TEXT(i18n(AS_TEXT_TEMP_FILES), DEFAULT_TEXT);
                        CLAY({.layout = {.padding = {.left = S(8)}}}) {CLAY_TEXT(i18n(AS_TEMP_FILES_EXPLANATION), BUTTON_TEXT);}
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = S(8), .padding = {.left = S(8)}}}) {
                            CLAY_TEXT(i18n(AS_TEXT_CURRENT), BUTTON_TEXT);
                            if (data->params.tempDir.count == 0)
                                CLAY_TEXT(i18n(AS_TEXT_TEMP_DEFAULT), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BUTTONS, .fontSize = S(buttons_font_size), .textColor = c10n(COLOR_OVERLAY0) }));
                            else
                                CLAY_TEXT(CLAY_SB_STRING(data->params.tempDir), BUTTON_TEXT);
                        }
                        RenderHeaderButton(i18n(AS_SELECT_TEMP));
                    }

                    CLAY({
                        .id = CLAY_ID(ID_BUTTON_SELECT_MAGICK),
                        .layout = {
                            .childGap = S(8),
                        }
                    }) {
                        CLAY_TEXT(i18n(AS_MAGICK_EXEC), DEFAULT_TEXT);
                        RenderHeaderButton(CLAY_SB_STRING(data->params.magickBinary));
                    }
                } else if (data->selectedDocumentIndex == MAGICK_WELCOME_PAGE_I) {
                    CLAY({
                        .layout = {
                            .childGap = S(20),
                            .padding = CLAY_PADDING_ALL(S(20)),
                            .childAlignment = {
                                .x = CLAY_ALIGN_X_CENTER,
                                .y = CLAY_ALIGN_Y_CENTER,
                            },
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            .sizing = {
                                .width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0),
                            }
                        }
                    }) {
                        CLAY_TEXT(i18n(AS_TEXT_WELCOME),
                            CLAY_TEXT_CONFIG({
                                .fontId = FONT_ID_WELCOME,
                                .fontSize = S(welcome_font_size),
                                .textColor = c10n(COLOR_TEXT),
                                .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                            }));
                        CLAY({
                            .layout = {
                                .sizing = {
                                    .width = CLAY_SIZING_FIT(S(500)),
                                    .height = CLAY_SIZING_FIT(S(500)),
                                }
                            },
                            .image = { .imageData = &data->params.logo },
                            .aspectRatio = (float) data->params.logo.width / data->params.logo.height,
                        });
                        CLAY_TEXT(i18n(AS_TEXT_SLOGAN), CLAY_TEXT_CONFIG({
                            .fontId = FONT_ID_DOCUMENT,
                            .fontSize = S(document_font_size),
                            .textColor = c10n(COLOR_TEXT),
                            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                        }));
                        CLAY_TEXT(i18n(tips[data->params.tip]), CLAY_TEXT_CONFIG({
                            .fontId = FONT_ID_DOCUMENT,
                            .fontSize = S(document_font_size),
                            .textColor = c10n(COLOR_TEXT),
                            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                        }));

                        Clay_String start = i18n(AS_START_USING);
                        CLAY({
                            .layout = {
                                .padding = defaultPadding,
                                .childAlignment = {
                                    .x = CLAY_ALIGN_X_CENTER,
                                    .y = CLAY_ALIGN_Y_CENTER,
                                },
                            },
                            .backgroundColor = BUTTON_COLOR,
                            .id = CLAY_SID(start),
                            .cornerRadius = BUTTON_RADIUS,
                        }) {
                            CLAY_TEXT(start, CLAY_TEXT_CONFIG({
                                .fontId = FONT_ID_DOCUMENT,
                                .fontSize = S(document_font_size),
                                .textColor = c10n(COLOR_TEXT),
                                .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                                .wrapMode = CLAY_TEXT_WRAP_NONE,
                            }));
                        }
                    }
                } else if (data->selectedDocumentIndex == MAGICK_SETTINGS_I) {
                    CLAY() {
                        CLAY_TEXT(i18n(AS_SETTINGS), CLAY_TEXT_CONFIG({
                            .fontId = FONT_ID_DOCUMENT,
                            .fontSize = S(document_font_size),
                            .textColor = c10n(COLOR_TEXT),
                            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                        }));
                        CLAY_TEXT(i18n(AS_SCALE), BUTTON_TEXT);

                        char* scale_str = (char *)(data->frameArena.memory + data->frameArena.offset);
#define little_float_len 4
                        snprintf(scale_str, little_float_len, "%f", app_scale);
                        data->frameArena.offset += little_float_len;
                        RenderHeaderButton((Clay_String) { .chars = (const char *) scale_str, .length = little_float_len, .isStaticallyAllocated = false});
                        // Clay_OnHover(HandleFlagInteraction, (intptr_t)clickData);
                    };
                }
            }
        }
    }

    return Clay_EndLayout();
}
