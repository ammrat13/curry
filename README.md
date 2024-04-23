# Curry C Functions

Currying library for C functions on x86-64, inspired by an extension for a lab
from [CS 240LX][1]. For every curried function, this library creates a thunk
that populates the arguments, calls the function, then frees itself.

I suspect one can do this without having to JIT compile code. Additionally, this
implementation reserves an entire page for every thunk, which is really
wasteful.

[1]: https://github.com/dddrrreee/cs240lx-24spr/tree/main/labs/5-jit-derive
