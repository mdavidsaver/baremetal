
from cStringIO import StringIO

class ProcInfo(object):
    def __init__(self):
        self.out = StringIO()
        self.out.write("#include <process.h>\n\n")

    def addproc(self, PC):
        out = self.out
        out.write("""

extern char stack_%(name)s_bottom,
            stack_%(name)s_top,
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

const process_config proc_%(name)s_info = {
    .stack_bottom = &stack_%(name)s_bottom,
    .stack_top = &stack_%(name)s_top;
    .data_start = &__%(name)s_data_start,
    .data_end = &__%(name)s_data_end,
    .data_load = &__%(name)s_data_load;
    .bss_start = &__%(name)s_bss_start,
    .bss_end = &__%(name)s_bss_end,
    .preinit_array_start = &__%(name)s_preinit_array_start,
    .preinit_array_end = &__%(name)s_preinit_array_end,
    .init_array_start = &__%(name)s_init_array_start,
    .init_array_end = &__%(name)s_init_array_end,
    .fini_array_start = &__%(name)s_fini_array_start,
    .fini_array_end = &__%(name)s_fini_array_end,
    .super = !!%(super)s,
    .name = "%(name)s"
};

"""%{'name':PC.name, 'super':PC.sup})

    def save(self, fname):
        from .common import writeout
        writeout(fname, self.out)
