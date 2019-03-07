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
#ifndef __JOLT_LVGL_GUI_H__
#define __JOLT_LVGL_GUI_H__

#include "lvgl/lvgl.h"
#include "lv_conf.h"
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

#include "menus/home.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "hal/hw_monitor.h"
#include "esp_log.h"
#include "jolttypes.h"
#include "bipmnemonic.h"

#include "jelfloader.h"
#include "mp.h"
#include "lang/lang.h"


#ifndef CONFIG_JOLT_GUI_LOADING_BUF_SIZE
    #define CONFIG_JOLT_GUI_LOADING_BUF_SIZE 30
#endif

/**********************
 *   GLOBAL VARIABLES
 **********************/

extern lv_theme_t *jolt_gui_theme;

#define FOREACH_JOLT_GUI_OBJ_ID(x) \
    x(JOLT_GUI_OBJ_ID_UNINITIALIZED)    /**< If you don't set a free_num, then it's 0 */ \
    x(JOLT_GUI_OBJ_ID_CONT_TITLE)       /**< container or page holding title */ \
    x(JOLT_GUI_OBJ_ID_CONT_BODY)        /**< container or page holding body */ \
    x(JOLT_GUI_OBJ_ID_BACK)             /**< The inivisible back button */ \
    x(JOLT_GUI_OBJ_ID_ENTER)            /**< The invisible enter button */ \
    x(JOLT_GUI_OBJ_ID_PAGE)             /**< page */ \
    x(JOLT_GUI_OBJ_ID_LABEL_TITLE)      /**< Title Label */ \
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
    x(JOLT_GUI_OBJ_ID_MAX)


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

/**
 * @brief Identifiers so we can quickly transverse an object that makes up a screen.
 */
enum {
    FOREACH_JOLT_GUI_OBJ_ID(GENERATE_ENUM)
};
typedef uint8_t jolt_gui_obj_id_t;

#define FOREACH_JOLT_GUI_SCR_ID(x) \
    x(JOLT_GUI_SCR_ID_UNINITIALIZED)   /**<  */ \
    x(JOLT_GUI_SCR_ID_MENU)            /**<  */ \
    x(JOLT_GUI_SCR_ID_SCROLL)          /**<  */ \
    x(JOLT_GUI_SCR_ID_DIGIT_ENTRY)     /**<  */ \
    x(JOLT_GUI_SCR_ID_LOADINGBAR)      /**<  */ \
    x(JOLT_GUI_SCR_ID_PRELOADING)      /**<  */ \
    x(JOLT_GUI_SCR_ID_MAX)             /**<  */

/**
 * @brief Identifiers so we can quickly identify a screen type
 */
enum {
    FOREACH_JOLT_GUI_SCR_ID(GENERATE_ENUM)
};
typedef uint8_t jolt_gui_scr_id_t;

/*********************
 * Screen Management *
 *********************/
/**
 * @brief Deletes the currently focused screen.
 *
 * Internally it gets the currently focused object, and recursively finds the
 * parent, then deletes that parent.
 */
lv_res_t jolt_gui_scr_del();

/**************************************
 * STANDARD SCREEN CREATION FUNCTIONS *
 **************************************/

/**
 * @brief Creates a dummy parent object
 *
 * A single dummy parent object makes screens easier to delete.
 *
 * @return object
 */
lv_obj_t *jolt_gui_obj_parent_create();

/**
 * @brief Creates a container for the drawable area of the screen
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
 * @param[in,out] parent dummy parent object
 * @param[in] title NULL-terminated title string. If NULL, a blank title is used. 
 * @return title label object
 */
lv_obj_t *jolt_gui_obj_title_create(lv_obj_t *parent, const char *title);

/**
 * @brief Wraps lv_obj_del in a JOLT_GUI_CTX 
 */
void jolt_gui_obj_del(lv_obj_t *obj);

/***************
 * Group Stuff *
 ***************/

/**
 * @brief Creates all the LVGL groups and registers the in-device 
 *
 * Must be run before calling jolt_gui_create();
 */
void jolt_gui_group_create();

/**
 * @brief Adds object to main group 
 * @param[in,out] obj object to add to the main group
 */
void jolt_gui_group_add( lv_obj_t *obj );

/**
 * @brief Get the main group handle 
 * @return group handle
 */
lv_group_t *jolt_gui_group_main_get();

/**
 * @brief Get the back group handle 
 * @return group handle
 */
lv_group_t *jolt_gui_group_back_get();

/**
 * @brief Get the enter group handle 
 * @return group handle
 */
lv_group_t *jolt_gui_group_enter_get();

/**********
 * Action *
 **********/

/**
 * @brief executes callback on back-button
 * @param[in,out] parent screen parent object
 * @param[in] cb callback to execute
 * @return back button object
 */
lv_obj_t *jolt_gui_scr_set_back_action(lv_obj_t *parent, lv_action_t cb);

/**
 * @brief executes callback on back-button
 * @param[in,out] parent screen parent object
 * @param[in] cb callback to execute
 * @return back button object
 */
lv_obj_t *jolt_gui_scr_set_enter_action(lv_obj_t *parent, lv_action_t cb);

/**
 * @brief pass an object to the back callback
 * @param[in,out] parent screen parent object
 * @param[in] object to pass
 */
void jolt_gui_scr_set_back_param(lv_obj_t *parent, void *param);

/**
 * @brief pass an object to the enter callback
 * @param[in,out] parent screen parent object
 * @param[in] object to pass
 */
void jolt_gui_scr_set_enter_param(lv_obj_t *parent, void *param);

/**
 * @brief Send an "Enter" signal to the main group.
 * @param[in] dummy; unused; only here for signature consistency
 * @return LV_RES_OK
 */
lv_res_t jolt_gui_send_enter_main(lv_obj_t *dummy);

/**
 * @brief Send a "Left" signal to the main group.
 * @param[in] dummy; unused; only here for signature consistency
 * @return LV_RES_OK
 */
lv_res_t jolt_gui_send_left_main(lv_obj_t *dummy);

/**
 * @brief Send an "Enter" signal to the back group.
 * @param[in] dummy; unused; only here for signature consistency
 * @return LV_RES_OK
 */
lv_res_t jolt_gui_send_enter_back(lv_obj_t *dummy);

/**
 * @brief Send an "Enter" signal to the enter group.
 * @param[in] dummy; unused; only here for signature consistency
 * @return LV_RES_OK
 */
lv_res_t jolt_gui_send_enter_enter(lv_obj_t *dummy);

/**
 * @brief alias for lv_obj_get_free_ptr
 */
static inline void *jolt_gui_get_param( lv_obj_t *obj ){
    return lv_obj_get_free_ptr( obj );
}

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
lv_obj_t *jolt_gui_find(lv_obj_t *parent, LV_OBJ_FREE_NUM_TYPE id);

/**
 * @brief Convert the enumerated value to a constant string 
 * @param[in] obj ID value
 */
const char *jolt_gui_obj_id_str(jolt_gui_obj_id_t val);

/**
 * @brief Convert the enumerated value to a constant string 
 * @parami[in] screen ID value
 */
const char *jolt_gui_scr_id_str(jolt_gui_scr_id_t val);

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
    MPP_BREAK_HANDLER(3, ESP_LOGE(TAG, "JOLT_GUI_CTX break L%d", __LINE__)) \
    MPP_FINALLY(4, jolt_gui_sem_give() )

#define if_not(x) if(!(x))

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
        if( (label_title = jolt_gui_obj_title_create(parent, title)) \
                && (cont_body = jolt_gui_obj_cont_body_create( parent )) ) { \
            jolt_gui_group_add( parent ); \
        }\
        else{ \
            jolt_gui_sem_take(); \
            lv_obj_del( parent ); \
            jolt_gui_sem_give(); \
        } \
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
 * @brief Delete an lv_obj_t if pointer is non-NULL.
 */
#define LV_OBJ_DEL_SAFE(obj) { \
    void *x = obj; \
    if(NULL!=x) lv_obj_del(x); \
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


#endif
