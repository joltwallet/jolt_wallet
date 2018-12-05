#ifndef JELFLOADER_JELF_H__
#define JELFLOADER_JELF_H__

/* Enables Profiler Tools */
#ifndef CONFIG_ELFLOADER_PROFILER_EN
    #define CONFIG_ELFLOADER_PROFILER_EN true
#endif

typedef uint32_t Jelf_Addr;

/* Unpadded structure sizes are macro'd for indexing into the JELF binary */

#define EI_NIDENT 6
typedef struct {
    unsigned char  e_ident[EI_NIDENT];  /* Magic number and other info */
    uint8_t        e_signature[64];
    uint8_t        e_public_key[32];
    uint8_t        e_version_major;
    uint8_t        e_version_minor;
    uint16_t       e_entry_offset;      /* Entry point function offset */
    uint16_t       e_shnum;             /* Section header table entry count */
    uint32_t       e_shoff;
    uint32_t       e_coin_purpose;
    uint32_t       e_coin_path;
    char           e_bip32key[32];
} Jelf_Ehdr;
#define JELF_EHDR_SIZE 152

typedef struct {
    uint16_t         st_name;         /* Index, also Name */
    uint16_t         st_shndx;        /* Section index */
    uint32_t         st_value;        /* Unused most the time, but cant think of a good way of removing */
} Jelf_Sym;
#define JELF_SYM_SIZE 8

typedef struct {
    uint8_t       sh_type     :2;         /* Section type */
    uint8_t       sh_flags    :2;         /* Section flags */
    uint32_t      sh_offset   :19;        /* Section file offset */
    uint32_t      sh_size     :19;        /* Section size in bytes */
    uint16_t      sh_info     :14;        /* Additional section information */
} Jelf_Shdr;
#define JELF_SHDR_SIZE 7

typedef struct {
    uint16_t    r_offset;        /* Address */
    uint16_t    r_info;          /* Relocation type and symbol index */
    int16_t     r_addend;        /* Addend */
} Jelf_Rela;
#define JELF_RELA_SIZE 6

/* Relocation Types, used in the lowest 2 bits of Jelf_Rela.r_info */
#define R_XTENSA_NONE           0
#define R_XTENSA_32             1
#define R_XTENSA_ASM_EXPAND     2
#define R_XTENSA_SLOT0_OP       3

#define SHT_OTHER   0
#define SHT_RELA    1
#define SHT_NOBITS  2
#define SHT_SYMTAB  3

#define SHF_ALLOC      (1 << 0)
#define SHF_EXECINSTR  (1 << 1)


#define JELF_R_SYM(val)        ((val) >> 2)
#define JELF_R_TYPE(val)       ((val) & 0x03)
#define JELF_R_INFO(sym, type)     (((sym) << 2) + ((type) & 0x03))

#endif
