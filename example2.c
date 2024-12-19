#include <stdlib.h>
#include <string.h>

#define XO_ARGS_IMPL
#include "xo-args.h"

typedef struct allocation
{
    void* mem;
    size_t sz;
    size_t re;
} allocation;

typedef struct allocation_list
{
    allocation* allocations;
    size_t allocations_size;
    size_t allocations_reserved;

} allocation_list;

allocation_list g_allocation_list;

void* example2_alloc(size_t size)
{
    if (g_allocation_list.allocations_reserved == g_allocation_list.allocations_size)
    {
        g_allocation_list.allocations_reserved *= 2;
        g_allocation_list.allocations = realloc(g_allocation_list.allocations,
                                                g_allocation_list.allocations_reserved * sizeof(allocation));
    }

    void* mem = malloc(size);

    g_allocation_list.allocations[g_allocation_list.allocations_size].mem = mem;
    g_allocation_list.allocations[g_allocation_list.allocations_size].sz = size;
    g_allocation_list.allocations[g_allocation_list.allocations_size].re = 0;
    g_allocation_list.allocations_size++;

    return mem;
}

void* example2_realloc(void* mem, size_t size)
{
    for (size_t i = 0; i < g_allocation_list.allocations_size; ++i)
    {
        if(g_allocation_list.allocations[i].mem == mem)
        {
            printf("realloc %zu -> %zu\n", g_allocation_list.allocations[i].sz, size);
            g_allocation_list.allocations[i].sz = size;
            g_allocation_list.allocations[i].re++;
            void * new_mem = realloc(mem, size);
            g_allocation_list.allocations[i].mem = new_mem;
            return new_mem;
        }
    }
    assert(false);
    return NULL;
}

void example2_free(void* mem)
{
    for (size_t i = 0; i < g_allocation_list.allocations_size; ++i)
    {
        if(g_allocation_list.allocations[i].mem == mem)
        {
            allocation* curr = &g_allocation_list.allocations[i];
            allocation* last = &g_allocation_list.allocations[g_allocation_list.allocations_size-1];
            if (curr != last)
            {
                memcpy(curr, last, sizeof(allocation));
            }
            --g_allocation_list.allocations_size;

            break;
        }
    }
    free(mem);
}

///////////////////////////////////////////////////////////////////////////////
// Example1
int main(xo_argc_t const argc, xo_argv_t const argv)
{
    (void)argc; // unused
    (void)argv; // unused

    g_allocation_list.allocations_reserved = 32;
    g_allocation_list.allocations_size = 0;
    g_allocation_list.allocations = malloc(g_allocation_list.allocations_reserved * sizeof(allocation));

    xo_argc_t const mock_argc = 3;
    char const * mock_argv[mock_argc];
    mock_argv[0] = "/mock/example2";
    mock_argv[1] = "--foo";
    mock_argv[2] = "this is another example.";

    xo_args_ctx* args_ctx = xo_args_create_ctx_advanced(mock_argc,
                                                        mock_argv,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        example2_alloc,
                                                        example2_realloc,
                                                        example2_free,
                                                        NULL);
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

    xo_args_declare_arg(args_ctx, 
                        "bar", 
                        "b", 
                        XO_ARGS_TYPE_STRING
                        | XO_ARGS_ARG_OPTIONAL);

    xo_args_declare_arg(args_ctx, 
                        "baz", 
                        "B", 
                        XO_ARGS_TYPE_STRING
                        | XO_ARGS_ARG_OPTIONAL);

    // If xo_args_submit returns false then the parameters passed to the program are invalid
    // In that case the generated help text is printed to stdout and it makes sense to exit 
    // with a non 0 error code in most cases  
    if (false == xo_args_submit(args_ctx))
    {
        xo_args_destroy_ctx(args_ctx);
        args_ctx = NULL;
        return -1;
    }

    char* foo_value;
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
        allocation* allocation = &g_allocation_list.allocations[i];
        char const * const mem_str = (char*)allocation->mem;
        bool is_str = true;
        for(size_t j = 0; j < allocation->sz-1; ++j)
        {
            if (false == isalnum(mem_str[j]))
            {
                is_str = false;
                break;
            }
        }
        if (is_str)
        {
            printf("allocation [0x%p]: %zu\t(%zu)\t\"%s\"\n", allocation->mem, allocation->sz, allocation->re,  mem_str);
        }
        else
        {
            printf("allocation [0x%p]: %zu\t(%zu)\n",  allocation->mem, allocation->sz, allocation->re);
        }
        running_total += allocation->sz;
    }

    printf("----------\nTOTAL: %zu\n----------\n", running_total);

    xo_args_destroy_ctx(args_ctx);

    running_total = 0;
    for (size_t i = 0; i < g_allocation_list.allocations_size; ++i)
    {
        allocation* allocation = &g_allocation_list.allocations[i];
        running_total += allocation->sz;
    }

    printf("%zu bytes leaked\n", running_total);

    args_ctx = NULL;
    return 0;

}