#include "jolt_gui.h"
#include "stdio.h"
#include "jolt_gui_entry.h"
#include "jolt_gui_symbols.h"
#include "jolt_gui_statusbar.h"

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

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_action_t jolt_gui_scr_del() {
    lv_obj_t *scrn = lv_group_get_focused(jolt_gui_store.group.main);
    if( NULL == scrn ) {
        MSG("Nothing in focus\n");
        return 0;
    }
    if( scrn == jolt_gui_store.main_menu ) {
        MSG("Can't exit main menu\n");
        return 0;
    }

    // Disable any focus callback
    lv_group_set_focus_cb(jolt_gui_store.group.main, NULL);
    lv_obj_t *parent = scrn;
    lv_obj_t *tmp = scrn;
    while( (tmp = lv_obj_get_parent(tmp)) ) {
        if( tmp != lv_scr_act() ) {
            parent = tmp;
        }
        if( tmp == jolt_gui_store.main_menu ) {
            return 0;
        }
    }
    lv_obj_del(parent);
    return 0;
}

/* Creates a dummy invisible object to anchor lvgl objects on the screen */
lv_obj_t *jolt_gui_parent_create() {
    lv_obj_t *parent = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(parent, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(parent,0,0);
    lv_obj_set_style(parent, &lv_style_transp);
    return parent;
}

lv_obj_t *jolt_gui_scr_text_create(const char *title, const char *body) {
    /* Creates a page with scrollable text */
    lv_obj_t *parent = jolt_gui_parent_create();
    /* Create Statusbar Title */
    if( NULL != title ) {
        jolt_gui_obj_title_create(parent, title);
    }

    /* Create Page */
    lv_obj_t *page = lv_page_create(parent, NULL);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H);
    lv_obj_align(page, NULL, LV_ALIGN_IN_TOP_LEFT,
            0, CONFIG_JOLT_GUI_STATUSBAR_H);
    lv_page_set_sb_mode(page, LV_SB_MODE_AUTO);
    lv_group_add_obj(jolt_gui_store.group.main, page);
    lv_page_set_arrow_scroll(page, true);

    /* Create Text Label on Page */
    lv_obj_t *label = lv_label_create(page, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    {
        // Compute Width of Label
        lv_style_t *sb = lv_page_get_style(page, LV_PAGE_STYLE_SB);
        lv_obj_set_width(label, lv_page_get_scrl_width(page) - 
                sb->body.padding.inner - sb->body.padding.hor);  
    }
    lv_label_set_text(label, body);

    lv_group_focus_obj(page);

    jolt_gui_scr_set_back_action(parent, jolt_gui_scr_del);
    jolt_gui_scr_set_enter_action(parent, NULL);

    return parent;
}

/* Creates the statusbar title label for a screen */
lv_obj_t *jolt_gui_obj_title_create(lv_obj_t *parent, const char *title) {
    if( NULL == parent ) {
        parent = lv_scr_act();
    }
    lv_obj_t *label = lv_label_create(parent, NULL);

    static lv_style_t label_style;
    lv_style_copy(&label_style, &lv_style_plain);
    label_style.body.main_color = LV_COLOR_WHITE;
    label_style.body.padding.ver = 0;
    label_style.body.padding.inner = 0;
    label_style.text.font = &lv_font_monospace_8;
    label_style.body.border.opa = LV_OPA_TRANSP;
    label_style.body.border.part = 0;

    lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);
    lv_label_set_body_draw(label, true); // draw background
    lv_label_set_style(label, &label_style);
    lv_obj_align(label, jolt_gui_store.statusbar.container,
            LV_ALIGN_IN_LEFT_MID, 2, 0);
    lv_label_set_text(label, title);
    lv_obj_set_size(label, CONFIG_JOLT_GUI_TITLE_W, label_style.text.font->h_px);

    return label;
}

static void group_mod_cb(lv_style_t *style) {
    style->body.border.part = 0;
}

void jolt_gui_group_create() {
    /* Create Groups for user input */
    jolt_gui_store.group.main = lv_group_create();
    lv_group_set_refocus_policy(jolt_gui_store.group.main, LV_GROUP_REFOCUS_POLICY_PREV);
    jolt_gui_store.group.back = lv_group_create();
    lv_group_set_refocus_policy(jolt_gui_store.group.back, LV_GROUP_REFOCUS_POLICY_PREV);
    jolt_gui_store.group.enter = lv_group_create();
    lv_group_set_refocus_policy(jolt_gui_store.group.enter, LV_GROUP_REFOCUS_POLICY_PREV);
    lv_group_set_style_mod_cb(jolt_gui_store.group.main, group_mod_cb);
}

static lv_obj_t *jolt_gui_scr_set_action(lv_obj_t *parent, lv_action_t cb, 
        lv_group_t *g) {
    if( NULL == parent ) {
        parent = lv_scr_act();
    }
    else {
        // Remove any children already in group g
        lv_obj_t *child = NULL;
        lv_obj_type_t obj_type;
        while( NULL != (child = lv_obj_get_child(parent, child)) ) {
            lv_obj_get_type(child, &obj_type);
            if(strcmp("lv_btn", obj_type.type[0]) && g==lv_obj_get_group(child) ) {
                lv_obj_del(child);
                child = NULL;
            }
        }
    }
    lv_obj_t *btn = lv_btn_create(parent, NULL);
    lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, cb);
    lv_obj_set_size(btn, 0, 0);
    lv_group_remove_obj(btn);

    lv_group_add_obj(g, btn);
    lv_group_focus_obj(btn);

    return parent;
}

lv_obj_t *jolt_gui_scr_set_back_action(lv_obj_t *parent, lv_action_t cb) {
    return jolt_gui_scr_set_action(parent, cb, jolt_gui_store.group.back);
}

lv_obj_t *jolt_gui_scr_set_enter_action(lv_obj_t *parent, lv_action_t cb) {
    return jolt_gui_scr_set_action(parent, cb, jolt_gui_store.group.enter);
}

lv_action_t jolt_gui_send_enter_main(lv_obj_t *btn) {
    lv_group_send_data(jolt_gui_store.group.main, LV_GROUP_KEY_ENTER);
    return 0;
}

lv_action_t jolt_gui_send_left_main(lv_obj_t *btn) {
    lv_group_send_data(jolt_gui_store.group.main, LV_GROUP_KEY_LEFT);
    return 0;
}

void jolt_gui_sem_take() {
    xSemaphoreTake( jolt_gui_store.mutex, portMAX_DELAY );
}
void jolt_gui_sem_give() {
    xSemaphoreGive( jolt_gui_store.mutex );
}

