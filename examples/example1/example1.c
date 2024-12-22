#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xo-args.h>

///////////////////////////////////////////////////////////////////////////////
// Example1
int main(xo_argc_t const argc, xo_argv_t const argv)
{
    (void)argc; // unused
    (void)argv; // unused
    xo_argc_t const mock_argc = 3;
    char const * mock_argv[3];
    mock_argv[0] = "/mock/example1";
    mock_argv[1] = "--foo";
    mock_argv[2] = "this is an example.";

    xo_args_ctx* args_ctx = xo_args_create_ctx(mock_argc, mock_argv);
    // The context should only fail to be created if the arguments are invalid
    // such as argc being 0, or argv containing null arguments.
    if (NULL == args_ctx)
    {
        return -1;
    }

    xo_args_arg const * const foo = xo_args_declare_arg(args_ctx, 
                                                        "foo", 
                                                        "f", 
                                                        XO_ARGS_TYPE_STRING
                                                        | XO_ARGS_ARG_REQUIRED);

    // If xo_args_submit returns false then the parameters passed to the program are invalid
    // In that case the generated help text is printed to stdout and it makes sense to exit 
    // with a non 0 error code in most cases  
    if (false == xo_args_submit(args_ctx))
    {
        xo_args_destroy_ctx(args_ctx);
        args_ctx = NULL;
        return -1;
    }

    char const * foo_value;
    if (xo_args_try_get_string(foo, &foo_value))
    {
        printf("foo value: %s\n", foo_value);
    }
    else
    {
        printf("foo value: unset\n");
    }

    xo_args_destroy_ctx(args_ctx);
    args_ctx = NULL;
    return 0;

}


#define XO_ARGS_IMPL
#include <xo-args.h>