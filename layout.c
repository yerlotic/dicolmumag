#include "clay.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// #define LATTE
#include "colors.h"

const int FONT_ID_BODY_16 = 0;
const int FONT_LOAD_SIZE = 40;
const int button_font_size = 20;
const int sidebar_font_size = 23;
const int document_font_size = 30;
const int title_font_size = 40;

#define ADVANCED_SETTINGS 4

#define CLAY_DYNAMIC_STRING(string) (CLAY__INIT(Clay_String) { .isStaticallyAllocated = false, .length = strlen(string), .chars = (string) })
#define BUTTON_RADIUS CLAY_CORNER_RADIUS(10)
#define LAYOUT_RADIUS CLAY_CORNER_RADIUS(10)
#define DEFAULT_TEXT CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY_16, .fontSize = document_font_size, .textColor = COLOR_TEXT })

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
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = button_font_size,
            .textColor = COLOR_TEXT,
        }));
    }
}

void RenderNumberPicker(Clay_String name, char* value) {
    CLAY({
        .layout = { .padding = { 16, 16, 8, 8 }},
        .backgroundColor = BUTTON_COLOR,
        // .id = CLAY_SID(name),
        .cornerRadius = BUTTON_RADIUS,
    }) {
        CLAY_TEXT(CLAY_DYNAMIC_STRING(value), CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = button_font_size,
            .textColor = COLOR_TEXT,
        }));
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
        RenderNumberPicker(CLAY_STRING("pick"), value);
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
    uint32_t length;
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

enum {
    MAGICK_BEST_FIT       = 1 << 0,
    MAGICK_TRANSPARENT_BG = 1 << 1,
    MAGICK_OPEN_ON_DONE   = 1 << 2,
    MAGICK_RESIZE         = 1 << 3,
} MagickState;

typedef struct rgba {
    uint8_t r, g, b, a;
} rgba;

typedef struct rgba_str {
    // 3 for digits [0-255], 1 for \0
    char r[4], g[4], b[4], a[4];
} rgba_str;
// typedef struct ClayVideoDemo_Strings {
//
// } ClayVideoDemo_Strings;

typedef struct {
    uint32_t selectedDocumentIndex;
    float yOffset;
    ClayVideoDemo_Arena frameArena;
    bool shouldClose;
    int state;
    char* resultFile;
    uint16_t resize;  // max: 6 bytes for string repr
    char resize_str[6]; // max: 6 bytes for string repr
    char cur_char[6]; // max: 6 bytes for string repr
    rgba color;
    rgba_str color_str;
} ClayVideoDemo_Data;


typedef struct {
    uint32_t requestedDocumentIndex;
    uint32_t* selectedDocumentIndex;
    int* state;
} SidebarClickData;

void HandleSidebarInteraction(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData
) {
    SidebarClickData *clickData = (SidebarClickData*)userData;
    // If this button was clicked
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (clickData->requestedDocumentIndex < documents.length) {
            if (clickData->requestedDocumentIndex == 0) {
                *clickData->state = *clickData->state ^ MAGICK_BEST_FIT;
            } else if (clickData->requestedDocumentIndex == 1) {
                *clickData->state = (*clickData->state ^ MAGICK_TRANSPARENT_BG);
            } else if (clickData->requestedDocumentIndex == 2) {
                *clickData->state = (*clickData->state ^ MAGICK_OPEN_ON_DONE);
            } else if (clickData->requestedDocumentIndex == 3) {
                *clickData->state = (*clickData->state ^ MAGICK_RESIZE);
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
        .state = MAGICK_BEST_FIT | MAGICK_TRANSPARENT_BG | MAGICK_OPEN_ON_DONE | MAGICK_RESIZE,
        .selectedDocumentIndex = 0,
        .resultFile = "res.png",
        .resize = 1000,
        .resize_str = "1000",
        .color = { .r = 0, .b = 0, .g = 0, .a = 100 },
        // This should be set because these strings are only updated when color is updated
        .color_str = { .r = "0", .b = "0", .g = "0", .a = "100" },
    };
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
                    .x = CLAY_ALIGN_X_CENTER,
                }
            },
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
                CLAY_TEXT(CLAY_STRING("File"), CLAY_TEXT_CONFIG({
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = button_font_size,
                    .textColor = COLOR_TEXT,
                }));

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
                            RenderDropdownMenuItem(CLAY_STRING("Quit"));
                        }
                    }
                }
            }
            RenderHeaderButton(CLAY_STRING("Select_Images"));
            CLAY({ .layout = { .sizing = { CLAY_SIZING_GROW(0) }}}) {}
            CLAY_TEXT(CLAY_DYNAMIC_STRING(data->resultFile), DEFAULT_TEXT);
            CLAY({ .layout = { .sizing = { CLAY_SIZING_GROW(0) }}}) {}
            CLAY({
                .layout = { .padding = { 16, 16, 8, 8 }},
                .backgroundColor = BUTTON_COLOR,
                .id = CLAY_ID("Resize"),
                .cornerRadius = BUTTON_RADIUS,
            }) {
                CLAY_TEXT(CLAY_STRING("Resize: "), CLAY_TEXT_CONFIG({
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = button_font_size,
                    .textColor = COLOR_TEXT,
                }));
                CLAY_TEXT(CLAY_DYNAMIC_STRING(data->resize_str), CLAY_TEXT_CONFIG({
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = button_font_size,
                    .textColor = COLOR_TEXT,
                }));
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
                for (uint32_t i = 0; i < documents.length; i++) {
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
                .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() },
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .childGap = 16,
                    .padding = CLAY_PADDING_ALL(16),
                    .sizing = layoutExpand
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
                        .backgroundColor = contentBackgroundColor,
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
                            .backgroundColor = contentBackgroundColor,
                            .layout = {
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(105),
                                },
                                .padding = CLAY_PADDING_ALL(10),
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
