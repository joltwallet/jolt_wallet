/* Acknowledgements
 * This project is heavily inspired/based on the elfloader created by:
 *     niicoooo <1niicoooo1@gmail.com>
 * which in turn is mostly an ESP32-port of martinribelotta's ARMv7 elfloader.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "unaligned.h"
#include "jelf.h"
#include "jelfloader.h"

static const char* TAG = "JelfLoader";

#if ESP_PLATFORM

#include "esp_log.h"
#include "sodium.h"
#include "hal/storage/storage.h"

#define MSG(...)  ESP_LOGD(TAG, __VA_ARGS__);
#define INFO(...) ESP_LOGI(TAG, __VA_ARGS__);
#define ERR(...)  ESP_LOGE(TAG, __VA_ARGS__);

#else

#define MSG(...)
#define INFO(...) printf( __VA_ARGS__ )
#define ERR(...) printf( __VA_ARGS__ )

#endif //ESP_PLATFORM logging macros

/********************************
 * STATIC FUNCTIONS DECLARATION *
 ********************************/
static void app_signature_update(jelfLoaderContext_t *ctx, const uint8_t* data, size_t len);
static const char *type2String(int symt);
static int readSection(jelfLoaderContext_t *ctx, int n, Jelf_Shdr *h );
static jelfLoaderSection_t *findSection(jelfLoaderContext_t* ctx, int index);
static bool app_signature_init(jelfLoaderContext_t *ctx, 
        Jelf_Ehdr *header, const char *name);
static void inline app_signature_update(jelfLoaderContext_t *ctx, const uint8_t* data, size_t len);
static bool inline app_signature_check(jelfLoaderContext_t *ctx);
static int readSymbol(jelfLoaderContext_t *ctx, int n, Jelf_Sym *sym);
static Jelf_Addr findSymAddr(jelfLoaderContext_t* ctx, Jelf_Sym *sym);
static int relocateSymbol(Jelf_Addr relAddr, int type, Jelf_Addr symAddr,
        Jelf_Addr defAddr, uint32_t* from, uint32_t* to);
static int relocateSection(jelfLoaderContext_t *ctx, jelfLoaderSection_t *s);

/* Allocation Macros */
#define CEIL4(x) ((x+3)&~0x03)

#if ESP_PLATFORM

#include "esp_heap_caps.h"
#define LOADER_ALLOC_EXEC(size) heap_caps_malloc(size, MALLOC_CAP_EXEC | MALLOC_CAP_32BIT)
#define LOADER_ALLOC_DATA(size) heap_caps_malloc(size, MALLOC_CAP_8BIT)
#define LOADER_FREE( ptr ) heap_caps_free( ptr ) 

#else

#define LOADER_ALLOC_EXEC(size) malloc(size)
#define LOADER_ALLOC_DATA(size) malloc(size)
#define LOADER_FREE(size) free(size)

#endif

#if ESP_PLATFORM && CONFIG_JELFLOADER_PROFILER_EN

/******************
 * Profiler Tools *
 ******************/
/* Timers */
#include <esp_timer.h>
typedef struct profiler_timer_t{
    int32_t t;       // time in uS spent
    uint32_t n;      // times start has been called
    uint8_t running;    // if timer is running
} profiler_timer_t;

static profiler_timer_t profiler_readSection;
static profiler_timer_t profiler_readSymbol;
static profiler_timer_t profiler_relocateSymbol;
static profiler_timer_t profiler_findSymAddr;
static profiler_timer_t profiler_findSection;
static profiler_timer_t profiler_relocateSection;
 
// Caching Statistics
static uint32_t profiler_cache_hit;
static uint32_t profiler_cache_miss;

// Relocation Information
static uint32_t profiler_rel_count;
static uint32_t profiler_max_r_offset;
static uint32_t profiler_max_r_addend;

#define PROFILER_START(x) \
    if(!x.running) { \
        x.running = true; \
        x.t -= esp_timer_get_time(); \
    }

#define PROFILER_INC(x) \
    x.n++;

#define PROFILER_STOP(x) \
    if(x.running) { \
        x.running = false; \
        x.t += esp_timer_get_time(); \
    }

#define PROFILER_START_READSECTION     PROFILER_START(profiler_readSection)
#define PROFILER_START_READSYMBOL      PROFILER_START(profiler_readSymbol)
#define PROFILER_START_RELOCATESYMBOL  PROFILER_START(profiler_relocateSymbol)
#define PROFILER_START_FINDSYMADDR     PROFILER_START(profiler_findSymAddr)
#define PROFILER_START_FINDSECTION     PROFILER_START(profiler_findSection)
#define PROFILER_START_RELOCATESECTION PROFILER_START(profiler_relocateSection)

#define PROFILER_STOP_READSECTION      PROFILER_STOP(profiler_readSection)
#define PROFILER_STOP_READSYMBOL       PROFILER_STOP(profiler_readSymbol)
#define PROFILER_STOP_RELOCATESYMBOL   PROFILER_STOP(profiler_relocateSymbol)
#define PROFILER_STOP_FINDSYMADDR      PROFILER_STOP(profiler_findSymAddr)
#define PROFILER_STOP_FINDSECTION      PROFILER_STOP(profiler_findSection)
#define PROFILER_STOP_RELOCATESECTION  PROFILER_STOP(profiler_relocateSection)

#define PROFILER_INC_READSECTION       PROFILER_INC(profiler_readSection)
#define PROFILER_INC_READSYMBOL        PROFILER_INC(profiler_readSymbol)
#define PROFILER_INC_RELOCATESYMBOL    PROFILER_INC(profiler_relocateSymbol)
#define PROFILER_INC_FINDSYMADDR       PROFILER_INC(profiler_findSymAddr)
#define PROFILER_INC_FINDSECTION       PROFILER_INC(profiler_findSection)
#define PROFILER_INC_RELOCATESECTION   PROFILER_INC(profiler_relocateSection)

#define PROFILER_CACHE_HIT             profiler_cache_hit++;
#define PROFILER_CACHE_MISS            profiler_cache_miss++;

#define PROFILER_MAX_R_OFFSET(x)       if(x>profiler_max_r_offset) \
                                           profiler_max_r_offset = x;
#define PROFILER_MAX_R_ADDEND(x)       if(x>profiler_max_r_addend) \
                                           profiler_max_r_addend = x;
#define PROFILER_REL_COUNT(x)          profiler_rel_count += x;

/* Sets all profiler variables to 0 */
void jelfLoaderProfilerReset() {
    memset(&profiler_readSection, 0, sizeof(profiler_timer_t));
    memset(&profiler_readSymbol, 0, sizeof(profiler_timer_t));
    memset(&profiler_relocateSymbol, 0, sizeof(profiler_timer_t));
    memset(&profiler_findSymAddr, 0, sizeof(profiler_timer_t));
    memset(&profiler_findSection, 0, sizeof(profiler_timer_t));
    memset(&profiler_relocateSection, 0, sizeof(profiler_timer_t));
    profiler_cache_hit = 0;
    profiler_cache_miss = 0;
    profiler_max_r_offset = 0;
    profiler_max_r_addend = 0;
    profiler_rel_count = 0;
}

/* Prints the profiler results to uart console */
void jelfLoaderProfilerPrint() {
    uint32_t total_time;
    total_time = 
        profiler_readSection.t      +
        profiler_readSymbol.t       +
        profiler_relocateSymbol.t   +
        profiler_findSymAddr.t      + 
        profiler_relocateSection.t
        ;

    INFO("\n-----------------------------\n"
            "JELF Loader Profiling Results:\n"
            "Function Name          Time (uS)    Calls \n"
            "readSection:           %8d    %8d\n"
            "readSymbol:            %8d    %8d\n"
            "relocateSymbol:        %8d    %8d\n"
            "findSymAddr:           %8d    %8d\n"
            "relocateSection:       %8d    %8d\n"
            "total time:            %8d\n"
            "Cache Hit:             %8d\n"
            "Cache Miss:            %8d\n"
            "Relocations Performed: %8d\n"
            "Max RELA r_offset:     %8d (0x%08X)\n"
            "Max RELA r_addend:     %8d (0x%08X)\n"
            "-----------------------------\n\n",
            profiler_readSection.t,     profiler_readSection.n,
            profiler_readSymbol.t,      profiler_readSymbol.n,
            profiler_relocateSymbol.t,  profiler_relocateSymbol.n,
            profiler_findSymAddr.t,     profiler_findSymAddr.n,
            profiler_relocateSection.t, profiler_relocateSection.n,
            total_time,
            profiler_cache_hit, profiler_cache_miss,
            profiler_rel_count,
            profiler_max_r_offset, profiler_max_r_offset,
            profiler_max_r_addend, profiler_max_r_addend);
}

#else
// dummy macros

#define PROFILER_START_READSECTION
#define PROFILER_START_READSYMBOL
#define PROFILER_START_RELOCATESYMBOL 
#define PROFILER_START_FINDSYMADDR 
#define PROFILER_START_FINDSECTION
#define PROFILER_START_RELOCATESECTION 

#define PROFILER_STOP_READSECTION
#define PROFILER_STOP_READSYMBOL
#define PROFILER_STOP_RELOCATESYMBOL 
#define PROFILER_STOP_FINDSYMADDR 
#define PROFILER_STOP_FINDSECTION
#define PROFILER_STOP_RELOCATESECTION 

#define PROFILER_INC_READSECTION
#define PROFILER_INC_READSYMBOL
#define PROFILER_INC_RELOCATESYMBOL
#define PROFILER_INC_FINDSYMADDR
#define PROFILER_INC_FINDSECTION 
#define PROFILER_INC_RELOCATESECTION

#define PROFILER_CACHE_HIT
#define PROFILER_CACHE_MISS

#define PROFILER_MAX_R_OFFSET(x)
#define PROFILER_MAX_R_ADDEND(x)
#define PROFILER_REL_COUNT(x)

#endif

#if CONFIG_JELFLOADER_CACHE_LOCALITY
static int LOADER_GETDATA_CACHE(jelfLoaderContext_t *ctx,
        size_t off, char *buffer, size_t size) {
    uint8_t i;
    size_t amount_read;
    /* Check if the requested data is in the cache */
    jelfLoader_locality_cache_t *lru = &(ctx->locality_cache[0]);
    if( NULL != lru->data ) {
        /* Increment age of all chunks */
        for( i=0; i < CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_N; i++ ) {
            ctx->locality_cache[i].age++;
        }

        for( i=0; i < CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_N; i++ ) {
            if( false == ctx->locality_cache[i].valid ) {
                lru = &(ctx->locality_cache[i]);
                break;
            }

            /* Store the LRU cache */
            if( ctx->locality_cache[i].age > lru->age ) {
                lru = &(ctx->locality_cache[i]);
            }

            /* See if data is cached */
            if( off >= ctx->locality_cache[i].offset
                    && (off + size) <= (ctx->locality_cache[i].offset 
                        + CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_SIZE) ) {
                PROFILER_CACHE_HIT;
                #if CONFIG_JELFLOADER_PROFILER_EN
                MSG( "Hit! Hit Counter: %d. Offset: 0x%06x. Size: 0x%06X", 
                        profiler_cache_hit, (uint32_t)off, size );
                #endif
                ctx->locality_cache[i].age = 0;
                off_t locality_offset;
                locality_offset = off - ctx->locality_cache[i].offset;
                memcpy(buffer, ctx->locality_cache[i].data + locality_offset, size);
                return 0;
            }
        }
    }
    
    PROFILER_CACHE_MISS;
    #if CONFIG_JELFLOADER_PROFILER_EN
    MSG("Miss... Miss Counter: %d. Offset: 0x%06x. Size: 0x%06X",
            profiler_cache_miss, (uint32_t) off, size);
    #endif

    if( fseek(ctx->fd, off, SEEK_SET) != 0 ) {
        assert(0);
        goto err;
    }
    if( size > CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_SIZE ||
            NULL == lru->data) {
        /* Can't fit desired data into a chunk, don't cache */
        MSG("Can't fit read into locality cache chunk, reading directly to buffer.")
        amount_read = fread(buffer, 1, size, ctx->fd); 
    }
    else {
        amount_read = fread(lru->data, 1,
                CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_SIZE, ctx->fd); 
        lru->offset = off;
        lru->valid = true;
        lru->age = 0;
        memcpy(buffer, lru->data, size);
    }

    if( amount_read < size) {
        ERR("Requested %d bytes, but could only read %d.", size, amount_read);
        assert(0);
        goto err;
    }
    return 0;

err:
    return -1;
}
#define LOADER_GETDATA(ctx, off, buffer, size) \
    if( 0 != LOADER_GETDATA_CACHE(ctx, off, buffer, size)) { assert(0); goto err; } \
    app_signature_update(ctx, (uint8_t*)buffer, size);
#else // Use POSIX readers without caching
#define LOADER_GETDATA(ctx, off, buffer, size) \
    if(fseek(ctx->fd, off, SEEK_SET) != 0) { assert(0); goto err; }\
    if(fread(buffer, 1, size, ctx->fd) != size) { assert(0); goto err; }\
    app_signature_update(ctx, (uint8_t*)buffer, size);
#endif // CONFIG_JELFLOADER_CACHE_LOCALITY


/******************************************************
 * More specific readers to handle proper bitshifting *
 ******************************************************/
static int loader_shdr(jelfLoaderContext_t *ctx, size_t n, Jelf_Shdr *h) {
    #if CONFIG_JELFLOADER_CACHE_SHT
        uint8_t *buf = (uint8_t*)ctx->shdr_cache + n * JELF_SHDR_SIZE;
        MSG("loader_shdr cache: %p", ctx->shdr_cache); 
        MSG("loader_shdr buf: %p", buf);
    #else
        off_t offset = (off_t)((uint8_t*)ctx->e_shoff + n * JELF_SHDR_SIZE);
        uint8_t buf[JELF_SHDR_SIZE] = {0};
        LOADER_GETDATA(ctx, offset, (char *)&buf, sizeof(buf));
    #endif

    h->sh_type   = (buf[0] >> 6) & 0x03;
    h->sh_flags  = (buf[0] >> 4) & 0x03;
    h->sh_offset = ((buf[0] & 0x0F) ) |
            ((uint32_t)buf[1] << 4) |
            ((uint32_t)(buf[2] & 0xFE) << 11);
    h->sh_size   = ( buf[2] & 0x01 ) |
            ((uint32_t)buf[3] << 1) |
            ((uint32_t)buf[4] << 9) |
            ((uint32_t)(buf[5] & 0xC0) << 11); // I think this is correct
    h->sh_info   = (buf[5] & 0x03F) |
            ((uint32_t)buf[6] << 6);
    return 0;

#if !CONFIG_JELFLOADER_CACHE_SHT
err:
    return -1;
#endif
}

static int loader_sym(jelfLoaderContext_t *ctx, size_t offset, Jelf_Sym *h) {
    uint8_t buf[JELF_SYM_SIZE] = {0};
    LOADER_GETDATA(ctx, offset, (char *)&buf, sizeof(buf));
    h->st_name  = buf[0] | 
            ((uint32_t)buf[1] << 8);
    h->st_shndx = buf[2] |
            ((uint32_t)buf[3] << 8);
    h->st_value = buf[4] | 
            ((uint32_t)buf[5] <<  8) |
            ((uint32_t)buf[6] << 16) |
            ((uint32_t)buf[7] << 24);
    return 0;
err:
    return -1;
}

static int loader_rela(jelfLoaderContext_t *ctx, size_t offset, Jelf_Rela *h) {
    uint8_t buf[JELF_RELA_SIZE] = {0};
    LOADER_GETDATA(ctx, offset, (char *)&buf, sizeof(buf));
    h->r_offset = buf[0] |
            ((uint32_t)buf[1] << 8);
    h->r_info   = buf[2] |
            ((uint32_t)buf[3] << 8);
    h->r_addend = buf[4] |
            ((uint32_t)buf[5] << 8);
    return 0;
err:
    return -1;
}

/********************
 * STATIC FUNCTIONS *
 ********************/

static const char *type2String(int symt) {
#define STRCASE(name) case name: return #name;
    switch (symt) {
        STRCASE(R_XTENSA_NONE)
        STRCASE(R_XTENSA_32)
        STRCASE(R_XTENSA_ASM_EXPAND)
        STRCASE(R_XTENSA_SLOT0_OP)
        default:
            return "R_<unknown>";
    }
#undef STRCASE
}

/* Reads section header and name for given section index
 * n - Section Index to query
 * h - Returns Section Header
 */
static int readSection(jelfLoaderContext_t *ctx, int n, Jelf_Shdr *h ) {
    PROFILER_START_READSECTION;
    PROFILER_INC_READSECTION;

    /* Read Section Header */
    int res = loader_shdr(ctx, n, h);
    PROFILER_STOP_READSECTION;
    if ( 0 != res ){
        goto err;
    }

    return 0;
err:
    PROFILER_STOP_READSECTION;
    return -1;
}

/* Iterate through the singly linked list of sections until you find the one
 * that matches the provided index.
 * All these section structs are in RAM. */
static jelfLoaderSection_t *findSection(jelfLoaderContext_t* ctx, int index) {
    PROFILER_START_FINDSECTION;
    PROFILER_INC_FINDSECTION;

    for ( jelfLoaderSection_t* section=ctx->section;
            section != NULL;
            section = section->next ) {
        if (section->secIdx == index) {
            PROFILER_STOP_FINDSECTION;
            return section;
        }
    }
    PROFILER_STOP_FINDSECTION;
    return NULL;
}


/* Checks the application's digital signature data structures.
 * Returns true on successful initialization;
 * false if public_key doesn't match approved public_key. */
static bool app_signature_init(jelfLoaderContext_t *ctx, 
        Jelf_Ehdr *header, const char *name) {
     #if CONFIG_JOLT_APP_SIG_CHECK_EN
#if ESP_LOG_LEVEL >= ESP_LOG_INFO
    {
        /* Print App's 256-bit Public Key and 512-bit Signature */
        char pub_key[HEX_256] = { 0 };
        sodium_bin2hex(pub_key, sizeof(pub_key), header->e_public_key, 32);
        INFO("pub_key: %s", pub_key);
        char sig[HEX_512] = { 0 };
        sodium_bin2hex(sig, sizeof(sig), header->e_signature, 64);
        INFO("signature: %s", sig);
    }
#endif
    /* Copy over the app signature into the context */
    memcpy(ctx->app_signature, header->e_signature, sizeof(uint512_t));

    {
        /* First check to see if the public key is an accepted app key.
         * If it is valid, copy it into the ctx */
        uint256_t approved_pub_key = { 0 };
        size_t required_size;
        if( !storage_get_blob(NULL, &required_size, "user", "app_key") ) {
            ERR("Approved Public Key not found; using default");
            sodium_hex2bin( approved_pub_key, sizeof(approved_pub_key),
                    CONFIG_JOLT_APP_KEY_DEFAULT, 64, NULL, NULL, NULL);
        }
        else if( sizeof(approved_pub_key) != required_size ||
                !storage_get_blob(approved_pub_key, &required_size,
                    "user", "app_key")) {
            ERR("Stored Public Key Blob doesn't have expected len.");
            return false;
        }
        if( 0 != sodium_memcmp(approved_pub_key, header->e_public_key, sizeof(uint256_t)) ){
            ERR("Application Public Key doesn't match approved public key.");
            return false;
        }

        memcpy(ctx->app_public_key, approved_pub_key, sizeof(uint256_t));
    }

    {
        /* Initialize Signature Check Hashing */
        ctx->hs = malloc(sizeof(crypto_sign_state));
        crypto_sign_init(ctx->hs);

        /* Hash the app name */
        app_signature_update(ctx, (uint8_t*)name, strlen(name));

        /* Hash the JELF Header w/o signature */
        Jelf_Ehdr header_no_sig;
        memcpy(&header_no_sig, header, sizeof(header_no_sig));
        memset(&header_no_sig.e_signature, 0, sizeof(header_no_sig.e_signature));
        app_signature_update(ctx, (uint8_t*)&header_no_sig, sizeof(header_no_sig));
    }
    #endif
    return true;
}

static void inline app_signature_update(jelfLoaderContext_t *ctx, const uint8_t* data, size_t len){
#if CONFIG_JOLT_APP_SIG_CHECK_EN
    if( NULL != ctx->hs) {
        crypto_sign_update(ctx->hs, data, len);
    }
#endif
}

/* Returns true on valid signature */
static bool inline app_signature_check(jelfLoaderContext_t *ctx){
#if CONFIG_JOLT_APP_SIG_CHECK_EN
    return 0 == crypto_sign_final_verify( ctx->hs,
            ctx->app_signature, ctx->app_public_key);
#else
    return true;
#endif
}

static int readSymbol(jelfLoaderContext_t *ctx, int n, Jelf_Sym *sym) {
    PROFILER_START_READSYMBOL;
    PROFILER_INC_READSYMBOL;
    if( n >= ctx->symtab_count ){
        MSG("Symbol index exceeds symboltable");
        goto err;
    }

    off_t pos = ctx->symtab_offset + n * JELF_SYM_SIZE;
    if(!loader_sym(ctx, pos, sym)){
        PROFILER_STOP_READSYMBOL;
        return -1;
    }
    PROFILER_STOP_READSYMBOL;
    return 0;
err:
    PROFILER_STOP_READSYMBOL;
    return -1;
}

static Jelf_Addr findSymAddr(jelfLoaderContext_t* ctx, Jelf_Sym *sym) {
    PROFILER_START_FINDSYMADDR;
    PROFILER_INC_FINDSYMADDR;
    if( sym->st_name <= ctx->env->exported_size &&
            sym->st_name != 0 ) {
        PROFILER_STOP_FINDSYMADDR;
        MSG("USING EXPORTED SYMBOL %d", sym->st_name-1);
        /* One-Indexing because 0 is <unnamed> */
        return (Jelf_Addr)(ctx->env->exported[sym->st_name - 1]);
    }
    else{
        jelfLoaderSection_t *symSec = findSection(ctx, sym->st_shndx);
        if (symSec) {
            PROFILER_STOP_FINDSYMADDR;
            return ((Jelf_Addr) symSec->data) + sym->st_value;
        }
    }

    PROFILER_STOP_FINDSYMADDR;
    MSG("Couldn't find symbol address in loaded sections");
    return 0xffffffff;
}

static int relocateSymbol(Jelf_Addr relAddr, int type, Jelf_Addr symAddr,
        Jelf_Addr defAddr, uint32_t* from, uint32_t* to) {

    PROFILER_START_RELOCATESYMBOL;
    PROFILER_INC_RELOCATESYMBOL;

    if (symAddr == 0xffffffff) {
        if (defAddr == 0x00000000) {
            ERR("Relocation: undefined symAddr");
            goto err;
        } else {
            symAddr = defAddr;
        }
    }
    switch (type) {
        case R_XTENSA_32: {
            *from = unalignedGet32((void*)relAddr);
            *to  = symAddr + *from;
            unalignedSet32((void*)relAddr, *to);
            break;
        }
        case R_XTENSA_SLOT0_OP: {
            uint32_t v = unalignedGet32((void*)relAddr);
            *from = v;

            /* *** Format: L32R *** */
            if ((v & 0x00000F) == 0x000001) {
                int32_t delta =  symAddr - ((relAddr + 3) & 0xfffffffc);
                if (delta & 0x0000003) {
                    ERR("Relocation: L32R error");
                    goto err;
                }
                delta =  delta >> 2;
                unalignedSet8((void*)(relAddr + 1), ((uint8_t*)&delta)[0]);
                unalignedSet8((void*)(relAddr + 2), ((uint8_t*)&delta)[1]);
                *to = unalignedGet32((void*)relAddr);
                break;
            }

            /* *** Format: CALL *** */
            /* *** CALL0, CALL4, CALL8, CALL12, J *** */
            if ((v & 0x00000F) == 0x000005) {
                int32_t delta =  symAddr - ((relAddr + 4) & 0xfffffffc);
                if (delta & 0x0000003) {
                    ERR("Relocation: CALL error");
                    return -1;
                }
                delta =  delta >> 2;
                delta =  delta << 6;
                delta |= unalignedGet8((void*)(relAddr + 0));
                unalignedSet8((void*)(relAddr + 0), ((uint8_t*)&delta)[0]);
                unalignedSet8((void*)(relAddr + 1), ((uint8_t*)&delta)[1]);
                unalignedSet8((void*)(relAddr + 2), ((uint8_t*)&delta)[2]);
                *to = unalignedGet32((void*)relAddr);
                break;
            }

            /* *** J *** */
            if ((v & 0x00003F) == 0x000006) {
                int32_t delta =  symAddr - (relAddr + 4);
                delta =  delta << 6;
                delta |= unalignedGet8((void*)(relAddr + 0));
                unalignedSet8((void*)(relAddr + 0), ((uint8_t*)&delta)[0]);
                unalignedSet8((void*)(relAddr + 1), ((uint8_t*)&delta)[1]);
                unalignedSet8((void*)(relAddr + 2), ((uint8_t*)&delta)[2]);
                *to = unalignedGet32((void*)relAddr);
                break;
            }

            /* *** Format: BRI8  *** */
            /* *** BALL, BANY, BBC, BBCI, BBCI.L, BBS,  BBSI, BBSI.L, BEQ,
             * *** BGE,  BGEU, BLT, BLTU, BNALL, BNE,  BNONE, LOOP, 
             * *** BEQI, BF, BGEI, BGEUI, BLTI, BLTUI, BNEI,  BT, LOOPGTZ,
             * *** LOOPNEZ *** */
            if ( ( (v & 0x00000F) == 0x000007 ) 
                    || ( (v & 0x00003F) == 0x000026 )
                    || ( (v & 0x00003F) == 0x000036 && (v & 0x0000FF) != 0x000036) ) {
                int32_t delta =  symAddr - (relAddr + 4);
                unalignedSet8((void*)(relAddr + 2), ((uint8_t*)&delta)[0]);
                *to = unalignedGet32((void*)relAddr);
                if ((delta < - (1 << 7)) || (delta >= (1 << 7))) {
                    ERR("Relocation: BRI8 out of range");
                    goto err;
                }
                break;
            }

            /* *** Format: BRI12 *** */
            /* *** BEQZ, BGEZ, BLTZ, BNEZ *** */
            if ((v & 0x00003F) == 0x000016) {
                int32_t delta =  symAddr - (relAddr + 4);
                delta =  delta << 4;
                delta |=  unalignedGet32((void*)(relAddr + 1));
                unalignedSet8((void*)(relAddr + 1), ((uint8_t*)&delta)[0]);
                unalignedSet8((void*)(relAddr + 2), ((uint8_t*)&delta)[1]);
                *to = unalignedGet32((void*)relAddr);
                delta =  symAddr - (relAddr + 4);
                if ((delta < - (1 << 11)) || (delta >= (1 << 11))) {
                    ERR("Relocation: BRI12 out of range");
                    goto err;
                }
                break;
            }

            /* *** Format: RI6  *** */
            /* *** BEQZ.N, BNEZ.N *** */
            if ((v & 0x008F) == 0x008C) {
                int32_t delta =  symAddr - (relAddr + 4);
                int32_t d2 = delta & 0x30;
                int32_t d1 = (delta << 4) & 0xf0;
                d2 |=  unalignedGet32((void*)(relAddr + 0));
                d1 |=  unalignedGet32((void*)(relAddr + 1));
                unalignedSet8((void*)(relAddr + 0), ((uint8_t*)&d2)[0]);
                unalignedSet8((void*)(relAddr + 1), ((uint8_t*)&d1)[0]);
                *to = unalignedGet32((void*)relAddr);
                if ((delta < 0) || (delta > 0x111111)) {
                    ERR("Relocation: RI6 out of range");
                    goto err;
                }
                break;
            }

            ERR("Relocation: unknown opcode %08X", v);
            goto err;
            break;
        }
        case R_XTENSA_ASM_EXPAND: {
            *from = unalignedGet32((void*)relAddr);
            *to = unalignedGet32((void*)relAddr);
            break;
        }
        default:
            MSG("Relocation: undefined relocation %d %s", type, type2String(type));
            assert(0);
            goto err;
    }

    PROFILER_STOP_RELOCATESYMBOL;
    return 0;
err:
    PROFILER_STOP_RELOCATESYMBOL;
    return -1;
}

static int relocateSection(jelfLoaderContext_t *ctx, jelfLoaderSection_t *s) {
    PROFILER_START_RELOCATESECTION;
    PROFILER_INC_RELOCATESECTION;

    Jelf_Shdr sectHdr;

    PROFILER_STOP_RELOCATESECTION;
    /* Read the current section header */
    //MSG("meow %p", s);
    if (readSection(ctx, s->relSecIdx, &sectHdr) != 0) {
        ERR("Error reading section header");
        goto err;
    }
    PROFILER_START_RELOCATESECTION;

    if (!(s->relSecIdx)) {
        PROFILER_STOP_RELOCATESECTION;
        MSG("  Section %d: no relocation index", s->secIdx);
        return 0;
    }

    if (!(s->data)) {
        ERR("Section not loaded: %d", s->secIdx);
        goto err;
    }

    int r = 0;
    Jelf_Rela rel;
    size_t relEntries = sectHdr.sh_size / sizeof(rel);
    PROFILER_REL_COUNT(relEntries);
    MSG("RELOCATING SECTION")
    MSG("  Offset   Sym  Type                      relAddr  "
        "symAddr  defValue                    Name + addend");
    for (size_t relCount = 0; relCount < relEntries; relCount++) {
        off_t offset = sectHdr.sh_offset + relCount * (sizeof(rel));
        MSG("Reading in ELF32_Rela from offset 0x%x", (uint32_t)offset);
        r |= loader_rela(ctx, offset, &rel);
        PROFILER_MAX_R_OFFSET(rel.r_offset);
        PROFILER_MAX_R_ADDEND(rel.r_addend);

        Jelf_Sym sym = { 0 };
        int symEntry = JELF_R_SYM( rel.r_info); // SymbolTable Index
        int relType  = JELF_R_TYPE(rel.r_info); // RelocationType

        /* data to be updated address */
        Jelf_Addr relAddr = ((Jelf_Addr) s->data) + rel.r_offset;
        PROFILER_STOP_RELOCATESECTION;
        readSymbol(ctx, symEntry, &sym);
        PROFILER_START_RELOCATESECTION;

        /* Target Symbol Address */
        PROFILER_STOP_RELOCATESECTION;
        // Returns 0xffffffff if symbol cannot be found
        Jelf_Addr symAddr = findSymAddr(ctx, &sym) + rel.r_addend;
        PROFILER_START_RELOCATESECTION;

        uint32_t from = 0, to = 0;
        if (relType == R_XTENSA_NONE || relType == R_XTENSA_ASM_EXPAND) {
            #if 0
            MSG("  %08X %04X %04X %-20s %08X          %08X"
                "                    %s + %X",
                rel.r_offset, symEntry, relType, type2String(relType),
                relAddr, sym.st_value, name, rel.r_addend);
            #endif
        }
        else if ( (symAddr == 0xffffffff) && (sym.st_value == 0x00000000) ) {
            ERR("Relocation - undefined symAddr");
            MSG("  %08X %04X %04X %-20s %08X %08X %08X"
                "                     + %X",
                rel.r_offset, symEntry, relType, type2String(relType),
                relAddr, symAddr, sym.st_value, rel.r_addend);
            r |= -1;
        }
        else if(relocateSymbol(relAddr, relType, symAddr, sym.st_value, &from, &to) != 0) {
            ERR("  %08X %04X %04X %-20s %08X %08X %08X %08X->%08X  + %X",
                    rel.r_offset, symEntry, relType, type2String(relType),
                    relAddr, symAddr, sym.st_value, from, to, rel.r_addend);
            r |= -1;
        }
        else {
            MSG("  %08X %04X %04X %-20s %08X %08X %08X %08X->%08X  + %X",
                    rel.r_offset, symEntry, relType, type2String(relType),
                    relAddr, symAddr, sym.st_value, from, to, rel.r_addend);
        }
    }
    PROFILER_STOP_RELOCATESECTION;
    return r;
err:
    PROFILER_STOP_RELOCATESECTION;
    ERR("Error reading relocation data");
    return -1;
}


/********************
 * PUBLIC FUNCTIONS *
 ********************/

int jelfLoaderRun(jelfLoaderContext_t *ctx, int argc, char **argv) {
    int res = -1;;

    if (!ctx->exec) {
        MSG("No Entrypoint set.");
        return 0;
    }

    /* Free up loading cache */
    #if CONFIG_JELFLOADER_CACHE_SHT
    if( NULL != ctx->shdr_cache ) {
        free( ctx->shdr_cache );
        ctx->shdr_cache = NULL;
    }
    #endif
    #if CONFIG_ELFLOADER_CACHE_LOCALITY
    for(uint8_t i=0; i < CONFIG_ELFLOADER_CACHE_LOCALITY_CHUNK_N; i++ ) {
        if( NULL != ctx->locality_cache[i].data) {
            free(ctx->locality_cache[i].data);
            ctx->locality_cache[i].data = NULL;
        }
    }
    #endif

    /* Siganture Check */
#if CONFIG_CONFIG_JOLT_APP_SIG_CHECK_EN
    if( !app_signature_check(ctx) ) {
        ERR("Invalid Signature");
        uint8_t out[BIN_512] = { 0 };
        char out_hex[HEX_512] = { 0 };
        crypto_hash_sha512_final(&(((crypto_sign_state*)(ctx->hs))->hs), out);
        sodium_bin2hex(out_hex, sizeof(out_hex), out, sizeof(out));
        INFO("App Hash: %s", out_hex);
    }
    else
#endif
    {
        typedef int (*func_t)(int, char**);
        func_t func = (func_t)ctx->exec;
        MSG("Running...");
        res = func(argc, argv);
        MSG("Result: %08X", res);
    }
    return res;
}

int jelfLoaderRunAppMain(jelfLoaderContext_t *ctx) {
    return jelfLoaderRun(ctx, 0, NULL);
}

int jelfLoaderRunConsole(jelfLoaderContext_t *ctx, int argc, char **argv) {
    /* Just more explicit way of running app */
    return jelfLoaderRun(ctx, argc, argv);
}

/* First Operation: Performs the following:
 *     * Allocates space for the returned Context (constant size)
 *     * Populates Context with:
 *         * Pointer/FD to the beginning of the ELF file
 *         * Pointer to env, which is a table to exported host function_names and their pointer in memory. 
 *         * Number of sections in the ELF file.
 *         * offset to SectionHeaderTable, which maps an index to a offset in ELF where the section header begins.
 *         * offset to StringTable, which is a list of null terminated strings.
 */
jelfLoaderContext_t *jelfLoaderInit(LOADER_FD_T fd, const char *name,
        const jelfLoaderEnv_t *env) {
    Jelf_Ehdr header;
    jelfLoaderContext_t *ctx;

    /* Debugging Size Sanity Check */
    MSG("Jelf_Ehdr: %d", sizeof(Jelf_Ehdr));
    MSG("Jelf_Sym:  %d", sizeof(Jelf_Sym));
    MSG("Jelf_Shdr: %d", sizeof(Jelf_Shdr));
    MSG("Jelf_Rela: %d", sizeof(Jelf_Rela));

    /***********************************************
     * Initialize the context object with pointers *
     ***********************************************/
    ctx = malloc(sizeof(jelfLoaderContext_t));
    if( NULL == ctx ) {
        ERR( "Insufficient memory for ElfLoaderContext_t" );
        goto err;
    }
    memset(ctx, 0, sizeof(jelfLoaderContext_t));
    ctx->fd = fd;
    ctx->env = env;

    /*********************************************************************
     * Load the JELF header (Ehdr), located at the beginning of the file *
     *********************************************************************/
    LOADER_GETDATA(ctx, 0, (char*)&header, JELF_EHDR_SIZE);

    /* Make sure that we have a correct and compatible ELF header. */
    char JelfMagic[] = { 0x7f, 'J', 'E', 'L', 'F', '\0' };
    if ( 0 != memcmp(header.e_ident, JelfMagic, strlen(JelfMagic)) ) {
        ERR("Bad JELF Identification");
        ERR("File Magic Identifier: %s", header.e_ident);
        goto err;
    }

    /* Version Compatability Check */
    // todo: make this more advanced as versioning evolves
    assert( header.e_version_major == 0 );
    assert( header.e_version_minor == 1 );

    /* Debug Sanity Checks */
    MSG( "SectionHeaderTableOffset: %08X", header.e_shoff );
    MSG( "SectionHeaderTableEntries: %d", header.e_shnum );
    MSG( "Derivation Purpose: %08X", header.e_coin_purpose );
    MSG( "Derivation Path: %08X", header.e_coin_path );
    MSG( "bip32key: %s", header.e_bip32key);

    /**********************************
     * Initialize App Signature Check *
     **********************************/
    if( !app_signature_init(ctx, &header, name) ) {
        goto err;
    }
    
    /*****************************
     * Initialize Locality Cache *
     *****************************/
    /* Locality cache is initialized after signature checking so that the 
     * overall memory footprint is relatively consistent. */
    #if CONFIG_JELFLOADER_CACHE_LOCALITY
    {
        for(uint8_t i=0; i < CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_N; i++ ) {
            ctx->locality_cache[i].data = malloc(
                    CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_SIZE );
                if( NULL ==  ctx->locality_cache[i].data ) {
                    ERR("Insufficient memory for Locality Cache Data");
                    goto err;
                }
            ctx->locality_cache[i].valid = false;
        }
    }
    #endif

    #if CONFIG_JELFLOADER_CACHE_SHT
    {
        /**************************************
         * Cache sectionheadertable to memory *
         **************************************/
        size_t sht_size = header.e_shnum * JELF_SHDR_SIZE;
        MSG("Allocating %d bytes for section header cache.", sht_size);
        ctx->shdr_cache = malloc(sht_size);
        if( NULL == ctx->shdr_cache  ) {
            ERR("Insufficient memory for section header table cache");
            goto err;
        }
        /* Populate the cache */
        LOADER_GETDATA(ctx, header.e_shoff, (char *)(ctx->shdr_cache), sht_size);
    }
    #endif

    /* Populate context with ELF Header information*/
    ctx->e_shnum = header.e_shnum; // Number of Sections
    ctx->e_shoff = header.e_shoff; // offset to SectionHeaderTable
    ctx->entry_index = header.e_entry_offset;
    ctx->coin_purpose = header.e_coin_purpose;
    ctx->coin_path = header.e_coin_path;
    strlcpy(ctx->bip32_key, header.e_bip32key, sizeof(ctx->bip32_key));

    return ctx;

err:
    if(NULL != ctx) {
        jelfLoaderFree(ctx);
    }
    return NULL;
}

jelfLoaderContext_t *jelfLoaderLoad(jelfLoaderContext_t *ctx) {
    MSG("Scanning ELF sections         relAddr      size");
    // Iterate through all section_headers
    for (int n = 1; n < ctx->e_shnum; n++) {
        MSG("Loading section %d", n);
        Jelf_Shdr sectHdr = { 0 };

        /***********************
         * Read Section Header *
         ***********************/
        // Read the section header at index n
        // Populates "secHdr" with the Section's Header
        if ( 0 != readSection(ctx, n, &sectHdr) ) {
            ERR("Error reading section");
            goto err;
        }

        #if 1
        /* Debug: Print out the bytes that make up sectHdr */
        {
            MSG("Type:   %d", sectHdr.sh_type);
            MSG("Flags:  %d", sectHdr.sh_flags);
            MSG("Offset: %d", sectHdr.sh_offset);
            MSG("Size:   %d", sectHdr.sh_size);
            MSG("Info:   %d", sectHdr.sh_info);
        } 
        #endif

        /* This  section  occupies  memory during process execution */
        if (sectHdr.sh_flags & SHF_ALLOC) {
            MSG("Section %d has SHF_ALLOC flag.", n);
            if (!sectHdr.sh_size) {
                MSG("  section %2d no data", n);
            } 
            else {
                /* Allocate space for Section Struct (not data) */
                jelfLoaderSection_t* section = malloc(sizeof(jelfLoaderSection_t));
                if( NULL == section ) {
                    ERR("Error allocating space for SHF_ALLOC section");
                    goto err;
                }
                memset(section, 0, sizeof(jelfLoaderSection_t));

                /* Populate the Section Elements */
                section->secIdx = n;
                section->size = sectHdr.sh_size;

                /* Add it to the beginning of the SinglyLinkedList */
                section->next = ctx->section;
                ctx->section = section;

                /* Allocate memory */
                MSG("Section %d attempting to allocate %d bytes.", n, sectHdr.sh_size);

                section->data = ( sectHdr.sh_flags & SHF_EXECINSTR ) ?
                        LOADER_ALLOC_EXEC(CEIL4(sectHdr.sh_size)) : // Executable Memory
                        LOADER_ALLOC_DATA(CEIL4(sectHdr.sh_size)) ; // Normal Memory

                if (!section->data) {
                    ERR("Section %d malloc failed.", n);
                    goto err;
                }

                /* Load Section into allocated data */
                if (sectHdr.sh_type != SHT_NOBITS) {
                    if( sectHdr.sh_flags & SHF_EXECINSTR ){
                        /* To get around LoadStoreErrors with reading single 
                         * bytes from instruction memory */
                        char *tmp = LOADER_ALLOC_DATA(CEIL4(sectHdr.sh_size));
                        LOADER_GETDATA( ctx, sectHdr.sh_offset, tmp,
                                CEIL4(sectHdr.sh_size) );
                        memcpy(section->data, tmp, CEIL4(sectHdr.sh_size));
                        LOADER_FREE(tmp);
                    }
                    else{
                        LOADER_GETDATA( ctx, sectHdr.sh_offset, section->data,
                                CEIL4(sectHdr.sh_size) );
                    }
                }

                MSG("  section %2d %08X %6i", n,
                        (unsigned int) section->data, sectHdr.sh_size);
            }
        }
        /* Relocation Entries with Addends */
        else if ( SHT_RELA == sectHdr.sh_type ) {
            /* sh_info holds extra information that depends on sh_type.
             * For sh_type SHT_RELA:
             *     The section header index of the section to which the 
             *     relocation applies.
             */

            /* If the index is greater than the number of sections that exist,
             * it must be erroneous */
            if (sectHdr.sh_info >= n) {
                ERR("Rela section: bad linked section (%i -> %i)",
                        n, sectHdr.sh_info);
                goto err;
            }

            /* iterate through the singly linked list of sections until you
             * find the one that matches sh_info */
            jelfLoaderSection_t* section = findSection(ctx, sectHdr.sh_info);
            if (section == NULL) { // Cannot find section
                MSG("  section %2d -> %2d: ignoring", n, sectHdr.sh_info);
            } else {
                section->relSecIdx = n;
                MSG("  section %2d -> %2d: ok", n, sectHdr.sh_info);
            }
        }
        else if ( SHT_SYMTAB == sectHdr.sh_type ) {
            MSG("  section %2d", n);
                ctx->symtab_offset = sectHdr.sh_offset;
                ctx->symtab_count = sectHdr.sh_size / JELF_SYM_SIZE;
                MSG("symtab is %u bytes.", sectHdr.sh_size);
                MSG("symtab contains %u entires.", ctx->symtab_count);
        }
    }
    if (ctx->symtab_offset == 0 ) {
        ERR("Missing .symtab");
        goto err;
    }
    MSG("successfully loaded sections");

    Jelf_Sym sym;
    LOADER_GETDATA( ctx,
            ctx->symtab_offset + ctx->entry_index * JELF_SYM_SIZE,
            (char*)&sym, CEIL4(JELF_SYM_SIZE) );

    jelfLoaderSection_t *symbol_section = findSection(ctx, sym.st_shndx);
    if( NULL == symbol_section ) {
        ERR("Error setting entrypoint.");
        goto err;
    }
    ctx->exec = (void*)(((Jelf_Addr) symbol_section->data) + sym.st_value);
    MSG("successfully set entrypoint");

    return ctx;
err:
    return NULL;
}

jelfLoaderContext_t *jelfLoaderRelocate(jelfLoaderContext_t *ctx) {
    MSG("Relocating sections");
    int r = 0;
    uint32_t count = 0;
    for (jelfLoaderSection_t* section = ctx->section;
            section != NULL;
            section = section->next) {
        r |= relocateSection(ctx, section);
        count++;
    }
    if (r != 0) {
        MSG("Relocation failed");
        goto err;
    }
    MSG("Successfully relocated %d sections.", count);

    /* Siganture Check */
#if CONFIG_JOLT_APP_SIG_CHECK_EN
    if( !app_signature_check(ctx) ) {
        ERR("Invalid Signature");
        uint8_t out[BIN_512] = { 0 };
        char out_hex[HEX_512] = { 0 };
        crypto_hash_sha512_final(&(((crypto_sign_state*)(ctx->hs))->hs), out);
        sodium_bin2hex(out_hex, sizeof(out_hex), out, sizeof(out));
        INFO("App Hash: %s", out_hex);
        goto err;
    }
#endif

    return ctx;

err:
    return NULL;
}

void jelfLoaderFree(jelfLoaderContext_t *ctx) {
    if (NULL == ctx) {
        return;
    }
    #if CONFIG_JELFLOADER_CACHE_LOCALITY
    for(uint8_t i=0; i < CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_N; i++ ) {
        if( NULL != ctx->locality_cache[i].data) {
            free(ctx->locality_cache[i].data);
            ctx->locality_cache[i].data = NULL;
        }
    }
    #endif

#if CONFIG_JOLT_APP_SIG_CHECK_EN
    if( NULL != ctx->hs ) {
        free(ctx->hs);
    }
#endif

    jelfLoaderSection_t* section = ctx->section;
    jelfLoaderSection_t* next;
    while(section != NULL) {
        if (section->data) {
            LOADER_FREE(section->data);
        }
        next = section->next;
        free(section);
        section = next;
    }
    free(ctx);
}

