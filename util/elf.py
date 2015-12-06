from __future__ import print_function

from struct import Struct

def loadobj(flist):
    """Load object code.
    Takes a list of file names and/or file-like objects
    and return an iterator yielding ELF instances
    """
    from .ar import AR
    for inp in flist:
        if not hasattr(inp, 'read'):
            inp = open(inp,'rb')
        buf = inp.read(8)
        inp.seek(0)
        if buf[0:4]==b'\x7fELF':
            elf = ELF(inp)
            if elf.typecode!=1:
                raise RuntimeError('ELF file is not relocatable')
            yield elf
        elif buf[0:8]==b'!<arch>\n':
            with AR(inp) as arch:
                for mem in arch.getmembers():
                    if mem.name=='/':
                        continue # skip symtab
                    elf = ELF(mem.open())
                    if elf.typecode!=1:
                        raise RuntimeError('ELF file is not relocatable')
                    yield elf
        else:
            raise RuntimeError("Input is not a static library or ELF file")

class ELF(object):
    def __init__(self, inp):
        if not hasattr(inp, 'read'):
            inp = open(inp,'rb')
        self._fp = inp
        self.name = inp.name
        self._readheader()
        self._readprog()
        self._readsect()

    def close(self):
        self._fp.close()
        self._fp = self.name = None

    @property
    def type(self):
        return self._ftypes.get(self._ftype, str(self._ftype))

    @property
    def typecode(self):
        return self._ftype

    def __enter__(self):
        return self
    def __exit__(self,A,B,C):
        self.close()

    def _unpack(self, fmt, buf):
        """Unpack with file endianness and replace 'T' with the appropriate
          word size specifier.
        """
        fmt = self._end+fmt.replace('T',self._fmt)
        S = Struct(fmt)
        try:
            return S.unpack(buf[:S.size])
        except:
            print("Unpack ", len(buf), ':', repr(buf))
            raise

    def _read(self, off, size):
        self._fp.seek(off)
        if self._fp.tell()!=off:
            raise RuntimeError("File truncated before 0x%x"%off)
        buf = self._fp.read(size)
        if len(buf)!=size:
            raise RuntimeError("File truncated before 0x%x"%(off+size))
        return buf

    _ftypes = {0:'None', 1:'Rel', 2:'Exec', 3:'DSO', 4:'Core'}

    def _readheader(self):
        'read file header'
        buf = self._fp.read(20) # read fields which are the same for 32/64
        if len(buf)!=20 or buf[0:4]!=b'\x7fELF':
            raise RuntimeError("Not an ELF file: %s"%(buf[0:4],))

        ws, be, self.version, self.abi = Struct('!BBBB').unpack(buf[4:8])

        self._ws = self.wordsize = 32*ws
        if self._ws not in [32,64]:
            raise RuntimeError("Unknown word size %d"%ws)
        self._fmt = {32:'I', 64:'Q'}[self._ws]

        self._be = be-1
        if self._be not in [0,1]:
            raise RuntimeError("Unknown endian %d"%be)
        self._end = {1:'>', 0:'<'}[self._be]
        self._be = self.bigendian = bool(self._be)

        if self.version!=1:
            raise RuntimeError("Unsupported version %d"%self.version)

        self._ftype, self.iset = self._unpack('HH', buf[16:])

        N = 52 if self._ws==32 else 64
        buf = buf + self._fp.read(N-len(buf))
        if len(buf)!=N:
            raise RuntimeError("Truncated header")

        ver2, self.entry, self._ptbl, self._stbl, self._flags, self._hsize = self._unpack('ITTTIH', buf[20:])
        self._ptblsize, self._ptblcnt, self._stblsize, self._stblcnt, self._snameidx = self._unpack('HHHHH', buf[42:])

        if ver2!=self.version:
            raise RuntimeError("Version mis-match %u != %u (corruption?)"%(self.version, ver2))

    class Section(object):
        def __init__(self, elf, idx, **kws):
            self.__dict__.update(kws)
            self.index = idx
            self._elf = elf

    class ProgSection(Section):
        _types = {0:'Null',1:'Load',2:'Dyn',3:'Interp',4:'Note',5:'Shlib',6:'PHDR'}
        @property
        def type(self):
            return self._types.get(self._type, hex(self._type))

    class SectSection(Section):
        _types = {0:'Null',1:'PROGBITS',2:'Symbol Table',3:'String table',4:'RELA',
                  5:'Hash',6:'Dynamic',7:'Note',8:'NOBITS',9:'REL',10:'SHLIB',11:'DYNSYM'}
        @property
        def type(self):
            return self._types.get(self._type, hex(self._type))
        @property
        def name(self):
            return self._elf._string(self._nameidx)

    def _readprog(self):
        'read program header'

        #print('Program header at',hex(self._ptbl))
        #print(' Entry size', self._ptblsize)
        #print(' Entry count', self._ptblcnt)

        buf = self._read(self._ptbl, self._ptblsize*self._ptblcnt)

        L = []
        laddr = 0
        for idx in range(self._ptblcnt):
            ent = buf[(idx*self._ptblsize):((idx+1)*self._ptblsize)]
            assert len(ent)==self._ptblsize

            if self._ws==32:
                T, off, vaddr, fsize, msize, flags, align = self._unpack('IIIxxxxIIII', ent)
            else:
                T, flags, off, vaddr, fsize, msize, align = self._unpack('IIQQxxxxxxxxQQQ', ent)

            L.append(self.ProgSection(self, idx, _type=T, offset=off, lma=laddr, lsize=fsize,
                                      vma=vaddr, vsize=msize, flags=flags, align=align))

            laddr += fsize

        self.progs = L

    def _readsect(self):
        'Read section headers'

        #print('Sections header at',hex(self._ptbl))
        #print(' Entry size', self._ptblsize)
        #print(' Entry count', self._ptblcnt)

        buf = self._read(self._stbl, self._stblsize*self._stblcnt)

        L = []
        strs = []

        for idx in range(self._stblcnt):
            faddr = self._stbl + idx*self._stblsize
            ent = buf[(idx*self._stblsize):((idx+1)*self._stblsize)]
            assert len(ent)==self._stblsize

            if self._ws==32:
                nameidx, T, flags, vaddr, off, vsize, linkidx, info, align, esize = \
                  self._unpack('IIIIIIIIII', ent)
            else:
                raise NotImplementedError("Decoding of 64-bit section table")

            S = self.SectSection(self, idx, _type=T, offset=off, flags=flags, vsize=vsize,
                                 vma=vaddr, info=info, align=align, entsize=esize, _nameidx=nameidx)
            L.append(S)

            if T==3:
                # String table
                strs.append(self._read(S.offset, S.vsize))

        self.strings = b''.join(strs)
        self.sections = L            

    def _string(self, idx):
        if idx==0:
            return '<unnamed>'
        elif idx>=len(self.strings):
            return '<outofrange>'
        eidx = self.strings.find(b'\0', idx)
        if eidx==-1:
            eidx=None
        return self.strings[idx:eidx].decode('ascii')

def main():
    import sys
    class C2D(object):
        def __init__(self, o):
            self.o = o
        def __getitem__(self, k):
            return getattr(self.o, k)
    with ELF(sys.argv[1]) as F:
        print("""version: %(version)s
BE: %(bigendian)s
ABI: %(abi)s
File type: %(type)s
Instruction set: %(iset)s
Start address: 0x%(entry)x
"""%C2D(F))

        print("Program Headers")
        for S in F.progs:
            print(""" Type: %(type)s
 Address: 0x%(lma)x -> 0x%(vma)x
 Size: %(lsize)s -> %(vsize)s
 Alignment: %(align)s
 Flags: 0x%(flags)x
"""%C2D(S))

        print("Sections")
        for S in F.sections:
            print("%(name)s\t%(type)s\t@0x%(vma)x\tSize: %(vsize)s\tAlign: %(align)s\tFlags: 0x%(flags)x"%C2D(S))
    
if __name__=='__main__':
    main()
