#include "spp_recv_buf.h"

#include "esp_log.h"
#include "string.h"
#include "driver/uart.h"

/* Singly Linked List Structs */
typedef struct spp_receive_data_node{
    int32_t len;
    uint8_t *node_buff;
    struct spp_receive_data_node *next_node;
} spp_receive_data_node_t;

typedef struct spp_receive_data_buff{
    int32_t node_num;
    int32_t buf_size;
    spp_receive_data_node_t *first_node;
} spp_receive_data_buff_t;

static spp_receive_data_buff_t SppRecvDataBuff = {
    .node_num   = 0,
    .buf_size  = 0,
    .first_node = NULL
};



static const char TAG[] = "gatts_spp_buf";

static spp_receive_data_node_t *p1 = NULL;
static spp_receive_data_node_t *p2 = NULL;

/* Allocate and Add to the SppRecvDataBuff singly linked list */
bool store_wr_buffer(esp_ble_gatts_cb_param_t *p_data) {
    p1 = (spp_receive_data_node_t *) 
            malloc( sizeof(spp_receive_data_node_t) );
    if(p1 == NULL){
        ESP_LOGI(TAG, "malloc error %s %d\n", __func__, __LINE__);
        return false;
    }

    if(p2 != NULL){
        p2->next_node = p1;
    }
    p1->len = p_data->write.len;
    SppRecvDataBuff.buf_size += p_data->write.len;
    p1->next_node = NULL;
    p1->node_buff = (uint8_t *)malloc(p_data->write.len);

    p2 = p1;
    memcpy(p1->node_buff, p_data->write.value, p_data->write.len);
    if( SppRecvDataBuff.node_num == 0 ) {
        SppRecvDataBuff.first_node = p1;
        SppRecvDataBuff.node_num++;
    }
    else {
        SppRecvDataBuff.node_num++;
    }
    return true;
}

/* Free the singly linked list */
void free_write_buffer(void) {
    p1 = SppRecvDataBuff.first_node;

    while(p1 != NULL){
        p2 = p1->next_node;
        free(p1->node_buff);
        free(p1);
        p1 = p2;
    }

    SppRecvDataBuff.node_num = 0;
    SppRecvDataBuff.buf_size = 0;
    SppRecvDataBuff.first_node = NULL;
}

/* Print the singly linked list buffer */
void print_write_buffer(void) {
    p1 = SppRecvDataBuff.first_node;

    while(p1 != NULL) {
        uart_write_bytes(UART_NUM_0, (char *)(p1->node_buff), p1->len);
        p1 = p1->next_node;
    }
}


