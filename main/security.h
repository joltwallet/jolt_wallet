#ifndef __NANORAY_SECURITY_H__
#define __NANORAY_SECURITY_H__

#include "nvs.h"
#include "nano_lib.h"

nvs_handle *nvs_user; // Namespace for holding user configs
nvs_handle *nvs_public; // Namespace for holding public keys, etc.
nvs_handle *nvs_secret; // Namespace for holding encrypted data

/* Structure to store anything that if modified could perform something
 * malicious
 */
typedef struct vault_t{
    char mnemonic[MNEMONIC_BUF_LEN];
    uint512_t master_seed;
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

typedef struct vault_rpc_t {
    enum vault_rpc_type_t type;

} vault_rpc_t

extern vault_t *vault; // global vault variable

nl_err_t vault_init();
void vault_access_task(void *menu8g2);
nl_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace);

#endif
