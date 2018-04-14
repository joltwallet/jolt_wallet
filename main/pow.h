#ifndef __NANORAY_POW_H__
#define __NANORAY_POW_H__

#define LOCAL_WORK_PRIORITY 2

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct work_payload_t{
    QueueHandle_t *queue;
    unsigned char *hash;
    uint64_t start;
} work_payload_t;


/* It's really a work_payload_t pointer */
void pow_task(void *work_payload);

#endif
