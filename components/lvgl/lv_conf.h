/**
 * @file lv_conf.h
 *
 */

/*
 * COPY THIS FILE AS `lv_conf.h` NEXT TO the `lvgl` FOLDER
 */

#if ESP_PLATFORM

#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_attr.h"

#ifndef LV_CONF_H
#define LV_CONF_H
/* clang-format off */

/*====================
   Graphical settings
 *====================*/

/* Maximal horizontal and vertical resolution to support by the library.*/
#define LV_HOR_RES_MAX          (128)
#define LV_VER_RES_MAX          (64)

/* Color depth:
 * - 1:  1 byte per pixel
 * - 8:  RGB233
 * - 16: RGB565
 * - 32: ARGB8888
 */
#define LV_COLOR_DEPTH     1

/* Swap the 2 bytes of RGB565 color.
 * Useful if the display has a 8 bit interface (e.g. SPI)*/
#define LV_COLOR_16_SWAP   0

/* 1: Enable screen transparency.
 * Useful for OSD or other overlapping GUIs.
 * Requires `LV_COLOR_DEPTH = 32` colors and the screen's style should be modified: `style.body.opa = ...`*/
#define LV_COLOR_SCREEN_TRANSP    0

/*Images pixels with this color will not be drawn (with chroma keying)*/
#define LV_COLOR_TRANSP    LV_COLOR_LIME         /*LV_COLOR_LIME: pure green*/

/* Enable anti-aliasing (lines, and radiuses will be smoothed) */
#define LV_ANTIALIAS        0

/* Default display refresh period.
 * Can be changed in the display driver (`lv_disp_drv_t`).*/
#define LV_DISP_DEF_REFR_PERIOD      20      /*[ms]*/

/* Dot Per Inch: used to initialize default sizes.
 * E.g. a button with width = LV_DPI / 2 -> half inch wide
 * (Not so important, you can adjust it to modify default sizes and spaces)*/
#define LV_DPI              147     /*[px]*/

/*=========================
   Memory manager settings
 *=========================*/

/* LittelvGL's internal memory manager's settings.
 * The graphical objects and other related data are stored here. */

/* 1: use custom malloc/free, 0: use the built-in `lv_mem_alloc` and `lv_mem_free` */
#define LV_MEM_CUSTOM      1
#if LV_MEM_CUSTOM == 0
/* Size of the memory used by `lv_mem_alloc` in bytes (>= 2kB)*/
#  define LV_MEM_SIZE    (32U * 1024U)

/* Complier prefix for a big array declaration */
#  define LV_MEM_ATTR

/* Set an address for the memory pool instead of allocating it as an array.
 * Can be in external SRAM too. */
#  define LV_MEM_ADR          0

/* Automatically defrag. on free. Defrag. means joining the adjacent free cells. */
#  define LV_MEM_AUTO_DEFRAG  1
#else       /*LV_MEM_CUSTOM*/
#  define LV_MEM_CUSTOM_INCLUDE <stdlib.h>   /*Header for the dynamic memory function*/
#  define LV_MEM_CUSTOM_ALLOC   malloc       /*Wrapper to malloc*/
#  define LV_MEM_CUSTOM_FREE    free         /*Wrapper to free*/
#endif     /*LV_MEM_CUSTOM*/

/* Garbage Collector settings
 * Used if lvgl is binded to higher level language and the memory is managed by that language */
#define LV_ENABLE_GC 0
#if LV_ENABLE_GC != 0
#  define LV_GC_INCLUDE "gc.h"                           /*Include Garbage Collector related things*/
#  define LV_MEM_CUSTOM_REALLOC   your_realloc           /*Wrapper to realloc*/
#  define LV_MEM_CUSTOM_GET_SIZE  your_mem_get_size      /*Wrapper to lv_mem_get_size*/
#endif /* LV_ENABLE_GC */

/*=======================
   Input device settings
 *=======================*/

/* Input device default settings.
 * Can be changed in the Input device driver (`lv_indev_drv_t`)*/

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD          33

/* Drag threshold in pixels */
#define LV_INDEV_DEF_DRAG_LIMIT           10

/* Drag throw slow-down in [%]. Greater value -> faster slow-down */
#define LV_INDEV_DEF_DRAG_THROW           20

/* Long press time in milliseconds.
 * Time to send `LV_EVENT_LONG_PRESSSED`) */
#define LV_INDEV_DEF_LONG_PRESS_TIME      400

/* Repeated trigger period in long press [ms]
 * Time between `LV_EVENT_LONG_PRESSED_REPEAT */
#define LV_INDEV_DEF_LONG_PRESS_REP_TIME  50

/*==================
 * Feature usage
 *==================*/

/*1: Enable the Animations */
#define LV_USE_ANIMATION        1
#if LV_USE_ANIMATION
typedef void * lv_anim_user_data_t;
#endif

/* 1: Enable shadow drawing*/
#define LV_USE_SHADOW           0

/* 1: Enable object groups (for keyboard/encoder navigation) */
#define LV_USE_GROUP            1
#if LV_USE_GROUP
typedef void * lv_group_user_data_t;
#endif  /*LV_USE_GROUP*/

/* 1: Enable GPU interface*/
#define LV_USE_GPU              0

/* 1: Enable file system (might be required for images */
#define LV_USE_FILESYSTEM       0

/* 1: Enable indexed (palette) images */
#define LV_IMG_CF_INDEXED   1

/* 1: Enable alpha indexed images */
#define LV_IMG_CF_ALPHA     0

/*Declare the type of the user data of image decoder (can be e.g. `void *`, `int`, `struct`)*/
typedef void * lv_img_decoder_user_data_t;

/*1: Add a `user_data` to drivers and objects*/
#define LV_USE_USER_DATA 1

/*=====================
 *  Compiler settings
 *====================*/
/* Define a custom attribute to `lv_tick_inc` function */
#define LV_ATTRIBUTE_TICK_INC IRAM_ATTR

/* Define a custom attribute to `lv_task_handler` function */
#define LV_ATTRIBUTE_TASK_HANDLER IRAM_ATTR

/* With size optimization (-Os) the compiler might not align data to
 * 4 or 8 byte boundary. This alignment will be explicitly applied where needed.
 * E.g. __attribute__((aligned(4))) */
#define LV_ATTRIBUTE_MEM_ALIGN


/* 1: Variable length array is supported*/
#define LV_COMPILER_VLA_SUPPORTED            1

/* 1: Initialization with non constant values are supported */
#define LV_COMPILER_NON_CONST_INIT_SUPPORTED 1

/*===================
 *  HAL settings
 *==================*/

/* 1: use a custom tick source.
 * It removes the need to manually update the tick with `lv_tick_inc`) */
#define LV_TICK_CUSTOM     1
#if LV_TICK_CUSTOM == 1
#define LV_TICK_CUSTOM_INCLUDE  "esp_timer.h"       /*Header for the sys time function*/
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (esp_timer_get_time()/1000)     /*Expression evaluating to current systime in ms*/
#endif   /*LV_TICK_CUSTOM*/

typedef void * lv_disp_drv_user_data_t;             /*Type of user data in the display driver*/
typedef void * lv_indev_drv_user_data_t;            /*Type of user data in the input device driver*/

/*================
 * Log settings
 *===============*/

/*1: Enable the log module*/
#define LV_USE_LOG      1
#if LV_USE_LOG
/* How important log should be added:
 * LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
 * LV_LOG_LEVEL_INFO        Log important events
 * LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
 * LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
 */
#  define LV_LOG_LEVEL    LV_LOG_LEVEL_WARN

/* 1: Print the log with 'printf';
 * 0: user need to register a callback with `lv_log_register_print`*/
#  define LV_LOG_PRINTF   1
#endif  /*LV_USE_LOG*/

/*================
 *  THEME USAGE
 *================*/
#define LV_THEME_LIVE_UPDATE    0   /*1: Allow theme switching at run time. Uses 8..10 kB of RAM*/

#define LV_USE_THEME_TEMPL      0   /*Just for test*/
#define LV_USE_THEME_DEFAULT    0   /*Built mainly from the built-in styles. Consumes very few RAM*/
#define LV_USE_THEME_ALIEN      0   /*Dark futuristic theme*/
#define LV_USE_THEME_NIGHT      0   /*Dark elegant theme*/
#define LV_USE_THEME_MONO       0   /*Mono color theme for monochrome displays*/
#define LV_USE_THEME_MATERIAL   0   /*Flat theme with bold colors and light shadows*/
#define LV_USE_THEME_ZEN        0   /*Peaceful, mainly light theme */
#define LV_USE_THEME_NEMO       0   /*Water-like theme based on the movie "Finding Nemo"*/

/*==================
 *    FONT USAGE
 *===================*/

/* More info about fonts: https://docs.littlevgl.com/#Fonts
 * To enable a built-in font use 1,2,4 or 8 values
 * which will determine the bit-per-pixel. Higher value means smoother fonts */
#define LV_USE_FONT_DEJAVU_10              0
#define LV_USE_FONT_DEJAVU_10_LATIN_SUP    0
#define LV_USE_FONT_DEJAVU_10_CYRILLIC     0
#define LV_USE_FONT_SYMBOL_10              0

#define LV_USE_FONT_DEJAVU_20              0
#define LV_USE_FONT_DEJAVU_20_LATIN_SUP    0
#define LV_USE_FONT_DEJAVU_20_CYRILLIC     0
#define LV_USE_FONT_SYMBOL_20              0

#define LV_USE_FONT_DEJAVU_30              0
#define LV_USE_FONT_DEJAVU_30_LATIN_SUP    0
#define LV_USE_FONT_DEJAVU_30_CYRILLIC     0
#define LV_USE_FONT_SYMBOL_30              0

#define LV_USE_FONT_DEJAVU_40              0
#define LV_USE_FONT_DEJAVU_40_LATIN_SUP    0
#define LV_USE_FONT_DEJAVU_40_CYRILLIC     0
#define LV_USE_FONT_SYMBOL_40              0

#define LV_USE_FONT_MONOSPACE_8            1

/* Optionally declare your custom fonts here.
 * You can use these fonts as default font too
 * and they will be available globally. E.g.
 * #define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(my_font_1) \
 *                                LV_FONT_DECLARE(my_font_2) \
 */
#define LV_USE_FONT_CROX3HB_NUMERIC 1 /* Used for Pin Entry Roller Digits */
#define LV_USE_FONT_DEJAVU_40_NUMERIC 1 /* Used for Misc big numbers */
#define LV_USE_FONT_PIXELMIX_7 1 /* Default small font */

#if CONFIG_JOLT_LANG_ENGLISH_EN || CONFIG_JOLT_LANG_SPANISH_EN
    #define LV_USE_FONT_PIXELMIX_7_LATIN_SUP 1
#endif

#define LV_FONT_CUSTOM_DECLARE \
        LV_FONT_DECLARE(jolt_gui_symbols) \
        LV_FONT_DECLARE(lv_font_jolt_gui_symbols) \
        LV_FONT_DECLARE(lv_font_crox3hb_numeric) \
        LV_FONT_DECLARE(lv_font_dejavu_40_numeric) \
        LV_FONT_DECLARE(lv_font_pixelmix_7) \
        LV_FONT_DECLARE(lv_font_pixelmix_7_latin_sup)

#define LV_FONT_DEFAULT        &lv_font_pixelmix_7

/*=================
 *  Text settings
 *=================*/

/* Select a character encoding for strings.
 * Your IDE or editor should have the same character encoding
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ISO8859_1
 * */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

 /*Can break (wrap) texts on these chars*/
#define LV_TXT_BREAK_CHARS                  " ,.;:-"

/* If a character is at least this long, will break wherever "prettiest" */
#define LV_TXT_LINE_BREAK_LONG_LEN          12

/* Minimum number of characters of a word to put on a line before a break */
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN  3

/* Minimum number of characters of a word to put on a line after a break */
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/*===================
 *  LV_OBJ SETTINGS
 *==================*/

#define FOREACH_JOLT_GUI_OBJ_ID(x) \
    x(JOLT_GUI_OBJ_ID_UNINITIALIZED)    /**< If you don't set a free_num, then it's 0 */ \
    x(JOLT_GUI_OBJ_ID_INVALID)          /**< Error handling */ \
    x(JOLT_GUI_OBJ_ID_CONT_TITLE)       /**< container or page holding title */ \
    x(JOLT_GUI_OBJ_ID_CONT_BODY)        /**< container or page holding body */ \
    x(JOLT_GUI_OBJ_ID_BACK)             /**< The inivisible back button */ \
    x(JOLT_GUI_OBJ_ID_ENTER)            /**< The invisible enter button */ \
    x(JOLT_GUI_OBJ_ID_PAGE)             /**< page */ \
    x(JOLT_GUI_OBJ_ID_LABEL_0)          /**< Primary text */ \
    x(JOLT_GUI_OBJ_ID_LABEL_1)          /**< Secondary text */ \
    x(JOLT_GUI_OBJ_ID_LABEL_2)          /**< 3rd text */ \
    x(JOLT_GUI_OBJ_ID_LABEL_3)          /**< 4th text */ \
    x(JOLT_GUI_OBJ_ID_LABEL_4)          /**< 5th text */ \
    x(JOLT_GUI_OBJ_ID_LOADINGBAR)       /**< Loading Bar Object */ \
    x(JOLT_GUI_OBJ_ID_PRELOADING)       /**< Loading Bar Object */ \
    x(JOLT_GUI_OBJ_ID_IMG_QR)           /**< QR code object */ \
    x(JOLT_GUI_OBJ_ID_SLIDER)           /**<  */ \
    x(JOLT_GUI_OBJ_ID_LIST)             /**<  */ \
    x(JOLT_GUI_OBJ_ID_ROLLER)           /**<  */ \
    x(JOLT_GUI_OBJ_ID_DECIMAL_POINT)    /**<  */ \
    x(JOLT_GUI_OBJ_ID_DIGIT_ENTRY)      /**<  */ \
    x(JOLT_GUI_OBJ_ID_MAX)              /**<  */


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

/**
 * @brief Identifiers so we can quickly transverse an object that makes up a screen.
 */
enum { FOREACH_JOLT_GUI_OBJ_ID(GENERATE_ENUM) };
typedef uint8_t jolt_gui_obj_id_t;

#define FOREACH_JOLT_GUI_SCR_ID(x) \
    x(JOLT_GUI_SCR_ID_UNINITIALIZED)   /**<  */ \
    x(JOLT_GUI_SCR_ID_INVALID)          /**< Error handling */ \
    x(JOLT_GUI_SCR_ID_DIGIT_ENTRY)     /**<  */ \
    x(JOLT_GUI_SCR_ID_LOADINGBAR)      /**<  */ \
    x(JOLT_GUI_SCR_ID_MENU)            /**<  */ \
    x(JOLT_GUI_SCR_ID_PRELOADING)      /**<  */ \
    x(JOLT_GUI_SCR_ID_SCROLL)          /**<  */ \
    x(JOLT_GUI_SCR_ID_SLIDER)          /**<  */ \
    x(JOLT_GUI_SCR_ID_MAX)             /**<  */

/**
 * @brief Identifiers so we can quickly identify a screen type
 */
enum { FOREACH_JOLT_GUI_SCR_ID(GENERATE_ENUM) };
typedef uint8_t jolt_gui_scr_id_t;

/* Called whenever the event is LV_EVENT_SHORT_CLICKED */
struct _lv_obj_t;
typedef uint8_t (*lv_action_t)(struct _lv_obj_t * obj);

#define JOLT_GUI_USE_RESERVED 1 /* Use the `reserved` field of lv_obj_t. Reserved must be at least 6 bits. Saves potentially 4 bytes per lv_obj */

#if JOLT_GUI_USE_RESERVED
typedef struct {
    void *param;               /**< Free parameters. Consumer should only attach to the active object. */
} lv_obj_user_data_t;          /*Declare the type of the user data of object (can be e.g. `void *`, `int`, `struct`)*/
#else
typedef struct {
    void *param;               /**< Free parameters. Consumer should only attach to the active object. */
    uint8_t id:5;              /**< Either a screen id or an object id */
    uint8_t is_scr:1;          /**< 0=obj; 1=screen (parent object) */
} lv_obj_user_data_t;          /*Declare the type of the user data of object (can be e.g. `void *`, `int`, `struct`)*/
#endif

/*1: enable `lv_obj_realaign()` based on `lv_obj_align()` parameters*/
#define LV_USE_OBJ_REALIGN          0

/* Enable to make the object clickable on a larger area.
 * LV_EXT_CLICK_AREA_OFF or 0: Disable this feature
 * LV_EXT_CLICK_AREA_TINY: The extra area can be adjusted horizontally and vertically (0..255 px)
 * LV_EXT_CLICK_AREA_FULL: The extra area can be adjusted in all 4 directions (-32k..+32k px)
 */
#define LV_USE_EXT_CLICK_AREA  LV_EXT_CLICK_AREA_OFF

/*==================
 *  LV OBJ X USAGE
 *================*/
/*
 * Documentation of the object types: https://docs.littlevgl.com/#Object-types
 */

/*Arc (dependencies: -)*/
#define LV_USE_ARC      1

/*Bar (dependencies: -)*/
#define LV_USE_BAR      1

/*Button (dependencies: lv_cont*/
#define LV_USE_BTN      1
#if LV_USE_BTN != 0
/*Enable button-state animations - draw a circle on click (dependencies: LV_USE_ANIMATION)*/
#  define LV_BTN_INK_EFFECT   1
#endif

/*Button matrix (dependencies: -)*/
#define LV_USE_BTNM     0

/*Calendar (dependencies: -)*/
#define LV_USE_CALENDAR 0

/*Canvas (dependencies: lv_img)*/
#define LV_USE_CANVAS   0

/*Check box (dependencies: lv_btn, lv_label)*/
#define LV_USE_CB       0

/*Chart (dependencies: -)*/
#define LV_USE_CHART    0
#if LV_USE_CHART
#  define LV_CHART_AXIS_TICK_LABEL_MAX_LEN    20
#endif

/*Container (dependencies: -*/
#define LV_USE_CONT     1

/*Drop down list (dependencies: lv_page, lv_label, lv_symbol_def.h)*/
#define LV_USE_DDLIST    1
#if LV_USE_DDLIST != 0
/*Open and close default animation time [ms] (0: no animation)*/
#  define LV_DDLIST_DEF_ANIM_TIME     200
#endif

/*Gauge (dependencies:lv_bar, lv_lmeter)*/
#define LV_USE_GAUGE    0

/*Image (dependencies: lv_label*/
#define LV_USE_IMG      1

/*Image Button (dependencies: lv_btn*/
#define LV_USE_IMGBTN   0
#if LV_USE_IMGBTN
/*1: The imgbtn requires left, mid and right parts and the width can be set freely*/
#  define LV_IMGBTN_TILED 0
#endif

/*Keyboard (dependencies: lv_btnm)*/
#define LV_USE_KB       0

/*Label (dependencies: -*/
#define LV_USE_LABEL    1
#if LV_USE_LABEL != 0
/*Hor, or ver. scroll speed [px/sec] in 'LV_LABEL_LONG_ROLL/ROLL_CIRC' mode*/
#  define LV_LABEL_DEF_SCROLL_SPEED       25
#  define LV_LABEL_WAIT_CHAR_COUNT        4 /* Waiting period at beginning/end of animation cycle */
#  define LV_LABEL_TEXT_SEL               0 /*Enable selecting text of the label */
#endif

/*LED (dependencies: -)*/
#define LV_USE_LED      0

/*Line (dependencies: -*/
#define LV_USE_LINE     0

/*List (dependencies: lv_page, lv_btn, lv_label, (lv_img optionally for icons ))*/
#define LV_USE_LIST     1
#if LV_USE_LIST != 0
/*Default animation time of focusing to a list element [ms] (0: no animation)  */
#  define LV_LIST_DEF_ANIM_TIME  50
#endif

/*Line meter (dependencies: *;)*/
#define LV_USE_LMETER   0

/*Message box (dependencies: lv_rect, lv_btnm, lv_label)*/
#define LV_USE_MBOX     0

/*Page (dependencies: lv_cont)*/
#define LV_USE_PAGE     1

/*Preload (dependencies: lv_arc)*/
#define LV_USE_PRELOAD      1
#if LV_USE_PRELOAD != 0
#  define LV_PRELOAD_DEF_ARC_LENGTH   60      /*[deg]*/
#  define LV_PRELOAD_DEF_SPIN_TIME    1000    /*[ms]*/
#  define LV_PRELOAD_DEF_ANIM         LV_PRELOAD_TYPE_SPINNING_ARC
#endif

/*Roller (dependencies: lv_ddlist)*/
#define LV_USE_ROLLER    1
#if LV_USE_ROLLER != 0
/*Focus animation time [ms] (0: no animation)*/
#  define LV_ROLLER_DEF_ANIM_TIME     100

/*Number of extra "pages" when the roller is infinite*/
#  define LV_ROLLER_INF_PAGES         3
#endif

/*Slider (dependencies: lv_bar)*/
#define LV_USE_SLIDER    1

/*Spinbox (dependencies: lv_ta)*/
#define LV_USE_SPINBOX       0

/*Switch (dependencies: lv_slider)*/
#define LV_USE_SW       1

/*Text area (dependencies: lv_label, lv_page)*/
#define LV_USE_TA       0
#if LV_USE_TA != 0
#  define LV_TA_CURSOR_BLINK_TIME 400     /*ms*/
#  define LV_TA_PWD_SHOW_TIME     1500    /*ms*/
#endif

/*Table (dependencies: lv_label)*/
#define LV_USE_TABLE    0
#if LV_USE_TABLE
#  define LV_TABLE_COL_MAX    12
#endif

/*Tab (dependencies: lv_page, lv_btnm)*/
#define LV_USE_TABVIEW      0
#  if LV_USE_TABVIEW != 0
/*Time of slide animation [ms] (0: no animation)*/
#  define LV_TABVIEW_DEF_ANIM_TIME    300
#endif

/*Tileview (dependencies: lv_page) */
#define LV_USE_TILEVIEW     0
#if LV_USE_TILEVIEW
/*Time of slide animation [ms] (0: no animation)*/
#  define LV_TILEVIEW_DEF_ANIM_TIME   300
#endif

/*Window (dependencies: lv_cont, lv_btn, lv_label, lv_img, lv_page)*/
#define LV_USE_WIN      0

/*==================
 * Non-user section
 *==================*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)    /* Disable warnings for Visual Studio*/
#  define _CRT_SECURE_NO_WARNINGS
#endif

/*--END OF LV_CONF_H--*/

/*Be sure every define has a default value*/
#include "lvgl/src/lv_conf_checker.h"

#endif /*LV_CONF_H*/

#endif /*End of "Content enable"*/
