from __future__ import print_function

import logging
import re
from collections import namedtuple

_log = logging.getLogger(__name__)
SPAM = logging.DEBUG=5
logging.addLevelName('SPAM', SPAM)
def spam(*args, **kws):
    _log.log(SPAM, *args, **kws)
_log.spam = spam
del spam

_WS = r'[ \t\r]+'
_KW = r'[a-zA-Z_][a-zA-Z0-9_]*'
_INT = r'[+-]?(?:0[xb])?[0-9]+'
_STR = r'"[^\r\n"]*"'
_LIT = r'[{}\[\]=]'
_NL = r'[\n]+'
_CMT = r'#[^\n]*\n'

_pat = '|'.join(['(%s)'%e for e in [_WS, _KW, _INT, _STR, _LIT, _NL, _CMT]])

_exp = re.compile(_pat, re.MULTILINE)

_tup = namedtuple("Node", ['name','value','children','lineno'])

def lex(s):
    P, L = 0, len(s)
    lineno = 1
    linestart = 0
    while P<L:
        M = _exp.match(s, P)
        if not M:
            raise RuntimeError('Unknown charactor "%s" at %d:%d'%(s[P],lineno,P-linestart+1))
        P = M.end(0)
        ws, kw, ival, strg, lit, nl, cmt = M.groups()
        if ws:
            continue
        elif kw:
            yield ('KW', kw, lineno)
        elif ival:
            yield ('INT', int(ival,0), lineno)
        elif strg:
            yield ('STR', strg[1:-1], lineno)
        elif lit:
            yield (lit, None, lineno)
        elif nl:
            lineno += len(nl)
            linestart = P
        elif cmt:
            lineno += 1
            linestart = P
        else:
            raise AssertionError("Logic error")

def parseAST(s):
    """
     blks : blks blk
          | blk
          | <empty>
     blk : KW '=' value
         | KW value '{' blks '}'
     value : INT
           | STR
           | '[' values ']'
     values : values value
            | value
            | <empty>
            
     ST==0 # [... blk]
       KW -> push val -> ST=1
       '}' -> pop blk -> append blk -> ST=0
       eof -> done
       . -> error
     ST==1 # [... blk] KW
       '{' -> pop val -> assemble(val,None,[]) -> push -> ST=0
       '=' -> ST=2
       INT|STR -> push val -> ST=3
       . -> error
     ST==2 # [... blk] KW
       INT|STR -> pop val1 -> assemble blk(val1,val,None) -> append -> ST=0
       '[' -> push [] -> ST=4
       . error
     ST==3 # [... blk] KW INT|STR
       '{' -> pop val2 -> pop val1 -> assemble blk(val1,val2,[]) -> push blk -> ST=0
       . error
     ST==4 # [... blk] KW []
       INT|STR -> append -> ST=4
       ']' -> pop val2 -> pop val1 -> assemble blk(val1,val2,None) -> append -> ST=0
       . error
    """
    stack = [(None,None,[],None)] # ('name','value',[(...)],lineno)
    top =  stack[0]
    lst = stack[0][2]
    lineno = 0
    ST = 0
    for tok in lex(s):
        T = tok[0]
        _log.spam("Parse '%s' state=%d, stack[-3:]=%s",tok,ST,stack[-3:])
        if T=='KW':
            lineno = tok[2]

        if ST==0:
            # [... blk]
            if T=='KW':
                stack.append(tok[1])
                ST = 1
            elif T=='}':
                if len(stack)==1:
                    raise RuntimeError("Unbalanced '}' on line %d"%tok[2])

                blk = stack.pop()
                lst = stack[-1][2]
                lst.append(blk)
            else:
                raise RuntimeError("Expected keyword or '}' on line %d"%tok[2])

        elif ST==1:
            # [... blk] KW
            if T=='{':
                ent = _tup(stack[-1], None, [], lineno)
                stack[-1] = ent
                lst = ent[2]
                ST = 0
            elif T=='=':
                ST = 2
            elif T in ('INT', 'STR'):
                stack.append(tok[1])
                ST = 3
            else:
                raise RuntimeError("Expected value or '{' on line %d"%tok[2])

        elif ST==2:
            # [... blk] KW
            if T=='[':
                stack.append([])
                ST = 4
            elif T in ('INT', 'STR'):
                ent = _tup(stack[-1], tok[1], None, lineno)
                lst.append(ent)
                stack.pop()
                ST = 0

        elif ST==3:
            # [... blk] KW INT|STR
            if T=='{':
                ent = _tup(stack[-2], stack[-1], [], lineno)
                stack.pop()
                stack[-1] = ent
                lst = ent[2]
                ST = 0
            else:
                raise RuntimeError("Expected keyword or '{' on line %d"%tok[2])

        elif ST==4:
            # [... blk] KW [...]
            if T==']':
                ent = _tup(stack[-2], stack[-1], None, lineno)
                stack = stack[:-2]
                lst.append(ent)
                ST = 0
            elif T in ('INT', 'STR'):
                stack[-1].append(tok[1])
        else:
            raise AssertionError("Logic Error")

    _log.spam("Final state=%d, stack=%s",ST,stack)
    if ST!=0 or len(stack)!=1:
        raise RuntimeError("Expected '}' on line %d"%tok[2])

    assert stack[0] is top
    return top[2]

def main():
    import sys
    logging.basicConfig(level=logging.DEBUG)
    with open(sys.argv[2]) as F:
        text = F.read()
        if sys.argv[1]=='lex':
            P=0
            for T in lex(text):
                print(T)

        elif sys.argv[1]=='parse':
            from pprint import pprint
            pprint(parseAST(text))

        else:
            print("Unknown "+sys.argv[1])
            sys.exit(1)

if __name__=='__main__':
    main()
