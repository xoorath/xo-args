#include "CommandLine.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xo-args/xo-args.h>

char const * _DuplicateString(char const * in)
{
    size_t const len = strlen(in);
    char * buff = malloc(len + 1);
    memcpy(buff, in, len + 1);
    return buff;
}

char const ** _DuplicateStringArray(char const ** in, size_t count)
{
    char const ** newArray =
        (char const **)malloc(sizeof(char const *) * count);
    for (size_t i = 0; i < count; ++i)
    {
        newArray[i] = _DuplicateString(in[i]);
    }
    return newArray;
}

//////////////////////////////////////////////////////////////////////////
CommandLine_t * CreateCommandLine(int const argc,
                                  char const * const * const argv)
{
    CommandLine_t * cmd = (CommandLine_t *)malloc(sizeof(CommandLine_t));
    memset(cmd, 0, sizeof(CommandLine_t));

    xo_args_ctx * const ctx = xo_args_create_ctx_advanced(
        argc,
        argv,
        "sqlite3",
        "1.0.0",
        "FILENAME is the name of an SQLite database. A new database is created "
        "if the file does not previously exist. Defaults to :memory:.",
        NULL,
        NULL,
        NULL,
        NULL);

    // Declare
    //////////////////////////////////////////////////////////////////////////
    xo_args_arg const * const arg_ArchiveArgs =
        xo_args_declare_arg(ctx,
                            "A",
                            "A",
                            "ARGS...",
                            "run \".archive ARGS\" and exit",
                            XO_ARGS_TYPE_STRING_ARRAY);

    xo_args_arg const * const arg_Append =
        xo_args_declare_arg(ctx,
                            "append",
                            "append",
                            NULL,
                            "append the database to the end of the file",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_ASCII =
        xo_args_declare_arg(ctx,
                            "ascii",
                            "ascii",
                            NULL,
                            "set output mode to 'ascii'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Bail =
        xo_args_declare_arg(ctx,
                            "bail",
                            "bail",
                            NULL,
                            "stop after hitting an error",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Batch = xo_args_declare_arg(
        ctx, "batch", "batch", NULL, "force batch I/O", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Box =
        xo_args_declare_arg(ctx,
                            "box",
                            "box",
                            NULL,
                            "set output mode to 'box'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Column =
        xo_args_declare_arg(ctx,
                            "column",
                            "column",
                            NULL,
                            "set output mode to 'column'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Command =
        xo_args_declare_arg(ctx,
                            "cmd",
                            "cmd",
                            "COMMAND",
                            "run \"COMMAND\" before reading stdin",
                            XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_CSV =
        xo_args_declare_arg(ctx,
                            "csv",
                            "csv",
                            NULL,
                            "set output mode to 'csv'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Deserialize =
        xo_args_declare_arg(ctx,
                            "deserialize",
                            "deserialize",
                            NULL,
                            "open the database using sqlite3_deserialize()",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Echo =
        xo_args_declare_arg(ctx,
                            "echo",
                            "echo",
                            NULL,
                            "print inputs before execution",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_InitFilename =
        xo_args_declare_arg(ctx,
                            "init",
                            "init",
                            "FILENAME",
                            "read/process named file",
                            XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_Header = xo_args_declare_arg(
        ctx, "header", "header", NULL, "turn headers on", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_HTML =
        xo_args_declare_arg(ctx,
                            "html",
                            "html",
                            NULL,
                            "set output mode to HTML",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Interactive =
        xo_args_declare_arg(ctx,
                            "interactive",
                            "interactive",
                            NULL,
                            "force interactive I/O",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_JSON =
        xo_args_declare_arg(ctx,
                            "json",
                            "json",
                            NULL,
                            "set output mode to 'json'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Line =
        xo_args_declare_arg(ctx,
                            "line",
                            "line",
                            NULL,
                            "set output mode to 'line'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_List =
        xo_args_declare_arg(ctx,
                            "list",
                            "list",
                            NULL,
                            "set output mode to 'list'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_LookAside =
        xo_args_declare_arg(ctx,
                            "lookaside",
                            "lookaside",
                            "SIZE N",
                            "use N entries of SZ bytes for lookaside memory",
                            XO_ARGS_TYPE_INT_ARRAY);

    xo_args_arg const * const arg_Markdown =
        xo_args_declare_arg(ctx,
                            "markdown",
                            "markdown",
                            NULL,
                            "set output mode to 'markdown'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_MaxSize =
        xo_args_declare_arg(ctx,
                            "maxsize",
                            "maxsize",
                            "N",
                            "maximum size for a --deserialize database",
                            XO_ARGS_TYPE_INT);

    xo_args_arg const * const arg_MemTrace =
        xo_args_declare_arg(ctx,
                            "memtrace",
                            "memtrace",
                            NULL,
                            "trace all memory allocations and deallocations",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_MMap =
        xo_args_declare_arg(ctx,
                            "mmap",
                            "mmap",
                            "N",
                            "default mmap size set to N",
                            XO_ARGS_TYPE_INT);

    xo_args_arg const * const arg_NewLine =
        xo_args_declare_arg(ctx,
                            "newline",
                            "newline",
                            "SEP",
                            "set output row separator. Default: '\\n'",
                            XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_NoFollow =
        xo_args_declare_arg(ctx,
                            "nofollow",
                            "nofollow",
                            NULL,
                            "refuse to open symbolic links to database files",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Nonce =
        xo_args_declare_arg(ctx,
                            "nonce",
                            "nonce",
                            "STRING",
                            "set the safe-mode escape nonce",
                            XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_NoRowIDInView =
        xo_args_declare_arg(ctx,
                            "no-rowid-in-view",
                            "no-rowid-in-view",
                            NULL,
                            "Disable rowid-in-view using sqlite3_config()",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_NullValue =
        xo_args_declare_arg(ctx,
                            "nullvalue",
                            "nullvalue",
                            "TEXT",
                            "set text string for NULL values. Default ''",
                            XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_PageCache = xo_args_declare_arg(
        ctx,
        "pagecache",
        "pagecache",
        "SIZE N",
        "use N slots of SZ bytes each for page cache memory",
        XO_ARGS_TYPE_INT_ARRAY);

    xo_args_arg const * const arg_PageCacheTrace =
        xo_args_declare_arg(ctx,
                            "pcachetrace",
                            "pcachetrace",
                            NULL,
                            "trace all page cache operations",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Quote =
        xo_args_declare_arg(ctx,
                            "quote",
                            "quote",
                            NULL,
                            "set output mode to 'quote'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Readonly =
        xo_args_declare_arg(ctx,
                            "readonly",
                            "readonly",
                            NULL,
                            "open the database read-only",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Safe = xo_args_declare_arg(
        ctx, "safe", "safe", NULL, "enable safe-mode", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Separator =
        xo_args_declare_arg(ctx,
                            "separator",
                            "separator",
                            "SEP",
                            "set output column separator. Default: '|'",
                            XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_Stats =
        xo_args_declare_arg(ctx,
                            "stats",
                            "stats",
                            NULL,
                            "print memory stats before each finalize",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Table =
        xo_args_declare_arg(ctx,
                            "table",
                            "table",
                            NULL,
                            "set output mode to 'table'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Tabs =
        xo_args_declare_arg(ctx,
                            "tabs",
                            "tabs",
                            NULL,
                            "set output mode to 'tabs'",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_UnsafeTesting =
        xo_args_declare_arg(ctx,
                            "unsafe-testing",
                            "unsafe-testing",
                            NULL,
                            "allow unsafe commands and modes for testing",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_VFSName =
        xo_args_declare_arg(ctx,
                            "vfs",
                            "vfs",
                            "NAME",
                            "use NAME as the default VFS",
                            XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_VFSTrace =
        xo_args_declare_arg(ctx,
                            "vfstrace",
                            "vfstrace",
                            NULL,
                            "enable tracing of all VFS calls",
                            XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Zip =
        xo_args_declare_arg(ctx,
                            "zip",
                            "zip",
                            NULL,
                            "open the file as a ZIP Archive",
                            XO_ARGS_TYPE_SWITCH);

    // Submit
    //////////////////////////////////////////////////////////////////////////

    if (false == xo_args_submit(ctx))
    {
        xo_args_destroy_ctx(ctx);
        free(cmd);
        return NULL;
    }

    // Extract
    //////////////////////////////////////////////////////////////////////////

    if (xo_args_try_get_string_array(
            arg_ArchiveArgs, &cmd->ArchiveArgs, &cmd->ArchiveArgsCount))
    {
        cmd->ArchiveArgs =
            _DuplicateStringArray(cmd->ArchiveArgs, cmd->ArchiveArgsCount);
    }

    xo_args_try_get_bool(arg_Append, &cmd->Append);

    bool ASCII;
    if (xo_args_try_get_bool(arg_ASCII, &ASCII) && ASCII)
    {
        cmd->OutputMode |= OutputMode_ASCII;
    }

    xo_args_try_get_bool(arg_Bail, &cmd->Bail);

    xo_args_try_get_bool(arg_Batch, &cmd->Batch);

    bool box;
    if (xo_args_try_get_bool(arg_Box, &box) && box)
    {
        cmd->OutputMode |= OutputMode_Box;
    }

    bool column;
    if (xo_args_try_get_bool(arg_Column, &column) && column)
    {
        cmd->OutputMode |= OutputMode_Column;
    }

    xo_args_try_get_string(arg_Command, &cmd->Command);

    bool CSV;
    if (xo_args_try_get_bool(arg_CSV, &CSV) && CSV)
    {
        cmd->OutputMode |= OutputMode_CSV;
    }

    xo_args_try_get_bool(arg_Deserialize, &cmd->Deserialize);

    xo_args_try_get_bool(arg_Echo, &cmd->Echo);

    xo_args_try_get_string(arg_InitFilename, &cmd->InitFilename);

    xo_args_try_get_bool(arg_Header, &cmd->Header);

    bool HTML;
    if (xo_args_try_get_bool(arg_HTML, &HTML) && HTML)
    {
        cmd->OutputMode |= OutputMode_HTML;
    }

    xo_args_try_get_bool(arg_Interactive, &cmd->Interactive);

    bool JSON;
    if (xo_args_try_get_bool(arg_JSON, &JSON) && JSON)
    {
        cmd->OutputMode |= OutputMode_JSON;
    }

    bool line;
    if (xo_args_try_get_bool(arg_Line, &line) && line)
    {
        cmd->OutputMode |= OutputMode_Line;
    }

    bool list;
    if (xo_args_try_get_bool(arg_List, &list) && list)
    {
        cmd->OutputMode |= OutputMode_List;
    }

    int64_t const * lookasideArray;
    size_t lookasideArrayCount;
    if (xo_args_try_get_int_array(
            arg_LookAside, &lookasideArray, &lookasideArrayCount))
    {
        if (lookasideArrayCount != 2)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: lookaside should have two values: SZ bytes and N "
                   "entries for lookaside memory.\n");
            return NULL;
        }

        if (lookasideArray[0] <= 0 || lookasideArray[0] > UINT32_MAX)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: Invalid byte count for lookaside. Must be >0 and "
                   "<%" PRIu32 "\n",
                   UINT32_MAX);
            return NULL;
        }

        if (lookasideArray[1] <= 0 || lookasideArray[1] > UINT32_MAX)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: Invalid entry count for lookaside. Must be >0 and "
                   "<%" PRIu32 "\n",
                   UINT32_MAX);
            return NULL;
        }

        cmd->LookasideBytes = (uint32_t)lookasideArray[0];
        cmd->LookasideEntries = (uint32_t)lookasideArray[1];
    }

    bool markdown;
    if (xo_args_try_get_bool(arg_Markdown, &markdown) && markdown)
    {
        cmd->OutputMode |= OutputMode_Markdown;
    }

    int64_t maxSize = 0;
    if (xo_args_try_get_int(arg_MaxSize, &maxSize))
    {
        if (false == cmd->Deserialize)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: The maxsize option is for a --deserialize database "
                   "only.");
            return NULL;
        }
        if (maxSize <= 0 || maxSize > UINT32_MAX)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: Invalid maxsize. Must be >0 and <%" PRIu32 "\n",
                   UINT32_MAX);
            return NULL;
        }
        cmd->MaxSize = (uint32_t)maxSize;
    }

    xo_args_try_get_bool(arg_MemTrace, &cmd->MemTrace);

    int64_t MMapSize = 0;
    if (xo_args_try_get_int(arg_MMap, &maxSize))
    {
        if (MMapSize <= 0 || MMapSize > UINT32_MAX)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: Invalid mmap value. Must be >0 and <%" PRIu32 "\n",
                   UINT32_MAX);
            return NULL;
        }
        cmd->MMap = (uint32_t)MMapSize;
    }

    if (xo_args_try_get_string(arg_NewLine, &cmd->NewLine))
    {
        cmd->NewLine = _DuplicateString(cmd->NewLine);
    }
    else
    {
        cmd->NewLine = _DuplicateString("\n");
    }

    xo_args_try_get_bool(arg_NoFollow, &cmd->NoFollow);
    if (xo_args_try_get_string(arg_Nonce, &cmd->Nonce))
    {
        cmd->Nonce = _DuplicateString(cmd->Nonce);
    }

    xo_args_try_get_bool(arg_NoRowIDInView, &cmd->NoRowIDInView);

    if (xo_args_try_get_string(arg_NullValue, &cmd->NullValue))
    {
        cmd->NullValue = _DuplicateString(cmd->NullValue);
    }
    else
    {
        cmd->NullValue = _DuplicateString("");
    }

    int64_t const * pageCacheArray;
    size_t pageCacheArrayCount;
    if (xo_args_try_get_int_array(
            arg_PageCache, &pageCacheArray, &pageCacheArrayCount))
    {
        if (pageCacheArrayCount != 2)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: pagecache should have two values: SZ bytes and N "
                   "entries for page cache memory.\n");
            return NULL;
        }

        if (pageCacheArray[0] <= 0 || pageCacheArray[0] > UINT32_MAX)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: Invalid byte count for pagecache. Must be >0 and "
                   "<%" PRIu32 "\n",
                   UINT32_MAX);
            return NULL;
        }

        if (pageCacheArray[1] <= 0 || pageCacheArray[1] > UINT32_MAX)
        {
            xo_args_destroy_ctx(ctx);
            DestroyCommandLine(cmd);
            printf("Error: Invalid entry count for pagecache. Must be >0 and "
                   "<%" PRIu32 "\n",
                   UINT32_MAX);
            return NULL;
        }
        cmd->PageCacheBytes = (uint32_t)pageCacheArray[0];
        cmd->PageCacheSize = (uint32_t)pageCacheArray[1];
    }

    xo_args_try_get_bool(arg_PageCacheTrace, &cmd->PageCacheTrace);

    bool quote;
    if (xo_args_try_get_bool(arg_Quote, &quote) && quote)
    {
        cmd->OutputMode |= OutputMode_Quote;
    }

    xo_args_try_get_bool(arg_Readonly, &cmd->Readonly);

    xo_args_try_get_bool(arg_Safe, &cmd->Safe);

    if (xo_args_try_get_string(arg_Separator, &cmd->Separator))
    {
        cmd->Separator = _DuplicateString(cmd->Separator);
    }
    else
    {
        cmd->Separator = _DuplicateString("|");
    }

    xo_args_try_get_bool(arg_Stats, &cmd->Stats);

    bool table;
    if (xo_args_try_get_bool(arg_Table, &table) && table)
    {
        cmd->OutputMode |= OutputMode_Table;
    }

    bool tabs;
    if (xo_args_try_get_bool(arg_Tabs, &tabs) && tabs)
    {
        cmd->OutputMode |= OutputMode_Tabs;
    }

    xo_args_try_get_bool(arg_UnsafeTesting, &cmd->UnsafeTesting);

    if (xo_args_try_get_string(arg_VFSName, &cmd->VFS))
    {
        cmd->VFS = _DuplicateString(cmd->VFS);
    }

    xo_args_try_get_bool(arg_VFSTrace, &cmd->VFSTrace);

    xo_args_try_get_bool(arg_Zip, &cmd->Zip);

    // Additional Validations
    //////////////////////////////////////////////////////////////////////////
    size_t outputTypeTemp = cmd->OutputMode;
    size_t bits = 0;
    for (; outputTypeTemp; ++bits)
    {
        outputTypeTemp &= outputTypeTemp - 1;
    }
    if (bits > 1)
    {
        xo_args_destroy_ctx(ctx);
        DestroyCommandLine(cmd);
        printf("Error: More than one output mode set.\n");
        return NULL;
    }

    xo_args_destroy_ctx(ctx);
    return cmd;
}

//////////////////////////////////////////////////////////////////////////
void DestroyCommandLine(CommandLine_t * const cmd)
{
    if (NULL != cmd)
    {
        if (NULL != cmd->ArchiveArgs)
        {
            for (size_t i = 0; i < cmd->ArchiveArgsCount; ++i)
            {
                free((void *)cmd->ArchiveArgs[i]);
            }
            cmd->ArchiveArgsCount = 0;
            free(cmd->ArchiveArgs);
            cmd->ArchiveArgs = NULL;
        }

        if (NULL != cmd->NewLine)
        {
            free((void *)cmd->NewLine);
            cmd->NewLine = NULL;
        }

        if (NULL != cmd->Nonce)
        {
            free((void *)cmd->Nonce);
            cmd->Nonce = NULL;
        }

        if (NULL != cmd->NullValue)
        {
            free((void *)cmd->NullValue);
            cmd->NullValue = NULL;
        }

        if (NULL != cmd->Separator)
        {
            free((void *)cmd->Separator);
            cmd->Separator = NULL;
        }

        if (NULL != cmd->VFS)
        {
            free((void *)cmd->VFS);
            cmd->VFS = NULL;
        }

        free(cmd);
    }
}

#define XO_ARGS_IMPL
#include <xo-args/xo-args.h>