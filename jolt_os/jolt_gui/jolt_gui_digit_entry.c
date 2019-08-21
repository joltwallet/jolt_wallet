/**
 *
 * Object structure
 *
 * * digit_entry (lv_cont)
 *     * list of digit rollers (lv_roller)
 *     * decimal point (lv_label)
 *
 * Event Handling:
 *     * lv_event_cb is set on the digit_entry (lv_cont) object
 *     * On the first roller, jolt_gui_event.cancel gets forwarded to the callback
 *     * On the last roller, jolt_gui_event.short_clicked gets forwarded to the callback
 *     * Any time a roller value changes, LV_EVENT_VALUE_CHANGED is sent to the callback
 *
 */

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"

#include "jolt_gui/jolt_gui.h"
#include <stdio.h>
#include "sodium.h"

/* Entry Screen Structures:
 * * SCREEN
 *   +--CONT_TITLE
 *       + LABEL_0 (title)
 *   +--CONT_BODY 
 *       + digit_entry (cont extended with metadata)
 *           +--ROLLER[0 ... n]
 *           +--LABEL_0 (decimal point)
 *
 *
 * Typical workflow:
 * jolt_gui_obj_t *scr = jolt_gui_scr_digit_entry_create("Title", 10, -1, cb);
 */

typedef struct {
    lv_cont_ext_t cont;       /*The ancestor container structure*/
    jolt_gui_obj_t *rollers[CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_MAX_LEN];
    jolt_gui_obj_t *decimal_point;
    lv_event_cb_t user_cb;
    uint8_t sel;
    uint8_t num_rollers;
    int8_t decimal_point_pos; // Position of decimal point from right. -1 for no decimal
} digit_entry_cont_ext_t;

/* Static Variables */
static const char TAG[] = "digit_entry";
static lv_signal_cb_t old_roller_signal = NULL;     /*Store the old signal function*/
static lv_signal_cb_t old_cont_signal = NULL;

/* Static Functions Declarations */
static jolt_gui_obj_t *digit_create(jolt_gui_obj_t *parent);
static unsigned concat_int(unsigned x, unsigned y);
static jolt_gui_obj_t *create_dp(jolt_gui_obj_t *parent);

static lv_res_t digit_entry_signal(jolt_gui_obj_t *digit_entry, lv_signal_t sign, void *param) {
    ESP_LOGD(TAG, "%s signal %d", __func__, sign);
    lv_res_t res = LV_RES_OK;
    digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr( digit_entry );

    if(sign == LV_SIGNAL_CONTROL){
        char c = *((char *)param);
        /* Deactivate the User's Callback */
        if( NULL != digit_entry->event_cb ){
            ext->user_cb = lv_obj_get_event_cb(digit_entry);
            lv_obj_set_event_cb(digit_entry, NULL);
        }

        switch(c){
            case LV_KEY_ENTER: {
                if( ext->sel == ext->num_rollers - 1 ){
                    /* Activate the User's Callback */
                    ESP_LOGD(TAG, "Activating User Callback");
                    lv_obj_set_event_cb(digit_entry, ext->user_cb);
                    /* Will get triggered when old_cont_signal is processed */
                }
                else{
                    /* Change the selected roller */
                    lv_roller_set_visible_row_count(ext->rollers[ext->sel], 1);
                    (ext->sel)++;
                    lv_roller_set_visible_row_count(ext->rollers[ext->sel], 3);
                }
                break;
            }
            case LV_KEY_ESC:{
                if( 0 == ext->sel ){
                    /* Activate the User's Callback */
                    ESP_LOGD(TAG, "Activating User Callback");
                    lv_obj_set_event_cb(digit_entry, ext->user_cb);
                    /* Will get triggered when old_cont_signal is processed */
                }
                else{
                    /* Change the selected roller */
                    lv_roller_set_visible_row_count(ext->rollers[ext->sel], 1);
                    (ext->sel)--;
                    lv_roller_set_visible_row_count(ext->rollers[ext->sel], 3);
                }
                break;
            }
            case LV_KEY_UP:
                /* Fall through */
            case LV_KEY_LEFT:
                /* Fall through */
            case LV_KEY_DOWN:
                /* Fall through */
            case LV_KEY_RIGHT:
                ESP_LOGD(TAG, "Roller value updated");
                lv_signal_send(ext->rollers[ext->sel], sign, param);
                res = lv_event_send(digit_entry, jolt_gui_event.value_changed, NULL);
                break;
            default:
                break;
        }
    }

    if( res == LV_RES_OK ){
        ESP_LOGV(TAG, "Old cont signal: %p", old_cont_signal);
        res = old_cont_signal(digit_entry, sign, param);
        if(res != LV_RES_OK) return res;
    }

    return res;
}

static lv_res_t new_roller_signal(jolt_gui_obj_t *roller, lv_signal_t sign, void * param) {
    lv_res_t res = LV_RES_OK;

    lv_roller_ext_t *ext = lv_obj_get_ext_attr(roller);

    if( sign == LV_SIGNAL_CLEANUP ){
        /* Securely zero out values */
        sodium_memzero(&(ext->ddlist.sel_opt_id), sizeof(ext->ddlist.sel_opt_id));
        sodium_memzero(&(ext->ddlist.sel_opt_id_ori), sizeof(ext->ddlist.sel_opt_id_ori));
    }

    /* Call Ancestor Signal Func */
    if( res == LV_RES_OK ){
        ESP_LOGV(TAG, "Old roller signal: %p", old_roller_signal);
        res = old_roller_signal(roller, sign, param);
        if(res != LV_RES_OK) return res;
    }

    return res;
}

/* Create and return a lv_roller */
static jolt_gui_obj_t *digit_create(jolt_gui_obj_t *parent) {
    jolt_gui_obj_t *roller;
    roller = lv_roller_create(parent, NULL);
    jolt_gui_obj_id_set(roller, JOLT_GUI_OBJ_ID_ROLLER);
    if( NULL == old_roller_signal) {
        old_roller_signal = lv_obj_get_signal_cb(roller);
    }
    lv_obj_set_signal_cb(roller, new_roller_signal);
    lv_roller_set_anim_time(roller, CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_ANIM_DIGIT_MS);
    lv_roller_set_options(roller, "0\n9\n8\n7\n6\n5\n4\n3\n2\n1", true);
    lv_roller_set_visible_row_count(roller, 1);
    lv_roller_set_align(roller, LV_LABEL_ALIGN_CENTER);
    return roller;
}

/**
 * @brief Create the decimal point label object 
 */
static jolt_gui_obj_t *create_dp(jolt_gui_obj_t *parent){
    jolt_gui_obj_t *label = NULL;
    JOLT_GUI_CTX{
        label = lv_label_create(parent, NULL);
        jolt_gui_obj_id_set(label, JOLT_GUI_OBJ_ID_DECIMAL_POINT);

        /* Style */
        static lv_style_t dp_style_obj;
        static lv_style_t *dp_style = NULL;
        if( NULL == dp_style ){
            dp_style = &dp_style_obj;
            lv_style_copy(dp_style, lv_label_get_style(label, LV_LABEL_STYLE_MAIN));
            dp_style->text.font = JOLT_GUI_FONT_DIGIT_ENTRY;
            dp_style->body.padding.left = 0;
            dp_style->body.padding.right = 0;
        }
        lv_label_set_style(label, LV_LABEL_STYLE_MAIN, dp_style);
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);

        /* Label Content */
        lv_label_set_text(label, ".");
    }
    return label;
}

jolt_gui_obj_t *jolt_gui_scr_digit_entry_create(const char *title,
        int8_t n, int8_t pos){

    /* Validate number of digits/rollers */
    if( n > CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_MAX_LEN || n < 1 ){
        return NULL;
    }

    /* Validate decimal place position */
    if( pos >= 0 && pos <= n){
        if( n + 1 > CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_MAX_LEN){
            /* Exceeds maximum number of spaces */
            return NULL;
        }
    }

    JOLT_GUI_SCR_CTX(title){
        jolt_gui_scr_id_set(parent, JOLT_GUI_SCR_ID_DIGIT_ENTRY);
        ESP_LOGD(TAG, "digit_entry screen created at: %p", parent);

        /* Create digit_entry object */
        jolt_gui_obj_t *digit_entry = lv_cont_create(cont_body, cont_body);
        BREAK_IF_NULL(lv_obj_allocate_ext_attr(digit_entry, sizeof(digit_entry_cont_ext_t)));
        ESP_LOGD(TAG, "digit_entry object created at: %p", digit_entry);
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(digit_entry);
        jolt_gui_obj_id_set(digit_entry, JOLT_GUI_OBJ_ID_DIGIT_ENTRY);
        lv_obj_set_pos(digit_entry, 0, 0);

        /* Initialize ext params */
        ext->sel = 0;
        ext->decimal_point = NULL;
        ext->decimal_point_pos = pos;
        ext->num_rollers = n;

        /* Create the rollers, left to right */
        for(int8_t i=0; i < n; i++){
            ESP_LOGD(TAG, "Creating roller %d", i);
            if(pos + i == n ){
                /* Create decimal place */
                ext->decimal_point = create_dp(digit_entry);
            }
            ext->rollers[i] = digit_create(digit_entry);
        }
        lv_roller_set_visible_row_count(ext->rollers[0], 3);
        uint8_t roller_height = lv_obj_get_height(ext->rollers[0]);

        /* Setup style */
        static lv_style_t digit_entry_style_obj;
        static lv_style_t *digit_entry_style = NULL;
        if( NULL == digit_entry_style ){
            uint8_t cont_height = lv_obj_get_height(digit_entry);
            digit_entry_style = &digit_entry_style_obj;
            lv_style_copy(digit_entry_style, lv_cont_get_style(digit_entry, LV_CONT_STYLE_MAIN));
            digit_entry_style->body.padding.top= (cont_height-roller_height)/2;
            digit_entry_style->body.padding.inner = 1;
            digit_entry_style->body.padding.left = 0;
            digit_entry_style->body.padding.right = 0;
        }
        lv_cont_set_style(digit_entry, LV_CONT_STYLE_MAIN, digit_entry_style);
        lv_cont_set_layout(digit_entry, LV_LAYOUT_PRETTY ); // automatically handles object spacing

       
        if( 0 == pos ) {
            ext->decimal_point = create_dp(digit_entry);
        }

        if( NULL == old_cont_signal) {
            old_cont_signal = lv_obj_get_signal_cb(digit_entry);
        }
        lv_obj_set_signal_cb(digit_entry, digit_entry_signal);

        jolt_gui_group_add( digit_entry );
        lv_group_focus_obj( digit_entry );
    }
    return parent;
}

int8_t jolt_gui_scr_digit_entry_get_arr(jolt_gui_obj_t *scr, uint8_t *arr, uint8_t arr_len) {
    int8_t n_entries = -1;
    JOLT_GUI_CTX{
        jolt_gui_obj_t *cont_body = NULL;
        jolt_gui_obj_t *digit_entry = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(scr, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_DIGIT_ENTRY);
        n_entries = jolt_gui_obj_digit_entry_get_arr(digit_entry, arr, arr_len);
    }
    return n_entries;
}


int8_t jolt_gui_obj_digit_entry_get_arr(jolt_gui_obj_t *digit_entry, uint8_t *arr, uint8_t arr_len) {
    int8_t n_entries = -1;
    digit_entry_cont_ext_t *ext = NULL;
    JOLT_GUI_CTX{
        ext = lv_obj_get_ext_attr(digit_entry);
        if( arr_len < ext->num_rollers ) {
            /* insufficient output buffer */
            ESP_LOGE(TAG, "%s (%d): insufficient output buffer", __func__, __LINE__);
            break;
        }
        else{
            n_entries = ext->num_rollers;
        }
        for(uint8_t i=0; i < ext->num_rollers; i++) {
            arr[i] = (10 - lv_roller_get_selected(ext->rollers[i])) % 10;
        }
    }
#if ESP_LOG_LEVEL >= 4 /* debug */
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

uint8_t jolt_gui_scr_digit_entry_get_hash(jolt_gui_obj_t *scr, uint8_t *hash) {
    uint8_t res = 0;
    JOLT_GUI_CTX{
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );

        jolt_gui_obj_t *cont_body = NULL;
        jolt_gui_obj_t *digit_entry = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(scr, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_DIGIT_ENTRY);
        res = jolt_gui_obj_digit_entry_get_hash(digit_entry, hash);
    }
    return res;
}

uint8_t jolt_gui_obj_digit_entry_get_hash(jolt_gui_obj_t *digit_entry, uint8_t *hash) {
    uint8_t res = 1;
    JOLT_GUI_CTX{
        int32_t val = jolt_gui_obj_digit_entry_get_int(digit_entry);
        if( val < 0 ){
            ESP_LOGE(TAG, "Aborting hash; digit_entry returned a value of %d", val);
            break;
        }
        else{
            ESP_LOGD(TAG, "Hashing value %d", val);
        }

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

static unsigned concat_int(unsigned x, unsigned y) {
    unsigned pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
}

double jolt_gui_scr_digit_entry_get_double(jolt_gui_obj_t *scr) {
    double res = -1;
    JOLT_GUI_CTX{
        jolt_gui_obj_t *cont_body = NULL;
        jolt_gui_obj_t *digit_entry = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(scr, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_DIGIT_ENTRY);
        res = jolt_gui_obj_digit_entry_get_double(digit_entry);
    }
    return res;
}

double jolt_gui_obj_digit_entry_get_double(jolt_gui_obj_t *digit_entry){
    double res = -1;
    JOLT_GUI_CTX{
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr( digit_entry );

        res = (double)jolt_gui_obj_digit_entry_get_int(digit_entry);

        if( res < 0 ) break;

        for(int8_t i = ext->decimal_point_pos; i>0; i--) {
            res /= 10; 
        }
    }
    return res;
}

int32_t jolt_gui_scr_digit_entry_get_int(jolt_gui_obj_t *scr) {
    int32_t res = -1;
    JOLT_GUI_CTX{
        jolt_gui_obj_t *cont_body = NULL;
        jolt_gui_obj_t *digit_entry = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(scr, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_DIGIT_ENTRY);
        res = jolt_gui_obj_digit_entry_get_int(digit_entry);
    }
    return res;
}

int32_t jolt_gui_obj_digit_entry_get_int(jolt_gui_obj_t *digit_entry) {
    int32_t res = 0;
    JOLT_GUI_CTX{
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(digit_entry);

        uint8_t array[sizeof(ext->rollers)] = { 0 };
        int8_t n = jolt_gui_obj_digit_entry_get_arr(digit_entry, array, sizeof(array));

        if( n <= 0 ){
            res = -1;
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

void jolt_gui_scr_digit_entry_set_pos(jolt_gui_obj_t *scr, int8_t pos) {
    JOLT_GUI_CTX{
        jolt_gui_obj_t *cont_body = NULL;
        jolt_gui_obj_t *digit_entry = NULL;
        cont_body = JOLT_GUI_FIND_AND_CHECK(scr, JOLT_GUI_OBJ_ID_CONT_BODY);
        digit_entry = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_DIGIT_ENTRY);
        digit_entry_cont_ext_t *ext = lv_obj_get_ext_attr(digit_entry);
        if( pos < 0 || pos-1 > ext->num_rollers ){
            break;
        }
        lv_roller_set_visible_row_count(ext->rollers[ext->sel], 1);
        ext->sel = ext->num_rollers - 1 - pos; 
        lv_roller_set_visible_row_count(ext->rollers[ext->sel], 3);
    }
}
