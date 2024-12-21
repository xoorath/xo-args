#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../xo-args.h"

typedef struct allocation
{
    void* memory;
    size_t size;
} allocation;

typedef struct program_state
{
    char * standard_output;
    size_t standard_output_size;
    size_t standard_output_reserved;

    char * assertion_output;
    size_t assertion_output_size;
    size_t assertion_output_reserved;

    allocation * allocations;
    size_t allocations_size;
    size_t allocations_reserved;
} program_state;

program_state g_program_state = {0};

////////////////////////////////////////////////////////////////////////////////
void initialize_program_state(void)
{
    if (NULL != g_program_state.standard_output)
    {
        free(g_program_state.standard_output);
    }
    g_program_state.standard_output_reserved = 1024;
    g_program_state.standard_output_size = 0;
    g_program_state.standard_output = (char*)malloc(
        g_program_state.standard_output_reserved);

    if (NULL != g_program_state.assertion_output)
    {
        free(g_program_state.assertion_output);
    }
    g_program_state.assertion_output_reserved = 1024;
    g_program_state.assertion_output_size = 0;
    g_program_state.assertion_output = (char*)malloc(
        g_program_state.assertion_output_reserved);

    if (NULL != g_program_state.assertion_output)
    {
        free(g_program_state.allocations);
    }
    g_program_state.allocations_reserved = 1024;
    g_program_state.allocations_size = 0;
    g_program_state.allocations = (allocation*)malloc(
        sizeof(allocation) * g_program_state.allocations_reserved); 
}

///////////////////////////////////////////////////////////////////////////////
int test_printf(char const * const fmt, ...)
{
    // The size of this buffer limits the kinds of strings we're able to print
    // in this program. This size is an arbitrary size larger than we expect.
    char buff[4096] = {0};
    va_list ap;
    va_start(ap, fmt);
    int const printed = vsnprintf(buff, sizeof(buff), fmt, ap);
    va_end(ap);
    size_t const new_size = g_program_state.standard_output_size + (size_t)printed;

    if (new_size >= g_program_state.standard_output_reserved)
    {
        g_program_state.standard_output_reserved += sizeof(buff);
        g_program_state.standard_output = realloc(g_program_state.standard_output,
                                                  g_program_state.standard_output_reserved);
    }
    memcpy(&g_program_state.standard_output[g_program_state.standard_output_size],
           buff, (size_t)printed);
        
    g_program_state.standard_output_size += (size_t)printed;
    g_program_state.standard_output[g_program_state.standard_output_size] = '\0';

    return printed;
}

////////////////////////////////////////////////////////////////////////////////
void* test_alloc(size_t const size)
{
    if (g_program_state.allocations_reserved == g_program_state.allocations_size)
    {
        g_program_state.allocations_reserved *= 2;
        g_program_state.allocations = realloc(g_program_state.allocations,
                                              sizeof(void*) 
                                              * g_program_state.allocations_reserved);
    }
    void* const mem = malloc(size);
    g_program_state.allocations[g_program_state.allocations_size].size = size;
    g_program_state.allocations[g_program_state.allocations_size++].memory = mem;
    return mem;
}

////////////////////////////////////////////////////////////////////////////////
void* test_realloc(void * const mem, size_t const size)
{
    for (size_t i = 0; i < g_program_state.allocations_size; ++i)
    {
        if (mem == g_program_state.allocations[i].memory)
        {
            g_program_state.allocations[i].memory = realloc(mem, size);
            g_program_state.allocations[i].size = size;
            return g_program_state.allocations[i].memory;
        }
    }
    assert(false && "reached a code path that was assumed to be unreachable");
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void test_free(void * const mem)
{
    for (size_t i = 0; i < g_program_state.allocations_size; ++i)
    {
        if (mem == g_program_state.allocations[i].memory)
        {
            free(g_program_state.allocations[i].memory);
            --g_program_state.allocations_size;
            if (i != g_program_state.allocations_size)
            {
                memcpy(&g_program_state.allocations[i],
                    &g_program_state.allocations[g_program_state.allocations_size],
                    sizeof(allocation));
            }
            return;
        }
    }
    assert(false && "reached a code path that was assumed to be unreachable");
}

////////////////////////////////////////////////////////////////////////////////
bool test_condition(bool condition, char const * const fmt, ...)
{
    if (true == condition)
    {
        printf("[PASS] ");
    }
    else
    {
        printf("[FAIL] ");
    }

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    
    printf("\n");
    return condition;
}

////////////////////////////////////////////////////////////////////////////////
bool test1(void)
{
    initialize_program_state();

    xo_argc_t const mock_argc = 1;
    char const * mock_argv[mock_argc];
    mock_argv[0] = "/mock/xo-args-tests";

    if (false == test_condition(g_program_state.allocations_size == 0,
                                "test setup assumption: allocator is unused at test start"))
    {
        return false;
    }

    xo_args_ctx * ctx = xo_args_create_ctx_advanced(mock_argc,
                                                    mock_argv,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    test_alloc,
                                                    test_realloc,
                                                    test_free,
                                                    test_printf);

    if (false == test_condition(g_program_state.allocations_size != 0,
                                "xo-args uses custom allocator function"))
    {
        return false;
    }

    xo_args_destroy_ctx(ctx);

    if (false == test_condition(g_program_state.allocations_size == 0,
                                "xo-args does not leak the context"))
    {
        return false;
    }

    ctx = xo_args_create_ctx_advanced(mock_argc,
                                     mock_argv,
                                     NULL,
                                     NULL,
                                     NULL,
                                     test_alloc,
                                     test_realloc,
                                     test_free,
                                     test_printf);

    if (false == xo_args_submit(ctx))
    {
        test_condition(false, "test setup assumption: xo_args_submit would"
                              " return true if setup with no arguments.");
        xo_args_destroy_ctx(ctx);
        return false;
    }

    xo_args_destroy_ctx(ctx);

    if (false == test_condition(g_program_state.allocations_size == 0,
                                "xo-args does not leak when submitting"))
    {
        return false;
    }

    if (false == test_condition(g_program_state.standard_output_size == 0,
                                "xo-args does not print when submit is"
                                " successful"))
    {
        printf("stdout: \n\"\"\"\n%s\n\"\"\"\n", g_program_state.standard_output);
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////////////////////////
int main(xo_argc_t const argc, xo_argv_t const argv)
{
    (void)argc;
    (void)argv;
    size_t failed_tests = 0;

    failed_tests += false == test1() ? 1 : 0;
    free(g_program_state.allocations);
    free(g_program_state.assertion_output);
    free(g_program_state.standard_output);
    printf("%zu tests failed\n", failed_tests);
    return (int)failed_tests;
}

// xo-args-tests.c implements xo-args rather than linking to 
// ../examples/common/xo-args.c so we can define our own assertion macro.
// todo: define our own assertion macro
#define XO_ARGS_IMPL
#include "../xo-args.h"
