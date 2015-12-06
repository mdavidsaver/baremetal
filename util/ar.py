from __future__ import print_function

from collections import OrderedDict
from tempfile import TemporaryFile
from struct import Struct

_entry = Struct('!16s12s6s6s8s10s2s')

def _decode(s):
    return int(s.rstrip(b' '))

class AR(object):
    def __init__(self, inp):
        if not hasattr(inp, 'read'):
            inp = open(inp,'rb')
        self._fp = inp
        self.name = inp.name
        self._readhead()

    def close(self):
        self._fp.close()
        self._fp = self.name = None

    def __enter__(self):
        return self
    def __exit__(self,A,B,C):
        self.close()

    def _read(self, size, zero=False):
        buf = self._fp.read(size)
        if len(buf)==size or (len(buf)==0 and zero):
            return buf
        raise RuntimeError("File truncated before 0x%x"%(off+size))

    class ArInfo(object):
        def __init__(self, ar, **kws):
            self.__dict__.update(kws)
            self._ar = ar
        def tobuf(self):
            self._ar._fp.seek(self.offset)
            return self._ar._fp.read(self.size)
        def open(self):
            self._ar._fp.seek(self.offset)
            F = TemporaryFile()
            F.write(self._ar._fp.read(self.size))
            F.seek(0)
            return F

    def _readhead(self):
        buf = self._read(8)
        if buf!=b'!<arch>\n':
            raise RuntimeError("Not an AR archive")

        needstring = False
        strtab = None

        files=[]
        while True:
            buf = self._read(_entry.size, zero=True)
            if len(buf)==0:
                break

            fname, fdate, uid, fgid, fmode, fsize, M = _entry.unpack(buf)
            if M!=b'\x60\x0a':
                print(repr(buf))
                raise RuntimeError('Corrupt header ending %d'%self._fp.tell())
            fname = fname.rstrip(b' ')
            fsize = _decode(fsize)
            if fsize<0:
                raise RuntimeError('Corrupt file size %d'%fsize)
            print("XX",fname,fsize)

            if fname==b'/' or fname==b'__.SYMDEF':
                fname = b'/'
                pass

            elif fname==b'//':
                strtab = self._read(fsize)
                continue

            elif fname.startswith(b'#1/'): # BSD long file name
                namelen = int(fname[3:])
                if namelen<=0 or namelen>fsize:
                    raise RuntimeError('Corrupt long file name at %d'%self._fp.tell())
                fname = self._read(namelen).rstrip(' ')
                fsize -= namelen

            elif fname[:1]==b'/': # SVR4/GNU long file name
                fname = int(fname[1:])
                needstring = True

            elif fname[-1:]==b'/': # SVR4/GNU regular name
                fname = fname[:-1]

            I = self.ArInfo(self, name=fname, size=fsize, offset=self._fp.tell())
            files.append(I)

            self._fp.seek(fsize, 1)

        if needstring and strtab is None:
            raise RuntimeError('AR missing string table')
        elif needstring:
            for F in files:
                if not isinstance(F.name, int):
                    continue
                eidx = strtab.find('/\n', F.name)
                if eidx==-1:
                    eidx = None
                    F.name = strtab[F.name:eidx]

        lkup = []
        for F in files:
            F.name = F.name.decode('ascii')
            lkup.append((F.name, F))

        self._files = OrderedDict(lkup)


    def getmembers(self):
        return iter(self._files.values())

    def getmember(self, name):
        return self._files[name]

def main():
    import sys
    cmd = sys.argv[2]
    with AR(sys.argv[1]) as ar:
        if cmd=='list':
            for F in ar.getmembers():
                print('%d\t%s'%(F.size, F.name))
        elif cmd=='cat':
            F = ar.getmember(sys.argv[3])
            if hasattr(sys.stdout, 'buffer'):
                sys.stdout.buffer.write(F.tobuf())
            else:
                sys.stdout.write(F.tobuf())
        else:
            print("Unknown command")
            sys.exit(1)

if __name__=='__main__':
    main()
