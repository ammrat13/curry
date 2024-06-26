# This is the return code called by thunks generated by `vcurry`. The thunks
# need to free themselves before returning to the caller. To do this, they call
# `munmap` with this as the return address.
#
# This has a few consequences. Notably, the stack is aligned to 0 mod 16 on
# entry, instead of the usual 8 mod 16. The top of the stack is the value to be
# restored into %rax, and the return address is below that. Once the return
# happens, all the alignment will be back to normal.
    .intel_syntax
    .global vcurry_return_trampoline
vcurry_return_trampoline:
    pop %rax
    ret
