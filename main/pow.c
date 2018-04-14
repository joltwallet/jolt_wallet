#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "nano_lib.h"
#include "pow.h"

void pow_task(void *work_payload){
    uint64_t work = nl_compute_local_pow(
            ((work_payload_t *)work_payload)->hash,
            ((work_payload_t *)work_payload)->start);
    printf("work: %llx\n", work);
    xQueueSend(*(((work_payload_t *)work_payload)->queue), &work, 0);
    vTaskSuspend(NULL);
}
