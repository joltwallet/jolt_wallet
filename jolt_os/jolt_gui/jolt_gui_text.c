#include "jolt_gui.h"
#include "jolt_gui_scroll.h"

/* Text Screen Structure:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--LABEL_0
 */

/* Creates a simple screen with a single scrollable text body
 * */
lv_obj_t *jolt_gui_scr_text_create(const char *title, const char *body) {
    lv_obj_t *parent = jolt_gui_scr_scroll_create(title);
    if( NULL != parent ) {
        lv_obj_t *label = jolt_gui_scr_scroll_add_text(parent, body);
        if( NULL == label ) {
            jolt_gui_obj_del(parent);
        }
    }
    return parent;
}

