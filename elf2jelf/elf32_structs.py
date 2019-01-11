from collections import OrderedDict, namedtuple
import bitstruct as bs
from common_structs import Unpacker

'''
Using an OrderedDict because it self documents quite well
'''

'''
ELF Header
'''
_Elf32_Ehdr_d = OrderedDict()
_Elf32_Ehdr_d['e_ident']     = 't%d' % (8*16) # 16 8-bit characters
_Elf32_Ehdr_d['e_type']      = 'u16'
_Elf32_Ehdr_d['e_machine']   = 'u16'
_Elf32_Ehdr_d['e_version']   = 'u32'
_Elf32_Ehdr_d['e_entry']     = 'u32'
_Elf32_Ehdr_d['e_phoff']     = 'u32'
_Elf32_Ehdr_d['e_shoff']     = 'u32'
_Elf32_Ehdr_d['e_flags']     = 'u32'
_Elf32_Ehdr_d['e_ehsize']    = 'u16'
_Elf32_Ehdr_d['e_phentsize'] = 'u16'
_Elf32_Ehdr_d['e_phnum']     = 'u16'
_Elf32_Ehdr_d['e_shentsize'] = 'u16'
_Elf32_Ehdr_d['e_shnum']     = 'u16'
_Elf32_Ehdr_d['e_shstrndx']  = 'u16'
Elf32_Ehdr = Unpacker( 'Elf32_Ehdr', _Elf32_Ehdr_d )

'''
Section Header
'''
_Elf32_Shdr_d = OrderedDict()
_Elf32_Shdr_d['sh_name']      = 'u32'
_Elf32_Shdr_d['sh_type']      = 'u32'
_Elf32_Shdr_d['sh_flags']     = 'u32'
_Elf32_Shdr_d['sh_addr']      = 'u32'
_Elf32_Shdr_d['sh_offset']    = 'u32'
_Elf32_Shdr_d['sh_size']      = 'u32'
_Elf32_Shdr_d['sh_link']      = 'u32'
_Elf32_Shdr_d['sh_info']      = 'u32'
_Elf32_Shdr_d['sh_addralign'] = 'u32'
_Elf32_Shdr_d['sh_entsize']   = 'u32'
Elf32_Shdr = Unpacker( 'Elf32_Shdr', _Elf32_Shdr_d )

Elf32_SHT_RELA   = 4
Elf32_SHT_NOBITS = 8

Elf32_SHF_ALLOC     = 1 << 1
Elf32_SHF_EXECINSTR = 1 << 2

'''
Symbol
'''
_Elf32_Sym_d = OrderedDict()
_Elf32_Sym_d['st_name']    = 'u32'
_Elf32_Sym_d['st_value']   = 'u32'
_Elf32_Sym_d['st_size']    = 'u32'
_Elf32_Sym_d['st_info']    = 'r8'
_Elf32_Sym_d['st_other']   = 'r8'
_Elf32_Sym_d['st_shndx']   = 'u16'
Elf32_Sym = Unpacker( 'Elf32_Sym', _Elf32_Sym_d )

'''
RELA
'''
_Elf32_Rela_d = OrderedDict()
_Elf32_Rela_d['r_offset']  = 'u32'
_Elf32_Rela_d['r_info']    = 'u32'
_Elf32_Rela_d['r_addend']  = 's32'
Elf32_Rela = Unpacker( 'Elf32_Rela', _Elf32_Rela_d )

Elf32_R_XTENSA_NONE       = 0
Elf32_R_XTENSA_32         = 1
Elf32_R_XTENSA_ASM_EXPAND = 11
Elf32_R_XTENSA_SLOT0_OP   = 20
