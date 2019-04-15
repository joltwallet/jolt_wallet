//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include "stdio.h"
#include "jolt_gui.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t *group;

/**********************
 *      MACROS
 **********************/
#if PC_SIMULATOR
    #define MSG(...) printf(__VA_ARGS__)
#elif ESP_PLATFORM
    #include "esp_log.h"
    static const char TAG[] = "jolt_gui";
    #define MSG(...) ESP_LOGI(TAG, __VA_ARGS__)
#else
    #define MSG(...) printf(__VA_ARGS__)
#endif

/*********************
 * Screen Management *
 *********************/

lv_res_t jolt_gui_scr_del() {
    lv_res_t res = LV_RES_OK;
    JOLT_GUI_CTX{
        lv_obj_t *scr = BREAK_IF_NULL(lv_group_get_focused(group));
        lv_obj_t *parent = scr;
        lv_obj_t *tmp = scr;
        while( (tmp = lv_obj_get_parent(tmp)) ) {
            if( tmp != lv_scr_act() ) {
                parent = tmp;
            }
        }
        ESP_LOGD(TAG, "Deleting screen %p", parent);
        jolt_gui_obj_del(parent);
        res = LV_RES_INV;
    }
    return res;
}

void jolt_gui_obj_id_set( lv_obj_t *obj, jolt_gui_obj_id_t id) {
    lv_obj_get_user_data( obj )->id = id;
    lv_obj_get_user_data( obj )->is_scr = 0;
} 

jolt_gui_obj_id_t jolt_gui_obj_id_get( const lv_obj_t *obj ) {
    if( lv_obj_get_user_data((lv_obj_t *)obj)->is_scr ){
        return JOLT_GUI_OBJ_ID_INVALID;
    }
    return lv_obj_get_user_data((lv_obj_t *)obj)->id;
}

void jolt_gui_scr_id_set( lv_obj_t *obj, jolt_gui_scr_id_t id) {
    lv_obj_get_user_data( obj )->id = id;
    lv_obj_get_user_data( obj )->is_scr = 1;
}

jolt_gui_scr_id_t jolt_gui_scr_id_get( const lv_obj_t *obj ) {
    if( !lv_obj_get_user_data((lv_obj_t *)obj)->is_scr ){
        return JOLT_GUI_SCR_ID_INVALID;
    }
    return lv_obj_get_user_data((lv_obj_t *)obj)->id;
}

/**************************************
 * STANDARD SCREEN CREATION FUNCTIONS *
 **************************************/

/* Creates a dummy invisible object to anchor lvgl objects on the screen */
lv_obj_t *jolt_gui_obj_parent_create() {
    lv_obj_t *parent = NULL;
    JOLT_GUI_CTX{
        parent = BREAK_IF_NULL(lv_obj_create(lv_scr_act(), NULL));
        lv_obj_set_size(parent, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_pos(parent,0,0);
        lv_obj_set_style(parent, &lv_style_transp);
    }
    return parent;
}

/* Creates the statusbar title label for a screen. Returns the 
 * label object. */
lv_obj_t *jolt_gui_obj_title_create(lv_obj_t *parent, const char *title) {
    if( NULL == parent ) {
        return NULL;
    }

    lv_obj_t *label = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *statusbar_label = statusbar_get_label();
        /* Create a non-transparent background to block out old titles */
        lv_obj_t *title_cont = BREAK_IF_NULL(lv_cont_create(parent, NULL));
        jolt_gui_obj_id_set(title_cont, JOLT_GUI_OBJ_ID_CONT_TITLE);
        lv_obj_align(title_cont, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
        lv_obj_set_size( title_cont,
                lv_obj_get_x(statusbar_label) - 1, // todo: use style padding
                CONFIG_JOLT_GUI_STATUSBAR_H-1);

        label = BREAK_IF_NULL(lv_label_create(title_cont, NULL));
        jolt_gui_obj_id_set(label, JOLT_GUI_OBJ_ID_LABEL_0);
        const lv_style_t *label_style = lv_obj_get_style(label);
        lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);
        lv_label_set_body_draw(label, false); // dont draw background
        lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
        if( NULL == title ){
            lv_label_set_text(label, "");
        }
        else{
            lv_label_set_text(label, title);
        }
        lv_obj_set_size(label, lv_obj_get_width(title_cont), label_style->text.font->h_px);
    }
    return label;
}

/* Creates the body container */ 
lv_obj_t *jolt_gui_obj_cont_body_create( lv_obj_t *parent ) {
    if( NULL == parent ) {
        return NULL;
    }

    lv_obj_t *cont = NULL;
    JOLT_GUI_CTX{
        cont = BREAK_IF_NULL(lv_cont_create(parent, NULL));
        jolt_gui_obj_id_set(cont, JOLT_GUI_OBJ_ID_CONT_BODY);
        lv_obj_set_size(cont, LV_HOR_RES, 
                LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H);
        lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_LEFT,
                0, CONFIG_JOLT_GUI_STATUSBAR_H);
    }
    return cont;
}

void jolt_gui_obj_del(lv_obj_t *obj){
    if( NULL == obj ) {
        return;
    }

    JOLT_GUI_CTX{
        lv_obj_del(obj);
    }
}

/***************
 * Group Stuff *
 ***************/
void jolt_gui_group_create() {
    /* Create Groups for user input */
    bool success = false;
    JOLT_GUI_CTX{
        lv_theme_t * th = lv_theme_get_current();
        group = BREAK_IF_NULL(lv_group_create());
        lv_group_set_refocus_policy(group, LV_GROUP_REFOCUS_POLICY_PREV);
        lv_group_set_style_mod_cb(group, th->group.style_mod);
        lv_group_set_style_mod_edit_cb(group, th->group.style_mod_edit);
        success = true;
    }
    if( !success ){
        esp_restart();
    }
}

void jolt_gui_group_add( lv_obj_t *obj ){
    JOLT_GUI_CTX{
        ESP_LOGD(TAG, "Adding %p to group", obj);
        lv_group_add_obj(group, obj);
        lv_group_focus_obj( obj );
    }
}

lv_group_t *jolt_gui_group_get() {
    return group;
}

/**********
 * Action *
 **********/

lv_obj_t *jolt_gui_scr_get_active(lv_obj_t *parent) {
    lv_obj_t *obj = NULL;
    JOLT_GUI_CTX{
        /* Find the BODY CONT */
        lv_obj_t *cont_body = JOLT_GUI_FIND_AND_CHECK( parent, JOLT_GUI_OBJ_ID_CONT_BODY );

        /* Find the child thats in the group */
        lv_obj_t *child = NULL;
        while( NULL != (child = lv_obj_get_child(cont_body, child)) ) {
            if( group == lv_obj_get_group(child) ) {
                obj = child;
                break;
            }
        }
    }
    if( NULL == obj ) {
        ESP_LOGW(TAG, "Couldn't find screen's active object.");
    }
    else {
        ESP_LOGD(TAG, "Active object: %p", obj);
    }
    return obj;
}

void jolt_gui_scr_set_event_cb(lv_obj_t *parent, lv_event_cb_t event_cb) {
    JOLT_GUI_CTX{
        lv_obj_t *active = jolt_gui_scr_get_active(parent);
        if( NULL != active ) {
            jolt_gui_obj_set_event_cb(active, event_cb);
        }
    }
}

void jolt_gui_scr_set_active_param( lv_obj_t *parent, void *param ) {
    JOLT_GUI_CTX{
        lv_obj_t *active = jolt_gui_scr_get_active(parent);
        if( NULL != active ) {
            jolt_gui_obj_set_param(active, param);
        }
    }
}

void *jolt_gui_scr_get_active_param( lv_obj_t *parent ) {
    void *param = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *active = jolt_gui_scr_get_active(parent);
        if( NULL != active ) {
            param = jolt_gui_obj_get_param( active );
        }
    }
    return param;
}

void jolt_gui_event_del( lv_obj_t *obj, lv_event_t event) {
    ESP_LOGD(TAG, "%s: event %d", __func__, event);
    if( LV_EVENT_CANCEL == event ) {
        jolt_gui_scr_del();
        ESP_LOGD(TAG, "Screen Deleted");
    }
}

/********
 * MISC *
 ********/
static SemaphoreHandle_t jolt_gui_mutex = NULL;

void jolt_gui_sem_take() {
    if( NULL == jolt_gui_mutex ){
        /* Create the jolt_gui_mutex; avoids need to explicitly initialize */
        jolt_gui_mutex = xSemaphoreCreateRecursiveMutex();
        if( NULL == jolt_gui_mutex ){
            esp_restart();
        }
    }
    xSemaphoreTakeRecursive( jolt_gui_mutex, portMAX_DELAY );
}

void jolt_gui_sem_give() {
    xSemaphoreGiveRecursive( jolt_gui_mutex );
}

/* Finds the first child object with the object identifier.
 * Returns NULL if child not found. */
lv_obj_t *jolt_gui_find(lv_obj_t *parent, jolt_gui_obj_id_t id) {
    lv_obj_t *child = NULL;
    if( NULL == parent ) {
        ESP_LOGW(TAG, "Cannot search a NULL pointer.");
        return NULL;
    }
    JOLT_GUI_CTX{
        ESP_LOGD(TAG, "Searching the %d children of %p for %s", 
                lv_obj_count_children(parent), parent, jolt_gui_obj_id_str(id));
        while( NULL != (child = lv_obj_get_child(parent, child)) ) {
            jolt_gui_obj_id_t child_id;
            child_id = jolt_gui_obj_id_get( child );
            ESP_LOGD(TAG, "Child: %p %s", child, jolt_gui_obj_id_str(child_id));
            if( child_id == id) {
                break;
            }
        }
    }
    return child;
}

/* Convert the enumerated value to a constant string */
const char *jolt_gui_obj_id_str(jolt_gui_obj_id_t val) {
    const char *names[] = { FOREACH_JOLT_GUI_OBJ_ID(GENERATE_STRING) };
    if( val >= JOLT_GUI_OBJ_ID_MAX ) {
        return "<unknown>";
    }
    return names[val];
}

/* Convert the enumerated value to a constant string */
const char *jolt_gui_scr_id_str(jolt_gui_scr_id_t val) {
    const char *names[] = { FOREACH_JOLT_GUI_SCR_ID(GENERATE_STRING) };
    if( val >= JOLT_GUI_SCR_ID_MAX ) {
        return "<unknown>";
    }
    return names[val];
}

/************
 * Wrappers *
 ************/
lv_obj_t *jolt_gui_obj_get_parent( const lv_obj_t *obj ){
    return lv_obj_get_parent(obj);
}

void jolt_gui_obj_set_event_cb(lv_obj_t *obj, lv_event_cb_t event_cb) {
    lv_obj_set_event_cb(obj, event_cb);
}

void *jolt_gui_obj_get_param( lv_obj_t *obj ){
    return lv_obj_get_user_data( obj )->param;
}

void jolt_gui_obj_set_param( lv_obj_t *obj, void *param ) {
    lv_obj_get_user_data(obj)->param = param;
}


