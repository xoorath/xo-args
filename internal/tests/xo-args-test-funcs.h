////////////////////////////////////////////////////////////////////////////////
// xo-args allows the user to provide functions for allocation and printing
// this header provides versions of those functions that have built in tracking
// and do not print to standard-out
#pragma once

#include <stdbool.h>
#include <stddef.h>

////////////////////////////////////////////////////////////////////////////////
// Creates the internal state needed for test_printf, test_alloc, test_realloc
// and test_free. Call test_global_shutdown when these functions are no longer
// needed.
void test_global_setup(void);

////////////////////////////////////////////////////////////////////////////////
// Tears down the internal state created with test_global_setup.
void test_global_shutdown(void);

////////////////////////////////////////////////////////////////////////////////
// Clears the global state (flushes stdout, allocations, asserts, etc.)
// This is similar to doing a shutdown and setup in one call - meant to be used
// in the middle of a test when testing multiple scenarios in one test.
void test_global_clear(void);

////////////////////////////////////////////////////////////////////////////////
// Prints to an internal buffer instead of stdout. That buffer can be read with
// test_get_stdout.
// limitation: anything printed with this function must be <= 4096 bytes after
// formatting has been applied.
int test_printf(char const * const fmt, ...);

////////////////////////////////////////////////////////////////////////////////
// Allocates size bytes of memory. This allocation is tracked. All tracked
// allocations can be read with test_get_allocations.
void * test_alloc(size_t size);

////////////////////////////////////////////////////////////////////////////////
// Reallocates mem to be size bytes of memory and returns the new address.
// This allocation is tracked. All tracked allocations can be read with
// test_get_allocations.
void * test_realloc(void * const mem, size_t size);

////////////////////////////////////////////////////////////////////////////////
// Frees mem. mem must be memory tracked with test_alloc / test_realloc.
// This also stops tracking of this allocation.
void test_free(void * const mem);

////////////////////////////////////////////////////////////////////////////////
// Gets the string of text printed via test_printf
char const * test_get_stdout(void);

////////////////////////////////////////////////////////////////////////////////
// Represents one tracked allocation from test_alloc/test_realloc
typedef struct allocation
{
    void * memory;
    size_t size;
    size_t reallocations;
} allocation;

////////////////////////////////////////////////////////////////////////////////
// Returns the array of allocations tracked via test_alloc/test_realloc
allocation const * test_get_allocations(size_t * const out_count);

////////////////////////////////////////////////////////////////////////////////
// Triggered by the XO_ARGS_ASSERT macro. When triggered the formatted message
// will be written to an internal buffer that can be read with
// test_get_assert_output and an internal counter will be incremented and read
// with test_get_assert_count
void test_on_assert(bool const condition, char const * const fmt, ...);

////////////////////////////////////////////////////////////////////////////////
// Gets the number of triggered asserts by test_on_assert.
size_t test_get_assert_count(void);

////////////////////////////////////////////////////////////////////////////////
// Gets the string messages of all triggered asserts separated by '\n'.
char const * test_get_assert_output(void);