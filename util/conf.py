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
        self._threads = set()

        for ent in ast:
            if ent.name=='bsp' and ent.value is None:
                self._setbsp(ent.children)
            elif ent.name=='process':
                self._addproc(ent.value, ent.children)
            elif ent.name=='thread':
                self._addthr(ent.value, ent.children)
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

    def _addproc(self, name, ast):
        if name in self._procs:
            raise RuntimeError("Duplicate process name '%s'"%name)
        P = {'name':name, 'id':len(self._procs),
             'sup':0,
             'threads':[],
        }
        for ent in ast:
            if ent.name=='objects':
                P['files'] = list(ent.value)
            elif ent.name=='supervisor':
                P['sup'] = int(ent.value)
            elif ent.name in []:
                P[ent.name] = ent.value
            else:
                _log.warning("Unknown process key: '%s'", ent)

        if P.get('files') is None:
            raise RuntimeError("process '%s' has not objects"%name)
        self._procs[name] = P

    def _addthr(self, name, ast):
        if name in self._threads:
            raise RuntimeError("Duplicate thread name '%s'"%name)
        T = {'name':name, 'prio':0, 'autostart':0,
             'stack_size':1024,
        }
        for ent in ast:
            if ent.name in ['proc', 'entry', 'autostart']:
                T[ent.name] = ent.value

        if T.get('entry') is None:
            raise RuntimeError("thread '%s' has not entry point (main function)"%name)
        try:
            P = self._procs[T['proc']]
        except KeyError:
            raise RuntimeError("thread '%s' has not process"%name)
        else:
            P['threads'].append(T)

    @property
    def procs(self):
        return self._procs.values()

    def proc(self, name):
        return self._procs[name]

    def __getitem__(self, k):
        return self._kv[k]
