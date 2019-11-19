/**
 * @file jelf.h
 * @brief Jolt ELF data structures
 * @author Brian Pugh
 */

#ifndef JELFLOADER_JELF_H__
#define JELFLOADER_JELF_H__

/* Enables Profiler Tools */
#ifndef CONFIG_ELFLOADER_PROFILER_EN
    #define CONFIG_ELFLOADER_PROFILER_EN true
#endif

typedef size_t Jelf_Addr;

/* Unpadded structure sizes are macro'd for indexing into the JELF binary */

/**
 * @brief Magic Header length
 */
#define JELF_EI_NIDENT 6

/**
 * @brief Public Key Length
 */
#define JELF_PUBLIC_KEY_LEN 32

/**
 * @brief BIP32 Seed phrase length.
 */
#define JELF_BIP32KEY_LEN 32

/**
 * @brief JELF header data structure
 */
typedef struct {
    unsigned char e_ident[JELF_EI_NIDENT];     /**< Magic number header */
    uint8_t e_public_key[JELF_PUBLIC_KEY_LEN]; /**< Public Key */
    uint8_t e_signature[64];                   /**< App Signature */
    uint8_t e_version_major;                   /**< Environment/Loader Major Version */
    uint8_t e_version_minor;                   /**< Environment/Loader Minor Version */
    uint8_t e_version_patch;                   /**< Environment/Loader Patch Version */
    uint8_t e_app_major;                       /**< App */
    uint8_t e_app_minor;                       /**< App */
    uint8_t e_app_patch;                       /**< App */
    uint16_t e_entry_index;                    /**< Entry point function offset */
    uint16_t e_shnum;                          /**< Section header table entry count */
    uint32_t e_coin_purpose;                   /**< BIP32 Coin Purpose */
    uint32_t e_coin_path;                      /**< BIP32 Coin Type */
    char e_bip32key[JELF_BIP32KEY_LEN];        /**< BIP32 Seed phrase */
} Jelf_Ehdr;
#define JELF_EHDR_SIZE_V0   152
#define JELF_EHDR_MANDATORY 9  // First 9 bytes of a JELF header will ALWAYS have fixed meaning.

/**
 * @brief JELF Symbol Entry
 */
typedef struct {
    uint32_t st_name : 12;  /**< Index, also Name */
    uint32_t st_shndx : 12; /**< Section index */
} Jelf_Sym;
#define JELF_SYM_SIZE 3

/**
 * @brief JELF Section Header
 */
typedef struct {
    uint8_t sh_type : 2;   /**< Section type */
    uint8_t sh_flags : 2;  /**< Section flags */
    uint32_t sh_size : 16; /**< Section size in bytes */
    uint16_t sh_info : 12; /**< Additional section information */
} Jelf_Shdr;
#define JELF_SHDR_SIZE 4

/**
 * @brief Relocation entries
 */
typedef struct {
    uint16_t r_offset; /**< offset bytes into section data to relocate*/
    uint16_t r_info;   /**< Relocation type and symbol index */
    int16_t r_addend;  /**< Addend */
} Jelf_Rela;
#define JELF_RELA_SIZE 6

/**
 * @brief Relocation Types
 * Used in the lowest 2 bits of Jelf_Rela.r_info
 */
#define R_XTENSA_NONE       0
#define R_XTENSA_32         1
#define R_XTENSA_ASM_EXPAND 2
#define R_XTENSA_SLOT0_OP   3

#define SHT_OTHER  0
#define SHT_RELA   1
#define SHT_NOBITS 2
#define SHT_SYMTAB 3

#define SHF_ALLOC     ( 1 << 0 )
#define SHF_EXECINSTR ( 1 << 1 )

#define JELF_R_SYM( val )        ( ( val ) >> 2 )
#define JELF_R_TYPE( val )       ( (val)&0x03 )
#define JELF_R_INFO( sym, type ) ( ( ( sym ) << 2 ) + ( (type)&0x03 ) )

#endif
