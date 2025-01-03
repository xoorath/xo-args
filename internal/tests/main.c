#include "utest.h"
#include <stdio.h>
#include <xo-args/xo-args.h>

#if defined(_MSC_VER)
#pragma warning(push)
// warning C4028: formal parameter 1 different from declaration
#pragma warning(disable : 4028)
#include <windows.h>
#pragma warning(pop)
#endif

int main(xo_argc_t const argc, xo_argv_t const argv)
{
    int const result = utest_main(argc, argv);
#if defined(_MSC_VER)
    if (IsDebuggerPresent())
    {
        system("pause");
    }
#endif
    return result;
}

UTEST_STATE();