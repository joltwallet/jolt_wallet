#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "filesystem.h"
#include "rom/miniz.h"
#include "decompress.h"

#include "esp_log.h"

static const char TAG[] = "decompress";

#define MINIZ_INFLATE_BUF_SIZE 4096

typedef struct decomp_t{
    tinfl_decompressor inflator;
    write_fun_t writer;
    void *cookie;
    uint8_t *buf;
    uint8_t *next_out;
} decomp_t;


decomp_t *decompress_obj_init( write_fun_t writer, void *cookie){
    decomp_t *d;

    d = calloc(1, sizeof(decomp_t));
    if( NULL == d ) goto exit;

    d->buf = malloc(MINIZ_INFLATE_BUF_SIZE);
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
    /* todo; flush */
    if(d->buf) {
        free(d->buf);
    }
    free(d);
}

int decompress_obj_chunk(decomp_t *d, uint8_t *data, size_t len) {
    int amount_written = 0;

    while(len > 0 ) {
        size_t in_bytes = len; /* input remaining */
        size_t out_bytes = d->buf + MINIZ_INFLATE_BUF_SIZE - d->next_out;
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
        if (status <= TINFL_STATUS_DONE || bytes_in_out_buf == MINIZ_INFLATE_BUF_SIZE) {
            // Output buffer full, or done. Flush to writer function.
            d->writer(d->buf, 1, bytes_in_out_buf, d->cookie);
            d->next_out = d->buf;
        }
    }

    return amount_written;
}
