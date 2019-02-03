#include "bluetooth_cfg.h"
#include "bluetooth_state.h"
#include "bluetooth_gatts_profile_a.h"
#include "hal/radio/spp_recv_buf.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"


#define GATTS_TABLE_TAG  "GATTS_SPP_DEMO"
static bool is_connected = false;

bool gatts_profile_a_is_connected() {
    return is_connected;
}

static uint8_t find_char_and_desr_index(uint16_t handle) {
    for(int i = 0; i < SPP_IDX_NB ; i++) {
        if( handle == spp_handle_table[i]) {
            return i;
        }
    }
    return 0xff; // error
}

/* Only used to handle events for SPP_PROFILE_A_APP_ID */
void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, 
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {

    esp_ble_gatts_cb_param_t *p_data = (esp_ble_gatts_cb_param_t *) param;
    uint8_t res = 0xff;

    ESP_LOGE(GATTS_TABLE_TAG, "GATTS event %d", event);
    switch (event) {
    	case ESP_GATTS_REG_EVT:
        	esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
            esp_ble_gap_config_local_privacy(true);
        	esp_ble_gap_config_adv_data_raw((uint8_t *)spp_adv_data,
                    sizeof(spp_adv_data));
        	esp_ble_gatts_create_attr_tab(spp_gatt_db, gatts_if, 
                    SPP_IDX_NB, SPP_SVC_INST_ID);
       	    break;
    	case ESP_GATTS_READ_EVT:
            res = find_char_and_desr_index(p_data->read.handle);
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT : handle = %d\n", res);
            if(res == SPP_IDX_SPP_STATUS_VAL){
                //TODO: client read the status characteristic
            }
       	    break;
    	case ESP_GATTS_WRITE_EVT: {
    	    res = find_char_and_desr_index(p_data->write.handle);
            if(p_data->write.is_prep == false){
                ESP_LOGD(GATTS_TABLE_TAG, "ESP_GATTS_WRITE_EVT : handle = %d\n", res);
                if(res == SPP_IDX_SPP_COMMAND_VAL){
                    /* Allocate memory for 1 MTU;
                     * send it off to the ble_in_queue */
                    ESP_LOGD(GATTS_TABLE_TAG, "SPP_IDX_SPP_COMMAND_VAL;"
                            " Allocating %d bytes.", spp_mtu_size-3);
                    uint8_t * spp_cmd_buff = NULL;
                    spp_cmd_buff = (uint8_t *)malloc((spp_mtu_size - 3) * sizeof(uint8_t));
                    if(spp_cmd_buff == NULL){
                        ESP_LOGE(GATTS_TABLE_TAG, "%s malloc failed\n", __func__);
                        break;
                    }
                    memset(spp_cmd_buff, 0, (spp_mtu_size - 3));
                    memcpy(spp_cmd_buff, p_data->write.value, p_data->write.len);
                    xQueueSend(ble_in_queue, &spp_cmd_buff, 10/portTICK_PERIOD_MS);
                }
                else if(res == SPP_IDX_SPP_DATA_NOTIFY_CFG){
                    ESP_LOGD(GATTS_TABLE_TAG, "SPP_IDX_SPP_DATA_NOTIFY_CFG");
                    if( (p_data->write.len == 2)
                            &&(p_data->write.value[0] == 0x01)
                            &&(p_data->write.value[1] == 0x00) ) {
                    }
                    else if( (p_data->write.len == 2)
                            &&(p_data->write.value[0] == 0x00)
                            &&(p_data->write.value[1] == 0x00) ) {
                    }
                }
                else if( res == SPP_IDX_SPP_DATA_RECV_VAL ) {
                    /* Phone/Computer sent string to Jolt */
                    ESP_LOGD(GATTS_TABLE_TAG, "SPP_IDX_SPP_DATA_RECV_VAL");
                    #ifdef SPP_DEBUG_MODE
                    esp_log_buffer_char(GATTS_TABLE_TAG,
                            (char *)(p_data->write.value),p_data->write.len);
                    #else
                    /*
                    uart_write_bytes(UART_NUM_0, 
                            (char *)(p_data->write.value), p_data->write.len);
                    uart_write_bytes(UART_NUM_0, "\n", 1);
                    */
                    #endif

                }
                else{
                    ESP_LOGI(GATTS_TABLE_TAG, "Unknown state machine attribute %d.",
                            res);
                    //TODO:
                }
            }
            else if( (p_data->write.is_prep == true)
                    && (res == SPP_IDX_SPP_DATA_RECV_VAL) ) {
                ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_PREP_WRITE_EVT : handle = %d\n", res);
                store_wr_buffer(p_data);
            }
      	 	break;
    	}
    	case ESP_GATTS_EXEC_WRITE_EVT:{
    	    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT\n");
    	    if(p_data->exec_write.exec_write_flag){
    	        print_write_buffer();
    	        free_write_buffer();
    	    }
    	    break;
    	}
    	case ESP_GATTS_MTU_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVENT;"
                    "Setting MTU size to %d bytes.", p_data->mtu.mtu);
    	    spp_mtu_size = p_data->mtu.mtu;
    	    break;
    	case ESP_GATTS_CONF_EVT:
    	    break;
    	case ESP_GATTS_UNREG_EVT:
        	break;
    	case ESP_GATTS_DELETE_EVT:
        	break;
    	case ESP_GATTS_START_EVT:
        	break;
    	case ESP_GATTS_STOP_EVT:
        	break;
    	case ESP_GATTS_CONNECT_EVT:
    	    is_connected = true;
            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
        	break;
    	case ESP_GATTS_DISCONNECT_EVT:
    	    is_connected = false;
            esp_ble_gap_start_advertising( (esp_ble_adv_params_t *)&spp_adv_pair_params );
    	    break;
    	case ESP_GATTS_OPEN_EVT:
    	    break;
    	case ESP_GATTS_CANCEL_OPEN_EVT:
    	    break;
    	case ESP_GATTS_CLOSE_EVT:
    	    break;
    	case ESP_GATTS_LISTEN_EVT:
    	    break;
    	case ESP_GATTS_CONGEST_EVT:
    	    break;
    	case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
    	    ESP_LOGI( GATTS_TABLE_TAG, "The number handle =%x\n",
                    param->add_attr_tab.num_handle );
    	    if ( param->add_attr_tab.status != ESP_GATT_OK ) {
    	        ESP_LOGE(GATTS_TABLE_TAG, 
                        "Create attribute table failed, error code=0x%x",
                        param->add_attr_tab.status);
    	    }
    	    else if (param->add_attr_tab.num_handle != SPP_IDX_NB){
                ESP_LOGE(GATTS_TABLE_TAG, "Create attribute table abnormally, "
                        "num_handle (%d) doesn't equal to HRS_IDX_NB(%d)",
                        param->add_attr_tab.num_handle, SPP_IDX_NB);
    	    }
    	    else {
    	        memcpy(spp_handle_table, param->add_attr_tab.handles,
                        sizeof(spp_handle_table));
    	        esp_ble_gatts_start_service(spp_handle_table[SPP_IDX_SVC]);
    	    }
    	    break;
    	}
    	default:
    	    break;
    }
}

