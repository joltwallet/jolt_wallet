#include "stdio.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "cJSON.h"

/**
 * @brief Adds a json node with key with heap metadata according to flags
 * @param[in] json parenting json node to add to
 * @param[in] key key to insert the node into json as
 * @param[in] flags esp-idf MALLOC_CAP_* flags
 * @return True on success, False on failure
 */
static bool add_heap_info_to_object( cJSON *json, const char *key, uint32_t flags) {
    cJSON *child = NULL;
    multi_heap_info_t info = { 0 };

    heap_caps_get_info(&info, flags);
    if( NULL == (child = cJSON_CreateObject()) ) goto end;
    cJSON_AddItemToObjectCS(json, key, child);
    if( NULL == cJSON_AddNumberToObject(child, "free", info.total_free_bytes) ) goto end;
    if( NULL == cJSON_AddNumberToObject(child, "used", info.total_allocated_bytes) ) goto end;

    return true;
end:
    return false;
}

int jolt_cmd_free(int argc, char** argv) {
    int return_code = -1;
    char *response = NULL;
    cJSON *json = NULL;

    if( NULL == (json = cJSON_CreateObject()) ) goto end;

    if( !add_heap_info_to_object(json, "exec", MALLOC_CAP_EXEC) ) goto end;
    if( !add_heap_info_to_object(json, "32bit", MALLOC_CAP_32BIT) ) goto end;
    if( !add_heap_info_to_object(json, "8bit", MALLOC_CAP_8BIT) ) goto end;
    if( !add_heap_info_to_object(json, "dma", MALLOC_CAP_DMA) ) goto end;

    response = cJSON_Print(json);
    printf(response);
    printf("\n");

    return_code = 0;

end:
    if( NULL != json ) cJSON_Delete(json);
    if( NULL != response ) free(response);
    return return_code;
}

