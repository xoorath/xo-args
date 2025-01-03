
#include "./xo-args-test-funcs.h"

#define XO_ARGS_ASSERT(condition, ...)                                         \
    test_on_assert(!!(condition), __VA_ARGS__)
#define XO_ARGS_IMPL
#include <xo-args/xo-args.h>
