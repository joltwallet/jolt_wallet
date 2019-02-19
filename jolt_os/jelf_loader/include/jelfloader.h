#ifndef JELFLOADER_H__
#define JELFLOADER_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "jolttypes.h"
#include "sodium.h"

#if ESP_PLATFORM
#include "rom/miniz.h"
#else
#include "miniz.h"
#endif

#define LOADER_FD_T FILE *

#ifndef CONFIG_JOLT_APP_SIG_CHECK_EN
#define CONFIG_JOLT_APP_SIG_CHECK_EN 1
#endif

#ifndef CONFIG_JOLT_APP_KEY_DEFAULT
#define CONFIG_JOLT_APP_KEY_DEFAULT \
        "03a107bff3ce10be1d70dd18e74bc09967e4d6309ba50d5f1ddc8664125531b8"
#endif


typedef struct {
    const void **exported;            /*!< Pointer to exported symbols array */
    unsigned int exported_size; /*!< Elements on exported symbol array */
} jelfLoaderEnv_t;


/* Singly Linked List Used to cache sections needed at runtime */
typedef struct jelfLoaderSection_t {
    void *data;
    uint16_t secIdx;
    size_t size;
    off_t relSecIdx;                  
    struct jelfLoaderSection_t* next; // Next Header in Singly Linked List
} jelfLoaderSection_t;

typedef struct inf_stream_t {
    tinfl_decompressor inf;

    uint8_t *in_buf;                  /* inflator input buffer to hold compressed bytes*/
    size_t in_buf_len;                /* Size of input buffer */

    const unsigned char *in_next;     // pointer to next compressed byte to read
    size_t in_avail;                  // number of compressed bytes available at next_in

    uint8_t *out_buf;                 /* miniz output buffer to hold uncompressed bytes*/
    size_t out_buf_len;                /* Size of output buffer. MUST BE A POWER OF 2 */
    uint8_t *out_next;                /* For internal tinfl state */

    uint8_t *out_read;                /* Pointer to available uncompressed data to read from*/ 
    size_t out_avail;
} inf_stream_t;

typedef struct jelfLoaderContext_t {
    LOADER_FD_T fd;
    void* exec;
    const jelfLoaderEnv_t *env;

    /* Section Header Stuff */
    uint8_t *shdr_cache;
    uint16_t entry_index;
    uint16_t e_shnum;
    off_t e_shoff;

    jelfLoaderSection_t *section; // First element of singly linked list sections.

    /* symtab stuff */
    size_t symtab_count;
    uint8_t *symtab_cache;
    uint32_t *symtab_aux;
    uint8_t symtab_aux_len;
    void *symtab_symbols;
    size_t symtab_cache_size;

    /* Coin Derivation Data */
    uint32_t coin_purpose;
    uint32_t coin_path;
    char bip32_key[33];

    /* Inflator */
    inf_stream_t inf_stream;

    /* Data Structs For Checking App Signature */
#if CONFIG_JOLT_APP_SIG_CHECK_EN
    crypto_hash_sha512_state *hs;
    uint8_t hash[BIN_512];
    uint8_t app_public_key[BIN_256];
    uint8_t app_signature[BIN_512];
#endif
} jelfLoaderContext_t;


int jelfLoaderRun(jelfLoaderContext_t *ctx, int argc, char **argv);
int jelfLoaderRunAppMain(jelfLoaderContext_t *ctx);
int jelfLoaderRunConsole(jelfLoaderContext_t *ctx, int argc, char **argv);

jelfLoaderContext_t *jelfLoaderInit(LOADER_FD_T fd, const char *name, const jelfLoaderEnv_t *env);
jelfLoaderContext_t *jelfLoaderLoad(jelfLoaderContext_t *ctx);
jelfLoaderContext_t *jelfLoaderRelocate(jelfLoaderContext_t *ctx);

void jelfLoaderFree( jelfLoaderContext_t *ctx );

bool jelfLoaderSigCheck(jelfLoaderContext_t *ctx); // can only be called after relocating
uint8_t *jelfLoaderGetHash(jelfLoaderContext_t *ctx); // return hash (in bytes, 64 long)

/* Sets all profiler variables to 0 */
void jelfLoaderProfilerReset(void);

/* Prints the profiler results to uart console */
void jelfLoaderProfilerPrint(void);

#endif
