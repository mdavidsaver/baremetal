from __future__ import print_function

import logging
logging.addLevelName('SPAM', logging.DEBUG-5)

def linkscript(A):
    from .armlink import LinkScript
    L = LinkScript()
    for P in A.config.procs:
        L.addproc(P['name'], P['files'])
    L.addextra(A.extra)
    L.save(A.outfile)

def proc_conf(A):
    from .procinfo import ProcInfo
    L = ProcInfo()
    for P in A.config.procs:
        L.addproc(P)
    L.save(A.outfile)

def objdep(A):
    from cStringIO import StringIO
    out = StringIO()
    out.write(A.targetname+":")
    for P in A.config.procs:
        [out.write(' %s'%F) for F in P['files']]
    out.write("\n")

    from .common import writeout
    writeout(A.outfile, out)

def getargs():
    def lvlname(s):
        L = logging.getLevelName(s)
        if isinstance(L, str):
            raise TypeError("Not a valid log level name")
        return L

    from argparse import ArgumentParser
    P = ArgumentParser()
    P.add_argument('-C','--config',default='yaos.conf', help='Config file')
    P.add_argument('-l','--level',help='log level', type=lvlname, default=logging.WARN)

    SP = P.add_subparsers(help='Sub-commands')

    X = SP.add_parser('linkscript')
    X.add_argument('-E','--extra', metavar='FILE', help='extra elf file to search for proc sections',
                   action='append', default=[])
    X.add_argument('outfile',help='Write linker script to this file')
    X.set_defaults(func=linkscript)

    X = SP.add_parser('proc-conf')
    X.add_argument('outfile',help='Write .c with process config')
    X.set_defaults(func=proc_conf)

    X = SP.add_parser('obj-dep')
    X.add_argument('outfile',help='Write .d make style dependency')
    X.add_argument('targetname',help='Make target name')
    X.set_defaults(func=objdep)

    return P.parse_args()

def main(A):
    from .conf import Config
    logging.basicConfig(level=A.level)
    A.config = Config(A.config)
    A.func(A)

if __name__=='__main__':
    main(getargs())
