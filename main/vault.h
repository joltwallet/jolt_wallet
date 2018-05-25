#ifndef __JOLT_VAULT_H__
#define __JOLT_VAULT_H__

#include "nvs.h"
#include "nano_lib.h"

/* Structure to store anything that if modified could perform something
 * malicious
 */
typedef struct vault_t{
    char mnemonic[MNEMONIC_BUF_LEN];
    uint512_t master_seed;
    bool valid;
} vault_t;

/* Add RPC command types here. If they are coin specific, it must be specified
 * between dummy commands "COIN_START" and "COIN_END". */
typedef enum vault_rpc_type_t {
    SYSCORE_START = 0,
    SYSCORE_END,

    NANO_START,
    NANO_BLOCK_SIGN, // Signs passed in block
    NANO_PUBLIC_KEY, // Derive public key at index
    NANO_CONTACT_UPDATE, // Update stored contact
    NANO_END

} vault_rpc_type_t;

typedef enum vault_rpc_response_t {
    RPC_SUCCESS = 0,
    RPC_FAILURE,
    RPC_CANCELLED,
    RPC_UNDEFINED,
    RPC_QUEUE_FULL
} vault_rpc_response_t;

typedef struct vault_rpc_t {
    enum vault_rpc_type_t type;
    uint64_t timestamp;
    QueueHandle_t response_queue;
    union{
        struct{
            char *mnemonic;
        } syscore_mnemonic_restore;
        /* NANO START */
        struct{
            struct nl_block_t block;
            uint32_t index;
        } nano_public_key;
        struct{
            struct nl_block_t block;
            uint32_t index;
            struct nl_block_t frontier;
        } nano_block_sign;
        struct{
            char *name;
            uint8_t index;
            uint8_t *public; // pointer to public key
        } nano_contact_update;
        /* NANO END */
    };
} vault_rpc_t;

vault_rpc_response_t vault_rpc(vault_rpc_t *rpc);
nl_err_t vault_init();
void vault_task(void *vault_in);
nl_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace);

#endif
