/**
 * @file jolt_gui.h
 * @brief Functions used to build up the GUI
 *
 * JoltOS heavily uses the open-source graphics library LVGL. 
 * 
 * To create parts of a GUI, you should only include jolt_gui.h
 *
 * All jolt_gui functions are protected by a recursive-mutex, so you can call
 * them anywhere. If you are directly using LVGL functions, you MUST wrap your
 * code in a JOLT_GUI_CTX which will automatically obtain and release the mutex. 
 *
 * @author Brian Pugh
 */
#ifndef JOLT_GUI_H__
#define JOLT_GUI_H__

#include "lvgl/lvgl.h"
#include "lv_conf.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "jolttypes.h"

#include "jelfloader.h"
#include "mp.h"
#include "lang/lang.h"

#include "jolt_helpers.h"

#ifndef CONFIG_JOLT_GUI_LOADING_BUF_SIZE
    #define CONFIG_JOLT_GUI_LOADING_BUF_SIZE 30
#endif

/**********************
 *   GLOBAL VARIABLES
 **********************/

extern lv_theme_t *jolt_gui_theme;

/********************
 * LVGL Abstraction *
 ********************/
// Insulate possible upstream LVGL changes from applications

/* Abstract away LVGL events and obj for apps */
#define JOLT_GUI_EVENT_BITFIELD_LEN 5
/* Only EVER append values to this struct */
typedef struct jolt_gui_event_enum_t{
    uint32_t pressed:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t pressing:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t press_list:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t short_clicked:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t long_pressed:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t long_pressed_repeat:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t clicked:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t released:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t drag_begin:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t drag_end:JOLT_GUI_EVENT_BITFIELD_LEN; 
    uint32_t drag_throw_begin:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t focused:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t defocused:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t value_changed:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t insert:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t selected:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t refresh:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t apply:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t cancel:JOLT_GUI_EVENT_BITFIELD_LEN;
    uint32_t delete:JOLT_GUI_EVENT_BITFIELD_LEN;
} jolt_gui_event_enum_t; /* Only used for constant mapping */
extern const jolt_gui_event_enum_t jolt_gui_event;
typedef uint8_t jolt_gui_event_t;
typedef lv_obj_t jolt_gui_obj_t;
typedef void (*jolt_gui_event_cb_t)(jolt_gui_obj_t *obj, jolt_gui_event_t event);

/*********************
 * Screen Management *
 *********************/

/**
 * Get the parenting screen for some object.
 * If object is NULL, return the currently active screen.
 *
 * @param obj object to get screen object of.
 * @return the parent screen object.
 */
jolt_gui_obj_t *jolt_gui_scr_get( jolt_gui_obj_t *obj );

/**
 * @brief Deletes the parenting screen of obj or currently focused screen.
 *
 * If no object is specificed, will delete currently focused screen.
 *
 * @param obj Object who's screen we want to delete. NULL for currently focused screen.
 */
lv_res_t jolt_gui_scr_del( jolt_gui_obj_t *obj );

/**
 * @brief Sets object id
 * @param[in] obj
 * @param[in] id
 */
void jolt_gui_obj_id_set( lv_obj_t *obj, jolt_gui_obj_id_t id );

/**
 * @brief Gets the ID of an object.
 * @param[in] obj Object to get ID of.
 * @return Object ID. Returns JOLT_GUI_OBJ_ID_INVALID if a screen.
 */
jolt_gui_obj_id_t jolt_gui_obj_id_get( const lv_obj_t *obj );

/**
 * @brief Sets screen id
 * @param[in] obj
 * @param[in] id
 */
void jolt_gui_scr_id_set( lv_obj_t *obj, jolt_gui_scr_id_t id );

/**
 * @brief Gets the screen type
 * @param[in]
 * @return Screen ID. JOLT_GUI_SCR_ID_INVALID if it's not a screen.
 */
jolt_gui_scr_id_t jolt_gui_scr_id_get( const lv_obj_t *obj );

/**
 * @brief Wraps lv_obj_del in a JOLT_GUI_CTX 
 */
void jolt_gui_obj_del(lv_obj_t *obj);


/**************************************
 * STANDARD SCREEN CREATION FUNCTIONS *
 **************************************/

/**
 * @brief Creates a dummy parent object
 *
 * A single dummy parent object makes screens easier to delete.
 *
 * Primarily for internal-use.
 *
 * @return object
 */
lv_obj_t *jolt_gui_obj_parent_create();

/**
 * @brief Creates a container for the drawable area of the screen
 *
 * Primarily for internal-use.
 *
 * @param[in,out] scr dummy parent object
 * @return container body
 */ 
lv_obj_t *jolt_gui_obj_cont_body_create( lv_obj_t *scr );

/**
 * @brief Creates a title in the top left statusbar. 
 *
 * LVGL internally allocates and copies the title string, so it doesn't need
 * to persist.
 *
 * Primarily for internal-use.
 *
 * @param[in,out] parent dummy parent object
 * @param[in] title NULL-terminated title string. If NULL, a blank title is used. 
 * @return title label object
 */
lv_obj_t *jolt_gui_obj_title_create(lv_obj_t *parent, const char *title);

/***************
 * Group Stuff *
 ***************/

/**
 * @brief Creates all the LVGL groups and registers the in-device 
 *
 * Must be run before calling jolt_gui_create();
 *
 * For internal-use only.
 */
void jolt_gui_group_create();

/**
 * @brief Adds object to group 
 * @param[in,out] obj object to add to the group
 */
void jolt_gui_group_add( lv_obj_t *obj );

/**
 * @brief Get the group handle 
 * @return group handle
 */
lv_group_t *jolt_gui_group_get();

/**********
 * Action *
 **********/
/**
 * @brief Get the active element of the screen.
 *
 * The active element is the object that part of the indev group and actually
 * takes action upon button input.
 *
 * @return Active object. Returns NULL if not found.
 */
lv_obj_t *jolt_gui_scr_get_active(lv_obj_t *parent);

/**
 * @brief Set the event callback for the active element of the screen
 */
void jolt_gui_scr_set_event_cb(lv_obj_t *parent, lv_event_cb_t event_cb);

/**
 * @brief Wrapper for lv_obj_set_event_cb
 */
void jolt_gui_obj_set_event_cb(lv_obj_t *obj, lv_event_cb_t event_cb);

/**
 * @brief Get the event param pointer in user_data
 */
void *jolt_gui_obj_get_param( lv_obj_t *obj );

/**
 * @brief set the event param pointer in user_data
 */
void jolt_gui_obj_set_param( lv_obj_t *obj, void *param );

/**
 * @brief set the event param pointer in the active element of the screen
 */
void jolt_gui_scr_set_active_param( lv_obj_t *parent, void *param );

/**
 * @brief get the event param pointer in the active element of the screen
 */
void *jolt_gui_scr_get_active_param( lv_obj_t *parent );

/**
 * @brief Event handler that deletes the current screen
 *
 * Used primarily as the default event handler for certain screen types.
 */
void jolt_gui_event_del( lv_obj_t *obj, lv_event_t event);

/*****************
 * System Events *
 *****************/
#if CONFIG_BT_ENABLED

#include "esp_gap_ble_api.h"

/**
 * @brief GUI-related callbacks that get tagged onto the BLE GAP Event loop
 */
void jolt_gui_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#endif

/********
 * MISC *
 ********/
/**
 * @brief Take the GUI semaphore
 */
void jolt_gui_sem_take();

/**
 * @brief Give the GUI semaphore
 */
void jolt_gui_sem_give();

/**
 * @brief Finds the first child object with the specified identifier
 * @param[in] parent Any LVGL object who's children we want to search
 * @param[in] id ID we are searching for.
 */
lv_obj_t *jolt_gui_find(const lv_obj_t *parent, jolt_gui_obj_id_t id);

/**
 * @brief Convert the enumerated value to a constant string 
 * @param[in] obj ID value
 */
const char *jolt_gui_obj_id_str(jolt_gui_obj_id_t val);

/**
 * @brief Convert the enumerated value to a constant string 
 * @param[in] screen ID value
 */
const char *jolt_gui_scr_id_str(jolt_gui_scr_id_t val);

/************
 * Wrappers *
 ************/
lv_obj_t *jolt_gui_obj_get_parent( const lv_obj_t *obj );

/********************
 * Meta Programming *
 ********************/
/**
 * @brief Threadsafe GUI object manipulation context.
 *
 * Wraps user code-block with recursive mutex take/give. NEVER call "return" or "goto" within the context. "break" will exit the context.*/
#define JOLT_GUI_CTX \
    MPP_BEFORE(1, jolt_gui_sem_take() ) \
    MPP_DO_WHILE(2, false) \
    MPP_BREAK_HANDLER(3, ESP_LOGE(TAG, "%s: JOLT_GUI_CTX break L%d", __func__, __LINE__)) \
    MPP_FINALLY(4, jolt_gui_sem_give() )

#define if_not(x) if(!(x))

/*****************
 * Configuration *
 *****************/
#define JOLT_GUI_FONT_BIG_NUM &modenine_40
#define JOLT_GUI_FONT_DIGIT_ENTRY &modenine_20
#define JOLT_GUI_FONT_DEFAULT &pixelmix

/**********
 * Macros *
 **********/
/**
 * @brief To be used in a JOLT_GUI_CTX; breaks if passed in value is NULL 
 */
#define BREAK_IF_NULL( obj ) ({\
        void *x = obj; \
        if( NULL == x ) break; \
        x; \
        })

/**
 * @brief Declares all the must have objects (parent, label_title, cont_body) in a Jolt Screen.
 * If failure, all objects will be NULL and deallocated.
 * Can be called inside or outside of a JOLT_GUI_CTX. Usually called outside
 * so that all the objects are exposed to be returned. */
#define JOLT_GUI_SCR_PREAMBLE( title ) \
    lv_obj_t *parent = NULL, *label_title = NULL, *cont_body = NULL; \
    if( (parent = jolt_gui_obj_parent_create()) ) { \
        if( !((label_title = jolt_gui_obj_title_create(parent, title)) \
                && (cont_body = jolt_gui_obj_cont_body_create( parent ))) ) { \
            jolt_gui_sem_take(); \
            lv_obj_del( parent ); \
            jolt_gui_sem_give(); \
        }\
    }

/**
 * @brief Finds the first child of the provided type.
 *
 * If it cannot be found, break.
 * Should be called within a JOLT_GUI_CTX
 */
#define JOLT_GUI_FIND_AND_CHECK( obj, type ) ({ \
    lv_obj_t *child = jolt_gui_find(obj, type); \
    if( NULL == child ) { \
        break; \
    } \
    child; \
    })

/**
 * @brief Delete an lv_obj_t if pointer is non-NULL. Sets the pointer to NULL after.
 */
#define JOLT_GUI_OBJ_DEL_SAFE(obj) { \
    if(NULL!=obj) { \
        jolt_gui_obj_del(obj); \
        obj = NULL; \
    } \
}

/**
 * @brief Similar to a JOLT_GUI_CTX, but will call JOLT_GUI_SCR_PREAMBLE before the
 * JOLT_GUI_CTX.
 *
 * If code breaks from JOLT_GUI_SCR_CTX, the parent object will be deleted and set to NULL.
 */
#define JOLT_GUI_SCR_CTX(title) \
    JOLT_GUI_SCR_PREAMBLE( title ) \
    MPP_BEFORE(1, jolt_gui_sem_take() ) \
    MPP_DO_WHILE(2, false) \
    MPP_BREAK_HANDLER(3, if(parent) {lv_obj_del(parent); parent=NULL;})\
    MPP_FINALLY(4, jolt_gui_sem_give() )

#include "jolt_gui_digit_entry.h"
#include "jolt_gui_first_boot.h"
#include "jolt_gui_loading.h"
#include "jolt_gui_menu.h"
#include "jolt_gui_qr.h"
#include "jolt_gui_scroll.h"
#include "jolt_gui_slider.h"
#include "jolt_gui_statusbar.h"
#include "jolt_gui_symbols.h"
#include "jolt_gui_text.h"
#include "jolt_gui_debug.h"
#include "jolt_gui_theme.h"
#include "jolt_gui_bignum.h"
#include "jolt_gui_err.h"
#include "jolt_gui_indev.h"
#include "jolt_gui_yesno.h"


#endif
