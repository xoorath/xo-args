[Readme.md](../../Readme.md) > **Example 2**

Example 2 demonstrates the memory management in xo-args by providing custom
alloc, realloc and free functions which track each allocation as it occurs.
The example should show that 428 bytes of memory are required and once the
context is destroyed no allocated memory remains.

No global state is required by xo-args. All allocations are tracked by the
xo-args context and freed on demand with xo_args_destroy_ctx.

xo-args does assume that the lifetime of argv is longer than the lifetime of the
context, but no other string lifetimes are assumed. The application name, 
documentation, argument names and argument short names are copied as they are
provided.