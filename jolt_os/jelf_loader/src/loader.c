/* Acknowledgements
 * This project is heavily inspired/based on the elfloader created by:
 *     niicoooo <1niicoooo1@gmail.com>
 * which in turn is mostly an ESP32-port of martinribelotta's ARMv7 elfloader.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "jelf.h"
#include "jelfloader.h"
#include "sdkconfig.h"
#include "sodium.h"
#include "unaligned.h"

#ifndef CONFIG_JOLT_APP_SIG_CHECK_EN
    #define CONFIG_JOLT_APP_SIG_CHECK_EN 1
#endif

#if ESP_PLATFORM

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

    #include "esp_log.h"
    #include "hal/storage/storage.h"
    #include "jolt_lib.h"

static const char *TAG = "JelfLoader";

    #define MSG( ... )  ESP_LOGD( TAG, __VA_ARGS__ );
    #define INFO( ... ) ESP_LOGI( TAG, __VA_ARGS__ );
    #define ERR( ... )  ESP_LOGE( TAG, __VA_ARGS__ );

#else

    #include <stdio.h>

    #define MSG( ... )
    #define INFO( ... )        \
        printf( __VA_ARGS__ ); \
        printf( "\n" );
    #define ERR( ... )         \
        printf( __VA_ARGS__ ); \
        printf( "\n" );

#endif  // ESP_PLATFORM logging macros

/********************************
 * STATIC FUNCTIONS DECLARATION *
 ********************************/
static int decompress_get( jelfLoaderContext_t *ctx, uint8_t *data, size_t len );
static void app_hash_update( jelfLoaderContext_t *ctx, const uint8_t *data, size_t len );
static const char *type2String( int symt );
static jelfLoaderSection_t *findSection( jelfLoaderContext_t *ctx, int index );
static bool app_hash_init( jelfLoaderContext_t *ctx, Jelf_Ehdr *header, const char *name );
static Jelf_Addr findSymAddr( jelfLoaderContext_t *ctx, Jelf_Sym *sym );
static int relocateSymbol( Jelf_Addr relAddr, int type, Jelf_Addr symAddr, uint32_t *from, uint32_t *to );
static int relocateSection( jelfLoaderContext_t *ctx, jelfLoaderSection_t *s );

/* Allocation Macros */
#define CEIL4( x ) ( ( x + 3 ) & ~0x03 )

#if ESP_PLATFORM

    /* ESP-IDF mallocs */
    #include "esp_heap_caps.h"
    #define LOADER_ALLOC_EXEC( size ) heap_caps_malloc( size, MALLOC_CAP_EXEC | MALLOC_CAP_32BIT )
    #define LOADER_ALLOC_DATA( size ) heap_caps_malloc( size, MALLOC_CAP_8BIT )
    #define LOADER_FREE( ptr )        heap_caps_free( ptr )

#else

    /* General purpose mallocs */
    #define LOADER_ALLOC_EXEC( size ) malloc( size )
    #define LOADER_ALLOC_DATA( size ) malloc( size )
    #define LOADER_FREE( size )       free( size )

#endif /* ESP_PLATFORM */

#if CONFIG_JELFLOADER_PROFILER_EN

    /******************
     * Profiler Tools *
     ******************/
    /* Timers */
    #if ESP_PLATFORM
        #include <esp_timer.h>
static uint64_t get_time() { return esp_timer_get_time(); }
    #else
        #include <time.h>
static uint64_t get_time()
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}
    #endif /* ESP_PLATFORM */

typedef struct profiler_timer_t {
    int32_t t;        // time in uS spent
    uint32_t n;       // times start has been called
    uint8_t running;  // if timer is running
} profiler_timer_t;

static profiler_timer_t profiler_relocateSymbol;
static profiler_timer_t profiler_findSymAddr;
static profiler_timer_t profiler_findSection;
static profiler_timer_t profiler_relocateSection;

// Relocation Information
static uint32_t profiler_rel_count;
static uint32_t profiler_max_r_offset;

    #define PROFILER_START( x ) \
        if( !x.running ) {      \
            x.running = true;   \
            x.t -= get_time();  \
        }

    #define PROFILER_INC( x ) x.n++;

    #define PROFILER_STOP( x ) \
        if( x.running ) {      \
            x.running = false; \
            x.t += get_time(); \
        }

    #define PROFILER_START_RELOCATESYMBOL  PROFILER_START( profiler_relocateSymbol )
    #define PROFILER_START_FINDSYMADDR     PROFILER_START( profiler_findSymAddr )
    #define PROFILER_START_FINDSECTION     PROFILER_START( profiler_findSection )
    #define PROFILER_START_RELOCATESECTION PROFILER_START( profiler_relocateSection )

    #define PROFILER_STOP_RELOCATESYMBOL  PROFILER_STOP( profiler_relocateSymbol )
    #define PROFILER_STOP_FINDSYMADDR     PROFILER_STOP( profiler_findSymAddr )
    #define PROFILER_STOP_FINDSECTION     PROFILER_STOP( profiler_findSection )
    #define PROFILER_STOP_RELOCATESECTION PROFILER_STOP( profiler_relocateSection )

    #define PROFILER_INC_RELOCATESYMBOL  PROFILER_INC( profiler_relocateSymbol )
    #define PROFILER_INC_FINDSYMADDR     PROFILER_INC( profiler_findSymAddr )
    #define PROFILER_INC_FINDSECTION     PROFILER_INC( profiler_findSection )
    #define PROFILER_INC_RELOCATESECTION PROFILER_INC( profiler_relocateSection )

    #define PROFILER_MAX_R_OFFSET( x ) \
        if( x > profiler_max_r_offset ) profiler_max_r_offset = x;
    #define PROFILER_REL_COUNT( x ) profiler_rel_count += x;

/* Sets all profiler variables to 0 */
void jelfLoaderProfilerReset()
{
    memset( &profiler_relocateSymbol, 0, sizeof( profiler_timer_t ) );
    memset( &profiler_findSymAddr, 0, sizeof( profiler_timer_t ) );
    memset( &profiler_findSection, 0, sizeof( profiler_timer_t ) );
    memset( &profiler_relocateSection, 0, sizeof( profiler_timer_t ) );
    profiler_max_r_offset = 0;
    profiler_rel_count    = 0;
}

/* Prints the profiler results to uart console */
void jelfLoaderProfilerPrint()
{
    uint32_t total_time;
    total_time = profiler_relocateSymbol.t + profiler_findSymAddr.t + profiler_relocateSection.t;

    INFO( "\n-----------------------------\n"
          "JELF Loader Profiling Results:\n"
          "Function Name          Time (uS)    Calls \n"
          "relocateSymbol:        %8d    %8u\n"
          "findSymAddr:           %8d    %8u\n"
          "relocateSection:       %8d    %8u\n"
          "total time:            %8u\n"
          "Relocations Performed: %8u\n"
          "Max RELA r_offset:     %8u (0x%08X)\n"
          "-----------------------------\n\n",
          profiler_relocateSymbol.t, profiler_relocateSymbol.n, profiler_findSymAddr.t, profiler_findSymAddr.n,
          profiler_relocateSection.t, profiler_relocateSection.n, total_time, profiler_rel_count,
          profiler_max_r_offset, profiler_max_r_offset );
}

#else /* CONFIG_JELFLOADER_PROFILER_EN */
/* Stubs */

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

    #define PROFILER_MAX_R_OFFSET( x )
    #define PROFILER_REL_COUNT( x )

void jelfLoaderProfilerReset()
{ /* Stub */
}

void jelfLoaderProfilerPrint()
{ /* Stub */
}

#endif /* CONFIG_JELFLOADER_PROFILER_EN */

#define LOADER_GETDATA_RAW( ctx, buffer, size ) fread( buffer, 1, size, ctx->fd )

#define LOADER_GETDATA( ctx, buffer, size ) \
    if( decompress_get( ctx, (uint8_t *)buffer, size ) != (int)size ) { goto err; };

/******************************************************
 * More specific readers to handle proper bitshifting *
 ******************************************************/
static int loader_ehdr( jelfLoaderContext_t *ctx, Jelf_Ehdr *h )
{
    uint8_t buffer[JELF_EHDR_SIZE];
    uint8_t *buf = buffer;
    LOADER_GETDATA( ctx, buf, JELF_EHDR_SIZE );

    memcpy( h->e_ident, buf, JELF_EI_NIDENT );
    buf += JELF_EI_NIDENT;

    memcpy( h->e_public_key, buf, JELF_PUBLIC_KEY_LEN );
    buf += JELF_PUBLIC_KEY_LEN;

    memcpy( &( h->e_version_major ), buf, sizeof( uint8_t ) );
    buf += sizeof( uint8_t );

    memcpy( &h->e_version_minor, buf, sizeof( uint8_t ) );
    buf += sizeof( uint8_t );

    memcpy( &h->e_version_patch, buf, sizeof( uint8_t ) );
    buf += sizeof( uint8_t );

    MSG( "Version %d.%d.%d", h->e_version_major, h->e_version_minor, h->e_version_patch );

    memcpy( &h->e_entry_index, buf, sizeof( uint16_t ) );
    buf += sizeof( uint16_t );

    memcpy( &h->e_shnum, buf, sizeof( uint16_t ) );
    h->e_shnum = *(uint16_t *)buf;
    buf += sizeof( uint16_t );

    memcpy( &h->e_coin_purpose, buf, sizeof( uint32_t ) );
    buf += sizeof( uint32_t );

    memcpy( &h->e_coin_path, buf, sizeof( uint32_t ) );
    buf += sizeof( uint32_t );

    memcpy( h->e_bip32key, buf, JELF_BIP32KEY_LEN );
    buf += JELF_BIP32KEY_LEN;

    return 0;
err:
    return -1;
}

static int loader_shdr( jelfLoaderContext_t *ctx, uint16_t n, Jelf_Shdr *h )
{
    uint8_t *buf = (uint8_t *)ctx->sht_cache + n * JELF_SHDR_SIZE;
    if( n > ctx->e_shnum ) {
        ERR( "Attempt to index past SectionHeaderTable" );
        return -1;
    }

    h->sh_type  = ( buf[0] >> 6 ) & 0x03;
    h->sh_flags = ( buf[0] >> 4 ) & 0x03;
    h->sh_size  = ( ( buf[0] & 0x0F ) ) | ( (uint32_t)buf[1] << 4 ) | ( ( uint32_t )( buf[2] & 0xF0 ) << 8 );
    h->sh_info  = ( buf[2] & 0x0F ) | ( (uint32_t)buf[3] << 4 );
    return 0;
}

static int loader_sym( jelfLoaderContext_t *ctx, uint16_t n, Jelf_Sym *h )
{
    uint8_t *buf = (uint8_t *)ctx->symtab_cache + n * JELF_SYM_SIZE;
    if( n >= ctx->symtab_count ) {
        ERR( "Attempt to index past SymbolTable ( %d >= %d )", n, ctx->symtab_count );
        return -1;
    }

    h->st_name  = buf[0] | ( ( uint32_t )( buf[1] & 0xF0 ) << 4 );
    h->st_shndx = ( buf[1] & 0x0F ) | ( (uint32_t)buf[2] << 4 );
    return 0;
}

static int loader_rela( jelfLoaderContext_t *ctx, Jelf_Rela *h )
{
    uint8_t buf[JELF_RELA_SIZE] = {0};
    LOADER_GETDATA( ctx, (char *)&buf, sizeof( buf ) );
    h->r_offset = buf[0] | ( (uint32_t)buf[1] << 8 );
    h->r_info   = buf[2] | ( (uint32_t)buf[3] << 8 );
    h->r_addend = buf[4] | ( (uint32_t)buf[5] << 8 );
    return 0;
err:
    return -1;
}

/********************
 * STATIC FUNCTIONS *
 ********************/

/* Reads len bytes of uncompressed data. *No seeking*.
 * Returns the number of bytes read. Returns -1 on error.
 *
 * offset - for initial debugging purposes only*/
static int decompress_get( jelfLoaderContext_t *ctx, uint8_t *inf_data, size_t inf_len )
{
    inf_stream_t *s       = &( ctx->inf_stream );
    size_t amount_written = 0;

    /* Empty the available uncompressed buffer from s->out_read */
    if( s->out_avail > 0 ) {
        if( inf_len <= s->out_avail ) {
            /* The uncompressed data is already in the out_buf */
            memcpy( inf_data, s->out_read, inf_len );
            s->out_read += inf_len;
            s->out_avail -= inf_len;
            return inf_len;
        }
        else {
            /* There's not enough data already uncompressed; copy over all avail */
            memcpy( inf_data, s->out_read, s->out_avail );
            s->out_read += s->out_avail;
            inf_data += s->out_avail;
            amount_written += s->out_avail;
            s->out_avail = 0;
        }
    }
    if( s->out_read == s->out_buf + s->out_buf_len ) { s->out_read = s->out_buf; }

    while( inf_len > amount_written ) {
        if( 0 == s->in_avail ) {
            /* Fetch more compressed data from disk */
            size_t n = LOADER_GETDATA_RAW( ctx, s->in_buf, s->in_buf_len );
            app_hash_update( ctx, s->in_buf, n );
            s->in_avail = n;
            s->in_next  = s->in_buf;
        }

        size_t in_bytes  = s->in_avail;
        size_t out_bytes = s->out_buf + s->out_buf_len - s->out_next;
        int flags        = TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_HAS_MORE_INPUT;

        /* Example values of params before and after call
         * tinfl_status tinfl_decompress(
         *     tinfl_decompressor *r,
         *     const mz_uint8 *pIn_buf_next,    0x3ffccf00 -> 0x3ffccf00 (ptr to compressed data)
         *     size_t *pIn_buf_size,            2048 -> 1708 (it used up 1708 bytes of the compressed bytes)
         *     mz_uint8 *pOut_buf_start,        0x3ffcd710 -> 0x3ffcd710 (ptr to uncompressed buffer)
         *     mz_uint8 *pOut_buf_next,         0x3ffcd710 -> 0x3ffcd710
         *     size_t *pOut_buf_size,           4096 -> 4096 (it filled the output buffer)
         *     const mz_uint32 decomp_flags);
         */

        tinfl_status status =
                tinfl_decompress( &( s->inf ), s->in_next, &in_bytes, s->out_buf, s->out_next, &out_bytes, flags );

        if( status <= TINFL_STATUS_FAILED || in_bytes == 0 ) { return -1; }
        s->in_next += in_bytes;
        s->in_avail -= in_bytes;

        s->out_next += out_bytes;
        s->out_avail += out_bytes;

        if( status <= TINFL_STATUS_DONE || s->out_next == s->out_buf + s->out_buf_len ) {
            size_t amount_remaining = inf_len - amount_written;
            size_t n_bytes_to_cpy   = s->out_next - s->out_read;
            if( n_bytes_to_cpy <= amount_remaining ) { /* Copy all of it over */
            }
            else {
                /* Copy some of it over */
                n_bytes_to_cpy = amount_remaining;
            }
            memcpy( inf_data, s->out_read, n_bytes_to_cpy );
            inf_data += n_bytes_to_cpy;
            amount_written += n_bytes_to_cpy;
            s->out_read += n_bytes_to_cpy;
            if( s->out_read == s->out_buf + s->out_buf_len ) { s->out_read = s->out_buf; }
            s->out_avail -= n_bytes_to_cpy;
            s->out_next = s->out_buf;
        }
    }

    return amount_written;
}

static const char *type2String( int symt )
{
#define STRCASE( name ) \
    case name: return #name;
    switch( symt ) {
        STRCASE( R_XTENSA_NONE )
        STRCASE( R_XTENSA_32 )
        STRCASE( R_XTENSA_ASM_EXPAND )
        STRCASE( R_XTENSA_SLOT0_OP )
        default: return "R_<unknown>";
    }
#undef STRCASE
}

/* Iterate through the singly linked list of sections until you find the one
 * that matches the provided index.
 * All these section structs are in RAM. */
static jelfLoaderSection_t *findSection( jelfLoaderContext_t *ctx, int index )
{
    PROFILER_START_FINDSECTION;
    PROFILER_INC_FINDSECTION;

    for( jelfLoaderSection_t *section = ctx->section; section != NULL; section = section->next ) {
        if( section->secIdx == index ) {
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
static bool app_hash_init( jelfLoaderContext_t *ctx, Jelf_Ehdr *header, const char *name )
{
    /* Initialize Signature Check Hashing */
    ctx->hs = LOADER_ALLOC_DATA( sizeof( crypto_hash_sha512_state ) );
    if( NULL == ctx->hs ) {
        ERR( "Failed to allocate memory for sha512 hash state" );
        goto err;
    }
    crypto_hash_sha512_init( ctx->hs );
    /* Hash the app name */
    app_hash_update( ctx, (uint8_t *)name, strlen( name ) );

    /* Copy over the app signature into the context.
     * The Signature is always the first raw 64 bytes.
     * No compression is performed on the signature. */
    {
        int n = LOADER_GETDATA_RAW( ctx, ctx->app_signature, JELF_SIGNATURE_LEN );
        if( JELF_SIGNATURE_LEN != n ) {
            ERR( "Failed to fetch signature" );
            goto err;
        }
    }

#if LOG_LOCAL_LEVEL >= 4 /* debug */
    {
        char sig_hex[129] = {0};
        sodium_bin2hex( sig_hex, sizeof( sig_hex ), ctx->app_signature, JELF_SIGNATURE_LEN );
        INFO( "App Signature: %s", sig_hex );
    }
#endif

    loader_ehdr( ctx, header );

#if CONFIG_JOLT_APP_SIG_CHECK_EN
    {
        /* First check to see if the public key is an accepted app key.
         * If it is valid, copy it into the ctx */
        uint256_t approved_pub_key = {0};
    #if ESP_PLATFORM
        size_t required_size;
        if( !storage_get_blob( NULL, &required_size, "user", "app_key" ) ) {
            ERR( "Approved Public Key not found; using default" );
            sodium_hex2bin( approved_pub_key, sizeof( approved_pub_key ), CONFIG_JOLT_APP_KEY_DEFAULT, 64, NULL, NULL,
                            NULL );
        }
        else if( sizeof( approved_pub_key ) != required_size ||
                 !storage_get_blob( approved_pub_key, &required_size, "user", "app_key" ) ) {
            ERR( "Stored Public Key Blob doesn't have expected len." );
            return false;
        }
        if( 0 != sodium_memcmp( approved_pub_key, header->e_public_key, sizeof( uint256_t ) ) ) {
            ERR( "Application Public Key doesn't match approved public key." );
            return false;
        }
    #endif

        memcpy( ctx->app_public_key, approved_pub_key, sizeof( uint256_t ) );
    }

#endif
    return true;
err:
    return false;
}

static inline void app_hash_update( jelfLoaderContext_t *ctx, const uint8_t *data, size_t len )
{
#if CONFIG_JOLT_APP_SIG_CHECK_EN
    crypto_hash_sha512_update( ctx->hs, data, len );
#endif
}

/**
 * @brief Compares the JELF version requirement vs JoltOS's compiled version
 *
 * Rules for a launch:
 *  1. Major version must absolutely match.
 *  2. minor version must be <= current version
 *  3. patch must be <= current version
 *
 * @return 0 on success. -1 if app is too old. Return -2 if JoltOS is too old.
 */
static int8_t check_version_compatability( Jelf_Ehdr *header )
{
#if ESP_PLATFORM
    if( header->e_version_major == JOLT_JELF_VERSION.major ) {
        if( header->e_version_minor == JOLT_JELF_VERSION.minor ) {
            // compare patch
            if( header->e_version_patch <= JOLT_JELF_VERSION.patch ) { return 0; }
            else {
                return -2;
            }
        }
        else if( header->e_version_minor < JOLT_JELF_VERSION.minor ) {
            return 0;
        }
        else {
            return -2;
        }
    }
    else if( header->e_version_major < JOLT_JELF_VERSION.major ) {
        // need to update app
        return -1;
    }
    else {
        // need to update JoltOS
        return -2;
    }
#else
    return 0;
#endif
}

/* Returns the Symbol's address in memory. This could either be coming from
 * the env (exported list of JoltOS) or from another section in the App.
 */
static Jelf_Addr findSymAddr( jelfLoaderContext_t *ctx, Jelf_Sym *sym )
{
    PROFILER_START_FINDSYMADDR;
    PROFILER_INC_FINDSYMADDR;
    if( sym->st_name <= ctx->env->exported_size && sym->st_name != 0 ) {
        PROFILER_STOP_FINDSYMADDR;
        MSG( "USING EXPORTED SYMBOL %d", sym->st_name - 1 );
        /* One-Indexing because 0 is <unnamed> */
        return ( Jelf_Addr )( ctx->env->exported[sym->st_name - 1] );
    }
    else {
        jelfLoaderSection_t *symSec = findSection( ctx, sym->st_shndx );
        if( symSec ) {
            PROFILER_STOP_FINDSYMADDR;
            return (Jelf_Addr)symSec->data;
        }
    }

    PROFILER_STOP_FINDSYMADDR;
    MSG( "Couldn't find symbol address in loaded sections" );
    return 0xffffffff;
}

/* relAddr - pointer into an allocated section that needs to be updated to point to symAddr.
 * type - assembly instruction type
 * symAddr - The Symbol's memory location
 * from - returned value
 * to - returned value
 *
 */
static int relocateSymbol( Jelf_Addr relAddr, int type, Jelf_Addr symAddr, uint32_t *from, uint32_t *to )
{
    PROFILER_START_RELOCATESYMBOL;
    PROFILER_INC_RELOCATESYMBOL;

    if( symAddr == 0xffffffff ) { goto err; }
    switch( type ) {
        case R_XTENSA_32: {
            *from = unalignedGet32( (void *)relAddr );
            *to   = symAddr + *from;
            unalignedSet32( (void *)relAddr, *to );
            break;
        }
        case R_XTENSA_SLOT0_OP: {
            uint32_t v = unalignedGet32( (void *)relAddr );
            *from      = v;

            /* *** Format: L32R *** */
            if( ( v & 0x00000F ) == 0x000001 ) {
                int32_t delta = symAddr - ( ( relAddr + 3 ) & 0xfffffffc );
                if( delta & 0x0000003 ) {
                    ERR( "Relocation: L32R error" );
                    goto err;
                }
                delta = delta >> 2;
                unalignedSet8( (void *)( relAddr + 1 ), ( (uint8_t *)&delta )[0] );
                unalignedSet8( (void *)( relAddr + 2 ), ( (uint8_t *)&delta )[1] );
                *to = unalignedGet32( (void *)relAddr );
                break;
            }

            /* *** Format: CALL *** */
            /* *** CALL0, CALL4, CALL8, CALL12, J *** */
            if( ( v & 0x00000F ) == 0x000005 ) {
                int32_t delta = symAddr - ( ( relAddr + 4 ) & 0xfffffffc );
                if( delta & 0x0000003 ) {
                    ERR( "Relocation: CALL error" );
                    return -1;
                }
                delta = delta >> 2;
                delta = delta << 6;
                delta |= unalignedGet8( (void *)( relAddr + 0 ) );
                unalignedSet8( (void *)( relAddr + 0 ), ( (uint8_t *)&delta )[0] );
                unalignedSet8( (void *)( relAddr + 1 ), ( (uint8_t *)&delta )[1] );
                unalignedSet8( (void *)( relAddr + 2 ), ( (uint8_t *)&delta )[2] );
                *to = unalignedGet32( (void *)relAddr );
                break;
            }

            /* *** J *** */
            if( ( v & 0x00003F ) == 0x000006 ) {
                int32_t delta = symAddr - ( relAddr + 4 );
                delta         = delta << 6;
                delta |= unalignedGet8( (void *)( relAddr + 0 ) );
                unalignedSet8( (void *)( relAddr + 0 ), ( (uint8_t *)&delta )[0] );
                unalignedSet8( (void *)( relAddr + 1 ), ( (uint8_t *)&delta )[1] );
                unalignedSet8( (void *)( relAddr + 2 ), ( (uint8_t *)&delta )[2] );
                *to = unalignedGet32( (void *)relAddr );
                break;
            }

            /* *** Format: BRI8  *** */
            /* *** BALL, BANY, BBC, BBCI, BBCI.L, BBS,  BBSI, BBSI.L, BEQ,
             * *** BGE,  BGEU, BLT, BLTU, BNALL, BNE,  BNONE, LOOP,
             * *** BEQI, BF, BGEI, BGEUI, BLTI, BLTUI, BNEI,  BT, LOOPGTZ,
             * *** LOOPNEZ *** */
            if( ( ( v & 0x00000F ) == 0x000007 ) || ( ( v & 0x00003F ) == 0x000026 ) ||
                ( ( v & 0x00003F ) == 0x000036 && ( v & 0x0000FF ) != 0x000036 ) ) {
                int32_t delta = symAddr - ( relAddr + 4 );
                unalignedSet8( (void *)( relAddr + 2 ), ( (uint8_t *)&delta )[0] );
                *to = unalignedGet32( (void *)relAddr );
                if( ( delta < -( 1 << 7 ) ) || ( delta >= ( 1 << 7 ) ) ) {
                    ERR( "Relocation: BRI8 out of range" );
                    goto err;
                }
                break;
            }

            /* *** Format: BRI12 *** */
            /* *** BEQZ, BGEZ, BLTZ, BNEZ *** */
            if( ( v & 0x00003F ) == 0x000016 ) {
                int32_t delta = symAddr - ( relAddr + 4 );
                delta         = delta << 4;
                delta |= unalignedGet32( (void *)( relAddr + 1 ) );
                unalignedSet8( (void *)( relAddr + 1 ), ( (uint8_t *)&delta )[0] );
                unalignedSet8( (void *)( relAddr + 2 ), ( (uint8_t *)&delta )[1] );
                *to   = unalignedGet32( (void *)relAddr );
                delta = symAddr - ( relAddr + 4 );
                if( ( delta < -( 1 << 11 ) ) || ( delta >= ( 1 << 11 ) ) ) {
                    ERR( "Relocation: BRI12 out of range" );
                    goto err;
                }
                break;
            }

            /* *** Format: RI6  *** */
            /* *** BEQZ.N, BNEZ.N *** */
            if( ( v & 0x008F ) == 0x008C ) {
                int32_t delta = symAddr - ( relAddr + 4 );
                int32_t d2    = delta & 0x30;
                int32_t d1    = ( delta << 4 ) & 0xf0;
                d2 |= unalignedGet32( (void *)( relAddr + 0 ) );
                d1 |= unalignedGet32( (void *)( relAddr + 1 ) );
                unalignedSet8( (void *)( relAddr + 0 ), ( (uint8_t *)&d2 )[0] );
                unalignedSet8( (void *)( relAddr + 1 ), ( (uint8_t *)&d1 )[0] );
                *to = unalignedGet32( (void *)relAddr );
                if( ( delta < 0 ) || ( delta > 0x111111 ) ) {
                    ERR( "Relocation: RI6 out of range" );
                    goto err;
                }
                break;
            }

            ERR( "Relocation: unknown opcode %08X", v );
            goto err;
        }
        case R_XTENSA_ASM_EXPAND: {
            *from = unalignedGet32( (void *)relAddr );
            *to   = unalignedGet32( (void *)relAddr );
            break;
        }
        default: MSG( "Relocation: undefined relocation %d %s", type, type2String( type ) ); goto err;
    }

    PROFILER_STOP_RELOCATESYMBOL;
    return 0;
err:
    PROFILER_STOP_RELOCATESYMBOL;
    return -1;
}

static int relocateSection( jelfLoaderContext_t *ctx, jelfLoaderSection_t *s )
{
    /* Iterate through the relocation information for the section. */
    PROFILER_START_RELOCATESECTION;
    PROFILER_INC_RELOCATESECTION;

    Jelf_Shdr sectHdr;

    /* Input Validation */
    if( !( s->relSecIdx ) ) {
        PROFILER_STOP_RELOCATESECTION;
        MSG( "  Section %d: no relocation index", s->secIdx );
        return 0;
    }
    else if( !( s->data ) ) {
        PROFILER_STOP_RELOCATESECTION;
        ERR( "Section not loaded: %d", s->secIdx );
        goto err;
    }

    /* Read the current rela section header */
    PROFILER_STOP_RELOCATESECTION;
    if( loader_shdr( ctx, s->relSecIdx, &sectHdr ) != 0 ) {
        ERR( "Error reading rela section header" );
        goto err;
    }
    PROFILER_START_RELOCATESECTION;

    int r             = 0;
    Jelf_Rela rel     = {0};
    size_t relEntries = sectHdr.sh_size / sizeof( rel );
    PROFILER_REL_COUNT( relEntries );
    MSG( "RELOCATING SECTION" )
    MSG( "  Sym  Type                      relAddr  "
         "symAddr  defValue                    Name + addend" );
    /* Read every RELA entry in the current RELA section */
    for( size_t relCount = 0; relCount < relEntries; relCount++ ) {
        r |= loader_rela( ctx, &rel );
        PROFILER_MAX_R_OFFSET( rel.r_offset );

        Jelf_Sym sym = {0};
        int symEntry = JELF_R_SYM( rel.r_info );   // SymbolTable Index
        int relType  = JELF_R_TYPE( rel.r_info );  // RelocationType

        /* data to be updated address */
        Jelf_Addr relAddr = ( (Jelf_Addr)s->data ) + rel.r_offset;
        PROFILER_STOP_RELOCATESECTION;
        if( 0 != loader_sym( ctx, symEntry, &sym ) ) { goto err; }
        PROFILER_START_RELOCATESECTION;

        /* Target Symbol Address */
        PROFILER_STOP_RELOCATESECTION;
        // Returns 0xffffffff if symbol cannot be found
        Jelf_Addr symAddr = findSymAddr( ctx, &sym ) + rel.r_addend;
        PROFILER_START_RELOCATESECTION;

        uint32_t from = 0, to = 0;
        if( relType == R_XTENSA_NONE || relType == R_XTENSA_ASM_EXPAND ) {}
        else if( ( symAddr == 0xffffffff ) ) {
            ERR( "Relocation - undefined symAddr" );
            ERR( "  %04X %04X %-20s %08X %08X %08X"
                 "             + %X",
                 symEntry, relType, type2String( relType ), relAddr, symAddr, sym.st_name, rel.r_addend );
            r |= -1;
        }
        else if( relocateSymbol( relAddr, relType, symAddr, &from, &to ) != 0 ) {
            ERR( "relocateSymbol fail" );
            ERR( "  %04X %04X %-20s %08X %08X %08X->%08X  + %X", symEntry, relType, type2String( relType ), relAddr,
                 symAddr, from, to, rel.r_addend );
            r |= -1;
        }
        else {
            MSG( "  %04X %04X %-20s %08X %08X %08X->%08X  + %X", symEntry, relType, type2String( relType ), relAddr,
                 symAddr, from, to, rel.r_addend );
        }
    }
    PROFILER_STOP_RELOCATESECTION;
    return r;

err:
    PROFILER_STOP_RELOCATESECTION;
    ERR( "Error reading relocation data" );
    return -1;
}

/********************
 * PUBLIC FUNCTIONS *
 ********************/

bool jelfLoaderSigCheck( const jelfLoaderContext_t *ctx )
{
    if( JELF_CTX_READY != ctx->state ) {
        ERR( "Invalid context state: %d", ctx->state );
        return false;
    }

    if( 0 == crypto_sign_verify_detached( ctx->app_signature, ctx->hash, sizeof( ctx->hash ), ctx->app_public_key ) ) {
        return true;
    }
    else {
        ERR( "Bad Signature" );
        char hash[HEX_512] = {0};
        sodium_bin2hex( hash, sizeof( hash ), jelfLoaderGetHash( ctx ), 64 );
        ERR( "App Hash: %s", hash );
        return false;
    }
}

uint8_t *jelfLoaderGetHash( const jelfLoaderContext_t *ctx )
{
    if( JELF_CTX_READY != ctx->state ) {
        ERR( "Invalid context state: %d", ctx->state );
        goto exit;
    }
    return (uint8_t *)ctx->hash;
exit:
    return NULL;
}

int jelfLoaderRun( jelfLoaderContext_t *ctx, int argc, char **argv )
{
    if( JELF_CTX_READY != ctx->state ) {
        ERR( "Invalid context state: %d", ctx->state );
        return -1;
    }

    int res = -1;

    if( !ctx->exec ) {
        MSG( "No Entrypoint set." );
        return 0;
    }

    /* Free up loading cache */
    if( NULL != ctx->sht_cache ) {
        LOADER_FREE( ctx->sht_cache );
        ctx->sht_cache = NULL;
    }
    if( NULL != ctx->symtab_cache ) {
        LOADER_FREE( ctx->symtab_cache );
        ctx->symtab_cache = NULL;
    }
    if( NULL != ctx->inf_stream.in_buf ) {
        LOADER_FREE( ctx->inf_stream.in_buf );
        ctx->inf_stream.in_buf = NULL;
    }
    if( NULL != ctx->inf_stream.out_buf ) {
        LOADER_FREE( ctx->inf_stream.out_buf );
        ctx->inf_stream.out_buf = NULL;
    }
    {
        typedef int ( *func_t )( int, char ** );
        func_t func = (func_t)ctx->exec;
        MSG( "Running... (%p)", func );
        res = func( argc, argv );
        MSG( "Result: %08X", res );
    }
    return res;
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
jelfLoaderStatus_t jelfLoaderInit( jelfLoaderContext_t **ctx_ptr, LOADER_FD_T fd, const char *name,
                                   const jelfLoaderEnv_t *env )
{
    Jelf_Ehdr header;
    jelfLoaderStatus_t response = JELF_LOADER_ERROR;
    jelfLoaderContext_t *ctx    = NULL;

    if( NULL == ctx_ptr ) { goto err; }

    /* Debugging Size Sanity Check */
    MSG( "Jelf_Ehdr: %d", sizeof( Jelf_Ehdr ) );
    MSG( "Jelf_Sym:  %d", sizeof( Jelf_Sym ) );
    MSG( "Jelf_Shdr: %d", sizeof( Jelf_Shdr ) );
    MSG( "Jelf_Rela: %d", sizeof( Jelf_Rela ) );

    /***********************************************
     * Initialize the context object with pointers *
     ***********************************************/
    *ctx_ptr = LOADER_ALLOC_DATA( sizeof( jelfLoaderContext_t ) );
    ctx      = *ctx_ptr;
    if( NULL == ctx ) {
        ERR( "Insufficient memory for ElfLoaderContext_t" );
        response = JELF_LOADER_OOM;
        goto err;
    }
    memset( ctx, 0, sizeof( jelfLoaderContext_t ) );
    ctx->fd  = fd;
    ctx->env = env;

    /* Initialize Inflator */
    MSG( "Initializing Inflator" );
    tinfl_init( &( ctx->inf_stream.inf ) );
    ctx->inf_stream.in_buf = LOADER_ALLOC_DATA( CONFIG_JELFLOADER_INPUTBUF_LEN );
    if( NULL == ctx->inf_stream.in_buf ) {
        ERR( "Insufficient memory for miniz input buffer" );
        response = JELF_LOADER_OOM;
        goto err;
    }
    ctx->inf_stream.in_buf_len = CONFIG_JELFLOADER_INPUTBUF_LEN;
    ctx->inf_stream.in_next    = ctx->inf_stream.in_buf;
    ctx->inf_stream.in_avail   = 0;

    ctx->inf_stream.out_buf = LOADER_ALLOC_DATA( CONFIG_JOLT_COMPRESSION_OUTPUT_BUFFER );
    if( NULL == ctx->inf_stream.out_buf ) {
        ERR( "Insufficient memory for miniz output buffer" );
        response = JELF_LOADER_OOM;
        goto err;
    }
    ctx->inf_stream.out_buf_len = CONFIG_JOLT_COMPRESSION_OUTPUT_BUFFER;
    ctx->inf_stream.out_next    = ctx->inf_stream.out_buf;

    ctx->inf_stream.out_read  = ctx->inf_stream.out_buf;
    ctx->inf_stream.out_avail = 0;

    /**********************************
     * Initialize App Signature Check *
     **********************************/
    /* Will populate the main JELF header */
    MSG( "Initializing Signature Checker" );
    if( !app_hash_init( ctx, &header, name ) ) {
        response = JELF_LOADER_INVALID_KEY;
        goto err;
    }

    /* Make sure that we have a correct and compatible ELF header. */
    char JelfMagic[] = {0x7f, 'J', 'E', 'L', 'F', '\0'};
    if( 0 != memcmp( header.e_ident, JelfMagic, strlen( JelfMagic ) ) ) {
        ERR( "Bad JELF Identification" );
        ERR( "File Magic Identifier: %s", header.e_ident );
        goto err;
    }

    /* Version Compatability Check */
    MSG( "Checking Version Compatability" );
    switch( check_version_compatability( &header ) ) {
        case 0:
            /* success */
            break;
        case -1: response = JELF_LOADER_VERSION_APP; goto err;
        case -2: response = JELF_LOADER_VERSION_JOLTOS; goto err;
        default: goto err;
    }

    /* Debug Sanity Checks */
    MSG( "SectionHeaderTableEntries: %d", header.e_shnum );
    MSG( "Derivation Purpose: 0x%08X", header.e_coin_purpose );
    MSG( "Derivation Path: 0x%08X", header.e_coin_path );
    MSG( "bip32key: %s", header.e_bip32key );

    {
        /**************************************
         * Cache sectionheadertable to memory *
         **************************************/
        size_t sht_size = header.e_shnum * JELF_SHDR_SIZE;
        ctx->sht_cache  = LOADER_ALLOC_DATA( sht_size );
        if( NULL == ctx->sht_cache ) {
            ERR( "Insufficient memory for section header table cache" );
            response = JELF_LOADER_OOM;
            goto err;
        }
        /* Populate the cache */
        LOADER_GETDATA( ctx, (char *)( ctx->sht_cache ), sht_size );
    }

    /* Populate context with ELF Header information*/
    ctx->e_shnum      = header.e_shnum;  // Number of Sections
    ctx->entry_index  = header.e_entry_index;
    ctx->coin_purpose = header.e_coin_purpose;
    ctx->coin_path    = header.e_coin_path;
    if( strlen( header.e_bip32key ) >= sizeof( ctx->bip32_key ) ) {
        ERR( "Malformed JELF header." );
        response = JELF_LOADER_MALFORMED;
        goto err;
    }
    strlcpy( ctx->bip32_key, header.e_bip32key, sizeof( ctx->bip32_key ) );

    ctx->state = JELF_CTX_INITIALIZED;

    return JELF_LOADER_OK;

err:
    if( NULL != ctx ) { jelfLoaderFree( ctx ); }
    *ctx_ptr = NULL;
    return response;
}

jelfLoaderStatus_t jelfLoaderLoad( jelfLoaderContext_t *ctx )
{
    jelfLoaderStatus_t response = JELF_LOADER_ERROR;

    if( NULL == ctx ) {
        ERR( "NULL context" );
        goto err;
    }

    if( JELF_CTX_INITIALIZED != ctx->state ) {
        ERR( "Invalid context state: %d", ctx->state );
        response = JELF_LOADER_INVALID_STATE;
        goto err;
    }

    MSG( "Scanning ELF sections         relAddr      size" );
    // Iterate through all section_headers in the section header table
    for( int n = 1; n < ctx->e_shnum; n++ ) {
        MSG( "Loading section %d", n );
        Jelf_Shdr sectHdr = {0};

        /***********************
         * Read Section Header *
         ***********************/
        // Read the section header at index n
        // Populates "secHdr" with the Section's Header
        if( 0 != loader_shdr( ctx, n, &sectHdr ) ) {
            ERR( "Error reading section" );
            response = JELF_LOADER_MALFORMED;
            goto err;
        }

        /* This  section  occupies  memory during process execution */
        if( sectHdr.sh_flags & SHF_ALLOC ) {
            MSG( "Section %d has SHF_ALLOC flag.", n );
            if( !sectHdr.sh_size ) { MSG( "  section %2d no data", n ); }
            else {
                /* Allocate space for Section Struct (not data) */
                jelfLoaderSection_t *section = LOADER_ALLOC_DATA( sizeof( jelfLoaderSection_t ) );
                if( NULL == section ) {
                    ERR( "Error allocating space for SHF_ALLOC section" );
                    response = JELF_LOADER_OOM;
                    goto err;
                }
                memset( section, 0, sizeof( jelfLoaderSection_t ) );

                /* Populate the Section Elements */
                section->secIdx = n;
                section->size   = sectHdr.sh_size;

                /* Add it to the beginning of the SinglyLinkedList */
                section->next = ctx->section;
                ctx->section  = section;

                /* Allocate memory */
                MSG( "Section %d attempting to allocate %d bytes.", n, sectHdr.sh_size );

                /* It's important that the returned pointer is always increasing
                 * or else the program may crash.
                 * Currently this is not enforced by design, but the allocations
                 * see to always be increasing. */
                section->data = ( sectHdr.sh_flags & SHF_EXECINSTR ) ? LOADER_ALLOC_EXEC( CEIL4( sectHdr.sh_size ) )
                                                                     :         // Executable Memory
                                        LOADER_ALLOC_DATA( sectHdr.sh_size );  // Normal Memory

                if( !section->data ) {
                    ERR( "Section %d malloc failed.", n );
                    response = JELF_LOADER_OOM;
                    goto err;
                }

                /* Load Section into allocated data (if there's data
                 * to load) */
                if( sectHdr.sh_type != SHT_NOBITS ) {
                    if( sectHdr.sh_flags & SHF_EXECINSTR ) {
                        /* To get around LoadStoreErrors with reading single
                         * bytes from instruction memory */
                        char *tmp = LOADER_ALLOC_DATA( CEIL4( sectHdr.sh_size ) );
                        /* Zero out the last 4 bytes, just in case */
                        memset( &tmp[CEIL4( sectHdr.sh_size ) - 4], 0, 4 );

                        LOADER_GETDATA( ctx, tmp, sectHdr.sh_size );
                        memcpy( section->data, tmp, CEIL4( sectHdr.sh_size ) );
                        LOADER_FREE( tmp );
                    }
                    else {
                        LOADER_GETDATA( ctx, section->data, sectHdr.sh_size );
                    }
                }
                else {
                    memset( section->data, 0, sectHdr.sh_size );
                }

                MSG( "  section %2d %08X %6i", n, (unsigned int)section->data, sectHdr.sh_size );
            }
        }
        /* Relocation Entries with Addends */
        else if( SHT_RELA == sectHdr.sh_type ) {
            /* sh_info holds extra information that depends on sh_type.
             * For sh_type SHT_RELA:
             *     The section header index of the section to which the
             *     relocation applies.
             */

            /* If the index is greater than the number of sections that exist,
             * it must be erroneous */
            if( sectHdr.sh_info >= n ) {
                ERR( "Rela section: bad linked section (%i -> %i)", n, sectHdr.sh_info );
                response = JELF_LOADER_LINK;
                goto err;
            }

            /* iterate through the singly linked list of sections until you
             * find the one that matches sh_info */
            jelfLoaderSection_t *section = findSection( ctx, sectHdr.sh_info );
            if( section == NULL ) {  // Cannot find section
                MSG( "  section %2d -> %2d: ignoring", n, sectHdr.sh_info );
            }
            else {
                section->relSecIdx = n;
                MSG( "  section %2d -> %2d: ok", n, sectHdr.sh_info );
            }
        }
        else if( SHT_SYMTAB == sectHdr.sh_type ) {
            MSG( "  section %2d symtab", n );

            /**************************
             * Cache symtab to memory *
             **************************/
            ctx->symtab_cache = LOADER_ALLOC_DATA( sectHdr.sh_size );
            if( NULL == ctx->symtab_cache ) {
                ERR( "Error allocating space for SymbolTable cache" );
                response = JELF_LOADER_OOM;
                goto err;
            }
            LOADER_GETDATA( ctx, (char *)( ctx->symtab_cache ), sectHdr.sh_size );
            ctx->symtab_count = sectHdr.sh_size / JELF_SYM_SIZE;
        }
    }
    if( NULL == ctx->symtab_cache ) {
        ERR( "Missing .symtab" );
        response = JELF_LOADER_SYMTAB;
        goto err;
    }
    MSG( "successfully loaded sections" );

    Jelf_Sym sym;
    if( 0 != loader_sym( ctx, ctx->entry_index, &sym ) ) { goto err; }

    jelfLoaderSection_t *symbol_section = findSection( ctx, sym.st_shndx );
    if( NULL == symbol_section ) {
        ERR( "Error setting entrypoint." );
        response = JELF_LOADER_ENTRYPOINT;
        goto err;
    }
    ctx->exec = (void *)( (Jelf_Addr)symbol_section->data );
    MSG( "successfully set entrypoint" );
    ctx->state = JELF_CTX_LOADED;

    return JELF_LOADER_OK;

err:
    if( NULL != ctx ) { ctx->state = JELF_CTX_ERROR; }
    return response;
}

jelfLoaderStatus_t jelfLoaderRelocate( jelfLoaderContext_t *ctx )
{
    jelfLoaderStatus_t response = JELF_LOADER_ERROR;

    if( NULL == ctx ) {
        ERR( "NULL context" );
        goto err;
    }

    if( JELF_CTX_LOADED != ctx->state ) {
        ERR( "Invalid context state: %d", ctx->state );
        response = JELF_LOADER_INVALID_STATE;
        goto err;
    }

    MSG( "Relocating sections" );
    int r          = 0;
    uint32_t count = 0;
    for( jelfLoaderSection_t *section = ctx->section; section != NULL; section = section->next ) {
        r |= relocateSection( ctx, section );
        count++;
    }
    if( r != 0 ) {
        MSG( "Relocation failed" );
        response = JELF_LOADER_RELOC;
        goto err;
    }
    MSG( "Successfully relocated %d sections.", count );

#if CONFIG_JOLT_APP_SIG_CHECK_EN
    /* Populate the hash field */
    crypto_hash_sha512_final( ctx->hs, ctx->hash );
    LOADER_FREE( ctx->hs );
    ctx->hs = NULL;
#endif
    ctx->state = JELF_CTX_READY;

    return JELF_LOADER_OK;

err:
    if( NULL != ctx ) { ctx->state = JELF_CTX_ERROR; }
    return response;
}

void jelfLoaderFree( jelfLoaderContext_t *ctx )
{
    if( NULL == ctx ) { return; }

#if CONFIG_JOLT_APP_SIG_CHECK_EN
    if( NULL != ctx->hs ) {
        LOADER_FREE( ctx->hs );
        ctx->hs = NULL;
    }
#endif

    if( NULL != ctx->sht_cache ) {
        LOADER_FREE( ctx->sht_cache );
        ctx->sht_cache = NULL;
    }
    if( NULL != ctx->symtab_cache ) {
        LOADER_FREE( ctx->symtab_cache );
        ctx->symtab_cache = NULL;
    }
    if( NULL != ctx->inf_stream.in_buf ) {
        LOADER_FREE( ctx->inf_stream.in_buf );
        ctx->inf_stream.in_buf = NULL;
    }
    if( NULL != ctx->inf_stream.out_buf ) {
        LOADER_FREE( ctx->inf_stream.out_buf );
        ctx->inf_stream.out_buf = NULL;
    }

    jelfLoaderSection_t *section = ctx->section;
    jelfLoaderSection_t *next;
    while( section != NULL ) {
        if( section->data ) { LOADER_FREE( section->data ); }
        next = section->next;
        free( section );
        section = next;
    }
    free( ctx );
}
