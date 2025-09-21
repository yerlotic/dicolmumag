#ifndef STRINGS_C
#define STRINGS_C

#include <stdint.h>

#define ADVANCED_SETTINGS_S "Advanced Settings"
#define ADVANCED_SETTINGS_Q "\"Advanced Settings\""

#define SELECT_GRAVITY "Change gravity"

#define OUTPUT_RES "Output resolution"

#define SELECT_TEMP "Change location"
#define TEMP_FILES "Temporary files"
#define TEMP_FILES_EXPLANATION "When changed, it uses available storage as memory. This option allows to work around memory issues but makes everything slower"

#define MAGICK_EXEC "Magick executable:"

typedef enum MagickStatus : uint8_t {
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
};

#endif // STRINGS_C

