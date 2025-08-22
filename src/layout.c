#include "thirdparty/clay.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// #define LATTE
#include "colors.c"
#include "thirdparty/cthreads.h"
#define NOB_IMPLEMENTATION
#include "thirdparty/nob.h"
#include "strings.h"

const uint8_t FONT_ID_BODY_16 = 0;
const uint8_t FONT_LOAD_SIZE = 40;
const uint8_t button_font_size = 20;
const uint8_t sidebar_font_size = 23;
const uint8_t document_font_size = 30;
const uint8_t title_font_size = 40;

#define CLAY_DYNAMIC_STRING(string) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = strlen(string), .chars = (string) })
#define CLAY_SB_STRING(sb) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = (sb).count, .chars = (sb).items })
#define BUTTON_RADIUS CLAY_CORNER_RADIUS(10)
#define LAYOUT_RADIUS CLAY_CORNER_RADIUS(10)
#define SANE_TEXT_CONFIG(font_size) CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_16, .fontSize = (font_size), .textColor = colors[colorscheme][COLOR_TEXT] })
#define DEFAULT_TEXT SANE_TEXT_CONFIG(document_font_size)
#define BUTTON_TEXT  SANE_TEXT_CONFIG(button_font_size)

#define MAGICK_RESIZE_IGNORE_RATIO    "!"
#define MAGICK_RESIZE_SHRINK_LARGER   ">"
#define MAGICK_RESIZE_ENLARGE_SMALLER "<"
#define MAGICK_RESIZE_FILL_AREA       "^"
#define MAGICK_RESIZE_PERCENTAGE      "%"
#define MAGICK_RESIZE_PIXEL_LIMIT     "@"

static inline void RenderHeaderButton(Clay_String text) {
    CLAY({
        .layout = {
            .padding = { 16, 16, 8, 8 },
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
            .fontId = FONT_ID_BODY_16,
            .fontSize = button_font_size,
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
            .padding = CLAY_PADDING_ALL(16),
            .sizing = {
                .width = CLAY_SIZING_GROW(0)
            },
    }}) {
        CLAY_TEXT(text, BUTTON_TEXT);
    }
}

static inline void RenderNumberPicker(char* value) {
    CLAY({
        .layout = { .padding = { 16, 16, 8, 8 }},
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
        CLAY({ .layout = { .sizing = { .width = 25 }}}) {
            CLAY_TEXT(text, CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_BODY_16,
                .fontSize = document_font_size,
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
        .cornerRadius = CLAY_CORNER_RADIUS(radius),
        .backgroundColor = color,
        .border = {
            .color = TERNARY_COLOR(Clay_Hovered(), colors[colorscheme][COLOR_SURFACE1], colors[colorscheme][COLOR_SURFACE0]),
            .width = CLAY_BORDER_OUTSIDE(border_width)
        },
        .layout = {
            .sizing = {
                .height = CLAY_SIZING_FIXED(size),
                .width = CLAY_SIZING_FIXED(size)
            },
        },
    }) {}
}

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
} ClayVideoDemo_Arena;

typedef enum {
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
} MagickState;

// Indexes for `documents` array
enum {
    MAGICK_BEST_FIT_I,
    MAGICK_TRANSPARENT_BG_I,
    MAGICK_OPEN_ON_DONE_I,
    MAGICK_RESIZE_I,
    MAGICK_SET_RESOLUTION_I,
    MAGICK_ADVANCED_SETTINGS,
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

enum {
    RESIZES_INPUT,
    RESIZES_OUTPUT_RES,
    RESIZES_OUTPUT_MARGIN,
};

#define gravity_len 10
typedef struct gravity_t {
    char *values[gravity_len];
    uint8_t selected;
} gravity_t;

typedef enum ProcStatus : uint8_t {
    PROCESS_RUNNING,
    PROCESS_EXITED_OK,
    PROCESS_CRASHED,
    PROCESS_WAS_TERMINATED,
} ProcStatus;

#define resizes_len 3
typedef struct magick_params_t {
    uint16_t state;
    Nob_Cmd inputFiles;
    Nob_String_Builder outputFile;
    Nob_String_Builder tempDir;

    gravity_t gravity;

    resize_element_t resizes[resizes_len];

    rgba color;
    rgba_str color_str;

    Nob_String_Builder magickBinary;
    Nob_Proc magickProc;
    ProcStatus threadRunning;
} magick_params_t;

typedef struct {
    uint8_t selectedDocumentIndex;
    MagickStatus errorIndex;
    float yOffset;
    ClayVideoDemo_Arena frameArena;
    magick_params_t params;
    struct cthreads_thread magickThread;
    bool shouldClose;
} ClayVideoDemo_Data;

typedef struct {
    uint16_t flag;
    uint16_t *state;
} FlagClickData;

typedef struct {
    uint8_t requestedDocumentIndex;
    uint8_t* selectedDocumentIndex;
    uint16_t* state;
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

static inline void RenderMagickColor(rgba *color, uint16_t state) {
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
        CLAY({ .id = CLAY_ID(id_str "W"), .layout = { .padding = { .left = 16, .top = 8, .bottom = 8 } } }) { \
            CLAY_TEXT(CLAY_DYNAMIC_STRING((element)->str.w), BUTTON_TEXT); \
        } \
        CLAY({ .id = CLAY_ID(id_str "Each"), .layout = { .padding = {.top = 8, .bottom = 8} }}) { CLAY_TEXT(CLAY_STRING("x"), BUTTON_TEXT); } \
        CLAY({ .id = CLAY_ID(id_str "H"), .layout = { .padding = { .right = 16, .top = 8, .bottom = 8 } } }) { \
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
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        *clickData->state ^= clickData->flag;
    }
}

static inline void RenderFlag(Clay_String text,
                uint16_t *state,
                uint16_t triggerFlag,
                uint16_t displayFlag,
                ClayVideoDemo_Arena *arena) {
    Clay_Color background = BUTTON_COLOR;
    if (*state & displayFlag)
        background = OPACITY(colors[colorscheme][COLOR_GREEN], 35);
    CLAY({
        .layout = {
            .padding = { 16, 16, 8, 8 },
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
        .backgroundColor = background,
        .id = CLAY_SID(text),
        .cornerRadius = BUTTON_RADIUS,
    }) {
        FlagClickData *clickData = (FlagClickData *)(arena->memory + arena->offset);
        *clickData = (FlagClickData) { .flag = triggerFlag, .state = state };
        arena->offset += sizeof(FlagClickData);
        Clay_OnHover(HandleFlagInteraction, (intptr_t)clickData);

        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = button_font_size,
            .textColor = colors[colorscheme][COLOR_TEXT],
            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
            .wrapMode = CLAY_TEXT_WRAP_NONE,
        }));
    }
}

#define RESIZE_INPUT_S "Resize"
#define RESIZE_OUTPUT_S "OutputDimentions"
#define RESIZE_OUTPUT_MARGIN_S "OutputMargin"

ClayVideoDemo_Data ClayVideoDemo_Initialize() {
    // Update DocumentArray
    documents.documents[MAGICK_BEST_FIT_I] = (Document){ .title = CLAY_STRING("Best fit"), .contents = CLAY_STRING("This ashlar option aligns images on both sides of the resulting image") };
    documents.documents[MAGICK_TRANSPARENT_BG_I] = (Document){ .title = CLAY_STRING("Transparent background"), .contents = CLAY_STRING("Makes the background transparent\nThis overrides background configuration in "ADVANCED_SETTINGS_Q" tab") };
    documents.documents[MAGICK_OPEN_ON_DONE_I] = (Document){ .title = CLAY_STRING("Open when done"), .contents = CLAY_STRING("Enable this to see the result right after it's done!\n\nNothing more\nsurely") };
    documents.documents[MAGICK_RESIZE_I] = (Document){ .title = CLAY_STRING("Enable Resize"), .contents = CLAY_STRING("This option enables resizes. You can configure how input images are resized in "ADVANCED_SETTINGS_Q) };
    documents.documents[MAGICK_SET_RESOLUTION_I] = (Document){ .title = CLAY_STRING("Set output resolution"), .contents = CLAY_STRING("With this option you can directly set the desired output resolution for the collage in "ADVANCED_SETTINGS_Q" tab\nIf this option is disabled, the resolution for the output image (filename at the top) will be chosen automatically") };
    documents.documents[MAGICK_ADVANCED_SETTINGS] = (Document){ .title = CLAY_STRING(ADVANCED_SETTINGS_S), .contents = CLAY_STRING("This tab contains advanced settings (surprise!)") };

    ClayVideoDemo_Data data = {
        .frameArena = { .memory = (intptr_t)malloc(1024) },
        .shouldClose = false,
        .selectedDocumentIndex = MAGICK_ADVANCED_SETTINGS,
        .errorIndex = 0,
        .params = {
            .state = MAGICK_BEST_FIT | MAGICK_OPEN_ON_DONE | MAGICK_RESIZE | MAGICK_SHRINK_LARGER,
            .outputFile = {0},
            .tempDir = {0},
            .magickBinary = {0},
            .inputFiles = {0},
            .resizes = {
                {
                    .id = CLAY_STRING(RESIZE_INPUT_S),
                    .values = {.w =  1000,  .h =  1000 },
                    .str    = {.w = "1000", .h = "1000"},
                    .step = 50,
                },
                {
                    .id = CLAY_STRING(RESIZE_OUTPUT_S),
                    .values = {.w =  4000,  .h =  4000 },
                    .str    = {.w = "4000", .h = "4000"},
                    .step = 50,
                },
                {
                    .id = CLAY_STRING(RESIZE_OUTPUT_MARGIN_S),
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
        },
        .magickThread = {0},
    };
    nob_sb_append_cstr(&data.params.outputFile, "res.png");

#ifdef APPIMAGE
    fprintf(stderr, "Appdir: %s\n", getenv("APPDIR"));
    nob_sb_append_cstr(&data.params.magickBinary, getenv("APPDIR"));
    nob_sb_append_cstr(&data.params.magickBinary, "/usr/bin/magick");
#else
    nob_sb_append_cstr(&data.params.magickBinary, "magick");
#endif // APPIMAGE

    nob_sb_append_null(&data.params.magickBinary);
    return data;
}

Clay_RenderCommandArray ClayVideoDemo_CreateLayout(ClayVideoDemo_Data *data) {
    data->frameArena.offset = 0;

    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };

    Clay_Color contentBackgroundColor = colors[colorscheme][COLOR_BASE];

    // Build UI here
    CLAY({
        .id = CLAY_ID("OuterContainer"),
        .backgroundColor = colors[colorscheme][COLOR_MANTLE],
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = layoutExpand,
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16,
        }
    }) {
        // Child elements go inside braces
        CLAY({ .id = CLAY_ID("HeaderBar"),
            .layout = {
                .sizing = {
                    .height = CLAY_SIZING_FIXED(60),
                    .width = CLAY_SIZING_GROW(0)
                },
                .padding = CLAY_PADDING_ALL(16),
                .childGap = 16,
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
                .id = CLAY_ID("FileButton"),
                .layout = { .padding = { 16, 16, 8, 8 }},
                .backgroundColor = BUTTON_COLOR,
                .cornerRadius = BUTTON_RADIUS,
            }) {
                CLAY_TEXT(CLAY_STRING("File"), BUTTON_TEXT);

                bool fileMenuVisible =
                    Clay_PointerOver(Clay_GetElementId(CLAY_STRING("FileButton")))
                    ||
                    Clay_PointerOver(Clay_GetElementId(CLAY_STRING("FileMenu")));

                if (fileMenuVisible) {
                    CLAY({
                        .id = CLAY_ID("FileMenu"),
                        .floating = {
                            .attachTo = CLAY_ATTACH_TO_PARENT,
                            .attachPoints = {
                                .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM
                            },
                        },
                        .layout = {
                            .padding = { 0, 0, 8, 8 }
                        }
                    }) {
                        CLAY({
                            .layout = {
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                .sizing = {
                                    .width = CLAY_SIZING_FIXED(200)
                                },
                            },
                            .backgroundColor = colors[colorscheme][COLOR_SURFACE0],
                            .cornerRadius = BUTTON_RADIUS,
                        }) {
                            // Render dropdown items here
                            RenderDropdownMenuItem(CLAY_STRING("Open result"));
                            RenderDropdownMenuItem(CLAY_STRING("Change colorscheme"));
                            Clay_String quit = CLAY_STRING("Quit");
                            CLAY({
                                .backgroundColor = colors[colorscheme][COLOR_SURFACE0],
                                .cornerRadius = BUTTON_RADIUS,
                                .id = CLAY_SID(quit),
                                .layout = {
                                    .padding = CLAY_PADDING_ALL(16),
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(0)
                                },
                            }}) {
                                CLAY_TEXT(quit, BUTTON_TEXT);
                                CLAY({ .layout = { .sizing = { CLAY_SIZING_GROW(0) }}}) {}
                                CLAY_TEXT(CLAY_STRING("Ctrl-Q"),
                                        CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_16, .fontSize = button_font_size, .textColor = colors[colorscheme][COLOR_OVERLAY0] }));
                            }
                        }
                    }
                }
            }
            RenderHeaderButton(CLAY_STRING("Select Images"));
            if (data->params.magickProc == NOB_INVALID_PROC)
                RenderHeaderButton(CLAY_STRING("Run"));
            else
                RenderHeaderButton(CLAY_STRING("Stop"));
            CLAY({ .layout = { .sizing = { CLAY_SIZING_GROW(0) }}}) {}
            if (errors[data->errorIndex][0] == '\0') {
                CLAY({.id = CLAY_ID("file")}) {CLAY_TEXT(CLAY_SB_STRING(data->params.outputFile), DEFAULT_TEXT);}
            } else {
                CLAY({.id = CLAY_ID("error")}) {
                    CLAY_TEXT(CLAY_DYNAMIC_STRING(errors[data->errorIndex]), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_16, .fontSize = document_font_size, .textColor = colors[colorscheme][COLOR_RED] }));
                }
            }
            CLAY({ .layout = { .sizing = { CLAY_SIZING_GROW(0) }}}) {}
            CLAY({
                .layout = { .padding = { 16, 16, 8, 8 }},
                .backgroundColor = BUTTON_COLOR,
                .id = CLAY_ID("ResizeAll"),
                .cornerRadius = BUTTON_RADIUS,
            }) {
                CLAY_TEXT(CLAY_STRING("Resize: "), BUTTON_TEXT);
                CLAY_TEXT(CLAY_DYNAMIC_STRING(data->params.resizes[RESIZES_INPUT].str.w), BUTTON_TEXT);
            }
            RenderHeaderButton(CLAY_STRING("Support"));
        }

        CLAY({
            .id = CLAY_ID("LowerContent"),
            .layout = { .sizing = layoutExpand, .childGap = 16 },

        }) {
            CLAY({
                .id = CLAY_ID("Sidebar"),
                .backgroundColor = contentBackgroundColor,
                .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() },
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 8,
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(260),
                        .height = CLAY_SIZING_GROW(0)
                    },
                },
                .cornerRadius = LAYOUT_RADIUS,
            }) {
                for (uint8_t i = 0; i < documents.length; i++) {
                    Document document = documents.documents[i];
                    Clay_LayoutConfig sidebarButtonLayout = {
                        .sizing = { .width = CLAY_SIZING_GROW(0) },
                        .padding = CLAY_PADDING_ALL(16)
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
                            .backgroundColor = OPACITY(colors[colorscheme][COLOR_GREEN], 35),
                            .cornerRadius = CLAY_CORNER_RADIUS(8)
                        }) {
                            Clay_OnHover(HandleSidebarInteraction, (intptr_t)clickData);
                            CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({
                                .fontId = FONT_ID_BODY_16,
                                .fontSize = sidebar_font_size,
                                .textColor = colors[colorscheme][COLOR_TEXT],
                            }));
                        }
                    } else {
                        CLAY({ .layout = sidebarButtonLayout, .backgroundColor = BUTTON_COLOR, .cornerRadius = CLAY_CORNER_RADIUS(8) }) {
                            Clay_OnHover(HandleSidebarInteraction, (intptr_t)clickData);
                            CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({
                                        .fontId = FONT_ID_BODY_16,
                                        .fontSize = sidebar_font_size,
                                        .textColor = colors[colorscheme][COLOR_TEXT],
                                        }));
                        }
                    }
                }
            }

            CLAY({ .id = CLAY_ID("MainContent"),
                .backgroundColor = contentBackgroundColor,
                .clip = { .vertical = true, /*.horizontal = true,*/ .childOffset = Clay_GetScrollOffset() },
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .childGap = 16,
                    .padding = CLAY_PADDING_ALL(16),
                    .sizing = {
                        .height = CLAY_SIZING_GROW(0),
                        .width = CLAY_SIZING_GROW(0),
                    }
                },
                .cornerRadius = LAYOUT_RADIUS,
            }) {
                Document selectedDocument = documents.documents[data->selectedDocumentIndex];
                CLAY_TEXT(selectedDocument.title, CLAY_TEXT_CONFIG({
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = title_font_size,
                    .textColor = colors[colorscheme][COLOR_TEXT]
                }));
                CLAY_TEXT(selectedDocument.contents, DEFAULT_TEXT);
                if (data->selectedDocumentIndex == MAGICK_ADVANCED_SETTINGS) {
                    CLAY({
                        .id = CLAY_ID("ColorSettings"),
                        .layout = {
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    }) {
                        CLAY({.layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM }}) {
                            CLAY_TEXT(CLAY_STRING("Background Color: "), DEFAULT_TEXT);
                            RenderMagickColor(&data->params.color, data->params.state);
                        }
                        CLAY({
                            .id = CLAY_ID("ColorSettingsRGB"),
                            .layout = {
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(105),
                                },
                                .padding = CLAY_PADDING_ALL(10),
                                .childGap = 8,
                            },
                        }) {
                            RenderColorChannel(CLAY_STRING("r"), colors[colorscheme][COLOR_RED],   data->params.color_str.r);
                            RenderColorChannel(CLAY_STRING("g"), colors[colorscheme][COLOR_GREEN], data->params.color_str.g);
                            RenderColorChannel(CLAY_STRING("b"), colors[colorscheme][COLOR_BLUE],  data->params.color_str.b);
                            RenderColorChannel(CLAY_STRING("a"), colors[colorscheme][COLOR_TEXT],  data->params.color_str.a);
                        }
                        if (data->params.state & MAGICK_TRANSPARENT_BG)
                            CLAY_TEXT(CLAY_STRING("Transparent background setting overrides this option"), DEFAULT_TEXT);
                    }
                    CLAY({
                        .id = CLAY_ID("GravitySettings"),
                        .layout = {
                            .childGap = 8,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        }
                    }) {
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 8}}) {
                            CLAY_TEXT(CLAY_STRING("Gravity: "), DEFAULT_TEXT);
                            CLAY({
                                    .backgroundColor = BUTTON_COLOR,
                                    .id = CLAY_ID("Gravity"),
                                    .cornerRadius = BUTTON_RADIUS,
                                    }) {
                                CLAY({ .id = CLAY_ID("GravitySelection"), .layout = { .padding = { 16, 16, 8, 8 } } }) {
                                    CLAY_TEXT(CLAY_DYNAMIC_STRING(data->params.gravity.values[data->params.gravity.selected]), BUTTON_TEXT);
                                }
                            }
                        }
                    }
                    CLAY({.id = CLAY_ID("ResizeSettings"), .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM, .childGap = 8}}) {
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 8}}) {
                            CLAY_TEXT(CLAY_STRING("Resize each image: "), DEFAULT_TEXT);
                            RenderResize(&data->params.resizes[RESIZES_INPUT], RESIZE_INPUT_S);
                        }
                        RenderFlag(CLAY_STRING("Ignore aspect ratio"), &data->params.state, MAGICK_IGNORE_RATIO, MAGICK_IGNORE_RATIO, &data->frameArena);
                        // Well... this would only work for two
                        RenderFlag(CLAY_STRING("Only Shrink Larger"), &data->params.state,
                                (data->params.state & MAGICK_ENLARGE_SMALLER) ? MAGICK_ENLARGE_SMALLER | MAGICK_SHRINK_LARGER : MAGICK_SHRINK_LARGER, MAGICK_SHRINK_LARGER, &data->frameArena);
                        RenderFlag(CLAY_STRING("Only Enlarge Smaller"), &data->params.state,
                                (data->params.state & MAGICK_SHRINK_LARGER) ? MAGICK_SHRINK_LARGER | MAGICK_ENLARGE_SMALLER : MAGICK_ENLARGE_SMALLER, MAGICK_ENLARGE_SMALLER, &data->frameArena);
                        RenderFlag(CLAY_STRING("Fill area"), &data->params.state, MAGICK_FILL_AREA, MAGICK_FILL_AREA, &data->frameArena);
                    }

                    CLAY({
                        .id = CLAY_ID("OutputSettings"),
                        .layout = {
                            .childGap = 8,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            .childAlignment = {
                                .x = CLAY_ALIGN_X_LEFT,
                                .y = CLAY_ALIGN_Y_CENTER,
                            },
                        }
                    }) {
                        CLAY_TEXT(CLAY_STRING(OUTPUT_RES), DEFAULT_TEXT);
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 8, .padding = 8, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}}) {
                            CLAY_TEXT(CLAY_STRING("Dimentions:"), BUTTON_TEXT);
                            RenderResize(&data->params.resizes[RESIZES_OUTPUT_RES], RESIZE_OUTPUT_S);
                        }
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 8, .padding = 8, .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}}) {
                            CLAY_TEXT(CLAY_STRING("Margin:"), BUTTON_TEXT);
                            RenderResize(&data->params.resizes[RESIZES_OUTPUT_MARGIN], RESIZE_OUTPUT_MARGIN_S);
                        }
                    }

                    CLAY({
                        .id = CLAY_ID("LocationSettings"),
                        .layout = {
                            .childGap = 8,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        }
                    }) {
                        CLAY_TEXT(CLAY_STRING(TEMP_FILES), DEFAULT_TEXT);
                        CLAY({.layout = {.padding = 8}}) {CLAY_TEXT(CLAY_STRING(TEMP_FILES_EXPLANATION), BUTTON_TEXT);}
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 8, .padding = 8}}) {
                            CLAY_TEXT(CLAY_STRING("Current:"), BUTTON_TEXT);
                            if (data->params.tempDir.count == 0)
                                CLAY_TEXT(CLAY_STRING("default"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_16, .fontSize = button_font_size, .textColor = colors[colorscheme][COLOR_OVERLAY0] }));
                            else
                                CLAY_TEXT(CLAY_SB_STRING(data->params.tempDir), BUTTON_TEXT);
                        }
                        RenderHeaderButton(CLAY_STRING(SELECT_TEMP));
                    }

                    CLAY({
                        .id = CLAY_ID("MagickBinary"),
                        .layout = {
                            .childGap = 8,
                        }
                    }) {
                        CLAY_TEXT(CLAY_STRING(MAGICK_EXEC), DEFAULT_TEXT);
                        RenderHeaderButton(CLAY_SB_STRING(data->params.magickBinary));
                    }
                }
            }
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    for (int32_t i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
    }
    return renderCommands;
}
