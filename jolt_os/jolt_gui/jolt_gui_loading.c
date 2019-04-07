#include "jolt_gui.h"
#include "jolt_gui_loading.h"

// todo: need to declare a custom ext for this to hold autoupdate params


/* Loading and Preloading Screen Structures:
 * * SCREEN
 *   +--LABEL_TITLE
 *   +--CONT_BODY
 *       +--LOADINGBAR or PRELOADING
 *       +--LABEL_0 (loading label)
 */

static const char TAG[] = "scr_loading";

typedef struct autoupdate_param_t {
    lv_obj_t *scr;
    lv_task_t *task;
    int8_t *progress;
} autoupdate_param_t;


/* Update the loading screen.
 * Title and Text are optional.*/
void jolt_gui_scr_loadingbar_update(lv_obj_t *parent,
        const char *title, const char *text,
        uint8_t percentage) {
    JOLT_GUI_CTX{
        /* Find Objects */
        lv_obj_t *label_title   = NULL;
        {
            lv_obj_t *cont_title = NULL;
            cont_title  = JOLT_GUI_FIND_AND_CHECK(parent,     JOLT_GUI_OBJ_ID_CONT_TITLE);
            label_title = JOLT_GUI_FIND_AND_CHECK(cont_title, JOLT_GUI_OBJ_ID_LABEL_0);
        }

        lv_obj_t *bar_loading   = NULL;
        lv_obj_t *label_loading = NULL;
        {
            lv_obj_t *cont_body = NULL;
            cont_body     = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
            bar_loading   = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_LOADINGBAR);
            label_loading = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_LABEL_0);
        }

        if( percentage > 100 ) {
            percentage = 100;
        }
        // todo: set animation time based on config value
        //lv_bar_set_value_anim(bar_loading, percentage, CONFIG_JOLT_GUI_LOADINGBAR_ANIM_MS);
        lv_bar_set_value(bar_loading, percentage, true);
        if( text ) {
            lv_label_set_text(label_loading, text);
        }
        if( title ) {
            lv_label_set_text(label_title, title);
        }
    }
}

/* lv_task that periodically updates the loading screen */
static void autoupdate_task(void *input) {
    lv_obj_t *scr = input;
    autoupdate_param_t *param = jolt_gui_obj_get_param( scr );
    if( *(param->progress) <= 100 && *(param->progress) >= 0 ) {
        // The +10 makes it look better
        jolt_gui_scr_loadingbar_update(scr, NULL, NULL,
                *(param->progress) + 10);
    }
}

void jolt_gui_scr_loadingbar_autoupdate(lv_obj_t *parent, int8_t *progress) {
    autoupdate_param_t *param;
    param = malloc( sizeof(autoupdate_param_t) );
    if( NULL == param ) {
        ESP_LOGE(TAG, "Unable to allocate memory for autoupdate params");
        return;
    }
    JOLT_GUI_CTX{
        // todo: maybe set this as the loadingbar object param
        jolt_gui_obj_set_param(parent, param);
        param->scr      = parent;
        param->progress = progress;
        param->task = lv_task_create(autoupdate_task, 100, LV_TASK_PRIO_HIGH, parent);
    }
}

/* Deletes the update task and free's its parameters. DOES NOT delete parent */
void jolt_gui_scr_loadingbar_autoupdate_del(lv_obj_t *parent) {
    JOLT_GUI_CTX{
        autoupdate_param_t *param = jolt_gui_obj_get_param( parent );
        if( NULL == param ) {
            break;
        }
        if( NULL != param->task ) {
            lv_task_del( param->task );
        }
        free( param );
    }
}

/* Creates and returns a loading screen.
 * Defaults the back and enter action to NULL.
 * */
lv_obj_t *jolt_gui_scr_loadingbar_create(const char *title) {
    JOLT_GUI_SCR_CTX(title){
        /* Set screen ID */
        jolt_gui_scr_id_set(parent, JOLT_GUI_SCR_ID_LOADINGBAR);

        /* Create Loading Bar */
        lv_obj_t *bar = BREAK_IF_NULL(lv_bar_create(cont_body, NULL));
        jolt_gui_obj_id_set(bar, JOLT_GUI_OBJ_ID_LOADINGBAR);
        lv_obj_set_size(bar, 
                CONFIG_JOLT_GUI_LOADINGBAR_W, CONFIG_JOLT_GUI_LOADINGBAR_H);
        lv_obj_align(bar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
        lv_bar_set_value(bar, 1, true);

        /* Create Loading Label */
        lv_obj_t *label = BREAK_IF_NULL(lv_label_create(cont_body, NULL));
        jolt_gui_obj_id_set(label, JOLT_GUI_OBJ_ID_LABEL_0);
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
        lv_label_set_text(label, "Initializing");
        lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);

        /* Set Label Size and Position */
        lv_style_t *label_style = lv_obj_get_style(label);
        lv_obj_set_size(label, lv_obj_get_width(cont_body),
                label_style->text.font->h_px);
        lv_obj_align(label, bar, LV_ALIGN_OUT_TOP_MID, 0, -10);

        jolt_gui_scr_set_event_cb(parent, NULL);
    }
    return parent;
}

/* Creates and returns a preloading screen.
 * A preloading screen has no progression and is used for tasks of unknown 
 * lenth and/or progression.
 *
 * Defaults the back and enter action to NULL.
  * */
lv_obj_t *jolt_gui_scr_preloading_create(const char *title, const char *text) {
    JOLT_GUI_SCR_CTX( title ){
        /* Set screen ID */
        jolt_gui_scr_id_set(parent, JOLT_GUI_SCR_ID_PRELOADING);

        /* Create PreLoading Object */
        lv_obj_t *preload = BREAK_IF_NULL(lv_preload_create(cont_body, NULL));
        jolt_gui_obj_id_set(preload, JOLT_GUI_OBJ_ID_PRELOADING);
        lv_obj_set_size(preload, 30, 30);
        lv_obj_align(preload, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

        /* Create Loading Label */
        lv_obj_t *label = BREAK_IF_NULL(lv_label_create(cont_body, NULL));
        jolt_gui_obj_id_set(label, JOLT_GUI_OBJ_ID_LABEL_0);
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
        if( NULL == text ){
            lv_label_set_text(label, "");
        }
        else {
            lv_label_set_text(label, text);
        }
        lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);
        lv_style_t *label_style = lv_obj_get_style(label);
        lv_obj_set_size(label, lv_obj_get_width(cont_body),
                label_style->text.font->h_px);
        lv_obj_align(label, preload, LV_ALIGN_OUT_TOP_MID, 0, -6);

        jolt_gui_scr_set_event_cb(parent, NULL);
    }
    return parent;
}

void jolt_gui_scr_preloading_update(lv_obj_t *parent,
        const char *title, const char *text) {
    JOLT_GUI_CTX{
        /* Find Objects */
        lv_obj_t *label_title   = NULL;
        {
            lv_obj_t *cont_title = NULL;
            cont_title  = JOLT_GUI_FIND_AND_CHECK(parent,
                    JOLT_GUI_OBJ_ID_CONT_TITLE);
            label_title = JOLT_GUI_FIND_AND_CHECK(cont_title,
                    JOLT_GUI_OBJ_ID_LABEL_0);
        }

        lv_obj_t *label_loading = NULL;
        {
            lv_obj_t *cont_body = NULL;
            cont_body     = JOLT_GUI_FIND_AND_CHECK(parent,
                    JOLT_GUI_OBJ_ID_CONT_BODY);
            label_loading = JOLT_GUI_FIND_AND_CHECK(cont_body,
                    JOLT_GUI_OBJ_ID_LABEL_0);
        }

        if( text ) {
            lv_label_set_text(label_loading, text);
        }
        if( title ) {
            lv_label_set_text(label_title, title);
        }
    }
}
