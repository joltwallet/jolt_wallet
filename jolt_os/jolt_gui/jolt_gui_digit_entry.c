#include "jolt_gui/jolt_gui.h"
#include <stdio.h>
#include "sodium.h"

/* Entry Screen Structures:
 * * SCREEN
 *   +--LABEL_0 (title)
 *   +--CONT_BODY (extended with metadata)
 *       +--ROLLER[0 ... n]
 *       +--LABEL_0 (decimal point)
 *
 *
 * Typical workflow:
 * lv_obj_t *scr = jolt_gui_scr_digit_entry_create("Title", 10, -1, cb);
 */

typedef struct {
    lv_cont_ext_t cont;       /*The ancestor container structure*/
    uint8_t sel;
    uint8_t num_rollers;
    lv_obj_t *rollers[CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_MAX_LEN];
    lv_obj_t *decimal_point;
    int8_t decimal_point_pos; // Position of decimal point from right. -1 for no decimal
    lv_action_t back_cb;
    lv_action_t enter_cb;
} digit_entry_cont_ext_t;

/* Static Variables */
static const char TAG[] = "digit_entry";
static lv_signal_func_t old_roller_signal = NULL;     /*Store the old signal function*/

/* Static Functions Declarations */
static lv_obj_t *digit_create(lv_obj_t *parent);
static void loop_roller( lv_obj_t *rol);
static unsigned concat_int(unsigned x, unsigned y);
static lv_obj_t *create_dp(lv_obj_t *parent);

static void loop_roller( lv_obj_t *rol) {
    /* Loops roller position */
    lv_roller_ext_t *rol_ext = lv_obj_get_ext_attr(rol);
    uint8_t id = rol_ext->ddlist.sel_opt_id;
    if( id < 10 ) { // todo refine loop point
        lv_roller_set_selected(rol, id + 10, false);
    }
    if( id > 25 ) { // todo refine loop point
        lv_roller_set_selected(rol, id - 10, false);
    }
}

static lv_res_t new_roller_signal(lv_obj_t *roller, lv_signal_t sign, void * param) {
    lv_res_t res = LV_RES_OK;

    lv_obj_t *cont_body = lv_obj_get_parent(roller);
    lv_obj_t *parent = lv_obj_get_parent(cont_body);
    digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(cont_body);
    lv_roller_ext_t *rol_ext = lv_obj_get_ext_attr(roller);

    bool call_ancestor_signal = true;

    if(sign == LV_SIGNAL_FOCUS) {
        lv_roller_set_visible_row_count(roller, 3);
    }
    else if(sign == LV_SIGNAL_DEFOCUS) {
        lv_roller_set_visible_row_count(roller, 1);
    }
    else if(sign == LV_SIGNAL_CONTROLL){
        char c = *((char *)param);
        
        loop_roller(ext->rollers[ext->sel]);

        call_ancestor_signal = false;
        switch(c){
            case LV_GROUP_KEY_LEFT: {
                /* Preserve the currently selected roller value */
                rol_ext->ddlist.sel_opt_id_ori = rol_ext->ddlist.sel_opt_id;

                if( ext->sel > 0 ) {
                    (ext->sel)--;
                    lv_group_focus_obj(ext->rollers[ext->sel]);
                }
                else if(NULL != ext->back_cb){
                    // Call back callback
                    res = (ext->back_cb)(parent);
                }
                break;
            }
            case LV_GROUP_KEY_ENTER: {
                /* Preserve the currently selected roller value */
                rol_ext->ddlist.sel_opt_id_ori = rol_ext->ddlist.sel_opt_id;
                if(ext->sel < ext->num_rollers-1) {
                    (ext->sel)++;
                    lv_group_focus_obj(ext->rollers[ext->sel]);
                }
                else if(NULL != ext->enter_cb){
                    //perform enter action callback
                    res = (ext->enter_cb)(parent);
                }
                break;
            }
            default:
                call_ancestor_signal = true;
                break;
        }
    }
    else if( sign == LV_SIGNAL_CLEANUP){
        /* Securely zero out values */
        sodium_memzero(&(rol_ext->ddlist.sel_opt_id), sizeof(rol_ext->ddlist.sel_opt_id));
        sodium_memzero(&(rol_ext->ddlist.sel_opt_id_ori), sizeof(rol_ext->ddlist.sel_opt_id_ori));
    }

    /* Call Ancestor Signal Func */
    if( res == LV_RES_OK && call_ancestor_signal ){
        res = old_roller_signal(roller, sign, param);
        if(res != LV_RES_OK) return res;
    }

    return res;
}

/* Create and return a lv_roller */
static lv_obj_t *digit_create(lv_obj_t *parent) {
    lv_obj_t *roller;
    roller = lv_roller_create(parent, NULL);
    lv_roller_set_anim_time(roller, CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_ANIM_DIGIT_MS);
    lv_roller_set_options(roller, 
            "9\n8\n7\n6\n5\n4\n3\n2\n1\n0"
            "\n9\n8\n7\n6\n5\n4\n3\n2\n1\n0"
            "\n9\n8\n7\n6\n5\n4\n3\n2\n1\n0"
            "\n9\n8\n7\n6\n5\n4\n3\n2\n1\n0"
            );
    lv_roller_set_selected(roller, 19, false); // Set it to the middle 0 entry
    lv_roller_set_visible_row_count(roller, 1);
    lv_roller_set_align(roller, LV_LABEL_ALIGN_CENTER);
    return roller;
}

/* Create the decimal point label object */
static lv_obj_t *create_dp(lv_obj_t *parent){
    lv_obj_t *label = lv_label_create(parent, NULL);

    static lv_style_t dp_style_obj;
    static lv_style_t *dp_style = NULL;
    if( NULL == dp_style ){
        dp_style = &dp_style_obj;
        lv_style_copy(dp_style, lv_label_get_style(label));
        dp_style->text.font = &lv_font_crox3hb_numeric;
        dp_style->body.padding.hor = 0;
    }
    lv_label_set_style(label, dp_style);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);

    lv_label_set_text(label, ".");
    lv_obj_set_free_num(label, JOLT_GUI_OBJ_ID_DECIMAL_POINT);
    return label;
}

lv_obj_t *jolt_gui_scr_digit_entry_create(const char *title,
        int8_t n, int8_t pos){
    bool visible_decimal = false;
    if( pos >= 0 && pos <= n){
        /* Decimal will be visible */
        visible_decimal = true;
        if( n + 1 > CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_MAX_LEN){
            /* Exceeds maximum number of spaces */
            return NULL;
        }
    }
    if( n > CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_MAX_LEN ){
        return NULL;
    }

    JOLT_GUI_SCR_CTX(title){
        BREAK_IF_NULL(lv_obj_allocate_ext_attr(cont_body, sizeof(digit_entry_cont_ext_t)));
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(cont_body);

        /* Setup style */
        static lv_style_t cont_body_style_obj;
        static lv_style_t *cont_body_style = NULL;
        if( NULL == cont_body_style ){
            cont_body_style = &cont_body_style_obj;
            lv_style_copy(cont_body_style, lv_cont_get_style(cont_body));
            cont_body_style->body.padding.inner = 1;
            cont_body_style->body.padding.hor = 0;
        }
        lv_cont_set_style(cont_body, cont_body_style);
        lv_cont_set_layout(cont_body, LV_LAYOUT_PRETTY);

        /* Forward left/right actions get forwarded to the main group */
        ext->back_cb = &jolt_gui_scr_del;
        ext->enter_cb = NULL;
        BREAK_IF_NULL(jolt_gui_scr_set_back_action(parent, &jolt_gui_send_left_main));
        BREAK_IF_NULL(jolt_gui_scr_set_enter_action(parent, &jolt_gui_send_enter_main));

        lv_obj_set_free_num(parent, JOLT_GUI_SCR_ID_DIGIT_ENTRY);

        /* Initialize remaining ext params */
        ext->sel = 0;
        ext->decimal_point = NULL;
        if( visible_decimal ){
            ext->decimal_point_pos = pos;
        }
        ext->num_rollers = n;

        /* Create left to right */
        for(int8_t i=0; i < n; i++){
            if(pos + i == n ){
                /* Create decimal place */
                ext->decimal_point = create_dp(cont_body);
            }
            ext->rollers[i] = digit_create(cont_body);
            lv_obj_set_free_num(ext->rollers[i], JOLT_GUI_OBJ_ID_ROLLER);
            jolt_gui_group_add( ext->rollers[i] );
            if( NULL == old_roller_signal) {
                old_roller_signal = lv_obj_get_signal_func(ext->rollers[i]);
            }
            lv_obj_set_signal_func(ext->rollers[i], new_roller_signal);
        }
        if( 0 == pos ) {
            ext->decimal_point = create_dp(cont_body);
        }
        lv_group_focus_obj(ext->rollers[0]);
    }
    return parent;
}

int8_t jolt_gui_scr_digit_entry_get_arr(lv_obj_t *parent, uint8_t *arr, uint8_t arr_len) {
    int8_t n_entries = -1;
    digit_entry_cont_ext_t *ext = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *cont_body = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        ext = lv_obj_get_ext_attr(cont_body);
        if( arr_len < ext->num_rollers ) {
            /* insufficient output buffer */
            break;
        }
        else{
            n_entries = ext->num_rollers;
        }
        for(uint8_t i=0; i < ext->num_rollers; i++) {
            arr[i] = 9 - (lv_roller_get_selected(ext->rollers[i]) % 10);
        }
    }
#if ESP_LOG_LEVEL >= ESP_LOG_DEBUG
    if(n_entries > 0){
        printf("Entered Array: ");
        for(uint8_t i=0; i < ext->num_rollers; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }
#endif
    return n_entries;
}

uint8_t jolt_gui_scr_digit_entry_get_hash(lv_obj_t *parent, uint8_t *hash) {
    /* todo: error handling */
    uint8_t res = 1;
    JOLT_GUI_CTX{
        uint32_t val = jolt_gui_scr_digit_entry_get_int(parent);

        /* Convert pin into a 256-bit key */
        crypto_generichash_blake2b_state hs;
        crypto_generichash_init(&hs, NULL, 32, 32);
        crypto_generichash_update(&hs, (unsigned char *) &val, sizeof(val));
        crypto_generichash_final(&hs, hash, 32);

        /* Clean up local pin_array variable */
        sodium_memzero(&val, sizeof(val));
        res = 0;
    }
    return res;
}

void jolt_gui_scr_digit_entry_set_back_action(lv_obj_t *parent, lv_action_t cb){
    JOLT_GUI_CTX{
        lv_obj_t *cont_body = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(cont_body);
        ext->back_cb = cb;
    }
}

void jolt_gui_scr_digit_entry_set_enter_action(lv_obj_t *parent, lv_action_t cb){
    JOLT_GUI_CTX{
        lv_obj_t *cont_body = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(cont_body);
        ext->enter_cb = cb;
    }
}

static unsigned concat_int(unsigned x, unsigned y) {
    unsigned pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
}

double jolt_gui_scr_digit_entry_get_double(lv_obj_t *parent){
    double res = 0;
    JOLT_GUI_CTX{
        lv_obj_t *cont_body = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(cont_body);

        res = (double)jolt_gui_scr_digit_entry_get_int(parent);

        for(int8_t i = ext->decimal_point_pos; i>0; i--) {
            res /= 10; 
        }
    }
    return res;
}

uint32_t jolt_gui_scr_digit_entry_get_int(lv_obj_t *parent) {
    uint32_t res = 0;
    JOLT_GUI_CTX{
        lv_obj_t *cont_body = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(cont_body);

        uint8_t array[sizeof(ext->rollers)] = { 0 };
        int8_t n = jolt_gui_scr_digit_entry_get_arr(parent, array, sizeof(array));

        if( n <= 0 ){
            break;
        }
        for(uint8_t i=0; i < n; i++){
            res = concat_int(res, array[i]);
        }

        /* Securely wipe local array value */
        sodium_memzero(array, sizeof(array));
    }
    return res;
}

void jolt_gui_scr_digit_entry_set_pos(lv_obj_t *parent, int8_t pos) {
    JOLT_GUI_CTX{
        lv_obj_t *cont_body = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(cont_body);
        if( pos < 0 || pos-1 > ext->num_rollers ){
            break;
        }
        ext->sel = ext->num_rollers - 1 - pos; 
        lv_group_focus_obj(ext->rollers[ext->sel]);
    }
}
