#ifndef STRINGS_C
#define STRINGS_C

#include <stdint.h>
#include "thirdparty/clay.h"

uint8_t language = 0;

// Prefix: AS - AppString
CLAY_PACKED_ENUM {
    // Buttons
    AS_BUTTON_FILE = 0,
    AS_BUTTON_OPEN_RESULT,
    AS_BUTTON_CHANGE_UI_COLOR,
    AS_BUTTON_CHANGE_LANGUAGE,
    AS_BUTTON_QUIT,
    AS_BUTTON_RUN,
    AS_BUTTON_SELECT_IMAGES,
    AS_BUTTON_STOP,
    AS_BUTTON_SUPPORT,

    // Text
    AS_TEXT_TRANSPARENT_BG_WARNING,
    AS_TEXT_IGNORE_ASPECT,
    AS_TEXT_SHRINK_LARGER,
    AS_TEXT_ENLARGE_SMALLER,
    AS_TEXT_FILL_AREA,
    AS_TEXT_DIMENSIONS,
    AS_TEXT_MARGIN,
    AS_TEXT_CURRENT,
    AS_TEXT_WELCOME,
    AS_TEXT_SLOGAN,
    AS_TEXT_BEST_FIT,
    AS_TEXT_BEST_FIT_EXPL,
    AS_TEXT_TRANSPARENT_BG,
    AS_TEXT_TRANSPARENT_BG_EXPL,
    AS_TEXT_OPEN_ON_DONE,
    AS_TEXT_OPEN_ON_DONE_EXPL,
    AS_TEXT_ENABLE_RESIZE,
    AS_TEXT_ENABLE_RESIZE_EXPL,
    AS_TEXT_SET_OUTPUT_RES,
    AS_TEXT_SET_OUTPUT_RES_EXPL,
    AS_TEXT_ADVANCED_SETTINGS_EXPL,
    AS_TEXT_TEMP_FILES,
    AS_TEXT_TEMP_FILES_LOCATION,
    AS_TEXT_TEMP_DEFAULT,
    AS_TEXT_OUTPUT_PATH,
    AS_TEXT_INPUT_IMAGES,
    AS_TEXT_IMAGE_FILES,
    AS_TEXT_MAGICK_PATH,
    AS_TEXT_EXE_FILES,

    // Sections
    AS_SECTION_RESIZE,
    AS_SECTION_BACKGROUND_COLOR,
    AS_SECTION_GRAVITY,
    AS_SECTION_RESIZE_EACH,

    // Errors
    AS_MAGICK_ERROR_OK,
    AS_MAGICK_ERROR_CANCELLED,
    AS_MAGICK_ERROR_RUNNING,
    AS_MAGICK_ERROR_NOT_WORK,
    AS_MAGICK_ERROR_INVALID_BINARY_SELECTED,
    AS_MAGICK_ERROR_NO_FILES,
    AS_MAGICK_ERROR_PROCESS_CRASHED,
    AS_MAGICK_ERROR_PROCESS_TERMINATED,
    AS_MAGICK_ERROR_OS_BULLSHIT,

    // Tips
    AS_MAGICK_TIP_SCROLL,
    AS_MAGICK_TIP_COLOR_PICKER,

    // Misc
    AS_ADVANCED_SETTINGS_S,
    AS_TEXT_OUTPUT_RES,
    AS_SELECT_TEMP,
    AS_TEMP_FILES_EXPLANATION,
    AS_MAGICK_EXEC,
    AS_START_USING,

    AS_END
};

#define APP_LANGUAGES 2
#define APP_STRINGS AS_END + 1

#define APP_STRING(AS_INDEX, STRING) [AS_INDEX] = CLAY_STRING(STRING)


static const Clay_String strings[APP_LANGUAGES][APP_STRINGS] = {
    {
#define ADVANCED_SETTINGS_S "Advanced Settings"
#define ADVANCED_SETTINGS_Q "\"Advanced Settings\""

        // Buttons
        APP_STRING(AS_BUTTON_FILE, "File"),
        APP_STRING(AS_BUTTON_OPEN_RESULT, "Open result"),
        APP_STRING(AS_BUTTON_CHANGE_UI_COLOR, "Change colorscheme"),
        APP_STRING(AS_BUTTON_CHANGE_LANGUAGE, "Change language"),
        APP_STRING(AS_BUTTON_QUIT, "Quit"),
        APP_STRING(AS_BUTTON_RUN, "Run"),
        APP_STRING(AS_BUTTON_SELECT_IMAGES, "Select Images"),
        APP_STRING(AS_BUTTON_STOP, "Stop"),
        APP_STRING(AS_BUTTON_SUPPORT, "Support"),

        // Text
        APP_STRING(AS_TEXT_TRANSPARENT_BG_WARNING, "Transparent background setting overrides this option"),
        APP_STRING(AS_TEXT_IGNORE_ASPECT, "Ignore aspect ratio"),
        APP_STRING(AS_TEXT_SHRINK_LARGER, "Only Shrink Larger"),
        APP_STRING(AS_TEXT_ENLARGE_SMALLER, "Only Enlarge Smaller"),
        APP_STRING(AS_TEXT_FILL_AREA, "Fill area"),
        APP_STRING(AS_TEXT_DIMENSIONS, "Dimentions:"),
        APP_STRING(AS_TEXT_MARGIN, "Margin:"),
        APP_STRING(AS_TEXT_CURRENT, "Current:"),
        APP_STRING(AS_TEXT_WELCOME, "Welcome to Dicolmumag"),
        APP_STRING(AS_TEXT_SLOGAN, "Create collages cuz why not"),
        APP_STRING(AS_TEXT_BEST_FIT, "Best fit"),
        APP_STRING(AS_TEXT_BEST_FIT_EXPL, "This ashlar option aligns images on both sides of the resulting image"),
        APP_STRING(AS_TEXT_TRANSPARENT_BG, "Transparent background"),
        APP_STRING(AS_TEXT_TRANSPARENT_BG_EXPL, "Makes the background transparent\nThis overrides background configuration in "ADVANCED_SETTINGS_Q"tab"),
        APP_STRING(AS_TEXT_OPEN_ON_DONE, "Open when done"),
        APP_STRING(AS_TEXT_OPEN_ON_DONE_EXPL, "Enable this to see the result right after it's done!\n\nNothing more\nsurely"),
        APP_STRING(AS_TEXT_ENABLE_RESIZE, "Enable Resize"),
        APP_STRING(AS_TEXT_ENABLE_RESIZE_EXPL, "This option enables resizes. You can configure how input images are resized in "ADVANCED_SETTINGS_Q),
        APP_STRING(AS_TEXT_SET_OUTPUT_RES, "Set output resolution"),
        APP_STRING(AS_TEXT_SET_OUTPUT_RES_EXPL, "With this option you can directly set the desired output resolution for the collage in "ADVANCED_SETTINGS_Q" tab\nIf this option is disabled, the resolution for the output image (filename at the top) will be chosen automatically"),
        APP_STRING(AS_TEXT_ADVANCED_SETTINGS_EXPL, "This tab contains advanced settings (surprise!)"),
        APP_STRING(AS_TEXT_TEMP_FILES, "Temporary files"),
        APP_STRING(AS_TEXT_TEMP_DEFAULT, "default"),
        APP_STRING(AS_TEXT_OUTPUT_PATH, "Path to output image"),
        APP_STRING(AS_TEXT_INPUT_IMAGES, "Source images"),
        APP_STRING(AS_TEXT_IMAGE_FILES, "Image files"),
        APP_STRING(AS_TEXT_MAGICK_PATH, "Path to magick executable"),
        APP_STRING(AS_TEXT_EXE_FILES, "Executable files"),

        // Sections
        APP_STRING(AS_SECTION_RESIZE, "Resize: "),
        APP_STRING(AS_SECTION_BACKGROUND_COLOR, "Background Color: "),
        APP_STRING(AS_SECTION_GRAVITY, "Gravity: "),
        APP_STRING(AS_SECTION_RESIZE_EACH, "Resize each image: "),

        // Errors
        APP_STRING(AS_MAGICK_ERROR_OK, ""),
        APP_STRING(AS_MAGICK_ERROR_CANCELLED, ""),
        APP_STRING(AS_MAGICK_ERROR_RUNNING, "Running..."),
        APP_STRING(AS_MAGICK_ERROR_NOT_WORK, "Magick binary does not work"),
        APP_STRING(AS_MAGICK_ERROR_INVALID_BINARY_SELECTED, "Invalid binary selected"),
        APP_STRING(AS_MAGICK_ERROR_NO_FILES, "Select some images"),
        APP_STRING(AS_MAGICK_ERROR_PROCESS_CRASHED, "Magick crashed!"),
        APP_STRING(AS_MAGICK_ERROR_PROCESS_TERMINATED, "Magick was terminated"),
        APP_STRING(AS_MAGICK_ERROR_OS_BULLSHIT, "OS thinks stuff is VERY wrong"),

        // Tips
        APP_STRING(AS_MAGICK_TIP_SCROLL, "Scroll resize on \"x\" to change both values!"),
        APP_STRING(AS_MAGICK_TIP_COLOR_PICKER, "Click on the color in "ADVANCED_SETTINGS_S" to launch the color picker"),

        // Misc
        APP_STRING(AS_ADVANCED_SETTINGS_S, ADVANCED_SETTINGS_S),
        APP_STRING(AS_TEXT_OUTPUT_RES, "Output resolution"),

        APP_STRING(AS_SELECT_TEMP, "Change location"),
        APP_STRING(AS_TEMP_FILES_EXPLANATION, "When changed, it uses available storage as memory. This option allows to work around memory issues but makes everything slower"),

        APP_STRING(AS_MAGICK_EXEC, "Magick executable:"),
        APP_STRING(AS_START_USING, "Start using!"),
    },
    {
#undef ADVANCED_SETTINGS_S
#define ADVANCED_SETTINGS_S "Расширенные настройки"
#undef ADVANCED_SETTINGS_Q
#define ADVANCED_SETTINGS_Q "\"Расширенные настройки\""

        // Buttons
        APP_STRING(AS_BUTTON_FILE, "Файл"),
        APP_STRING(AS_BUTTON_OPEN_RESULT, "Открыть результат"),
        APP_STRING(AS_BUTTON_CHANGE_UI_COLOR, "Поменять оформление"),
        APP_STRING(AS_BUTTON_CHANGE_LANGUAGE, "Поменять язык"),
        APP_STRING(AS_BUTTON_QUIT, "Выйти"),
        APP_STRING(AS_BUTTON_RUN, "Собрать"),
        APP_STRING(AS_BUTTON_SELECT_IMAGES, "Выбрать"),
        APP_STRING(AS_BUTTON_STOP, "Стоп"),
        APP_STRING(AS_BUTTON_SUPPORT, "Помогите"),

        // Text
        APP_STRING(AS_TEXT_TRANSPARENT_BG_WARNING, "Настройка \"Прозрачный фон\" выключает точную настройку цвета"),
        APP_STRING(AS_TEXT_IGNORE_ASPECT, "Игнорировать изначальное соотношение сторон"),
        APP_STRING(AS_TEXT_SHRINK_LARGER, "Сжимать большие"),
        APP_STRING(AS_TEXT_ENLARGE_SMALLER, "Увеличивать меньшие"),
        APP_STRING(AS_TEXT_FILL_AREA, "Заполнять площадь"),
        APP_STRING(AS_TEXT_DIMENSIONS, "Разрешение:"),
        APP_STRING(AS_TEXT_MARGIN, "Отступы:"),
        APP_STRING(AS_TEXT_WELCOME, "Привет от Dicolmumag!"),
        APP_STRING(AS_TEXT_SLOGAN, "Создавайте коллажи, потому что почему нет?"),
        APP_STRING(AS_TEXT_BEST_FIT, "Прилипание к краям"),
        APP_STRING(AS_TEXT_BEST_FIT_EXPL, "Эта настройка включает точное прилипание мозаики к краям коллажа"),
        APP_STRING(AS_TEXT_TRANSPARENT_BG, "Прозрачный фон"),
        APP_STRING(AS_TEXT_TRANSPARENT_BG_EXPL, "Делает фон прозрачным\nЭта настройка переопределяет настройки цвета во вкладке "ADVANCED_SETTINGS_Q),
        APP_STRING(AS_TEXT_OPEN_ON_DONE, "Открыть по завершении"),
        APP_STRING(AS_TEXT_OPEN_ON_DONE_EXPL, "Включите, чтобы увидеть результат сразу как он готов!\n\nБольше ничего\nуж точно"),
        APP_STRING(AS_TEXT_ENABLE_RESIZE, "Включить масштабирование"),
        APP_STRING(AS_TEXT_ENABLE_RESIZE_EXPL, "Эта настройка включает масштабирование. Вы можете настроить как изменяется размер картинок в \"Расширенных настройках\""),
        APP_STRING(AS_TEXT_SET_OUTPUT_RES, "Установить итоговое разрешение"),
        APP_STRING(AS_TEXT_SET_OUTPUT_RES_EXPL, "С этой настройках можно установить разрешение для коллажа во вкладке "ADVANCED_SETTINGS_Q"\nЕсли эта настройка выключена, то разрешение для итогового коллажа (имя файла наверху) будет выбрано автоматически"),
        APP_STRING(AS_TEXT_ADVANCED_SETTINGS_EXPL, "В этой вкладке расширенные настройки (сюрприз!)"),
        APP_STRING(AS_TEXT_TEMP_FILES, "Расположение промежуточных файлов"),
        APP_STRING(AS_TEXT_CURRENT, "Текущее:"),
        APP_STRING(AS_TEXT_TEMP_DEFAULT, "по умолчанию"),
        APP_STRING(AS_TEXT_OUTPUT_PATH, "Путь для сохранения коллажа"),
        APP_STRING(AS_TEXT_INPUT_IMAGES, "Исходные изображения"),
        APP_STRING(AS_TEXT_IMAGE_FILES, "Изображения"),
        APP_STRING(AS_TEXT_MAGICK_PATH, "Исполняемый файл magick"),
        APP_STRING(AS_TEXT_EXE_FILES, "Исполняемые файлы"),

        // Sections
        APP_STRING(AS_SECTION_RESIZE, "Масштабирование: "),
        APP_STRING(AS_SECTION_BACKGROUND_COLOR, "Цвет фона: "),
        APP_STRING(AS_SECTION_GRAVITY, "Гравитация: "),
        APP_STRING(AS_SECTION_RESIZE_EACH, "Масштабирование исходных картинок: "),

        // Errors
        APP_STRING(AS_MAGICK_ERROR_OK, ""),
        APP_STRING(AS_MAGICK_ERROR_CANCELLED, ""),
        APP_STRING(AS_MAGICK_ERROR_RUNNING, "Идёт сборка..."),
        APP_STRING(AS_MAGICK_ERROR_NOT_WORK, "Исполняемый файл Magick не работает"),
        APP_STRING(AS_MAGICK_ERROR_INVALID_BINARY_SELECTED, "Выбран неправильный исполняемый файл"),
        APP_STRING(AS_MAGICK_ERROR_NO_FILES, "Выберите изображения"),
        APP_STRING(AS_MAGICK_ERROR_PROCESS_CRASHED, "Ошибка выполнения!"),
        APP_STRING(AS_MAGICK_ERROR_PROCESS_TERMINATED, "Magick был остановлен"),
        APP_STRING(AS_MAGICK_ERROR_OS_BULLSHIT, "ОС думает, что что-то ОЧЕНЬ сильно сломалось"),

        // Tips
        APP_STRING(AS_MAGICK_TIP_SCROLL, "Если скроллить на \"x\", то оба значения будут меняться"),
        APP_STRING(AS_MAGICK_TIP_COLOR_PICKER, "Нажмите на цвет в Расширенных настройках, чтобы открыть выбор цвета"),

        // Misc
        // Misc
        APP_STRING(AS_ADVANCED_SETTINGS_S, ADVANCED_SETTINGS_S),
        APP_STRING(AS_TEXT_OUTPUT_RES, "Итоговое разрешение"),

        APP_STRING(AS_SELECT_TEMP, "Поменять расположение"),
        APP_STRING(AS_TEMP_FILES_EXPLANATION, "Когда изменено, будет использоваться место на диске вместо оперативной памяти. Позволяет обойти проблемы с нехваткой оперативной памяти, но значительно замедляет процесс сборки"),

        APP_STRING(AS_MAGICK_EXEC, "Исполняемый файл ImageMagick:"),
        APP_STRING(AS_START_USING, "Начнём!"),
    },
};

// ID for elements
#define ID_ERROR "error"
#define ID_FILE_BUTTON "FileButton"
#define ID_FILE_MENU "FileMenu"
#define ID_INPUT_FILE "file"
#define ID_QUIT "Quit"
#define ID_GRAVITY_SELECTION "GravitySelection"
#define ID_RESIZE_INPUT "Resize"
#define ID_RESIZE_OUTPUT "OutputDimentions"
#define ID_RESIZE_OUTPUT_MARGIN "OutputMargin"
#define ID_BUTTON_SELECT_MAGICK "MagickBinary"

typedef CLAY_PACKED_ENUM {
    MAGICK_ERROR_OK = 0,
    MAGICK_ERROR_CANCELLED,
    MAGICK_ERROR_RUNNING,
    MAGICK_ERROR_NOT_WORK,
    MAGICK_ERROR_INVALID_BINARY_SELECTED,
    MAGICK_ERROR_NO_FILES,
    MAGICK_ERROR_PROCESS_CRASHED,
    MAGICK_ERROR_PROCESS_TERMINATED,
    MAGICK_ERROR_OS_BULLSHIT,
} MagickStatus;

int errors[] = {
    [MAGICK_ERROR_OK] = AS_MAGICK_ERROR_OK,
    [MAGICK_ERROR_CANCELLED] = AS_MAGICK_ERROR_CANCELLED,
    [MAGICK_ERROR_RUNNING] = AS_MAGICK_ERROR_RUNNING,
    [MAGICK_ERROR_NOT_WORK] = AS_MAGICK_ERROR_NOT_WORK,
    [MAGICK_ERROR_INVALID_BINARY_SELECTED] = AS_MAGICK_ERROR_INVALID_BINARY_SELECTED,
    [MAGICK_ERROR_NO_FILES] = AS_MAGICK_ERROR_NO_FILES,
    [MAGICK_ERROR_PROCESS_CRASHED] = AS_MAGICK_ERROR_PROCESS_CRASHED,
    [MAGICK_ERROR_PROCESS_TERMINATED] = AS_MAGICK_ERROR_PROCESS_TERMINATED,
    [MAGICK_ERROR_OS_BULLSHIT] = AS_MAGICK_ERROR_OS_BULLSHIT,
};

typedef CLAY_PACKED_ENUM {
    MAGICK_TIP_SCROLL = 0,
    MAGICK_TIP_COLOR_PICKER,
    MAGICK_TIP_END
} MagickTips;
#define APP_TIPS MAGICK_TIP_END

int tips[APP_TIPS] = {
    [MAGICK_TIP_SCROLL] = AS_MAGICK_TIP_SCROLL,
    [MAGICK_TIP_COLOR_PICKER] = AS_MAGICK_TIP_COLOR_PICKER,
};

// Internationalization
#define i18n(TEXT_ID) strings[language][TEXT_ID]
#endif // STRINGS_C
