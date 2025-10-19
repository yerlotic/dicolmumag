#ifndef STRINGS_C
#define STRINGS_C

#include <stdint.h>
#include "thirdparty/clay.h"

// ID for elements
#define ID_COLOR_SETTINGS "ColorSettings"
#define ID_COLOR_SETTINGS_RGB "ColorSettingsRGB"
#define ID_ERROR "error"
#define ID_FILE_BUTTON "FileButton"
#define ID_FILE_MENU "FileMenu"
#define ID_GRAVITY "Gravity"
#define ID_GRAVITY_SELECTION "GravitySelection"
#define ID_GRAVITY_SETTINGS "GravitySettings"
#define ID_HEADER "HeaderBar"
#define ID_INPUT_FILE "file"
#define ID_LOCATION_SETTINGS "LocationSettings"
#define ID_LOWER_CONTENT "LowerContent"
#define ID_MAIN "MainContent"
#define ID_OUTER "OuterContainer"
#define ID_OUTPUT_SETTINGS "OutputSettings"
#define ID_QUIT "Quit"
#define ID_RESIZE_ALL "ResizeAll"
#define ID_RESIZE_INPUT "Resize"
#define ID_RESIZE_OUTPUT "OutputDimentions"
#define ID_RESIZE_OUTPUT_MARGIN "OutputMargin"
#define ID_RESIZE_SETTINGS "ResizeSettings"
#define ID_SIDEBAR "Sidebar"
#define ID_WELCOME "MagickWelcome"

// Buttons
#define BUTTON_CHANGE_UI_COLOR "Change colorscheme"
#define BUTTON_FILE "File"
#define BUTTON_OPEN_RESULT "Open result"
#define BUTTON_RUN "Run"
#define BUTTON_SELECT_IMAGES "Select Images"
#define BUTTON_SELECT_MAGICK "MagickBinary"
#define BUTTON_STOP "Stop"
#define BUTTON_SUPPORT "Support"

// Text
#define TEXT_TRANSPARENT_BG_WARNING "Transparent background setting overrides this option"
#define TEXT_IGNORE_ASPECT "Ignore aspect ratio"
#define TEXT_SHRINK_LARGER "Only Shrink Larger"
#define TEXT_ENLARGE_SMALLER "Only Enlarge Smaller"
#define TEXT_FILL_AREA "Fill area"
#define TEXT_DIMENSIONS "Dimentions:"
#define TEXT_MARGIN "Margin:"
#define TEXT_CURRENT "Current:"
#define TEXT_WELCOME "Welcome to Dicolmumag"
#define TEXT_SLOGAN "Create collages cuz why not"
#define TEXT_BEST_FIT "Best fit"
#define TEXT_BEST_FIT_EXPL "This ashlar option aligns images on both sides of the resulting image"
#define TEXT_TRANSPARENT_BG "Transparent background"
#define TEXT_TRANSPARENT_BG_EXPL "Makes the background transparent\nThis overrides background configuration in "ADVANCED_SETTINGS_Q" tab"
#define TEXT_OPEN_ON_DONE "Open when done"
#define TEXT_OPEN_ON_DONE_EXPL "Enable this to see the result right after it's done!\n\nNothing more\nsurely"
#define TEXT_ENABLE_RESIZE "Enable Resize"
#define TEXT_ENABLE_RESIZE_EXPL "This option enables resizes. You can configure how input images are resized in "ADVANCED_SETTINGS_Q
#define TEXT_SET_OUTPUT_RES "Set output resolution"
#define TEXT_SET_OUTPUT_RES_EXPL "With this option you can directly set the desired output resolution for the collage in "ADVANCED_SETTINGS_Q" tab\nIf this option is disabled, the resolution for the output image (filename at the top) will be chosen automatically"
#define TEXT_ADVANCED_SETTINGS_EXPL "This tab contains advanced settings (surprise!)"
#define TEXT_TEMP_FILES "Temporary files"
#define TEXT_TEMP_DEFAULT "default"

// Sections
#define SECTION_RESIZE "Resize: "
#define SECTION_BACKGROUND_COLOR "Background Color: "
#define SECTION_GRAVITY "Gravity: "
#define SECTION_RESIZE_EACH "Resize each image: "

// Misc
#define ADVANCED_SETTINGS_S "Advanced Settings"
#define ADVANCED_SETTINGS_Q "\"Advanced Settings\""

#define TEXT_OUTPUT_RES "Output resolution"

#define SELECT_TEMP "Change location"
#define TEMP_FILES_EXPLANATION "When changed, it uses available storage as memory. This option allows to work around memory issues but makes everything slower"

#define MAGICK_EXEC "Magick executable:"
#define START_USING "Start using!"

typedef CLAY_PACKED_ENUM {
    MAGICK_ERROR_OK,
    MAGICK_ERROR_CANCELLED,
    MAGICK_ERROR_NOT_WORK,
    MAGICK_ERROR_INVALID_BINARY_SELECTED,
    MAGICK_ERROR_NO_FILES,
    MAGICK_ERROR_PROCESS_CRASHED,
    MAGICK_ERROR_PROCESS_TERMINATED,
} MagickStatus;

const char* errors[] = {
    "",
    "",
    "Magick binary does not work",
    "Invalid binary selected",
    "Select some images",
    "Magick crashed!",
    "Magick was terminated",
    "OS thinks stuff is VERY wrong",
};

#endif // STRINGS_C

