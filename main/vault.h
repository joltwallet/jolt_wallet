#ifndef __NANORAY_VAULT_H__
#define __NANORAY_VAULT_H__

#include "nvs.h"
#include "nano_lib.h"

/* Structure to store anything that if modified could perform something
 * malicious
 */
typedef struct vault_t{
    char mnemonic[MNEMONIC_BUF_LEN];
    uint512_t master_seed;
    uint32_t index;
    bool valid;
} vault_t;

typedef enum vault_rpc_type_t {
    BLOCK_SIGN = 0, // Signs passed in block
    PUBLIC_KEY, // Derive public key at index
    SETTINGS_CHANGE, // Change a setting
    FACTORY_RESET, // Wipe all NVS
    BLUETOOTH_PAIR,
    BLUETOOTH_UNPAIR
} vault_rpc_type_t;

typedef enum vault_rpc_response_t {
    RPC_SUCCESS = 0,
    RPC_FAILURE
} vault_rpc_response_t;

typedef union vault_payload_t {
    union{
        struct nl_block_t block;
        uint32_t index;
    } public_key;
} vault_payload_t;

typedef struct vault_rpc_t {
    enum vault_rpc_type_t type;
    QueueHandle_t response_queue;
    vault_payload_t payload;
} vault_rpc_t;

nl_err_t vault_init();
void vault_task(void *vault_in);
nl_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace);

#endif
