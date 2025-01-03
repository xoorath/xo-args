#include "xo-args-test-funcs.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct program_state
{
    char * standard_output;
    size_t standard_output_size;
    size_t standard_output_reserved;

    char * assertion_output;
    size_t assertion_output_size;
    size_t assertion_output_reserved;
    size_t assertion_count;

    struct allocation * allocations;
    size_t allocations_size;
    size_t allocations_reserved;
};

static struct program_state g_program_state = { 0 };

////////////////////////////////////////////////////////////////////////////////
void test_global_setup(void)
{
    // Make sure we are starting from a clean slate:
    free(g_program_state.standard_output);
    free(g_program_state.assertion_output);
    free(g_program_state.allocations);
    memset(&g_program_state, 0, sizeof(g_program_state));

    g_program_state.standard_output_reserved = 1024;
    g_program_state.standard_output_size = 0;
    g_program_state.standard_output =
        (char *)malloc(g_program_state.standard_output_reserved);
    g_program_state.standard_output[0] = '\0';

    g_program_state.assertion_output_reserved = 1024;
    g_program_state.assertion_output_size = 0;
    g_program_state.assertion_output =
        (char *)malloc(g_program_state.assertion_output_reserved);
    g_program_state.assertion_output[0] = '\0';

    g_program_state.allocations_reserved = 1024;
    g_program_state.allocations_size = 0;
    g_program_state.allocations = (struct allocation *)malloc(
        sizeof(struct allocation) * g_program_state.allocations_reserved);
}

////////////////////////////////////////////////////////////////////////////////
void test_global_shutdown(void)
{
    free(g_program_state.standard_output);
    free(g_program_state.assertion_output);
    free(g_program_state.allocations);
    memset(&g_program_state, 0, sizeof(g_program_state));
}

////////////////////////////////////////////////////////////////////////////////
void test_global_clear(void)
{
    g_program_state.standard_output_size = 0;
    g_program_state.standard_output[0] = '\0';

    g_program_state.assertion_output_size = 0;
    g_program_state.assertion_output[0] = '\0';

    g_program_state.allocations_size = 0;
}

////////////////////////////////////////////////////////////////////////////////
int test_printf(char const * const fmt, ...)
{
    // The size of this buffer limits the kinds of strings we're able to print
    // in this program. This size is an arbitrary size larger than we expect.
    char buff[4096] = {0};
    va_list ap;
    va_start(ap, fmt);
    int const printed = vsnprintf(buff, sizeof(buff), fmt, ap);
    va_end(ap);
    size_t const new_size =
        g_program_state.standard_output_size + (size_t)printed;

    if (new_size >= g_program_state.standard_output_reserved)
    {
        g_program_state.standard_output_reserved += sizeof(buff);
        g_program_state.standard_output =
            realloc(g_program_state.standard_output,
                    g_program_state.standard_output_reserved);
    }
    memcpy(
        &g_program_state.standard_output[g_program_state.standard_output_size],
        buff,
        (size_t)printed);

    g_program_state.standard_output_size += (size_t)printed;
    g_program_state.standard_output[g_program_state.standard_output_size] =
        '\0';

    return printed;
}

////////////////////////////////////////////////////////////////////////////////
void * test_alloc(size_t size)
{
    if (g_program_state.allocations_reserved
        == g_program_state.allocations_size)
    {
        g_program_state.allocations_reserved *= 2;
        g_program_state.allocations =
            realloc(g_program_state.allocations,
                    sizeof(void *) * g_program_state.allocations_reserved);
    }
    void * const mem = malloc(size);
    g_program_state.allocations[g_program_state.allocations_size].size = size;
    g_program_state.allocations[g_program_state.allocations_size++].memory =
        mem;
    return mem;
}

////////////////////////////////////////////////////////////////////////////////
void * test_realloc(void * const mem, size_t size)
{
    for (size_t i = 0; i < g_program_state.allocations_size; ++i)
    {
        if (mem == g_program_state.allocations[i].memory)
        {
            g_program_state.allocations[i].memory = realloc(mem, size);
            g_program_state.allocations[i].size = size;
            g_program_state.allocations[i].reallocations++;
            return g_program_state.allocations[i].memory;
        }
    }
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
                       &g_program_state
                            .allocations[g_program_state.allocations_size],
                       sizeof(struct allocation));
            }
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
char const * test_get_stdout(void)
{
    return g_program_state.standard_output;
}

////////////////////////////////////////////////////////////////////////////////
struct allocation const * test_get_allocations(size_t * const out_count)
{
    *out_count = g_program_state.allocations_size;
    return g_program_state.allocations;
}

////////////////////////////////////////////////////////////////////////////////
void test_on_assert(bool const condition, char const * const fmt, ...)
{
    if (!condition)
    {
        ++g_program_state.assertion_count;
        // The size of this buffer limits the kinds of strings we're able to
        // print in this program. This size is an arbitrary size larger than we
        // expect.
        char buff[4096] = {0};
        va_list ap;
        va_start(ap, fmt);
        int const printed = vsnprintf(buff, sizeof(buff), fmt, ap);
        va_end(ap);
        size_t const new_size =
            g_program_state.assertion_output_size + (size_t)printed;

        if (new_size >= g_program_state.assertion_output_reserved)
        {
            g_program_state.assertion_output_reserved += sizeof(buff);
            g_program_state.assertion_output =
                realloc(g_program_state.assertion_output,
                        g_program_state.assertion_output_reserved);
        }
        memcpy(&g_program_state
                    .assertion_output[g_program_state.assertion_output_size],
               buff,
               (size_t)printed);

        g_program_state.assertion_output_size += (size_t)printed;
        g_program_state
            .assertion_output[g_program_state.assertion_output_size] = '\0';
    }
}

////////////////////////////////////////////////////////////////////////////////
size_t test_get_assert_count(void)
{
    return g_program_state.assertion_count;
}

////////////////////////////////////////////////////////////////////////////////
char const * test_get_assert_output(void)
{
    return g_program_state.assertion_output;
}