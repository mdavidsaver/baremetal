
import logging, re
from cStringIO import StringIO
from collections import namedtuple, OrderedDict
import math
from .elf import loadobj

_log = logging.getLogger(__name__)

#_proc = namedtuple('Proc', ['name','files','size','align'])
class _proc(object):
    _keys = ['name','files','size','align']
    name = files = size = align = None
    def __init__(self, *args, **kws):
        for K,V in zip(self._keys,args)+kws.items():
            setattr(self, K, V)

def lg(v):
    return math.ceil(math.log(v,2))

def round2(v):
    'Round up to power of two'
    return 2**lg(v)

class LinkScript(object):
    """Generate a linker script to layout a set of
    "processes" in a manner suitable for use with an ARMv7-R/M
    style MPU (memory protection w/ re-mapping).
    
    This script will analyze the object code for each "process"
    to ensure that the layout of .bss and .data is compatible
    with the alignment restrictions of the MPU.
    
    This will ensure that an MPU region can be defined
    for each process to protect it's .data and .bss regions
    """
    target = 'arm7m'
    pagesize = 32
    def __init__(self, **kws):
        self.__dict__.update(kws)
        self.procs = OrderedDict()

    def addproc(self, name, files):
        assert re.match(r'[a-zA-Z_][a-zA-Z0-9_]*', name)
        _log.info('Define process "%s" from: %s', name, files)
        bss, data = 0, 0
        for F in files:
            for elf in loadobj([F]):
                for S in elf.sections:
                    if S.name.startswith('.bss'):
                        bss += S.vsize
                    elif S.name.startswith('.data'):
                        data += S.vsize

        _log.info('process "%s" data=%d bss=%d', name, data, bss)

        total = max(32, data+bss)
        cand = round2(total)
        assert cand>=total

        self.procs[name] = _proc(name, files, total, 0)

    def addextra(self, files):
        names = dict([('.bss.proc.%s'%P.name,P) for P in self.procs.values()]+
                     [('.data.proc.%s'%P.name,P) for name in self.procs.values()])
        for F in files:
            for elf in loadobj([F]):
                for S in elf.sections:
                    if S.name in names:
                        _log.info('process "%s" extra %d', S.name, S.vsize)
                        names[S.name].size += S.vsize

    def finalize(self):
        for P in self.procs.values():
            P.align = round2(P.size)
        _log.info('process "%s" size %d align to %d', P.name, P.size, P.align)

    def save(self, fname):
        self.finalize()
        out = StringIO()
        # write prelude for code sections
        out.write("""

INCLUDE "target.ld"

SECTIONS
{
    __ram_start = ORIGIN(RAM);

    .text :
    {
      __rom_start = .;
      *(.text.start) /* ensure that entry point appears first */
      . = ALIGN(64);
      *(.text .text.*)
    } >ROM =0 /* =0 zeros unused */

    .rodata :
    {
      *(.rodata .rodata.*)
      . = ALIGN(4);
      PROVIDE_HIDDEN(__sos_procs_start = .);
      *(.sos.proc)
      PROVIDE_HIDDEN(__sos_procs_end = .);
    } >ROM =0

    /* begin c++ stuff */

    .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >ROM
    PROVIDE_HIDDEN (__exidx_start = .);
    .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >ROM
    PROVIDE_HIDDEN (__exidx_end = .);
""")
        
        # proc specific ROM
        for P in self.procs.values():
            out.write("""
    .preinit_array     :
    {
      PROVIDE_HIDDEN (__%s_preinit_array_start = .);
"""%P.name)
            for PF in P.files:
                out.write("      KEEP (%s(.preinit_array))\n"%PF)
            out.write("""
      PROVIDE_HIDDEN (__%s_preinit_array_end = .);
    } >ROM
    .init_array     :
    {
      PROVIDE_HIDDEN (__%s_init_array_start = .);
"""%(P.name,P.name))
            for PF in P.files:
                out.write("      KEEP (%s(SORT(.init_array.*)))\n"%PF)
                out.write("      KEEP (%s(.init_array ))\n"%PF)
            out.write("""
      PROVIDE_HIDDEN (__%s_init_array_end = .);
    } >ROM
    .fini_array     :
    {
      PROVIDE_HIDDEN (__%s_fini_array_start = .);
"""%(P.name,P.name))
            for PF in P.files:
                out.write("      KEEP (%s(SORT(.fini_array.*)))\n"%PF)
                out.write("      KEEP (%s(.fini_array ))\n"%PF)
            out.write("""
      PROVIDE_HIDDEN (__%s_fini_array_end = .);
    } >ROM
"""%P.name)

        out.write("""
    .jcr            : { KEEP (*(.jcr)) } >ROM
    __rom_end = .;
""")

        # proc specific RAM areas

        for P in self.procs.values():
            KW = {'name':P.name,'align':P.align}
            out.write("""
    .data.%(name)s :
    {
      . = ALIGN(%(align)d);
      __%(name)s_memory_start = .;
      __%(name)s_data_start = .;
      __%(name)s_data_load = LOADADDR(.data);
"""%KW)
            for PF in P.files:
                out.write("      %s(.data .data.* .gnu.linkonce.d.*)\n"%PF)
            out.write("""
      *(.data.proc.%(name)s)
      __%(name)s_data_end = .;
    } >RAM AT>ROM

    .bss.%(name)s :
    {
      __%(name)s_bss_start = .;
"""%KW)
            for PF in P.files:
                out.write("      %s(COMMON)\n"%PF)
                out.write("      %s(.bss .bss.* .gnu.linkonce.b.*)\n"%PF)
            out.write("""
      *(.bss.proc.%(name)s)\n
      . = ALIGN(%(align)d);
      __%(name)s_bss_end = .;
      __%(name)s_memory_end = .;
    } >RAM AT>ROM
"""%KW)

        # post-script, catch any remaining data/bss goes into the "global" area
        out.write("""

    .data : ALIGN(4)
    {
      __data_start = .;
      __data_load = LOADADDR(.data);
      *(.data .data.* .gnu.linkonce.d.*)
      __data_end = .;
    } >RAM AT>ROM

    .bss : ALIGN(4)
    {
      __bss_start = .;
      *(COMMON)
      *(.bss .bss.* .gnu.linkonce.b.*)
      __bss_end = .;
    } >RAM AT>ROM

    __after_all_load = .;
}


/* check some assumptions */
ASSERT(ADDR(.text)+SIZEOF(.text)<=ADDR(.rodata), "overlap between .text and .rodata")
""")

        from .common import writeout
        writeout(fname, out)


def main():
    import sys
    L = LinkScript()
    name = None
    files = []
    logging.basicConfig(level=logging.DEBUG)
    for cmd in sys.argv[2:]+['.']:
        if cmd=='.':
            if name is not None and files:
                L.addproc(name, files)
            name = None
            files = []
        elif name is None:
            name = cmd
        else:
            files.append(cmd)

    L.save(sys.argv[1])

if __name__=='__main__':
    main()
