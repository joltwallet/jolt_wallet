#include "jolt_gui.h"

/* Text Screen Structure:
 * * SCREEN
 *   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--LABEL_0
 */

static const char TAG[] = "jolt_gui_text";

jolt_gui_obj_t *jolt_gui_scr_text_create(const char *title, const char *body) {
    jolt_gui_obj_t *parent = NULL;
    JOLT_GUI_CTX{
        parent = BREAK_IF_NULL(jolt_gui_scr_scroll_create(title));
        jolt_gui_obj_t *label = jolt_gui_scr_scroll_add_text(parent, body);
        if( NULL == label ) jolt_gui_obj_del(parent);
    }
    return parent;
}

