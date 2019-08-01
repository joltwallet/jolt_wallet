#include "esp_vfs_dev.h"
#include "filesystem.h"
#include "esp32/rom/miniz.h"
#include "decompress.h"

#include "esp_log.h"

static const char TAG[] = "decompress";

typedef struct decomp_t{
    tinfl_decompressor inflator; /**< miniz decompressor object */
    write_fun_t writer;          /**< writer function to flush decompressed data to */
    void *cookie;                /**< identifier data to be passed into the writer function */
    uint8_t *buf;                /**< Output ringbuffer to decompress data into. */
    uint8_t *next_out;           /**< Pointer into buf on where to read from */
} decomp_t;


decomp_t *decompress_obj_init( write_fun_t writer, void *cookie){
    decomp_t *d;

    ESP_LOGD(TAG, "Initializing decomp object");

    d = calloc(1, sizeof(decomp_t));
    if( NULL == d ) goto exit;

    d->buf = malloc(CONFIG_JOLT_COMPRESSION_OUTPUT_BUFFER);
    if( NULL == d->buf ) goto exit;

    tinfl_init(&(d->inflator));
    d->writer = writer;
    d->cookie = cookie;
    d->next_out = d->buf;

    return d;

exit:
    decompress_obj_del(d);
    return NULL;
}

void decompress_obj_del( decomp_t *d ) {
    if( NULL == d ) {
        return;
    }
    if(d->buf) {
        free(d->buf);
    }
    free(d);
}

int decompress_obj_chunk(decomp_t *d, uint8_t *data, size_t len) {
    int amount_written = 0;

    while(len > 0 ) {
        size_t in_bytes = len; /* input remaining */
        size_t out_bytes = d->buf + CONFIG_JOLT_COMPRESSION_OUTPUT_BUFFER - d->next_out;
        int flags = TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_HAS_MORE_INPUT;
        int status;

        status = tinfl_decompress(&(d->inflator), data, &in_bytes,
                         d->buf, d->next_out, &out_bytes,
                         flags);
        if(TINFL_STATUS_FAILED == status || in_bytes == 0){
            ESP_ERROR_CHECK(ESP_FAIL);
        }
        len -= in_bytes;
        data += in_bytes;

        amount_written += out_bytes;
        d->next_out += out_bytes;

        size_t bytes_in_out_buf = d->next_out - d->buf;
        if (status <= TINFL_STATUS_DONE || bytes_in_out_buf == CONFIG_JOLT_COMPRESSION_OUTPUT_BUFFER) {
            // Output buffer full, or done. Flush to writer function.
            d->writer(d->buf, 1, bytes_in_out_buf, d->cookie);
            d->next_out = d->buf;
        }
    }

    return amount_written;
}
