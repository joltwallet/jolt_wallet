#include "stdio.h"
#include "esp_system.h"

int jolt_cmd_free(int argc, char** argv) {
    printf("Free: %d bytes\n", esp_get_free_heap_size());
    return 0;
}

