#include <inttypes.h>
#include <stdio.h>

#include "CommandLine.h"

#if defined(_MSC_VER)
#include <Windows.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// This example code implements the command line arguments for sqlite3 to show
// a reasonably complex example usage. The goal isn't to match the arguments
// or the help output exactly, just how an app developer might approach adding
// a large and varied set of arguments with nice help text and custom input
// error handling.
//
// SQLite was chosen because it is also in the public domain:
// https://sqlite.org/copyright.html
int main(int argc, char const * const * argv)
{
    char const * const mock_argv[] = {"/mock/sqlite3.exe",
                                      "--zip",
                                      "-pagecache",
                                      "5",
                                      "6",
                                      "-tabs",
                                      "-newline=\\n",
                                      "-A",
                                      "alpha",
                                      "beta",
                                      "charlie"};

    // If no arguments are provided: use the mock args instead
    if (argc == 1)
    {
        argc = (int const)(sizeof(mock_argv) / sizeof(mock_argv[0]));
        argv = (char const * const *)mock_argv;
    }

    CommandLine_t const * const cmd = CreateCommandLine(argc, argv);
    if (NULL == cmd)
    {
#if defined(_MSC_VER)
        if (IsDebuggerPresent())
        {
            system("pause");
        }
#endif
        return -1;
    }

    printf("ArchiveArgsCount = %zu\n", cmd->ArchiveArgsCount);
    for (size_t i = 0; i < cmd->ArchiveArgsCount; ++i)
    {
        printf("ArchiveArgs[%zu] = \"%s\"\n", i, cmd->ArchiveArgs[i]);
    }
    printf("Append = %s\n", cmd->Append ? "true" : "false");
    switch (cmd->OutputMode)
    {
    case OutputMode_Default:
        puts("OutputType = Default");
        break;
    case OutputMode_ASCII:
        puts("OutputType = ASCII");
        break;
    case OutputMode_Box:
        puts("OutputType = Box");
        break;
    case OutputMode_Column:
        puts("OutputType = Column");
        break;
    case OutputMode_CSV:
        puts("OutputType = CSV");
        break;
    case OutputMode_HTML:
        puts("OutputType = HTML");
        break;
    case OutputMode_JSON:
        puts("OutputType = JSON");
        break;
    case OutputMode_Line:
        puts("OutputType = Line");
        break;
    case OutputMode_List:
        puts("OutputType = List");
        break;
    case OutputMode_Markdown:
        puts("OutputType = Markdown");
        break;
    case OutputMode_Quote:
        puts("OutputType = Quote");
        break;
    case OutputMode_Table:
        puts("OutputType = Table");
        break;
    case OutputMode_Tabs:
        puts("OutputType = Tabs");
        break;
    default:
        break;
    }
    printf("Bail = %s\n", cmd->Bail ? "true" : "false");
    printf("Batch = %s\n", cmd->Batch ? "true" : "false");
    printf("Command = %s\n", cmd->Command);
    printf("Deserialize = %s\n", cmd->Deserialize ? "true" : "false");
    printf("Echo = %s\n", cmd->Echo ? "true" : "false");
    printf("InitFilename = %s\n", cmd->InitFilename);
    printf("Header = %s\n", cmd->Header ? "true" : "false");
    printf("Interactive = %s\n", cmd->Interactive ? "true" : "false");
    printf("Lookaside = SIZE %" PRIu32 " N %" PRIu32 "\n",
           cmd->LookasideEntries,
           cmd->LookasideBytes);
    printf("MaxSize = %" PRIu32 "\n", cmd->MaxSize);
    printf("MemTrace = %s\n", cmd->MemTrace ? "true" : "false");
    printf("MMap = %" PRIu32 "\n", cmd->MMap);
    printf("NewLine = %s\n", cmd->NewLine);
    printf("NoFollow = %s\n", cmd->NoFollow ? "true" : "false");
    printf("Nonce = %s\n", cmd->Nonce);
    printf("NoRowIDInView = %s\n", cmd->NoRowIDInView ? "true" : "false");
    printf("NullValue = %s\n", cmd->NullValue);
    printf("PageCache = SIZE %" PRIu32 " N %" PRIu32 "\n",
           cmd->PageCacheSize,
           cmd->PageCacheBytes);
    printf("PageCacheTrace = %s\n", cmd->PageCacheTrace ? "true" : "false");
    printf("Readonly = %s\n", cmd->Readonly ? "true" : "false");
    printf("Safe = %s\n", cmd->Safe ? "true" : "false");
    printf("Separator = %s\n", cmd->Separator);
    printf("Stats = %s\n", cmd->Stats ? "true" : "false");
    printf("UnsafeTesting = %s\n", cmd->UnsafeTesting ? "true" : "false");
    printf("VFS = %s\n", cmd->VFS);
    printf("VFSTrace = %s\n", cmd->VFSTrace ? "true" : "false");
    printf("Zip = %s\n", cmd->Zip ? "true" : "false");

    DestroyCommandLine((CommandLine_t *)cmd);
#if defined(_MSC_VER)
    if (IsDebuggerPresent())
    {
        system("pause");
    }
#endif
    return 0;
}