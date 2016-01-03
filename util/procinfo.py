
from cStringIO import StringIO

class ProcInfo(object):
    def __init__(self):
        self.out = StringIO()
        self.out.write("#include <process.h>\n#include <stddef.h>\n")

    def addproc(self, PC):
        out = self.out

        out.write("""

static
thread proc_%(name)s_threads[];

extern char __%(name)s_memory_start,
            __%(name)s_memory_end,
            __%(name)s_data_start,
            __%(name)s_data_end,
            __%(name)s_data_load,
            __%(name)s_bss_start,
            __%(name)s_bss_end,
            __%(name)s_preinit_array_start,
            __%(name)s_preinit_array_end,
            __%(name)s_init_array_start,
            __%(name)s_init_array_end,
            __%(name)s_fini_array_start,
            __%(name)s_fini_array_end;

static
const process_config proc_%(name)s_info = {
    .memory_start = &__%(name)s_memory_start,
    .memory_end = &__%(name)s_memory_end,
    .data_start = &__%(name)s_data_start,
    .data_end = &__%(name)s_data_end,
    .data_load = &__%(name)s_data_load,
    .bss_start = &__%(name)s_bss_start,
    .bss_end = &__%(name)s_bss_end,
    .preinit_array_start = &__%(name)s_preinit_array_start,
    .preinit_array_end = &__%(name)s_preinit_array_end,
    .init_array_start = &__%(name)s_init_array_start,
    .init_array_end = &__%(name)s_init_array_end,
    .fini_array_start = &__%(name)s_fini_array_start,
    .fini_array_end = &__%(name)s_fini_array_end,
    .threads = proc_%(name)s_threads,
    .super = !!%(sup)d,
    .autostart = !!%(autostart)d,
    .name = "%(name)s"
};

process proc_%(name)s = {
    .info = &proc_%(name)s_info,
};

const process *proc_%(name)s_p
__attribute__((section(".sos.proc")))
 = &proc_%(name)s;

"""%PC)

        for T in PC['threads'].values():
            print('XXXXX',T)

            out.write("""
extern int %(entry)s(const char *);

static
char thread_%(name)s_stack[%(stack_size)d]
  __attribute__((section(".bss.proc.%(proc)s"),aligned(4)));

static
const thread_config thread_%(name)s_conf = {
    .prio = %(prio)d,
    .entry = &%(entry)s,
    .stack = thread_%(name)s_stack,
    .stack_size = sizeof(thread_%(name)s_stack),
    .proc_main = !!%(proc_main)d,
    .name = "%(name)s"
};
"""%T)

        out.write("""
static
thread proc_%(name)s_threads[] = {
"""%PC)

        for T in PC['threads'].values():

            out.write("""
    {
        .info = &thread_%(name)s_conf,
        .proc = &proc_%(proc)s,
    },
"""%T)

        out.write("""
    {.info=NULL}
};
""")

    def save(self, fname):
        from .common import writeout
        writeout(fname, self.out)
