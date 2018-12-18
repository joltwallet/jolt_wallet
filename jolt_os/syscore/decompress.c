#include "heatshrink_decoder.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "filesystem.h"
#include "decompress.h"

#include "esp_log.h"

static const char TAG[] = "decompress";


/* Decompress a ".hs" file.
 * Requires full filename including spiffs path */
int decompress_hs_file(char *src_fn){
    /* Sanity Checks */
    size_t len = strlen(src_fn);
    if( len <= 3 ) {
        ESP_LOGE(TAG, "Filename too short");
        return -1;
    }
    // todo: make maximum filename length
    if( len > CONFIG_SPIFFS_OBJ_NAME_LEN ) {
        ESP_LOGE(TAG, "Filename too long");
    }

    /* Confirm the suffix is ".hs" */
    if( 0 != strcmp( src_fn+len-3, ".hs" ) ) {
        ESP_LOGE(TAG, "File does not have a \".hs\" extension.");
        return -1;
    }

    /* Make a copy of the filename without the ".hs" suffix */
    char dst_fn[CONFIG_SPIFFS_OBJ_NAME_LEN] = { 0 };
    strncpy(dst_fn, src_fn, len-3);

    FILE* compressed   = fopen(src_fn, "rb");
    FILE* decompressed = fopen(dst_fn, "wb");

    int res = decompress_fd_to_fd(compressed, decompressed);

    fclose(compressed);
    fclose(decompressed);

    return res;
}

/* Reads compressed file from storage, writes decompressed file to storage */
int decompress_fd_to_fd(FILE *compressed, FILE *decompressed) {
    size_t file_size = decompress_check_size( compressed );

    heatshrink_decoder *hsd = heatshrink_decoder_alloc(
            CONFIG_JOLT_HEATSHRINK_BUFFER,
            CONFIG_JOLT_HEATSHRINK_WINDOW,
            CONFIG_JOLT_HEATSHRINK_LOOKAHEAD);
  
    size_t sink_head = 0, to_sink = 0;
    uint8_t write_buf[CONFIG_JOLT_HEATSHRINK_BUFFER];
    size_t out_head = 0, out_remaining = sizeof(write_buf);

    for( ;; ) { // iterate over the entire compressed file
        // Sinking
        uint8_t read_buf[CONFIG_JOLT_HEATSHRINK_BUFFER];
        size_t count = 0;
        if( to_sink == 0 ) { // refill buffer if read_buf is empty
            to_sink = fread(read_buf, 1, sizeof(read_buf), compressed);
            //ESP_LOGI(TAG, "first 4 bytes: 0x%02x%02x%02x%02x", read_buf[0], read_buf[1], read_buf[2], read_buf[3]);
            if( 0 == to_sink ) {
                break;
            }
            ESP_LOGD(TAG, "Read %d bytes into read_buf", to_sink);
            sink_head = 0;
        }
        heatshrink_decoder_sink(hsd, &read_buf[sink_head], to_sink, &count);
        to_sink -= count;
        sink_head = count;
        
        // Polling
        HSD_poll_res pres;
        do { // poll decoder until it stops producing
            pres = heatshrink_decoder_poll(hsd, &write_buf[out_head], out_remaining, &count);
            ESP_LOGD(TAG, "Decompressed %d bytes", count);
            out_remaining -= count;
            out_head += count;
            if( HSDR_POLL_MORE == pres && 0 == count) {
                /* Write the write buffer to disk */
                ESP_LOGD(TAG, "Writing Decompressed Buffer to Disk");
                size_t amount_written = fwrite(write_buf, 1, out_head, decompressed);
                if(amount_written != out_head){
                    ESP_LOGE(TAG, "Filesystem full, error decompressing.");
                    heatshrink_decoder_free(hsd);
                    return -1;
                }
                out_head = 0; out_remaining = sizeof(write_buf);
            }
        } while( HSDR_POLL_MORE == pres );
    }
    HSD_finish_res fres = heatshrink_decoder_finish(hsd);
    heatshrink_decoder_free(hsd);

    /* Write the remaining buffer amount to disk */
    size_t amount_written = fwrite(write_buf, 1, out_head, decompressed);
    if(amount_written != out_head){
        ESP_LOGE(TAG, "Filesystem full, error decompressing.");
        return -1;
    }

    switch(fres){
        case HSDR_FINISH_DONE:
            ESP_LOGI(TAG, "Decompression complete");
            return 0;
        case HSDR_FINISH_MORE:
            ESP_LOGE(TAG, "Insufficient decompression buffer");
            return -1;
        default:
            ESP_LOGE(TAG, "Decompression finsih NULL arguments");
            return -1;
    }

}

/* Decompresses file into already allocated memory.
 * Assumes fd is pointing to beginning of compressed data 
 * (not the reported size header). */
int decompress_fd_to_alloc_mem(FILE *fd, uint8_t *mem, const size_t mem_size) {
    size_t file_size = decompress_check_size( fd );

    heatshrink_decoder *hsd = heatshrink_decoder_alloc(
            CONFIG_JOLT_HEATSHRINK_BUFFER,
            CONFIG_JOLT_HEATSHRINK_WINDOW,
            CONFIG_JOLT_HEATSHRINK_LOOKAHEAD);
  
    size_t sink_head = 0, to_sink = 0;
    size_t out_head = 0, out_remaining = mem_size;

    for( ;; ) { // iterate over the entire compressed file
        // Sinking
        uint8_t read_buf[CONFIG_JOLT_HEATSHRINK_BUFFER];
        size_t count = 0;
        if( to_sink == 0 ) { // refill buffer if read_buf is empty
            to_sink = fread(read_buf, 1, sizeof(read_buf), fd);
            //ESP_LOGI(TAG, "first 4 bytes: 0x%02x%02x%02x%02x", read_buf[0], read_buf[1], read_buf[2], read_buf[3]);
            if( 0 == to_sink ) {
                break;
            }
            ESP_LOGD(TAG, "Read %d bytes into read_buf", to_sink);
            sink_head = 0;
        }
        heatshrink_decoder_sink(hsd, &read_buf[sink_head], to_sink, &count);
        to_sink -= count;
        sink_head = count;
        
        // Polling
        HSD_poll_res pres;
        do { // poll decoder until it stops producing
            pres = heatshrink_decoder_poll(hsd, &mem[out_head], out_remaining, &count);
            ESP_LOGD(TAG, "Decompressed %d bytes", count);
            out_remaining -= count;
            out_head += count;
            if( HSDR_POLL_MORE == pres && 0 == count) {
                ESP_LOGE(TAG, "Didn't allocate enough space for decompressed app");
                heatshrink_decoder_free(hsd);
                return -1;
            }
        } while( HSDR_POLL_MORE == pres );
    }
    HSD_finish_res fres = heatshrink_decoder_finish(hsd);
    heatshrink_decoder_free(hsd);

    switch(fres){
        case HSDR_FINISH_DONE:
            ESP_LOGI(TAG, "Decompression complete");
            return 0;
        case HSDR_FINISH_MORE:
            ESP_LOGE(TAG, "Insufficient decompression buffer");
            return -1;
        default:
            ESP_LOGE(TAG, "Decompression finsih NULL arguments");
            return -1;
    }
}

/* Reports the expected decompressed size.
 * FD will now be pointing at compressed data */
size_t decompress_check_size(FILE *fd){
    fseek(fd, 0, SEEK_SET);
    size_t uncompressed_size;
    fread(&uncompressed_size, 4, 1,fd); // read the uncompressed size we prepended during build
    uncompressed_size += 1; // 1 more byte is required
    ESP_LOGI(TAG, "File claims to be %d bytes uncompressed", uncompressed_size);
    return uncompressed_size;
}

/* Decompresses file into mem. This function malloc's mem which will need
 * to be freed externally.
 * Requires full filename */
uint8_t *decompress_fn_to_mem(char *fn) {
    uint8_t *mem = NULL;
    FILE *f = NULL;
    f = fopen(fn, "rb");

    size_t uncompressed_size = decompress_check_size(f);
    if( NULL == (mem = malloc(uncompressed_size)) ) {
        ESP_LOGE(TAG, "Couldn't allocate space for program buffer for %s.", fn);
        fclose(f);
        return NULL;
    }

    if( 0 != decompress_fd_to_alloc_mem(f, mem, uncompressed_size) ){
        free(mem);
        mem = NULL;
    }

    fclose(f);
    return mem;
}

