#include "clay.h"
#include <stdlib.h>
#include <string.h>

// #define LATTE
#include "colors.c"
#include "thirdparty/cthreads.h"
#define NOB_IMPLEMENTATION
#include "nob.h"
#include "strings.h"

const uint8_t FONT_ID_BODY_16 = 0;
const uint8_t FONT_LOAD_SIZE = 40;
const uint8_t button_font_size = 20;
const uint8_t sidebar_font_size = 23;
const uint8_t document_font_size = 30;
const uint8_t title_font_size = 40;

#define ADVANCED_SETTINGS 4

#define CLAY_DYNAMIC_STRING(string) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = strlen(string), .chars = (string) })
#define CLAY_SB_STRING(sb) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = (sb).count, .chars = (sb).items })
#define BUTTON_RADIUS CLAY_CORNER_RADIUS(10)
#define LAYOUT_RADIUS CLAY_CORNER_RADIUS(10)
#define SANE_TEXT_CONFIG(font_size) CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_16, .fontSize = (font_size), .textColor = COLOR_TEXT })
#define DEFAULT_TEXT SANE_TEXT_CONFIG(document_font_size)
#define BUTTON_TEXT  SANE_TEXT_CONFIG(button_font_size)

#define MAGICK_RESIZE_IGNORE_RATIO    "!"
#define MAGICK_RESIZE_SHRINK_LARGER   ">"
#define MAGICK_RESIZE_ENLARGE_SMALLER "<"
#define MAGICK_RESIZE_FILL_AREA       "^"
#define MAGICK_RESIZE_PERCENTAGE      "%"
#define MAGICK_RESIZE_PIXEL_LIMIT     "@"

void RenderHeaderButton(Clay_String text) {
    CLAY({
        .layout = {
            .padding = { 16, 16, 8, 8 },
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
            // .sizing = CLAY_SIZING_FIT(200, 200)
        },
        .backgroundColor = BUTTON_COLOR,
        .id = CLAY_SID(text),
        .cornerRadius = BUTTON_RADIUS,
    }) {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = button_font_size,
            .textColor = COLOR_TEXT,
            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
            .wrapMode = CLAY_TEXT_WRAP_NONE,
        }));
    }
}

void RenderDropdownMenuItem(Clay_String text) {
    CLAY({
        .backgroundColor = COLOR_SURFACE0,
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

void RenderNumberPicker(char* value) {
    CLAY({
        .layout = { .padding = { 16, 16, 8, 8 }},
        .backgroundColor = BUTTON_COLOR,
        // .id = CLAY_SID(name),
        .cornerRadius = BUTTON_RADIUS,
    }) {
        CLAY_TEXT(CLAY_DYNAMIC_STRING(value), BUTTON_TEXT);
    }
}

void RenderColorChannel(Clay_String text, Clay_Color color, char *value) {
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

void RenderColor(Clay_Color color) {
    uint8_t border_width = 5;
    uint8_t radius = 5;
    uint8_t size = 50;
    CLAY({
        .cornerRadius = CLAY_CORNER_RADIUS(radius),
        .backgroundColor = color,
        .border = {
            .color = TERNARY_COLOR(Clay_Hovered(), COLOR_SURFACE1, COLOR_SURFACE0),
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

Document documentsRaw[5];

DocumentArray documents = {
    .length = 5,
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
} MagickState;

typedef struct rgba {
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
    char w[6], h[6];  // max: 6 bytes for string repr
} resize_str_t;

// typedef struct ClayVideoDemo_Strings {
//
// } ClayVideoDemo_Strings;

typedef struct {
    uint8_t selectedDocumentIndex;
    float yOffset;
    ClayVideoDemo_Arena frameArena;
    bool shouldClose;
    uint16_t state;
    Nob_String_Builder outputFile;
    Nob_String_Builder tempDir;
    Nob_Cmd inputFiles;
    resize_t resize;
    resize_str_t resize_str;
    char cur_char[6]; // max: 6 bytes for string repr
    rgba color;
    rgba_str color_str;
    Nob_Proc magickProc;
    struct cthreads_thread magickThread;
    bool threadRunning;
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

void HandleSidebarInteraction(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData
) {
    (void) elementId;
    SidebarClickData *clickData = (SidebarClickData*)userData;
    // If this button was clicked
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (clickData->requestedDocumentIndex < documents.length) {
            if (clickData->requestedDocumentIndex == 0) {
                *clickData->state ^= MAGICK_BEST_FIT;
            } else if (clickData->requestedDocumentIndex == 1) {
                *clickData->state ^= MAGICK_TRANSPARENT_BG;
            } else if (clickData->requestedDocumentIndex == 2) {
                *clickData->state ^= MAGICK_OPEN_ON_DONE;
            } else if (clickData->requestedDocumentIndex == 3) {
                *clickData->state ^= MAGICK_RESIZE;
            }
            // Select the corresponding document
            *clickData->selectedDocumentIndex = clickData->requestedDocumentIndex;
        }
    }
}

void RenderMagickColor(ClayVideoDemo_Data *data) {
    if (data->state & MAGICK_TRANSPARENT_BG) {
        RenderColor((Clay_Color) { .r = 0, .g = 0, .b = 0, .a = 0 });
    } else {
        RenderColor((Clay_Color) { .r = data->color.r,
                                   .g = data->color.g,
                                   .b = data->color.b,
                                   .a = data->color.a });
    }
}

void RenderResize(ClayVideoDemo_Data *data) {
    CLAY({
        .backgroundColor = BUTTON_COLOR,
        .id = CLAY_ID("Resize"),
        .cornerRadius = BUTTON_RADIUS,
    }) {
        CLAY({ .id = CLAY_ID("ResizeW"), .layout = { .padding = { .left = 16, .top = 8, .bottom = 8 } } }) {
            CLAY_TEXT(CLAY_DYNAMIC_STRING(data->resize_str.w), BUTTON_TEXT);
        }
        CLAY({ .id = CLAY_ID("ResizeEach"), .layout = { .padding = {.top = 8, .bottom = 8} }}) { CLAY_TEXT(CLAY_STRING("x"), BUTTON_TEXT); }
        CLAY({ .id = CLAY_ID("ResizeH"), .layout = { .padding = { .right = 16, .top = 8, .bottom = 8 } } }) {
            CLAY_TEXT(CLAY_DYNAMIC_STRING(data->resize_str.h), BUTTON_TEXT);
        }
    }
}
void HandleFlagInteraction(
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

void RenderFlag(Clay_String text,
                uint16_t *state,
                uint16_t triggerFlag,
                uint16_t displayFlag,
                ClayVideoDemo_Arena *arena) {
    Clay_Color background = BUTTON_COLOR;
    if (*state & displayFlag)
        background = OPACITY(COLOR_GREEN, 35);
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
            .textColor = COLOR_TEXT,
            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
            .wrapMode = CLAY_TEXT_WRAP_NONE,
        }));
    }
}

ClayVideoDemo_Data ClayVideoDemo_Initialize() {
    // Update DocumentArray
    documents.documents[0] = (Document){ .title = CLAY_STRING("Best fit"), .contents = CLAY_STRING("This ashlar option aligns images on both sides of the resulting image") };
    documents.documents[1] = (Document){ .title = CLAY_STRING("Transparent background"), .contents = CLAY_STRING("Makes the background transparent") };
    documents.documents[2] = (Document){ .title = CLAY_STRING("Open when done"), .contents = CLAY_STRING("Enable this to see the result right after it's done!\n\nNothing more\nsurely") };
    documents.documents[3] = (Document){ .title = CLAY_STRING("Enable Resize"), .contents = CLAY_STRING("This option enables resizes") };
    documents.documents[ADVANCED_SETTINGS] = (Document){ .title = CLAY_STRING("Advanced settings"), .contents = CLAY_STRING("This section contains advanced settings") };

    ClayVideoDemo_Data data = {
        .frameArena = { .memory = (intptr_t)malloc(1024) },
        .shouldClose = false,
        .state = MAGICK_BEST_FIT | MAGICK_OPEN_ON_DONE | MAGICK_RESIZE | MAGICK_SHRINK_LARGER,
        .selectedDocumentIndex = ADVANCED_SETTINGS,
        .outputFile = {0},
        .tempDir = {0},
        .inputFiles = {0},
        .resize = {.w = 1000, .h = 1000},
        .resize_str = {.w = "1000", .h = "1000"},
        .color = { .r = 0, .b = 0, .g = 0, .a = 100 },
        // This should be set because these strings are only updated when color is updated
        .color_str = { .r = "0", .b = "0", .g = "0", .a = "100" },
        .magickProc = NOB_INVALID_PROC,
        .threadRunning = false,
        .magickThread = {0},
    };
    nob_sb_append_cstr(&data.outputFile, "res.png");
    return data;
}

Clay_RenderCommandArray ClayVideoDemo_CreateLayout(ClayVideoDemo_Data *data) {
    data->frameArena.offset = 0;

    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };

    Clay_Color contentBackgroundColor = COLOR_BASE;

    // Build UI here
    CLAY({
        .id = CLAY_ID("OuterContainer"),
        .backgroundColor = COLOR_MANTLE,
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
                .padding = { 16, 16, 16, 16 },
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
                            .backgroundColor = COLOR_SURFACE0,
                            .cornerRadius = BUTTON_RADIUS,
                        }) {
                            // Render dropdown items here
                            RenderDropdownMenuItem(CLAY_STRING("Open result"));
                            Clay_String quit = CLAY_STRING("Quit");
                            CLAY({
                                .backgroundColor = COLOR_SURFACE0,
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
                                        CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_16, .fontSize = button_font_size, .textColor = COLOR_OVERLAY0 }));
                            }
                        }
                    }
                }
            }
            RenderHeaderButton(CLAY_STRING("Select Images"));
            if (data->magickProc == NOB_INVALID_PROC)
                RenderHeaderButton(CLAY_STRING("Run"));
            else
                RenderHeaderButton(CLAY_STRING("Stop"));
            CLAY({ .layout = { .sizing = { CLAY_SIZING_GROW(0) }}}) {}
            CLAY({.id = CLAY_ID("file")}) {CLAY_TEXT(CLAY_SB_STRING(data->outputFile), DEFAULT_TEXT);}
            CLAY({ .layout = { .sizing = { CLAY_SIZING_GROW(0) }}}) {}
            CLAY({
                .layout = { .padding = { 16, 16, 8, 8 }},
                .backgroundColor = BUTTON_COLOR,
                .id = CLAY_ID("ResizeAll"),
                .cornerRadius = BUTTON_RADIUS,
            }) {
                CLAY_TEXT(CLAY_STRING("Resize: "), BUTTON_TEXT);
                CLAY_TEXT(CLAY_DYNAMIC_STRING(data->resize_str.w), BUTTON_TEXT);
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
                    *clickData = (SidebarClickData) { .requestedDocumentIndex = i, .selectedDocumentIndex = &data->selectedDocumentIndex, .state = &data->state };
                    data->frameArena.offset += sizeof(SidebarClickData);
                    if (
                        (i == 0 && data->state & MAGICK_BEST_FIT) ||
                        (i == 1 && data->state & MAGICK_TRANSPARENT_BG) ||
                        (i == 2 && data->state & MAGICK_OPEN_ON_DONE) ||
                        (i == 3 && data->state & MAGICK_RESIZE)
                       ) {
                        CLAY({
                            .layout = sidebarButtonLayout,
                            .backgroundColor = OPACITY(COLOR_GREEN, 35),
                            .cornerRadius = CLAY_CORNER_RADIUS(8)
                        }) {
                            Clay_OnHover(HandleSidebarInteraction, (intptr_t)clickData);
                            CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({
                                .fontId = FONT_ID_BODY_16,
                                .fontSize = sidebar_font_size,
                                .textColor = COLOR_TEXT,
                            }));
                        }
                    } else {
                        CLAY({ .layout = sidebarButtonLayout, .backgroundColor = BUTTON_COLOR, .cornerRadius = CLAY_CORNER_RADIUS(8) }) {
                            Clay_OnHover(HandleSidebarInteraction, (intptr_t)clickData);
                            CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({
                                        .fontId = FONT_ID_BODY_16,
                                        .fontSize = sidebar_font_size,
                                        .textColor = COLOR_TEXT,
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
                    .textColor = COLOR_TEXT
                }));
                CLAY_TEXT(selectedDocument.contents, DEFAULT_TEXT);
                if (data->selectedDocumentIndex == ADVANCED_SETTINGS) {
                    CLAY({
                        .id = CLAY_ID("ColorSettings"),
                        .layout = {
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    }) {
                        CLAY({.layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM }}) {
                            CLAY_TEXT(CLAY_STRING("Background Color: "), DEFAULT_TEXT);
                            RenderMagickColor(data);
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
                            RenderColorChannel(CLAY_STRING("r"), COLOR_RED,   data->color_str.r);
                            RenderColorChannel(CLAY_STRING("g"), COLOR_GREEN, data->color_str.g);
                            RenderColorChannel(CLAY_STRING("b"), COLOR_BLUE,  data->color_str.b);
                            RenderColorChannel(CLAY_STRING("a"), COLOR_TEXT,  data->color_str.a);
                        }
                        if (data->state & MAGICK_TRANSPARENT_BG)
                            CLAY_TEXT(CLAY_STRING("Transparent background setting overrides this option"), DEFAULT_TEXT);
                    }
                    CLAY({.id = CLAY_ID("ResizeSettings"), .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM, .childGap = 8}}) {
                        CLAY({.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 8}}) {
                            CLAY_TEXT(CLAY_STRING("Resize: "), DEFAULT_TEXT);
                            RenderResize(data);
                        }
                        RenderFlag(CLAY_STRING("Ignore aspect ratio"), &data->state, MAGICK_IGNORE_RATIO, MAGICK_IGNORE_RATIO, &data->frameArena);
                        // Well... it would only work for two
                        RenderFlag(CLAY_STRING("Only Shrink Larger"), &data->state,
                                (data->state & MAGICK_ENLARGE_SMALLER) ? MAGICK_ENLARGE_SMALLER | MAGICK_SHRINK_LARGER : MAGICK_SHRINK_LARGER, MAGICK_SHRINK_LARGER, &data->frameArena);
                        RenderFlag(CLAY_STRING("Only Enlarge Smaller"), &data->state,
                                (data->state & MAGICK_SHRINK_LARGER) ? MAGICK_SHRINK_LARGER | MAGICK_ENLARGE_SMALLER : MAGICK_ENLARGE_SMALLER, MAGICK_ENLARGE_SMALLER, &data->frameArena);
                        RenderFlag(CLAY_STRING("Fill area"), &data->state, MAGICK_FILL_AREA, MAGICK_FILL_AREA, &data->frameArena);
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
                            if (data->tempDir.count == 0)
                                CLAY_TEXT(CLAY_STRING("default"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_16, .fontSize = button_font_size, .textColor = COLOR_OVERLAY0 }));
                            else
                                CLAY_TEXT(CLAY_SB_STRING(data->tempDir), BUTTON_TEXT);
                        }
                        RenderHeaderButton(CLAY_STRING(SELECT_TEMP));
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
