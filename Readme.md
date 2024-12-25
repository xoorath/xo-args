![xo-args.png](https://git.merveilles.town/xo/xo-args-internal/raw/branch/main/docs/xo-args-wordmark.png)

xo-args is a single header file library for defining and parsing command line
arguments.

Status: Work in progress as of: 2024 December

License: [CC0 1.0 Universal](./License.md)

# Quick Start:

```c 
// Define XO_ARGS_IMPL before including xo-args.h in a single c or cpp file.
#define XO_ARGS_IMPL
#include "xo-args.h"
```

```c
#include <stdbool.h>
#include <stdio.h>

#include "xo-args.h"

int main(int argc, char** argv)
{
    xo_args_ctx* args_ctx = xo_args_create_ctx(argc, argv);

    xo_args_arg const * const foo = xo_args_declare_arg(args_ctx, 
                                                        "foo", 
                                                        "f", 
                                                        XO_ARGS_TYPE_STRING
                                                        | XO_ARGS_ARG_REQUIRED);

    if (false == xo_args_submit(args_ctx))
    {
        xo_args_destroy_ctx(args_ctx);
        return -1;
    }


    char const * foo_value;
    if (xo_args_try_get_string(foo, &foo_value))
    {
        printf("foo value: %s\n", foo_value);
    }
    else
    {
        // unreachable because foo was defined with XO_ARGS_ARG_REQUIRED
    }
    
    xo_args_destroy_ctx(args_ctx);
    return 0;
}
```

# Examples

* [example1.c](./examples/example1/example1.c): Basic usage
* [example2.c](./examples/example2/example2.c): Demonstrates memory allocations
* [example3.c](./examples/example3/example3.cpp): C++ demo

# Contributing

See [xo-args-internal](https://git.merveilles.town/xo/xo-args-internal).