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
        lv_obj_t *scrn = BREAK_IF_NULL(lv_group_get_focused(jolt_gui_store.group.main));
        lv_obj_t *parent = scrn;
        lv_obj_t *tmp = scrn;
        while( (tmp = lv_obj_get_parent(tmp)) ) {
            if( tmp != lv_scr_act() ) {
                parent = tmp;
            }
        }
        jolt_gui_obj_del(parent);
        res = LV_RES_INV;
    }
    return res;
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
    lv_obj_t *label = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *statusbar_label = statusbar_get_label();
        /* Create a non-transparent background to block out old titles */
        lv_obj_t *title_cont = BREAK_IF_NULL(lv_cont_create(parent, NULL));
        lv_obj_set_free_num(title_cont, JOLT_GUI_OBJ_ID_CONT_TITLE);
        lv_obj_align(title_cont, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
        lv_obj_set_size( title_cont,
                lv_obj_get_x(statusbar_label) - 1, // todo: use style padding
                CONFIG_JOLT_GUI_STATUSBAR_H-1);

        label = BREAK_IF_NULL(lv_label_create(title_cont, NULL));
        lv_obj_set_free_num(label, JOLT_GUI_OBJ_ID_LABEL_0);
        lv_style_t *label_style = lv_obj_get_style(label);
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
    lv_obj_t *cont = NULL;
    JOLT_GUI_CTX{
        cont = BREAK_IF_NULL(lv_cont_create(parent, NULL));
        lv_obj_set_free_num(cont, JOLT_GUI_OBJ_ID_CONT_BODY);
        lv_obj_set_size(cont, LV_HOR_RES, 
                LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H);
        lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_LEFT,
                0, CONFIG_JOLT_GUI_STATUSBAR_H);
    }
    return cont;
}

void jolt_gui_obj_del(lv_obj_t *obj){
    JOLT_GUI_CTX{
        /* Some objects may require special actions before deleting */
        jolt_gui_scr_id_t id = lv_obj_get_free_num(obj);
        switch( id ) {
            case JOLT_GUI_SCR_ID_LOADING:
                /* Check and Delete the auto updater task */
                ESP_LOGI(TAG, "meow");
                jolt_gui_scr_loading_autoupdate_del( obj );
                break;
            default:
                break;
        }
        lv_obj_del(obj);
    }
}

/***************
 * Group Stuff *
 ***************/
static void group_mod_cb(lv_style_t *style) {
    /* Intentionally Empty */
}

void jolt_gui_group_create() {
    /* Create Groups for user input */
    bool success = false;
    JOLT_GUI_CTX{
        jolt_gui_store.group.main = BREAK_IF_NULL(lv_group_create());
        lv_group_set_refocus_policy(jolt_gui_store.group.main, LV_GROUP_REFOCUS_POLICY_PREV);
        jolt_gui_store.group.back = BREAK_IF_NULL(lv_group_create());
        lv_group_set_refocus_policy(jolt_gui_store.group.back, LV_GROUP_REFOCUS_POLICY_PREV);
        jolt_gui_store.group.enter = BREAK_IF_NULL(lv_group_create());
        lv_group_set_refocus_policy(jolt_gui_store.group.enter, LV_GROUP_REFOCUS_POLICY_PREV);
        lv_group_set_style_mod_cb(jolt_gui_store.group.main, group_mod_cb);
        success = true;
    }
    if( !success ){
        esp_restart();
    }
}

/* Adds object to main group */
void jolt_gui_group_add( lv_obj_t *obj ){
    JOLT_GUI_CTX{
        lv_group_add_obj(jolt_gui_store.group.main, obj);
        lv_group_focus_obj( obj );
    }
}

/**********
 * Action *
 **********/

lv_obj_t *jolt_gui_scr_set_action(lv_obj_t *parent, lv_action_t cb, 
        lv_group_t *g) {
    lv_obj_t *btn = NULL;
    JOLT_GUI_CTX{
        /* Remove any children buttons already in group g */
        lv_obj_t *child = NULL;
        lv_obj_type_t obj_type;
        while( NULL != (child = lv_obj_get_child(parent, child)) ) {
            lv_obj_get_type(child, &obj_type);
            if( 0==strcmp("lv_btn", obj_type.type[0]) && g==lv_obj_get_group(child) ) {
                lv_obj_del(child);
                child = NULL;
            }
        }

        btn = BREAK_IF_NULL(lv_btn_create(parent, NULL));
        lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, cb);
        lv_obj_set_size(btn, 0, 0);
        lv_group_remove_obj(btn);
        lv_group_add_obj(g, btn);
        lv_group_focus_obj(btn);

        if( g == jolt_gui_store.group.enter ) {
            lv_obj_set_free_num(btn, JOLT_GUI_OBJ_ID_ENTER);
        }
        else if ( g == jolt_gui_store.group.back) {
            lv_obj_set_free_num(btn, JOLT_GUI_OBJ_ID_BACK);
        }
    }
    return btn;
}

/* Perform callback whenever user presses "back" button */
lv_obj_t *jolt_gui_scr_set_back_action(lv_obj_t *parent, lv_action_t cb) {
    lv_obj_t *btn = NULL;
    JOLT_GUI_CTX{
        LV_OBJ_FREE_NUM_TYPE type = lv_obj_get_free_num( parent );
        switch( type ) {
            /* Some screens need to wrap callbacks. Wrapping them here 
             * enables a unified interface for seting back actions. */
            case JOLT_GUI_SCR_ID_DIGIT_ENTRY:{
                jolt_gui_scr_digit_entry_set_back_action(parent, cb);
                break;
            }
            default:{
                /* Usually, just create a button in the back group as a child of the screen */
                btn = jolt_gui_scr_set_action(parent, cb, jolt_gui_store.group.back);
                break;
            }
        }
    }
    return btn;
}

/* Perform callback whenever user presses "enter" button */
lv_obj_t *jolt_gui_scr_set_enter_action(lv_obj_t *parent, lv_action_t cb) {
    lv_obj_t *btn = NULL;
    JOLT_GUI_CTX{
        LV_OBJ_FREE_NUM_TYPE type = lv_obj_get_free_num( parent );
        switch( type ) {
            /* Some screens need to wrap callbacks. Wrapping them here 
             * enables a unified interface for seting back actions. */
            case JOLT_GUI_SCR_ID_DIGIT_ENTRY:{
                jolt_gui_scr_digit_entry_set_enter_action(parent, cb);
                break;
            }
            default:{
                /* Usually, just create a button in the back group as a child of the screen */
                btn = jolt_gui_scr_set_action(parent, cb, jolt_gui_store.group.enter);
                break;
            }
        }
    }
    return btn;
}

void jolt_gui_scr_set_back_param(lv_obj_t *parent, void *param) {
    lv_obj_t *btn = NULL;
    JOLT_GUI_CTX{
        btn = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_BACK);
        lv_obj_set_free_ptr(btn, param);
    }
}

void jolt_gui_scr_set_enter_param(lv_obj_t *parent, void *param) {
    lv_obj_t *btn = NULL;
    JOLT_GUI_CTX{
        btn = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_ENTER);
        lv_obj_set_free_ptr(btn, param);
    }
}

/* Send LV_GROUP_KEY_ENTER to main group */
lv_res_t jolt_gui_send_enter_main(lv_obj_t *dummy) {
    return lv_group_send_data(jolt_gui_store.group.main, LV_GROUP_KEY_ENTER);
}

/* Send LV_GROUP_KEY_ELEFT to main group */
lv_res_t jolt_gui_send_left_main(lv_obj_t *dummy) {
    return lv_group_send_data(jolt_gui_store.group.main, LV_GROUP_KEY_LEFT);
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

/* Finds the first child object with the free_num identifier.
 * Returns NULL if child not found. */
lv_obj_t *jolt_gui_find(lv_obj_t *parent, LV_OBJ_FREE_NUM_TYPE id) {
    lv_obj_t *child = NULL;
    JOLT_GUI_CTX{
        ESP_LOGD(TAG, "Searchng the %d children of %p", lv_obj_count_children(parent), parent);
        while( NULL != (child = lv_obj_get_child(parent, child)) ) {
            LV_OBJ_FREE_NUM_TYPE child_id;
            child_id = lv_obj_get_free_num(child);
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
#define STRCASE(name) case name: return #name;
    switch(val){
        STRCASE(JOLT_GUI_OBJ_ID_UNINITIALIZED)
        STRCASE(JOLT_GUI_OBJ_ID_CONT_TITLE)   
        STRCASE(JOLT_GUI_OBJ_ID_CONT_BODY)    
        STRCASE(JOLT_GUI_OBJ_ID_PAGE)         
        STRCASE(JOLT_GUI_OBJ_ID_LABEL_0)      
        STRCASE(JOLT_GUI_OBJ_ID_LABEL_1)      
        STRCASE(JOLT_GUI_OBJ_ID_LABEL_2)      
        STRCASE(JOLT_GUI_OBJ_ID_LABEL_3)      
        STRCASE(JOLT_GUI_OBJ_ID_LABEL_4)      
        STRCASE(JOLT_GUI_OBJ_ID_BAR_LOADING)  
        STRCASE(JOLT_GUI_OBJ_ID_PRELOADING)   
        STRCASE(JOLT_GUI_OBJ_ID_IMG_QR)       
        STRCASE(JOLT_GUI_OBJ_ID_SLIDER)
        STRCASE(JOLT_GUI_OBJ_ID_LIST)
        default:
            return "<unknown>";
    }
#undef STRCASE
}

