from __future__ import print_function

import logging
from collections import OrderedDict

from .clex import parseAST

_log = logging.getLogger(__name__)

class Config(object):
    def __init__(self, file):
        with open(file,'r') as F:
            ast = parseAST(F.read())
        self._kv = {}
        self._procs = OrderedDict()

        for ent in ast:
            if ent.name=='bsp' and ent.value is None:
                self._setbsp(ent.children)
            elif ent.name=='process':
                self._addproc(ent.value, ent.children)
            else:
                _log.warning("Unknown section: %s", ast)

    def _setbsp(self, ast):
        for ent in ast:
            if ent.children is not None:
                _log.warning("Unexpected bsp sub-sub-section: '%s'", ent)
            if ent.value is None:
                _log.warning("Unknown bsp sub-section: '%s'", ent)
            else:
                self._kv[ent.name] = ent.value

    class Proc(object):
        sup = 0
        files = None

    def _addproc(self, name, ast):
        P = self.Proc()
        P.name = name
        for ent in ast:
            if ent.name=='objects':
                P.files = list(ent.value)
            elif ent.name=='supervisor':
                P.sup = int(ent.value)
            else:
                _log.warning("Unknown process key: '%s'", ent)

        if P.files is None:
            raise RuntimeError("process '%s' has not objects"%name)
        self._procs[name] = P

    @property
    def procs(self):
        return self._procs.values()

    def proc(self, name):
        return self._procs[name]

    def __getitem__(self, k):
        return self._kv[k]
