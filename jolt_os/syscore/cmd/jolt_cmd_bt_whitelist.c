#include "stdio.h"
#include "esp_system.h"
#include "sdkconfig.h"


#if CONFIG_BT_ENABLED

#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"

#endif

int jolt_cmd_bt_whitelist(int argc, char **argv){
    /* Bonded Devices */

#if CONFIG_BT_ENABLED
    {
        int dev_num = esp_ble_get_bond_device_num();
        printf("There are %d bonded BLE devices.\n", dev_num);

        esp_ble_bond_dev_t *dev_list = malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
        esp_ble_get_bond_device_list(&dev_num, dev_list);
        for (int i = 0; i < dev_num; i++) {
            printf("%d. bd_addr: ", i+1);
            for(uint8_t j=0; j<ESP_BD_ADDR_LEN; j++){
                printf("%02X ", dev_list[i].bd_addr[j]);
            }
            printf("\n");
        }
        free(dev_list);
    }
    printf("-----------------\n");
    /* White List */
    {
        esp_err_t err;
        uint16_t len;
        err = esp_ble_gap_get_whitelist_size( &len );
        if( ESP_OK != err ){
            return -1;
        }
        printf("There are %d slots free in the whitelist.\n", len);
    }
    return 0;
#else
    printf("JoltOS was compiled without bluetooth support.\n");
    return -1;
#endif

}

