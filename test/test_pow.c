#include "unity.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "nano_lib.h"
#include "pow.h"


TEST_CASE("Multicore PoW", "[nanoray]"){
    QueueHandle_t work_queue = xQueueCreate( 10, sizeof(uint64_t));

    TaskHandle_t h_core0 = NULL;
    TaskHandle_t h_core1 = NULL;

    uint64_t work;
    uint256_t previous;
    nl_err_t res;
    uint64_t t_start;
    uint64_t t_end;
    uint32_t t_duration;

    nl_generate_seed(previous);

    work_payload_t work_payload_0;
    work_payload_0.queue = &work_queue;
    work_payload_0.hash = previous;
    work_payload_0.start = 0;

    work_payload_t work_payload_1;
    work_payload_1.queue = &work_queue;
    work_payload_1.hash = previous;
    work_payload_1.start = 1ULL << 32;

    for(uint8_t i =0; i < 10; i++){
        nl_generate_seed(previous);

        t_start = esp_timer_get_time();

        xTaskCreatePinnedToCore(pow_task,
                "MultiPoW0", 4096,
                (void *) &work_payload_0, LOCAL_WORK_PRIORITY, &h_core0, 0);
        xTaskCreatePinnedToCore(pow_task,
                "MultiPoW1", 4096,
                (void *) &work_payload_1, LOCAL_WORK_PRIORITY, &h_core1, 1);

        xQueueReceive(work_queue, &work, portMAX_DELAY);

        if(h_core0 != NULL){
            vTaskDelete(h_core0);
        }
        if(h_core1 != NULL){
            vTaskDelete(h_core1);
        }

        res = nl_pow_verify(previous, work);
        t_end = esp_timer_get_time();
        t_duration = (t_end - t_start) / 1000000;
        TEST_ASSERT_FALSE(res);
        printf("PoW %d time: %ds\n", i, t_duration);
    }
}
