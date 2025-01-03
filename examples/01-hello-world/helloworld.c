#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xo-args/xo-args.h>

int main(int argc, char ** argv)
{
    // We'll use mock data for argc/argv in this example
    (void)argc; // unused
    (void)argv; // unused

    // STEP 1: Create an xo-args context with your argc/argv
    //////////////////////////////////////////////////////////////////////////
    char const * const mock_argv[] = {"/mock/helloworld.exe",
                                      "--message",
                                      "Hello World!",
                                      "--repeat=5",
                                      "-V"};
    int const mock_argc = sizeof(mock_argv) / sizeof(mock_argv[0]);

    // You can also use xo_args_create_ctx_advanced for more customization
    // options.
    xo_args_ctx * const args_ctx = xo_args_create_ctx(mock_argc, mock_argv);

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

    xo_args_arg const * const arg_verbose =
        xo_args_declare_arg(args_ctx,
                            "verbose",
                            "V",
                            NULL,
                            "print additional info",
                            XO_ARGS_TYPE_SWITCH);

    // Submit returning false means there was some error such as bad user input
    if (!xo_args_submit(args_ctx))
    {
        xo_args_destroy_ctx(args_ctx);
        return -1;
    }

    // STEP 3: Get the value of any arguments you declared above
    //////////////////////////////////////////////////////////////////////////

    char const * message;
    int64_t repeat = 10;
    bool verbose = false;

    xo_args_try_get_bool(arg_verbose, &verbose);
    if (verbose)
        puts("[verbose] verbose = true");

    // Arguments declared with XO_ARGS_ARG_REQUIRED will always have a value
    xo_args_try_get_string(arg_message, &message);
    if (verbose)
        printf("[verbose] message = \"%s\"\n", message);

    if (xo_args_try_get_int(arg_repeat, &repeat))
    {
        if (verbose)
            printf("[verbose] repeat = %" PRId64 "\n", repeat);
    }
    else
    {
        if (verbose)
            printf("[verbose] repeat = (default) %" PRId64 "\n", repeat);
    }

    for (int64_t i = 0; i < repeat; ++i)
    {
        puts(message);
    }

    xo_args_destroy_ctx(args_ctx);
    return 0;
}

#define XO_ARGS_IMPL
#include <xo-args/xo-args.h>