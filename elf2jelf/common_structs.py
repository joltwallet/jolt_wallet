import math
from collections import OrderedDict, namedtuple
import bitstruct as bs
from copy import deepcopy

import ipdb as pdb

'''
Defines all the structs to parse from an ELF32 file
'''

'''
Index into a string table
'''
def index_strtab(s, index):
    null_terminator = s.find(0, index)
    return s[index:null_terminator]

'''
Convenience class to unpack data into a namedtuple
'''
class Unpacker:
    def __init__(self, name:str, d: OrderedDict):
        self.d = d
        self.fstr = self._parse_format_str( d )
        self.compiled_fstr = bs.compile(self.fstr)
        self.name = name
        self.names = namedtuple( name, d.keys() )

    def _parse_format_str(self, d :OrderedDict):
        '''
        Concatenates all string values of an OrderedDict
        '''
        fstr = ''.join(d.values())
        fstr += '<' # Least Significant Byte First
        return fstr

    def pack(self, *datas):
        # Reverse text and raw bytes
        datas = list(datas)
        for i, (k, v) in enumerate(self.d.items()):
            if v[0] == 't' or v[0] == 'r':
                if v[0] == 'r':
                    b = bytearray(int(v[1:])//8)
                    b[:len(datas[i])] = datas[i]
                else:
                    len_diff = (int(v[1:])//8) - len(datas[i])
                    b = datas[i] + '\0'*len_diff
                datas[i] = b[::-1]
        return self.compiled_fstr.pack(*tuple(datas))

    def unpack(self, data):
        '''
        Returns a named tuple of unpacking provided data
        '''
        unpacked = self.compiled_fstr.unpack(data)
        tup = self.names(*unpacked)

        # Have to reverse for the t and r types since they should be observed
        # as independent streams of bytes
        d = tup._asdict()
        for k, v in self.d.items():
            if v[0] == 't' or v[0] == 'r':
                d[k] = d[k][::-1]
        return self.names(**d)

    def size_bits(self):
        return self.compiled_fstr.calcsize()

    def size_bytes(self):
        return int(math.ceil(self.size_bits() / 8))


