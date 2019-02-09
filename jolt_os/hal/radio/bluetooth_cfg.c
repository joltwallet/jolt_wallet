/* bluetooth_cfg holds constants that configure various bluetooth components */

#include "bluetooth_cfg.h"

/* General Notes
 *    * Characteristic *declarations* should always be read only
 *
 */

/// SPP Service
static const uint16_t spp_service_uuid = 0xFFE0;

#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))
static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_write          = ESP_GATT_CHAR_PROP_BIT_WRITE_NR | ESP_GATT_CHAR_PROP_BIT_READ;

/* SPP Service - data receive characteristic, read&write without response */
static const uint16_t spp_data_receive_uuid        = ESP_GATT_UUID_SPP_DATA_RECEIVE;
static const uint8_t  spp_data_receive_val[20]     = {0x00};

/* SPP Service - data notify characteristic, notify&read */
static const uint16_t spp_data_notify_uuid         = ESP_GATT_UUID_SPP_DATA_NOTIFY;
static const uint8_t  spp_data_notify_val[20]      = { 0x00 };
static const uint8_t  spp_data_notify_ccc[2]       = { 0x00, 0x00 };

/* SPP Service - command characteristic, read&write without response */
static const uint16_t spp_command_uuid             = ESP_GATT_UUID_SPP_COMMAND_RECEIVE;
static const uint8_t  spp_command_val[10]          = { 0x00 };

uint16_t spp_mtu_size = 23;

// Serial Port Profile Advertising Data
// todo: refine and document
const uint8_t spp_adv_data[23] = { /* Max Advertising Payload 31 bytes */
    0x02,0x01,0x06,             /* 2 bytes long; Flags Type; 
                                 *     * LE_GENERAL_DISC_MODE
                                 *     * BR_EDR_NOT_SUPPORTED
                                 */
    0x03,0x03,0xE0,0xFF,        /* 3 byte long; 16-bit Service Class UUIDs;
                                 *     * 0xFFE0
                                 */
    0x05,0x09,'J','o','l','t'   /* 5 bytes long; Complete Local Name */
};

/* Advertising Parameters */
/* For pairing with new devices */
const esp_ble_adv_params_t spp_adv_pair_params = {
    .adv_int_min        = 0x200, /* Minimum advertising interval for undirected and low duty cycle directed advertising. N * 0.625 mS */
    .adv_int_max        = 0x400,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_RANDOM,
    //.own_addr_type      = BLE_ADDR_TYPE_PUBLIC, // todo; double check
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/* For pairing with whitelisted devices */
const esp_ble_adv_params_t spp_adv_wht_params = {
    .adv_int_min        = 0x200, /* Minimum advertising interval for undirected and low duty cycle directed advertising. N * 0.625 mS */
    .adv_int_max        = 0x400,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_RANDOM,
    //.own_addr_type      = BLE_ADDR_TYPE_PUBLIC, // todo; double check
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    //.adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_WLST_CON_WLST, // esp-idf issue
};

const esp_gatts_attr_db_t spp_gatt_db[SPP_IDX_NB] = {
    /* SPP -  Service Declaration */
    [SPP_IDX_SVC] = {
        .attr_control = {
            // response of R/W operation will be replied by GATT stack automatically
            .auto_rsp = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&primary_service_uuid, // UUID value
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = sizeof(spp_service_uuid),
            .length      = sizeof(spp_service_uuid),
            .value       = (uint8_t *)&spp_service_uuid
        }
    },

    //SPP -  data receive characteristic Declaration
    [SPP_IDX_SPP_DATA_RECV_DECL] = {
        {
            ESP_GATT_AUTO_RSP
        },
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_declaration_uuid,
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = CHAR_DECLARATION_SIZE,
            .length      = CHAR_DECLARATION_SIZE, 
            .value       = (uint8_t *)&char_prop_read
        }
    },

    //SPP -  data receive characteristic Value
    [SPP_IDX_SPP_DATA_RECV_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&spp_data_receive_uuid,
            .perm        = ESP_GATT_PERM_READ ,
            //.perm        = ESP_GATT_PERM_READ_ENC_MITM | ESP_GATT_PERM_WRITE_SIGNED_MITM,
            .max_length  = SPP_DATA_MAX_LEN,
            .length      = sizeof(spp_data_receive_val),
            .value       = (uint8_t *)spp_data_receive_val
        }
    },

    //SPP -  data notify characteristic Declaration
    [SPP_IDX_SPP_DATA_NOTIFY_DECL]  = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_declaration_uuid,
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = CHAR_DECLARATION_SIZE,
            .length      = CHAR_DECLARATION_SIZE,
            .value       = (uint8_t *)&char_prop_read
        }
    },

    //SPP -  data notify characteristic Value
    [SPP_IDX_SPP_DATA_NOTIFY_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&spp_data_notify_uuid,
            .perm        = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            //.perm        = ESP_GATT_PERM_READ_ENC_MITM | ESP_GATT_PERM_WRITE_SIGNED_MITM,
            .max_length  = SPP_DATA_MAX_LEN,
            .length      = sizeof(spp_data_notify_val),
            .value       = (uint8_t *)spp_data_notify_val
        }
    },

    //SPP -  data notify characteristic - Client Characteristic Configuration Descriptor
    [SPP_IDX_SPP_DATA_NOTIFY_CFG] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_client_config_uuid,
            .perm        = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            //.perm        = ESP_GATT_PERM_READ_ENC_MITM | ESP_GATT_PERM_WRITE_SIGNED_MITM,
            .max_length  = sizeof(uint16_t),
            .length      = sizeof(spp_data_notify_ccc),
            .value       = (uint8_t *)spp_data_notify_ccc
        }
    },

    //SPP -  command characteristic Declaration
    [SPP_IDX_SPP_COMMAND_DECL] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_declaration_uuid,
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = CHAR_DECLARATION_SIZE,
            .length      = CHAR_DECLARATION_SIZE,
            .value       = (uint8_t *)&char_prop_read
        }
    },

    //SPP -  command characteristic Value
    [SPP_IDX_SPP_COMMAND_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {
           .uuid_length =  ESP_UUID_LEN_16,
           .uuid_p      =  (uint8_t *)&spp_command_uuid, 
            .perm        = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            //.perm        = ESP_GATT_PERM_READ_ENC_MITM | ESP_GATT_PERM_WRITE_SIGNED_MITM,
           .max_length  =  SPP_CMD_MAX_LEN,
           .length      =  sizeof(spp_command_val),
           .value       =  (uint8_t *)spp_command_val
        }
    },
};


