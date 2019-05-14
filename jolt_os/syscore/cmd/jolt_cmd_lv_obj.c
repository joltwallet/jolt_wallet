#include "jolt_lib.h"

#define PRINT_AND_END(x) if(print_response(x)) return_code = 0; else return_code=1; goto end;

static bool print_response(cJSON * json) {
    char *response = NULL;
    response = cJSON_Print(json);
    if( NULL == response ) return false;
    printf(response);
    printf("\n");
    free(response);
    return true;
}

int jolt_cmd_lv_obj(int argc, char** argv) {
    int return_code = -1;
    cJSON *json = NULL;

    if( NULL == (json = cJSON_CreateObject()) ) goto end;

    uint16_t n_objs = lv_obj_count_children_recursive(lv_scr_act());
    cJSON_AddNumberToObject(json, "num_obj", n_objs);
    cJSON_AddNumberToObject(json, "lv_obj_size", sizeof(lv_obj_t));
    cJSON_AddNumberToObject(json, "total_bytes", sizeof(lv_obj_t) * n_objs);
    PRINT_AND_END(json);

end:
    if( NULL != json ) cJSON_Delete(json);
    return return_code;
}
