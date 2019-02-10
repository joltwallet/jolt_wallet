#if JOLT_GUI_TEST_MENU
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/test_screens.h"
#include "jolt_helpers.h"
#include <driver/adc.h>

#include "esp_http_client.h" 

static const char TAG[] = "test_screens";

static lv_res_t jolt_gui_test_number_enter_cb(lv_obj_t *parent){
    double d_val = jolt_gui_scr_digit_entry_get_double(parent);
    uint32_t i_val = jolt_gui_scr_digit_entry_get_int(parent);
    ESP_LOGI(TAG, "Entry as Double: %f", d_val);
    ESP_LOGI(TAG, "Entry as Integer: %d", i_val);
    return jolt_gui_scr_del( parent );
}

lv_res_t jolt_gui_test_number_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_digit_entry_create( "Number Test", 7, 2); 
    jolt_gui_scr_set_enter_action(scr, jolt_gui_test_number_enter_cb);
    return LV_RES_OK;
}

lv_res_t jolt_gui_test_qrcode_create(lv_obj_t *btn) {
    const char data[] = "Meow";
    lv_obj_t *scr = jolt_gui_scr_qr_create("QR Test", "Meow", sizeof(data));
	return LV_RES_OK;
}

void test_loading_task(void *param) {
    lv_obj_t *scr = (lv_obj_t *)param;
    for(uint8_t i=0;i < 101; vTaskDelay(pdMS_TO_TICKS(1000)), i+=10){
        if(i==50){
            jolt_gui_scr_loading_update(scr, "Almost Done", "woof", i);
        }
        else if(i>50){
            jolt_gui_scr_loading_update(scr, NULL, "bark", i);
        }
        else{
            jolt_gui_scr_loading_update(scr, NULL, "meow", i);
        }
    }
    lv_obj_del(scr);
    vTaskDelete(NULL);
}

lv_res_t jolt_gui_test_loading_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_loading_create("Loading Test");
    if(NULL == scr){
        ESP_LOGE(TAG, "NULL Loading Screen");
        return 1;
    }
    xTaskCreate(test_loading_task,
                "TestLoading", 4096,
                (void *) scr, 10, NULL);
    return LV_RES_OK;
}


/* Screen that gives info on the battery */
lv_task_t *test_battery_task_h = NULL;
lv_obj_t *test_battery_scr = NULL;

lv_res_t jolt_gui_test_battery_del(lv_obj_t *btn) {
    lv_task_del(test_battery_task_h);
    lv_obj_del(test_battery_scr);
    return LV_RES_INV;
}
void jolt_gui_test_battery_task(void *param) {
    if(NULL != test_battery_scr) {
        lv_obj_del(test_battery_scr);
    }
    int val = adc1_get_raw(JOLT_ADC1_VBATT);
    char buf[40];
    snprintf(buf, sizeof(buf), "Raw Value: %d\nPercentage: %d", val,
            statusbar_indicators[JOLT_GUI_STATUSBAR_INDEX_BATTERY].val);
    test_battery_scr = jolt_gui_scr_text_create("Battery", buf);
    jolt_gui_scr_set_back_action(test_battery_scr, jolt_gui_test_battery_del);
}

lv_res_t jolt_gui_test_battery_create(lv_obj_t *btn) {
    test_battery_task_h = lv_task_create(jolt_gui_test_battery_task, 300, LV_TASK_PRIO_LOW, NULL);
    return LV_RES_OK;
}

lv_res_t jolt_gui_test_alphabet_create(lv_obj_t * list_btn) {
    /* Dummy Text Page for Testing */
    lv_obj_t *scr = jolt_gui_scr_text_create("Alphabet", 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
            "abcdefghijklmnopqrstuvwxyz "
            "1234567890 "
            "!@#$%^&*()-+_="
            "{}[];':\",.<>?/\\"
            " ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"
            "The Quick Brown Fox, Jumped Over The Lazy Dog.\n"
            "Testing proper wrap around of very long words and address. "
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ."
            );
    jolt_gui_scr_scroll_add_monospace_text(scr,
            "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
    return LV_RES_OK;
}

lv_res_t jolt_gui_test_https_create( lv_obj_t *btn ) {
    esp_http_client_handle_t client;
    esp_http_client_config_t config = {
        .url    = "https://yapraiwallet.space/quake/api",
        /* todo: provide certificate in PEM format */ 
        //.cert_pem = howsmyssl_com_root_cert_pem_start,
        /*todo: allow async configuration*/
        //.is_async
        //.timeout_ms = 5000,
    };
    client = esp_http_client_init( &config );
    const char post_data[]  = "{ \"action\" : \"block_count\"}";
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform( client );

    if (err == ESP_OK) {
        int status_code    = esp_http_client_get_status_code( client );
        int content_length = esp_http_client_get_content_length( client );
        switch( status_code ) {
            case 200:{
                char *buf = malloc( content_length + 1 );
                int read_len = esp_http_client_read(client, buf, content_length);
                buf[read_len] = '\0';
                printf("Response:\n");
                printf(buf);
                printf("\n");
                free(buf);
                break;
            }
            default:
                ESP_LOGE(TAG, "HTTP Error Code %d", status_code);
                break;
        }
    }
    esp_http_client_cleanup( client );
    return LV_RES_OK;
}

#endif
