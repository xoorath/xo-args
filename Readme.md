![xo-args.png](https://git.merveilles.town/xo/xo-args-internal/raw/branch/main/docs/xo-args-wordmark.png)

# About

**xo-args** provides a way to declare named arguments for a program's argv
in a portable manner with a syntax that will be familiar to many users.
xo-args allows for typed arguments, arrays of values, and will generate help
text. It is designed to work with C99 or C++98 or newer.

Status: Work in progress as of: 2025 January

# Unlicense

xo-args is in the [public domain](UNLICENSE). 

# Quick Start

```c 
// Define XO_ARGS_IMPL before including xo-args.h in a single c or cpp file.
#define XO_ARGS_IMPL
#include "xo-args.h"
```

```c
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xo-args.h>

int main(int const argc, char const * const * const argv)
{
    // STEP 1: Create an xo-args context with your argc/argv
    //////////////////////////////////////////////////////////////////////////
    xo_args_ctx * const args_ctx = xo_args_create_ctx(argc, argv);

    // STEP 2: Declare some arguments and submit them to finalize your
    // declaration
    //////////////////////////////////////////////////////////////////////////
    xo_args_arg const * const arg_message =
        xo_args_declare_arg(args_ctx,
                            "message",
                            "m",
                            "MSG",
                            "a message to print to stdout some number of times "
                            "(see: --repeat)",
                            XO_ARGS_TYPE_STRING | XO_ARGS_ARG_REQUIRED);

    xo_args_arg const * const arg_repeat =
        xo_args_declare_arg(args_ctx,
                            "repeat",
                            "r",
                            "COUNT",
                            "the number of times to print the message",
                            XO_ARGS_TYPE_INT);

    // Submit returning false means there was some error such as bad user input
    // or --help/-h was one of the user provided arguments.
    if (!xo_args_submit(args_ctx))
    {
        xo_args_destroy_ctx(args_ctx);
        return -1;
    }

    // STEP 3: Get the value of any arguments you declared above
    //////////////////////////////////////////////////////////////////////////
    char const * message;
    int64_t repeat = 10;

    // Arguments declared with XO_ARGS_ARG_REQUIRED will always have a value.
    xo_args_try_get_string(arg_message, &message);
    xo_args_try_get_int(arg_repeat, &repeat);

    for (int64_t i = 0; i < repeat; ++i)
    {
        puts(message);
    }

    xo_args_destroy_ctx(args_ctx);
    return 0;
}
```

# Examples

* [01-hello-world](./examples/01-hello-world/): A basic example in C.
* [02-cpp](./examples/02-cpp/): A C++ example where xo-args is 
encapsulated.
* [03-sqlite3](./examples/03-sqlite3/): A complex example in C that 
re-creates the many arguments of sqlite3

# Supported Compilers and Platforms

* ✅ clang on Linux: Supported and regularly tested.
* ✅ gcc on Linux: Supported and regularly tested.
* ✅ msvc on Windows: Supported and regularly tested.
* ❔ clang everywhere else: Unknown. Probably supported.
* ❔ gcc everywhere else: Unknown. Probably supported.
* ❔ other compilers and platforms: Unknown. Probably supported.

# Contributing

See [xo-args-internal](https://git.merveilles.town/xo/xo-args-internal).