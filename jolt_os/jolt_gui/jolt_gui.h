#ifndef __JOLT_LVGL_GUI_H__
#define __JOLT_LVGL_GUI_H__

#include "lvgl/lvgl.h"
#include "lv_conf.h"
#include "jolt_gui_digit_entry.h"
#include "jolt_gui_first_boot.h"
#include "jolt_gui_loading.h"
#include "jolt_gui_menu.h"
#include "jolt_gui_pin.h"
#include "jolt_gui_qr.h"
#include "jolt_gui_scroll.h"
#include "jolt_gui_slider.h"
#include "jolt_gui_statusbar.h"
#include "jolt_gui_stretch.h"
#include "jolt_gui_symbols.h"
#include "jolt_gui_text.h"
#include "jolt_gui_debug.h"
#include "jolt_gui_theme.h"
#include "jolt_gui_bignum.h"
#include "jolt_gui_err.h"

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

/* This whole struct should be moved into globals */
struct {
    bool first_boot;
    struct {
        lv_group_t *main; // Parent group for user input
        lv_group_t *back; // Group used to handle back button
        lv_group_t *enter;
    } group;
    struct {
        CONFIDENTIAL char passphrase[BM_PASSPHRASE_BUF_LEN]; //currently not active
        CONFIDENTIAL uint256_t pin;
        CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];
        CONFIDENTIAL uint256_t mnemonic_bin;
        CONFIDENTIAL uint512_t master_seed;
    } derivation; // used for message passing; do not keep data here for long.
} jolt_gui_store;

extern lv_theme_t *jolt_gui_theme;

/* If you add more values here, also add them to jolt_gui_obj_id_str */
typedef enum {
    JOLT_GUI_OBJ_ID_UNINITIALIZED = 0,    // If you don't set a free_num, then it's 0
    JOLT_GUI_OBJ_ID_CONT_TITLE,       // container or page holding title
    JOLT_GUI_OBJ_ID_CONT_BODY,        // container or page holding body
    JOLT_GUI_OBJ_ID_BACK,             // The inivisible back button
    JOLT_GUI_OBJ_ID_ENTER,            // The invisible enter button
    JOLT_GUI_OBJ_ID_PAGE,             //
    JOLT_GUI_OBJ_ID_LABEL_TITLE,      // Title Label
    JOLT_GUI_OBJ_ID_LABEL_0,          // Primary text
    JOLT_GUI_OBJ_ID_LABEL_1,          // Secondary text
    JOLT_GUI_OBJ_ID_LABEL_2,          // 3rd text
    JOLT_GUI_OBJ_ID_LABEL_3,          // 4th text
    JOLT_GUI_OBJ_ID_LABEL_4,          // 5th text
    JOLT_GUI_OBJ_ID_BAR_LOADING,      // Loading Bar Object
    JOLT_GUI_OBJ_ID_PRELOADING,       // Loading Bar Object
    JOLT_GUI_OBJ_ID_IMG_QR,           // QR code object
    JOLT_GUI_OBJ_ID_SLIDER,
    JOLT_GUI_OBJ_ID_LIST,
    JOLT_GUI_OBJ_ID_ROLLER,
    JOLT_GUI_OBJ_ID_DECIMAL_POINT,
} jolt_gui_obj_id_t;

typedef enum {
    JOLT_GUI_SCR_ID_UNINITIALIZED = 0,
    JOLT_GUI_SCR_ID_MENU = 0x80000000, /* To avoid collisions with jolt_gui_obj_id_t */
    JOLT_GUI_SCR_ID_SCROLL,
    JOLT_GUI_SCR_ID_DIGIT_ENTRY,
    JOLT_GUI_SCR_ID_LOADING,
} jolt_gui_scr_id_t;

/*********************
 * Screen Management *
 *********************/
/* Deletes the container of the currently focused object */
lv_res_t jolt_gui_scr_del();

/**************************************
 * STANDARD SCREEN CREATION FUNCTIONS *
 **************************************/
/* These functions for the base to create new screen types */

/* Creates a parent object for a new screen thats easy to delete */
lv_obj_t *jolt_gui_obj_parent_create();

/* Creates the body container */ 
lv_obj_t *jolt_gui_obj_cont_body_create( lv_obj_t *scr );

/* Creates a title in the top left statusbar. 
 * Allocates and copies the title string. */
lv_obj_t *jolt_gui_obj_title_create(lv_obj_t *parent, const char *title);

/* Wraps lv_obj_del in a JOLT_GUI_CTX */
void jolt_gui_obj_del(lv_obj_t *obj);

/***************
 * Group Stuff *
 ***************/

/* Run before running jolt_gui_create();
 * Creates all the groups and registers the in-device to the groups */
void jolt_gui_group_create();

/* Adds object to main group */
void jolt_gui_group_add( lv_obj_t *obj );

/**********
 * Action *
 **********/

/* Calls cb whenever the back button is pressed */
lv_obj_t *jolt_gui_scr_set_back_action(lv_obj_t *parent, lv_action_t cb);
lv_obj_t *jolt_gui_scr_set_enter_action(lv_obj_t *parent, lv_action_t cb);

lv_res_t jolt_gui_send_enter_main(lv_obj_t *dummy);
lv_res_t jolt_gui_send_left_main(lv_obj_t *dummy);

/*****************
 * System Events *
 *****************/
#include "esp_gap_ble_api.h"
void jolt_gui_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);


/********
 * MISC *
 ********/
void jolt_gui_sem_take();
void jolt_gui_sem_give();

/* Finds the first child object with the free_num identifier */
lv_obj_t *jolt_gui_find(lv_obj_t *parent, LV_OBJ_FREE_NUM_TYPE id);

/* Convert the enumerated value to a constant string */
const char *jolt_gui_obj_id_str(jolt_gui_obj_id_t val);

/********************
 * Meta Programming *
 ********************/
/* Wraps user code-block with recursive mutex take/give.
 * NEVER call return or goto within the context. break is fine.*/
#define JOLT_GUI_CTX \
    MPP_BEFORE(1, jolt_gui_sem_take() ) \
    MPP_DO_WHILE(2, false) \
    MPP_BREAK_HANDLER(3, ESP_LOGE(TAG, "JOLT_GUI_CTX break L%d", __LINE__)) \
    MPP_FINALLY(4, jolt_gui_sem_give() )

#define if_not(x) if(!(x))

/**********
 * Macros *
 **********/
/* To be used in a JOLT_GUI_CTX; breaks if passed in value is NULL */
#define BREAK_IF_NULL( obj ) ({\
        void *x = obj; \
        if( NULL == x ) break; \
        x; \
        })

/* Declares all the must have objects in a Jolt Screen.
 * If failure, all objects will be NULL.
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

/* Finds the first child of the provided type. If it cannot be found, break.
 * To be called in a JOLT_GUI_CTX*/
#define JOLT_GUI_FIND_AND_CHECK( obj, type ) ({ \
    lv_obj_t *child = jolt_gui_find(obj, type); \
    if( NULL == child ) { \
        break; \
    } \
    child; \
    })
//ESP_LOGE(TAG, "%s L%d: Could not find a child of type %s", __FILE__, __LINE__, jolt_gui_obj_id_str(type)); 

/* have to assign obj to a variable, otherwise if obj is a function it will 
 * call it multiple times */
#define LV_OBJ_DEL_SAFE(obj) { \
    void *x = obj; \
    if(NULL!=x) lv_obj_del(x); \
    }

/* Similar to a JOLT_GUI_CTX, but will call JOLT_GUI_SCR_PREAMBLE before the
 * JOLT_GUI_CTX. Also, if use breaks from JOLT_GUI_SCR_CTX, this will delete 
 * the parent object and set it to NULL. */
#define JOLT_GUI_SCR_CTX(title) \
    JOLT_GUI_SCR_PREAMBLE( title ) \
    MPP_BEFORE(1, jolt_gui_sem_take() ) \
    MPP_DO_WHILE(2, false) \
    MPP_BREAK_HANDLER(3, if(parent) {lv_obj_del(parent); parent=NULL;})\
    MPP_FINALLY(4, jolt_gui_sem_give() )


#endif
