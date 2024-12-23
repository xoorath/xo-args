#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xo-args.h>

typedef struct allocation
{
    void * memory;
    size_t size;
    size_t reallocations;
} allocation;

typedef struct allocation_list
{
    allocation * allocations;
    size_t allocations_size;
    size_t allocations_reserved;

} allocation_list;

allocation_list g_allocation_list;

////////////////////////////////////////////////////////////////////////////////
void * example_alloc(size_t size)
{
    if (g_allocation_list.allocations_reserved
        == g_allocation_list.allocations_size)
    {
        g_allocation_list.allocations_reserved *= 2;
        g_allocation_list.allocations = (allocation *)realloc(
            g_allocation_list.allocations,
            sizeof(void *) * g_allocation_list.allocations_reserved);
    }
    void * const mem = malloc(size);
    size_t const i = g_allocation_list.allocations_size++;
    allocation * a = &g_allocation_list.allocations[i];
    a->size = size;
    a->reallocations = 0;
    a->memory = mem;
    return mem;
}

////////////////////////////////////////////////////////////////////////////////
void * example_realloc(void * const mem, size_t size)
{
    for (size_t i = 0; i < g_allocation_list.allocations_size; ++i)
    {
        if (mem == g_allocation_list.allocations[i].memory)
        {
            g_allocation_list.allocations[i].memory = realloc(mem, size);
            g_allocation_list.allocations[i].size = size;
            g_allocation_list.allocations[i].reallocations++;
            return g_allocation_list.allocations[i].memory;
        }
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void example_free(void * const mem)
{
    for (size_t i = 0; i < g_allocation_list.allocations_size; ++i)
    {
        if (mem == g_allocation_list.allocations[i].memory)
        {
            free(g_allocation_list.allocations[i].memory);
            --g_allocation_list.allocations_size;
            if (i != g_allocation_list.allocations_size)
            {
                memcpy(&g_allocation_list.allocations[i],
                       &g_allocation_list
                            .allocations[g_allocation_list.allocations_size],
                       sizeof(struct allocation));
            }
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
bool example_is_alnum(char const c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
           || (c >= '0' && c <= '9');
}

////////////////////////////////////////////////////////////////////////////////
bool example_is_alnum_str(char const * const start, char const * const end)
{
    char const * const last = (NULL != end ? end : (start + strlen(start))) - 1;
    char const * curr = start;
    while (curr != last)
    {
        if (false == example_is_alnum(*curr))
        {
            return false;
        }
        ++curr;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Example2
int main(xo_argc_t const argc, xo_argv_t const argv)
{
    (void)argc; // unused
    (void)argv; // unused

    g_allocation_list.allocations_reserved = 32;
    g_allocation_list.allocations_size = 0;
    g_allocation_list.allocations =
        malloc(g_allocation_list.allocations_reserved * sizeof(allocation));

    xo_argc_t const mock_argc = 3;
    char const * mock_argv[3];
    mock_argv[0] = "/mock/example2";
    mock_argv[1] = "--foo";
    mock_argv[2] = "this is another example.";

    xo_args_ctx * args_ctx = xo_args_create_ctx_advanced(mock_argc,
                                                         mock_argv,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         example_alloc,
                                                         example_realloc,
                                                         example_free,
                                                         NULL);
    // The context should only fail to be created if the arguments are invalid
    // such as argc being 0, or argv containing null arguments.
    if (NULL == args_ctx)
    {
        return -1;
    }

    xo_args_arg const * const foo = xo_args_declare_arg(
        args_ctx, "foo", "f", XO_ARGS_TYPE_STRING | XO_ARGS_ARG_REQUIRED);

    xo_args_arg const * const bar = xo_args_declare_arg(
        args_ctx, "bar", "b", XO_ARGS_TYPE_STRING | XO_ARGS_ARG_OPTIONAL);

    xo_args_arg const * const baz = xo_args_declare_arg(
        args_ctx, "baz", "B", XO_ARGS_TYPE_STRING | XO_ARGS_ARG_OPTIONAL);

    // If xo_args_submit returns false then the parameters passed to the program
    // are invalid In that case the generated help text is printed to stdout and
    // it makes sense to exit with a non 0 error code in most cases
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

    size_t running_total = 0;
    for (size_t i = 0; i < g_allocation_list.allocations_size; ++i)
    {
        allocation const * const allocation = &g_allocation_list.allocations[i];
        printf("allocation [0x%p]: %zu\t(%zu)",
               allocation->memory,
               allocation->size,
               allocation->reallocations);
        if (allocation->memory == args_ctx)
        {
            printf("\txo_args_ctx* args_ctx\n");
        }
        else if (allocation->memory == foo)
        {
            printf("\txo_args_arg* foo\n");
        }
        else if (allocation->memory == bar)
        {
            printf("\txo_args_arg* bar\n");
        }
        else if (allocation->memory == baz)
        {
            printf("\txo_args_arg* baz\n");
        }
        else
        {
            char const * const mem_str = (char *)allocation->memory;
            bool is_str =
                example_is_alnum_str(mem_str, mem_str + allocation->size);
            if (is_str)
            {
                printf("\tchar* = \"%s\"\n", mem_str);
                
            }
            else
            {
                printf("\n");
            }
        }
        running_total += allocation->size;
    }

    printf("----------\nTOTAL: %zu\n----------\n", running_total);

    xo_args_destroy_ctx(args_ctx);

    running_total = 0;
    for (size_t i = 0; i < g_allocation_list.allocations_size; ++i)
    {
        allocation * allocation = &g_allocation_list.allocations[i];
        running_total += allocation->size;
    }

    printf("%zu bytes leaked\n", running_total);

    args_ctx = NULL;
    return 0;
}

#define XO_ARGS_IMPL
#include <xo-args.h>