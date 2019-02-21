#!/user/bin/env python3

'''
Converts an ELF file to a JELF file for The ESP32 Jolt

ELF File Structure that esp-idf creates:
+--------------+
| ELF Header   |
+--------------+
| Section 1    |
+--------------+
| Section 2    |
+--------------+
|   . . .      |
+--------------+

The Section Header Table is a Section like any other
   * Theres a pointer to the Section Header Table Section in the ELF Header.

Assumes symtab is at the end (ignoring strtab and shstrtab)

This is so that all st_shndx can be kept the same

Todo:
    * Generate Signature
'''

__author__  = 'Brian Pugh'
__email__   = 'bnp117@gmail.com'
__version__ = '0.0.1'
__status__  = 'development'

import argparse
import os, sys
import logging
from collections import OrderedDict, namedtuple
import bitstruct as bs
from common_structs import index_strtab
import math
import binascii
from binascii import hexlify, unhexlify
import zlib
import nacl.encoding
from nacl.signing import SigningKey
from nacl.bindings import \
        crypto_sign_ed25519ph_state, \
        crypto_sign_ed25519ph_update, \
        crypto_sign_ed25519ph_final_create

from pprint import pprint
import ipdb as pdb

this_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, this_path)

from jelf_loader import jelf_loader_hash

logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)

from elf32_structs import \
        Elf32_Ehdr, Elf32_Shdr, Elf32_Sym, Elf32_Rela, \
        Elf32_SHT_RELA, Elf32_SHT_NOBITS, \
        Elf32_SHF_ALLOC, Elf32_SHF_EXECINSTR, \
        Elf32_R_XTENSA_NONE, Elf32_R_XTENSA_32, \
        Elf32_R_XTENSA_ASM_EXPAND, Elf32_R_XTENSA_SLOT0_OP
from jelf_structs import \
        Jelf_Ehdr, Jelf_Shdr, Jelf_Sym, Jelf_Rela, \
        Jelf_SHT_OTHER, Jelf_SHT_RELA, Jelf_SHT_NOBITS, Jelf_SHT_SYMTAB, \
        Jelf_SHF_ALLOC, Jelf_SHF_EXECINSTR, \
        Jelf_R_XTENSA_NONE, Jelf_R_XTENSA_32, \
        Jelf_R_XTENSA_ASM_EXPAND, Jelf_R_XTENSA_SLOT0_OP

HARDEN = 0x80000000
log = logging.getLogger('elf2jelf')

def compress_data(data):
    w_bits = 12
    level = zlib.Z_BEST_COMPRESSION
    log.info("Compressing at level %d with window (dict) size %d", level, 2**w_bits)
    compressor = zlib.compressobj(level=level, method=zlib.DEFLATED,
            wbits=w_bits, memLevel=zlib.DEF_MEM_LEVEL, strategy=zlib.Z_DEFAULT_STRATEGY)
    compressed_data = compressor.compress(data)
    compressed_data += compressor.flush()
    compress_percentage = 100*(1-(len(compressed_data)/len(data)))
    log.info("Compressed data to %d bytes (%.2f%% smaller)" % \
            (len(compressed_data), compress_percentage) )
    return compressed_data

def align(x, base=4):
    return int( base * math.ceil(float(x)/base))

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_elf', nargs='?', type=str,
            help='Input ELF32 file to convert')
    parser.add_argument('--output', '-o', type=str, default=None,
            help='''
                Output Filename. Defaults to same as input name with a JELF
                extension''')
    parser.add_argument('--coin', '-c', type=str, default=None,
            help='''
            Coin Derivation (2 integers); for example "44'/165'. Note: you must wrap the argument in double quotes to be properly parsed."
                 ''')
    parser.add_argument('--bip32key', type=str, default='bitcoin_seed',
            help='''
                BIP32 Derivation Seed String Key
                 ''')
    parser.add_argument('--verbose', '-v', type=str, default='INFO',
            help='''
            Valid options:
            SILENT
            INFO
            DEBUG
            ''')
    parser.add_argument('--signing_key', type=str,
            default='000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F',
            help="256-bit private key in hexidecimal (len=64).")
    parser.add_argument('--export_only', action='store_true',
            help='''Only compile the new jolt_lib.c exports, then exit''')

    args = parser.parse_args()
    dargs = vars(args)
    return (args, dargs)

def read_export_list():
    """
    Reads in the txt file containing all functions to export from JoltOS
    """
    with open(os.path.join(this_path, 'export_list.txt'), 'r') as f:
        version_header = f.readline().rstrip()
        version_name, version_str = version_header.split(' ')
        assert(version_name == 'VERSION')
        major, minor = version_str.split('.')
        major = int(major)
        minor = int(minor)
        export_list = [line.rstrip() for line in f]
    return export_list, major, minor

def write_export_file(export_list, major, minor):
    """
    Writes the export struct used in jolt_lib.c
    """
    with open(os.path.join(this_path, 'jolt_lib_template.c')) as f:
        template = f.read()

    export_string = ''
    for f_name in export_list:
        export_string += '''    EXPORT_SYMBOL( %s ),\n''' % f_name

    jolt_lib = template % (export_string, len(export_list))

    # Write it to where the hardware firmware expects it
    with open(os.path.join(this_path, '..', 'jolt_os', 'jolt_lib.c'), 'w') as f:
        f.write(jolt_lib)

def get_ehdr(elf_contents):
    assert( Elf32_Ehdr.size_bytes() == 52 )
    ehdr = Elf32_Ehdr.unpack(elf_contents[0:])
    assert(ehdr.e_ident == \
            '\x7fELF\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00')
    assert(ehdr.e_machine == 94)
    return ehdr

def get_shstrtab(elf_contents, ehdr):
    """
    Reads and returns the SectionHeaderStringTable
    """
    assert( Elf32_Shdr.size_bytes() == 40 )
    offset = ehdr.e_shoff + ehdr.e_shstrndx * Elf32_Shdr.size_bytes()
    shstrtab_shdr = Elf32_Shdr.unpack(elf_contents[offset:])
    # Read the actual SectionHeaderTable
    shstrtab = elf_contents[shstrtab_shdr.sh_offset:
            shstrtab_shdr.sh_offset+shstrtab_shdr.sh_size]
    shstrtab_name = index_strtab(shstrtab, shstrtab_shdr.sh_name)
    assert( shstrtab_name == b'.shstrtab' )
    return shstrtab

def read_section_headers(elf_contents, ehdr, shstrtab):
    """
    Read all SectionHeaders, their names, and read in symtab, strtab
    """
    elf32_shdrs = []
    elf32_shdr_names = []
    elf32_symtab = None
    elf32_strtab = None
    # Iterate through the SectionHeader elements of the Table
    for i in range(ehdr.e_shnum):
        offset = ehdr.e_shoff + i * Elf32_Shdr.size_bytes()
        elf32_shdr = Elf32_Shdr.unpack(elf_contents[offset:])

        shdr_name = index_strtab(shstrtab, elf32_shdr.sh_name)
        log.debug("Read in Section Header %d. %s " % (i, shdr_name))

        if( shdr_name == b'.symtab' ):
            elf32_symtab = elf_contents[ elf32_shdr.sh_offset:
                    elf32_shdr.sh_offset+elf32_shdr.sh_size ]
        elif( shdr_name == b'.strtab' ):
            elf32_strtab = elf_contents[ elf32_shdr.sh_offset:
                    elf32_shdr.sh_offset+elf32_shdr.sh_size ]
        elf32_shdrs.append( elf32_shdr )
        elf32_shdr_names.append(shdr_name)
    return elf32_shdrs, elf32_shdr_names, elf32_symtab, elf32_strtab

def convert_shdrs( elf32_shdrs, elf32_shdr_names ):
    """
    Converts ALL ELF32 Section Headers to JELF Headers

    Also returns a list mapping
    mapping[0] returns the original index of the returned 0th section
    """
    jelf_shdrs_alloc = []
    jelf_shdrs_alloc_index = []
    jelf_shdrs_alloc_names = []

    jelf_shdrs_rela = []
    jelf_shdrs_rela_index = []
    jelf_shdrs_rela_names = []

    # odds and ends shdrs
    jelf_shdrs = []
    jelf_shdrs_index = []
    jelf_shdrs_names = []

    for i, (elf32_shdr, name) in enumerate(zip(elf32_shdrs, elf32_shdr_names)):
        jelf_shdr_d = OrderedDict()

        # Convert the "sh_type" field
        if elf32_shdr.sh_type == Elf32_SHT_RELA:
            jelf_shdr_d['sh_type'] = Jelf_SHT_RELA
        elif elf32_shdr.sh_type == Elf32_SHT_NOBITS:
            jelf_shdr_d['sh_type'] = Jelf_SHT_NOBITS
        else:
            jelf_shdr_d['sh_type'] = Jelf_SHT_OTHER

        # Convert the "sh_flag" field
        jelf_shdr_d['sh_flags'] = 0
        if elf32_shdr.sh_flags & Elf32_SHF_ALLOC:
            jelf_shdr_d['sh_flags'] |= Jelf_SHF_ALLOC
        if elf32_shdr.sh_flags & Elf32_SHF_EXECINSTR:
            jelf_shdr_d['sh_flags'] |= Jelf_SHF_EXECINSTR

        if elf32_shdr.sh_size > 2**16:
            raise("Overflow Detected")
        # for symtab and relas, this will be updated later;
        # All other sections maintain the same size
        jelf_shdr_d['sh_size'] = elf32_shdr.sh_size

        if elf32_shdr.sh_info > 2**12:
            raise("Overflow Detected")
        # index for the header in which the rela information applies to
        jelf_shdr_d['sh_info'] = elf32_shdr.sh_info

        # Separate the sections into the 3 groups (alloc, rela, other)
        log.debug(jelf_shdr_d)
        if jelf_shdr_d['sh_flags'] != 0:
            jelf_shdrs_alloc.append(jelf_shdr_d)
            jelf_shdrs_alloc_index.append(i)
            jelf_shdrs_alloc_names.append(elf32_shdr_names[i].decode())
        elif jelf_shdr_d['sh_type'] == Jelf_SHT_RELA:
            jelf_shdrs_rela.append(jelf_shdr_d)
            jelf_shdrs_rela_index.append(i)
            jelf_shdrs_rela_names.append(elf32_shdr_names[i].decode())
        else:
            jelf_shdrs.append(jelf_shdr_d)
            jelf_shdrs_index.append(i)
            jelf_shdrs_names.append(elf32_shdr_names[i].decode())
        del(jelf_shdr_d)

    # We need to put all literal sections at the beginning
    # order should be:
    # .literal.NAME1
    # .literal.NAME2
    # ... (other .literal)
    # .text.NAME1
    # .text.NAME2
    # ... (other .text)
    # .<OTHER_ALLOC>
    # ...
    # .rela.literal.NAME1
    # .rela.literal.NAME2
    # ... (other .rela.literal)
    # .rela.text.NAME1
    # .rela.text.NAME2
    # ... (other .rela.text)
    # .rela.<OTHER_ALLOC>

    literal_alloc = []
    literal_alloc_index = []
    literal_alloc_names = [] # for debugging
    literal_rela = []
    literal_rela_index = []
    literal_rela_names = [] # for debugging

    text_alloc = []
    text_alloc_index = []
    text_alloc_names = [] # for debugging
    text_rela = []
    text_rela_index = []
    text_rela_names = [] # for debugging

    other_alloc = [] # stuff like .rodata and .bss
    other_alloc_index = []
    other_alloc_names = [] # for debugging
    other_rela = []
    other_rela_index = []
    other_rela_names = [] # for debugging

    for entry, index, name in zip(jelf_shdrs_alloc, jelf_shdrs_alloc_index, jelf_shdrs_alloc_names):
        # have to find the pairing rela section
        if isinstance(name, bytes):
            name = name.decode()

        try:
            rela_index = jelf_shdrs_rela_names.index( ".rela" + name)
        except:
            # doesn't have a paired rela section
            other_alloc.append(entry)
            other_alloc_index.append(index)
            other_alloc_names.append(name)
            continue

        if name.startswith(".literal"):
            literal_alloc.append(entry)
            literal_alloc_index.append(index)
            literal_alloc_names.append(name)
            literal_rela.append(jelf_shdrs_rela[rela_index])
            literal_rela_index.append(jelf_shdrs_rela_index[rela_index])
            literal_rela_names.append('.rela'+name)
        elif name.startswith('.text'):
            text_alloc.append(entry)
            text_alloc_index.append(index)
            text_alloc_names.append(name)
            text_rela.append(jelf_shdrs_rela[rela_index])
            text_rela_index.append(jelf_shdrs_rela_index[rela_index])
            text_rela_names.append('.rela'+name)
        else:
            other_alloc.append(entry)
            other_alloc_index.append(index)
            other_alloc_names.append(name)
            other_rela.append(jelf_shdrs_rela[rela_index])
            other_rela_index.append(jelf_shdrs_rela_index[rela_index])
            other_rela_names.append('.rela'+name)

    # First is the null section, then the symtab
    null_index   = jelf_shdrs_names.index('')
    symtab_index = jelf_shdrs_names.index('.symtab')

    mapping = [jelf_shdrs_index[null_index]] \
            + [jelf_shdrs_index[symtab_index]] \
            + list(reversed(other_alloc_index + text_alloc_index+ literal_alloc_index)) \
            + other_rela_index \
            + text_rela_index \
            + literal_rela_index

    jelf_shdrs = [jelf_shdrs[null_index]] \
            + [jelf_shdrs[symtab_index]] \
            + list(reversed(other_alloc + text_alloc + literal_alloc)) \
            + other_rela \
            + text_rela \
            + literal_rela

    names = [jelf_shdrs_names[null_index]] \
            + [jelf_shdrs_names[symtab_index]] \
            + list(reversed(other_alloc_names + text_alloc_names + literal_alloc_names)) \
            + other_rela_names \
            + text_rela_names \
            + literal_rela_names

    for i in range(len(jelf_shdrs)):
        jelf_shdr_d = jelf_shdrs[i]
        jelf_shdrs[i]['sh_info'] = mapping.index(
                jelf_shdrs[i]['sh_info'])

    return jelf_shdrs, mapping

def convert_symtab(elf32_symtab, elf32_strtab, export_list, mapping):
    elf32_sym_size = Elf32_Sym.size_bytes()
    jelf_sym_size  = Jelf_Sym.size_bytes()

    symtab_nent = int( len(elf32_symtab)/elf32_sym_size )
    jelf_symtab = bytearray( symtab_nent * jelf_sym_size )

    jelf_symtab_header = [] # holds non-zero st_values

    fail = False;
    for i in range(symtab_nent):
        begin = i * elf32_sym_size
        end = begin + elf32_sym_size
        elf32_symbol = Elf32_Sym.unpack( elf32_symtab[begin:end] )
        del(begin, end)

        # Lookup Symbol name in exported function list
        sym_name = index_strtab(elf32_strtab, elf32_symbol.st_name).decode('ascii')

        # Convert the SymbolName to a 1-indexed Exported Function
        jelf_name_index = 0 # 0 means no name
        if sym_name == '':
            log.debug( "Symbol index %d has no name %d." % \
                    (i, elf32_symbol.st_name) )
        else:
            try:
                # Plus one because 0 means no name
                jelf_name_index = export_list.index(sym_name) + 1
                log.debug( ("Symbol index %d has name %s "
                    "and matched to exported function index %d.") % \
                        (i, sym_name, jelf_name_index) )
            except ValueError:
                if elf32_symbol.st_shndx == 0:
                    log.error( "Unlinked symbol: %s" % sym_name )
                    fail = True
                else:
                    # The function is internal to the app
                    # st_shndx is the thing that matters in this case
                    pass

        if elf32_symbol.st_shndx > 2**16:
            raise("Overflow Detected")
        if elf32_symbol.st_shndx > 0xFF00 or elf32_symbol.st_shndx == 0:
            # Special values
            # e.g. 0xFFF1 means SHN_ABS
            # we don't care about special values
            new_st_shndx = 0
        else:
            new_st_shndx = mapping.index(elf32_symbol.st_shndx)
        begin = i * jelf_sym_size
        end = begin + jelf_sym_size

        jelf_st_value = elf32_symbol.st_value
        if jelf_st_value > 0:
            jelf_symtab_header.append(jelf_st_value)
            jelf_st_value = len(jelf_symtab_header)
        jelf_symtab[begin:end] = Jelf_Sym.pack(
                jelf_name_index,
                new_st_shndx,
                jelf_st_value,
                )
        del(begin, end)
        if sym_name == "app_main":
            jelf_entrypoint_sym_idx = i

    if fail:
        exit(1);

    # Prepend jelf_symtab with the auxilary st_value table
    assert(len(jelf_symtab_header) <= 127)
    preamble = [len(jelf_symtab_header).to_bytes(length=1, byteorder='little')] \
            + [x.to_bytes(length=4, byteorder='little') for x in jelf_symtab_header]
    jelf_symtab = b''.join(preamble) + jelf_symtab

    return jelf_symtab, jelf_entrypoint_sym_idx

def convert_relas(elf_contents, elf32_shdrs, jelf_shdrs, mapping):
    """
    Returns dict jelf_relas where:
        keys: index into jelf_shdrs.
        values: bytearray of the complete converted rela section.
    Populates jelf_shdrs[i]['sh_size'] to reflect the change in size of
    each rela section.
    returns: jelf_relas, jelf_shdrs
    """
    # Sanity Check

    jelf_relas = {}
    for i in range(len(jelf_shdrs)):
        # only iterate over the RELA sections
        if jelf_shdrs[i]['sh_type'] != Jelf_SHT_RELA:
            continue

        # Get number of relocations in this section
        elf32_idx = mapping[i]
        n_relas = int(elf32_shdrs[elf32_idx].sh_size / Elf32_Rela.size_bytes())
        # 'sh_size' is currently as if we were using ELF32_SYM
        jelf_shdrs[i]['sh_size'] = n_relas * Jelf_Rela.size_bytes()
        jelf_sec_relas = bytearray( jelf_shdrs[i]['sh_size'] )

        for j in range(n_relas):
            # pointer into the binaries
            elf32_offset = elf32_shdrs[elf32_idx].sh_offset \
                    + j * Elf32_Rela.size_bytes()
            # offset into the newly allocated jelf section
            jelf_offset  = j * Jelf_Rela.size_bytes()

            rela = Elf32_Rela.unpack(elf_contents[elf32_offset:])

            elf32_r_type = rela.r_info & 0xFF

            # Convert r_info; 2 bit left shift for jelf_r_type
            jelf_r_info = ((rela.r_info & ~0xFF) >> 8) << 2
            if rela.r_offset > 2**16:
                raise("Overflow Detected")
            if jelf_r_info > 2**16:
                raise("Overflow Detected")
            if rela.r_addend > 2**15 or rela.r_addend < -2**15:
                raise("Overflow Detected")

            # Convert the type and store in bottom 2 bits of r_info
            if elf32_r_type == Elf32_R_XTENSA_NONE:
                jelf_r_info |= Jelf_R_XTENSA_NONE
            elif elf32_r_type == Elf32_R_XTENSA_32:
                jelf_r_info |= Jelf_R_XTENSA_32
            elif elf32_r_type == Elf32_R_XTENSA_ASM_EXPAND:
                jelf_r_info |= Jelf_R_XTENSA_ASM_EXPAND
            elif elf32_r_type == Elf32_R_XTENSA_SLOT0_OP:
                jelf_r_info |= Jelf_R_XTENSA_SLOT0_OP
            else:
                log.error("Failed on %d %s" % (i, elf32_shdr_names[elf32_idx]))
                raise("Unexpected RELA Type")

            # Pack the data into the rela section's bytearray
            jelf_sec_relas[jelf_offset:jelf_offset+Jelf_Rela.size_bytes()] = \
                    Jelf_Rela.pack(rela.r_offset, jelf_r_info, rela.r_addend)
            log.debug("Jelf RELA: r_offset: %d, r_info: %d, r_addend: %d" % (rela.r_offset, jelf_r_info, rela.r_addend));
        jelf_relas[i] = jelf_sec_relas
    return jelf_relas, jelf_shdrs

def write_jelf_sections(elf_contents,
        elf32_shdrs, elf32_shdr_names,
        jelf_contents, jelf_ptr,
        jelf_shdrs, jelf_relas, jelf_symtab, mapping):
    """
    Writes all sections to binary.
    Updates the SectionHeaders with the correct size, type, and offset
    """
    # Sanity Check
    assert( len(elf32_shdrs) == len(elf32_shdr_names) )

    for i in range(len(jelf_shdrs)):
        elf32_idx = mapping[i]
        name = elf32_shdr_names[elf32_idx]
        if name == b'.symtab':
            # Copy over our updated Jelf symtab
            jelf_shdrs[i]['sh_size'] = len(jelf_symtab)
            jelf_shdrs[i]['sh_type'] = Jelf_SHT_SYMTAB # custom
            new_jelf_ptr = jelf_ptr + jelf_shdrs[i]['sh_size']
            jelf_contents[jelf_ptr:new_jelf_ptr] = jelf_symtab
        elif name == b'.strtab' or name == b'.shstrtab':
            # Dont copy over since we're stripping it
            # We'll filter this out later
            jelf_shdrs[i] = None
            continue
        elif jelf_shdrs[i]['sh_type'] == Jelf_SHT_NOBITS:
            new_jelf_ptr = jelf_ptr
        elif jelf_shdrs[i]['sh_type'] == Jelf_SHT_RELA:
            new_jelf_ptr = jelf_ptr + jelf_shdrs[i]['sh_size']
            jelf_contents[jelf_ptr:new_jelf_ptr] = jelf_relas[i]
        else:
            # Copy over the unaltered section
            new_jelf_ptr = jelf_ptr + jelf_shdrs[i]['sh_size']
            assert(jelf_shdrs[i]['sh_size']==elf32_shdrs[elf32_idx].sh_size)
            jelf_contents[jelf_ptr:new_jelf_ptr] = \
                    elf_contents[
                            elf32_shdrs[elf32_idx].sh_offset :
                            elf32_shdrs[elf32_idx].sh_offset+jelf_shdrs[i]['sh_size']
                            ]
        jelf_ptr = new_jelf_ptr
    return jelf_contents, jelf_ptr, jelf_shdrs

def write_jelf_sectionheadertable(jelf_contents,
        jelf_shdrs, jelf_ptr):
    """
    Writes the SectionHeaderTable to jelf_contents at jelf_ptr
    """
    log.debug("SectionHeaderTable Offset: 0x%08X" % jelf_ptr)
    section_count = 0
    for i, jelf_shdr in enumerate(jelf_shdrs):
        if jelf_shdrs[i] is None:
            continue
        section_count += 1

        new_jelf_ptr = jelf_ptr + Jelf_Shdr.size_bytes()
        shdr_bytes = Jelf_Shdr.pack( *(jelf_shdr.values()) )
        log.debug(jelf_shdr)
        jelf_contents[jelf_ptr:new_jelf_ptr] = shdr_bytes
        jelf_ptr = new_jelf_ptr
    return jelf_contents, section_count

def main():
    args, dargs = parse_args()

    global log
    logging_level = args.verbose.upper()
    if logging_level == 'INFO':
        log.setLevel(logging.INFO)
    elif logging_level == 'DEBUG':
        log.setLevel(logging.DEBUG)
    else:
        raise("Invalid Logging Verbosity")

    ##################################
    # Read in the JoltOS Export List #
    ##################################
    export_list, _JELF_VERSION_MAJOR, _JELF_VERSION_MINOR = read_export_list()

    ###################################
    # Generate jolt_lib.c export list #
    ###################################
    write_export_file(export_list, _JELF_VERSION_MAJOR, _JELF_VERSION_MINOR)

    if args.export_only:
        return

    ####################
    # Read In ELF File #
    ####################
    log.info("Reading in %s" % args.input_elf)
    with open(args.input_elf, 'rb') as f:
        elf_contents = f.read()
    log.info("Read in %d bytes" % len(elf_contents))

    #####################
    # Unpack ELF Header #
    #####################
    ehdr = get_ehdr(elf_contents)

    ##########################################
    # Read SectionHeaderTable Section Header #
    ##########################################
    shstrtab = get_shstrtab(elf_contents, ehdr)

    ###########################
    # Process Section Headers #
    ###########################
    elf32_shdrs, elf32_shdr_names, elf32_symtab, elf32_strtab = \
            read_section_headers( elf_contents, ehdr, shstrtab )

    jelf_shdrs, mapping = convert_shdrs( elf32_shdrs, elf32_shdr_names )

    ###########################################
    # Convert the ELF32 symtab to JELF Format #
    ###########################################
    jelf_symtab, jelf_entrypoint_sym_idx = convert_symtab(elf32_symtab,
            elf32_strtab, export_list, mapping)

    #########################################
    # Convert the ELF32 RELA to JELF Format #
    #########################################
    jelf_relas, jelf_shdrs = convert_relas(elf_contents,
            elf32_shdrs, jelf_shdrs, mapping)

    ######################################
    # Allocate space for the JELF Binary #
    ######################################
    # over allocate for now
    jelf_contents = bytearray(len(elf_contents))
    # Skip the JELF Header and SectionHeaderTable
    jelf_ptr = Jelf_Ehdr.size_bytes() + len(jelf_shdrs)*Jelf_Shdr.size_bytes()

    #######################
    # Write JELF Sections #
    #######################
    jelf_contents, jelf_ptr, jelf_shdrs = write_jelf_sections(elf_contents,
            elf32_shdrs, elf32_shdr_names,
            jelf_contents, jelf_ptr,
            jelf_shdrs, jelf_relas, jelf_symtab, mapping)
    # trim jelf_contents to final length
    jelf_contents = jelf_contents[:jelf_ptr]

    ##################################################
    # Write Section Header Table to end of JELF File #
    ##################################################
    jelf_contents, jelf_ehdr_shnum = write_jelf_sectionheadertable(jelf_contents,
            jelf_shdrs, Jelf_Ehdr.size_bytes())
    log.info("Jelf Final Size: %d" % len(jelf_contents))

    ###########################
    # Parse Coin CLI Argument #
    ###########################
    if args.coin is None:
        raise("must specify coin derivation path")
    purpose_str, coin_str = args.coin.split('/')
    # Check for harden specifier
    if purpose_str[-1] == "'":
        purpose = int(purpose_str[:-1])
        purpose |= HARDEN
    else:
        purpose = int(purpose_str)
    log.info("Coin Purpose: 0x%08X" % purpose)
    if purpose_str[-1] == "'":
        coin = int(coin_str[:-1])
        coin |= HARDEN
    else:
        coin = int(coin_str)
    log.info("Coin Path: 0x%08X" % coin)

    if len(args.bip32key) >= 32:
        raise("BIP32Key too long!")

    #####################
    # Write JELF Header #
    #####################
    signing_key = nacl.signing.SigningKey(unhexlify(args.signing_key))
    pk = signing_key.verify_key.encode(encoder=nacl.encoding.RawEncoder)
    sk = signing_key.encode(encoder=nacl.encoding.RawEncoder)
    assert(len(pk)==32)
    assert(len(sk)==32)

    jelf_ehdr_d = OrderedDict()
    jelf_ehdr_d['e_ident']          = '\x7fJELF\x00'
    jelf_ehdr_d['e_public_key']     = pk
    jelf_ehdr_d['e_version_major']  = _JELF_VERSION_MAJOR
    jelf_ehdr_d['e_version_minor']  = _JELF_VERSION_MINOR
    jelf_ehdr_d['e_entry_index']    = jelf_entrypoint_sym_idx
    jelf_ehdr_d['e_shnum']          = len(jelf_shdrs)
    jelf_ehdr_d['e_coin_purpose']   = purpose
    jelf_ehdr_d['e_coin_path']      = coin
    jelf_ehdr_d['e_bip32key']       = args.bip32key

    jelf_contents[:Jelf_Ehdr.size_bytes()] = Jelf_Ehdr.pack(
            *jelf_ehdr_d.values() )

    # Parse Output Filename
    if args.output is None:
        path_bn, ext = os.path.splitext(args.input_elf)
        output_fn = path_bn + '.jelf'
    else:
        output_fn = args.output
    assert(output_fn[-5:]=='.jelf')

    #############################
    # Write JELF binary to file #
    #############################
    # write the compressed unsigned jelf file
    compressed_jelf = compress_data(jelf_contents);
    with open(output_fn, 'wb') as f:
        f.write(bytes(64) + compressed_jelf)

    # Get the transversal hash
    name_to_sign = os.path.basename(output_fn[:-5]).encode('utf-8')
    t_hash = jelf_loader_hash(output_fn.encode(), name_to_sign )

    log.debug("Secret Key: %s", hexlify(sk).decode('utf-8'))
    log.info("Public Key: %s", hexlify(pk).decode('utf-8'))
    log.info("Signed application name: %s" % name_to_sign)
    log.info("t_hash: %s" % t_hash.hex())
    signature = signing_key.sign(t_hash).signature
    assert(len(signature) == 64)
    log.info("C Signature: %s", hexlify(signature).decode('utf-8'))

    ########################################
    # Write Signed Compressed JELF binary to file #
    ########################################
    with open(output_fn, 'wb') as f:
        f.write(signature + compressed_jelf)

    log.info("Complete!")

if __name__=='__main__':
    main()
