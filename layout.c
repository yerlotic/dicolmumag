#include "clay.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "colors.h"

const int FONT_ID_BODY_16 = 0;
const int FONT_LOAD_SIZE = 40;
const int button_font_size = 20;
const int sidebar_font_size = 23;
const int document_font_size = 30;
const int title_font_size = 40;

#define BUTTON_RADIUS CLAY_CORNER_RADIUS(10)
#define LAYOUT_RADIUS CLAY_CORNER_RADIUS(10)

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
    .length = 3,
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
} MagickState;

typedef struct {
    uint32_t selectedDocumentIndex;
    float yOffset;
    ClayVideoDemo_Arena frameArena;
    bool shouldClose;
    int state;
    char* resultFile;
    uint16_t resize;  // max: 6 bytes for string repr
    char resize_str[6]; // max: 6 bytes for string repr
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
            }
            // Select the corresponding document
            *clickData->selectedDocumentIndex = clickData->requestedDocumentIndex;
        }
    }
}

ClayVideoDemo_Data ClayVideoDemo_Initialize() {
    documents.documents[0] = (Document){ .title = CLAY_STRING("Best fit"), .contents = CLAY_STRING("This ashlar option aligns images on both sides of the resulting image") };
    documents.documents[1] = (Document){ .title = CLAY_STRING("Transparent background"), .contents = CLAY_STRING("Makes the background transparent") };
    documents.documents[2] = (Document){ .title = CLAY_STRING("Open when done"), .contents = CLAY_STRING("Enable this to see the result right after it's done!\n\nNothing more\nsurely") };
    // documents.documents[3] = (Document){ .title = CLAY_STRING("Article 4"), .contents = CLAY_STRING("Article 4") };
    // documents.documents[4] = (Document){ .title = CLAY_STRING("Article 5"), .contents = CLAY_STRING("Article 5") };

    ClayVideoDemo_Data data = {
        .frameArena = { .memory = (intptr_t)malloc(1024) },
        .shouldClose = false,
        .state = MAGICK_BEST_FIT | MAGICK_TRANSPARENT_BG | MAGICK_OPEN_ON_DONE,
        .selectedDocumentIndex = 0,
        .resultFile = "",
        .resize_str = "",
        .resize = 1000,
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
        // .backgroundColor = { 43, 41, 51, 255 },
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
                Clay_String clay_str = (Clay_String) {
                    .length = strlen(data->resize_str),
                    .chars = data->resize_str,
                    .isStaticallyAllocated = true,
                    // .isStaticallyAllocated = false,
                };
                CLAY_TEXT(clay_str, CLAY_TEXT_CONFIG({
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
                        .width = CLAY_SIZING_FIXED(250),
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
                    if ((i == 0 && data->state & MAGICK_BEST_FIT) ||
                        (i == 1 && data->state & MAGICK_TRANSPARENT_BG) ||
                        (i == 2 && data->state & MAGICK_OPEN_ON_DONE)) {
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
                        CLAY({ .layout = sidebarButtonLayout, .backgroundColor = TOGGLE_COLOR, .cornerRadius = CLAY_CORNER_RADIUS(8) }) {
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
                CLAY_TEXT(selectedDocument.contents, CLAY_TEXT_CONFIG({
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = document_font_size,
                    .textColor = COLOR_TEXT
                }));
            }
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    for (int32_t i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
    }
    return renderCommands;
}
