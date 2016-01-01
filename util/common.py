from __future__ import print_function

import logging, sys
from shutil import copyfileobj

_log = logging.getLogger(__name__)

def writeout(fname, out):
    out.seek(0)
    if fname=='-':
        copyfileobj(out, sys.stdout)
        return
    writeout = False
    try:
        FP = open(fname,'r+')
    except IOError as e:
        if e.errno!=2:
            raise
        writeout = True
        _log.info("Create %s", fname)
        FP = open(fname, 'w')
    else:
        FP.seek(0,2)
        if out.tell()!=FP.tell():
            _log.debug("size differs")
            writeout = True
        else:
            FP.seek(0)
            out.seek(0)
            if out.read()!=FP.read(): # TODO, incremental
                _log.debug("content differs")
                writeout = True

    if writeout:
        _log.info("Writing %s", fname)
        out.seek(0)
        FP.seek(0)
        FP.truncate(0)
        copyfileobj(out, FP)
    else:
        _log.info("Keep existing %s", fname)
