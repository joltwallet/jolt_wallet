/**
 * @file jelfloader.h
 * @brief Jolt ELF application loading functions.
 * @author Brian Pugh
 */

#ifndef JELFLOADER_H__
#define JELFLOADER_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/jelf.h"
#include "jolttypes.h"
#include "sodium.h"

#if ESP_PLATFORM
    #include "esp32/rom/miniz.h"
#else
    #include "miniz.h"
#endif

#define LOADER_FD_T FILE *

#ifndef CONFIG_JOLT_APP_SIG_CHECK_EN
    #define CONFIG_JOLT_APP_SIG_CHECK_EN 1
#endif

#ifndef CONFIG_JOLT_APP_KEY_DEFAULT
    #define CONFIG_JOLT_APP_KEY_DEFAULT "03a107bff3ce10be1d70dd18e74bc09967e4d6309ba50d5f1ddc8664125531b8"
#endif

/**
 * @brief Functions exported from JoltOS, available to applications.
 */
typedef struct {
    const void **exported;      /**< Exported symbols array. */
    unsigned int exported_size; /**< Number of exported symbols. */
} jelfLoaderEnv_t;

/**
 * @brief Singly Linked List Used to cache ELF sections needed at runtime.
 */
typedef struct jelfLoaderSection_t {
    void *data;                       /**< Contents of the ALLOC section*/
    size_t size;                      /**< Length of data in bytes */
    uint16_t secIdx;                  /**< Section's Index */
    uint16_t relSecIdx;               /**< Accompanying RELA section index */
    struct jelfLoaderSection_t *next; /**< Next section in this SLL */
} jelfLoaderSection_t;

/**
 * @brief Inflator (Decompressor) Object
 *
 * Holds the state to stream data out of a compressed file.
 */
typedef struct inf_stream_t {
    tinfl_decompressor inf; /**< miniz inflator object */

    uint8_t *in_buf;   /**< inflator input buffer to hold compressed bytes*/
    size_t in_buf_len; /**< size of input buffer */

    const unsigned char *in_next; /**< pointer to next compressed byte to read */
    size_t in_avail;              /**< number of compressed bytes available at next_in */

    uint8_t *out_buf;   /**< miniz output buffer to hold uncompressed bytes */
    size_t out_buf_len; /**< Size of output buffer. MUST BE A POWER OF 2 */
    uint8_t *out_next;  /**< For internal tinfl state */

    uint8_t *out_read; /**< Pointer to available uncompressed data to read from*/
    size_t out_avail;  /**< Number of uncompressed bytes to read from */
} inf_stream_t;

/**
 * @brief Current context state/status
 *
 * Primarily used for error checking.
 */
enum {
    JELF_CTX_ERROR         = -1, /**< Context is in an irrepairable state */
    JELF_CTX_UNINITIALIZED = 0,  /**< Dummy State */
    JELF_CTX_INITIALIZED,        /**< Context has been allocated */
    JELF_CTX_LOADED,             /**< All sections have been loaded */
    JELF_CTX_READY,              /**< Application is ready to launch */
};
typedef int8_t jelfLoaderContext_state_t;

/**
 * @brief Application Loader Context
 *
 * Holds the entire state for loading, linking, and executing an application.
 */
typedef struct jelfLoaderContext_t {
    LOADER_FD_T fd;             /**< Application's File Descriptor */
    void *exec;                 /**< Application Entrypoint */
    const jelfLoaderEnv_t *env; /**< List of JoltOS exported functions */
    /* state - moved below for packing reasons */

    /* SectionHeaderTable */
    uint8_t *sht_cache;   /**< Cached copy of application's SectionHeaderTable */
    uint16_t entry_index; /**< Entrypoint index into the SymbolTable */
    uint16_t e_shnum;     /**< Number of entries into the SectionHeaderTable */

    jelfLoaderSection_t *section; /**< First element cached alloc sections */

    /* SymbolTable */
    uint8_t *symtab_cache;    /**< Cached copy of the application's SymTab Section */
    size_t symtab_count;      /**< Number of Symbols in the SymTab */

    /* Coin Derivation Data */
    uint32_t coin_purpose;                 /**< BIP32 Purpose; typically 44' */
    uint32_t coin_path;                    /**< BIP32 Coin Type */
    char bip32_key[JELF_BIP32KEY_LEN + 1]; /**< BIP32 Seed String; e.g. "bitcoin_seed" */
    jelfLoaderContext_state_t state;       /**< Current State of the launching procedure */

    /* Inflator */
    inf_stream_t inf_stream; /**< Inflator/Decompressor object */

#if CONFIG_JOLT_APP_SIG_CHECK_EN
    /* Data Structs For Checking App Signature.
     * The signature procedure is as follows:
     *     1. Hash the application name.
     *     2. Hash application data as we read from disk.
     *     3. Upon loading completion; verify the signature from the loading hash.
     * We hash as we load to prevent malicious data being swapped after a signature check.
     */
    crypto_hash_sha512_state *hs;    /**< Hash State */
    uint8_t hash[BIN_512];           /**< Final Hash */
    uint8_t app_public_key[BIN_256]; /**< Application's Public Key */
    uint8_t app_signature[BIN_512];  /**< Application's Signature */
#endif
} jelfLoaderContext_t;

/**
 * @brief Application loading return codes
 */
typedef enum jelfLoaderStatus_t {
    JELF_LOADER_OK = 0,         /**< Success */
    JELF_LOADER_ERROR,          /**< Generic error if one was not explicitly defined */
    JELF_LOADER_INVALID_STATE,  /**< Context is not in correct state to perform requested action */
    JELF_LOADER_INVALID_KEY,    /**< Application's public key doesn't match approved */
    JELF_LOADER_OOM,            /**< Out of memory */
    JELF_LOADER_VERSION_JOLTOS, /**< JoltOS is out of date */
    JELF_LOADER_VERSION_APP,    /**< Application is out of data */
    JELF_LOADER_MALFORMED,      /**< Malformed data */
    JELF_LOADER_RELOC,          /**< Error relocating section */
    JELF_LOADER_LINK,           /**< Invalid section linking data */
    JELF_LOADER_SYMTAB,         /**< Missing symbol table */
    JELF_LOADER_ENTRYPOINT,     /**< Missing entrypoint */
} jelfLoaderStatus_t;

/**
 * @brief Runs the EntryPoint of the program loaded in the give context.
 * @param[in,out] ctx Context the application was loaded into
 * @param[in] argc Argument Count
 * @param[in] argv Variable Arguments
 * @return exit code returned by EntryPoint
 */
int jelfLoaderRun( jelfLoaderContext_t *ctx, int argc, char **argv );

/**
 * @brief Run the application without arguments
 *
 * Run the application without arguments. Typically this is to invoke the GUI.
 * @param[in,out] ctx Context the application was loaded into
 * @return Application loader context
 */
inline int jelfLoaderRunAppMain( jelfLoaderContext_t *ctx ) { return jelfLoaderRun( ctx, 0, NULL ); }

/**
 * @brief Run the application's console entrypoint.
 * @param[in,out] ctx Context the application was loaded into
 * @param[in] argc Argument Count
 * @param[in] argv Variable Arguments
 * @return exit code returned by command
 */
inline int jelfLoaderRunConsole( jelfLoaderContext_t *ctx, int argc, char **argv )
{
    /* Just more explicit way of running app */
    return jelfLoaderRun( ctx, argc, argv );
}

/**
 * @brief Allocates the loader object, hasher, and reads in JELF Header info.
 *
 * First Operation in the launching procedure. Performs the following:
 *     * Allocates space for the returned Context (constant size)
 *     * Populates Context with:
 *         * Pointer/FD to the beginning of the ELF file
 *         * Pointer to env, which is a table to exported host function_names and their pointer in memory.
 *         * Number of sections in the ELF file.
 *         * offset to SectionHeaderTable, which maps an index to a offset in ELF where the section header begins.
 *         * offset to StringTable, which is a list of null terminated strings.
 *
 * @param[out] ctx_ptr Allocated JELF loading context
 * @param[in,out] fd
 * @param[in,out] name
 * @param[in,out] env Exported function environment
 * @return status code
 */
jelfLoaderStatus_t jelfLoaderInit( jelfLoaderContext_t **ctx_ptr, LOADER_FD_T fd, const char *name,
                                   const jelfLoaderEnv_t *env );

/**
 * @brief Loads ALLOC sections into context.
 * @param[in,out] ctx Context the application was loaded into.
 * @return Application loader context. Returns NULL on error.
 */
jelfLoaderStatus_t jelfLoaderLoad( jelfLoaderContext_t *ctx );

/**
 * @brief Loads RELA sections and performs relocation.
 * @param[in,out] ctx Context the application was loaded into
 * @return Application loader context. Returns NULL on error.
 */
jelfLoaderStatus_t jelfLoaderRelocate( jelfLoaderContext_t *ctx );

/**
 * @brief Deallocates the loader context.
 * @param[in,out] ctx Context the application was loaded into
 */
void jelfLoaderFree( jelfLoaderContext_t *ctx );

/**
 * @brief Verifies the application signature.
 *
 * Can only be called after relocating.
 * @param[in] ctx Context the application was loaded into
 * @return True on valid signature, false otherwise.
 */
bool jelfLoaderSigCheck( const jelfLoaderContext_t *ctx );

/**
 * @brief Gets the pointer to the 64-byte hash of context.
 *
 * Returns the hash pointer of the context struct. Does not trigger the
 * computation of a hash.
 * @param[in] ctx Context the application was loaded into
 * @return Pointer to context's hash.
 */
uint8_t *jelfLoaderGetHash( const jelfLoaderContext_t *ctx );  // return hash (in bytes, 64 long)

/**
 * @brief Sets all profiler timers and counters to 0
 */

void jelfLoaderProfilerReset( void );

/**
 * @brief Prints the profiler results to uart console
 */
void jelfLoaderProfilerPrint( void );

#endif
