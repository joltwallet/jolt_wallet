#ifndef __JOLT_CONSOLE_H__
#define __JOLT_CONSOLE_H__

typedef struct backend_t{
    bool valid;
} backend_t;

typedef struct backend_rpc_t {
    enum vault_rpc_type_t type;
    uint64_t timestamp;
    QueueHandle_t response_queue;
    uint32_t block_count;
} backend_rpc_t;

/* Add RPC command types here. If they are coin specific, it must be specified
 * between dummy commands "COIN_START" and "COIN_END". */
typedef enum backend_rpc_type_t {
    FREE = 0, // Wipe all NVS
    WIFI,
    
    NANO_CMD_START,
    NANO_BLOCK_COUNT,
    NANO_CMD_END
    
} backend_rpc_type_t;

typedef enum backend_rpc_response_t {
    RPC_CMD_SUCCESS = 0,
    RPC_CMD_FAILURE,
    RPC_CMD_CANCELLED,
    RPC_CMD_UNDEFINED,
    RPC_CMD_QUEUE_FULL
} backend_rpc_response_t;

void initialize_console();
void menu_console(menu8g2_t *prev);
backend_rpc_response_t backend_rpc(backend_rpc_t *rpc);
void backend_task(void *backend_in);

#endif
