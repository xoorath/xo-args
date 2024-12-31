#include "CommandLine.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xo-args.h>

char const * _DuplicateString(char const * in)
{
    size_t const len = strlen(in);
    char * buff = malloc(len+1);
    memcpy(buff, in, len+1);
    return buff;
}

char const ** _DuplicateStringArray(char const ** in, size_t count)
{
    char const ** newArray = (char const **)malloc(sizeof(char const *) * count);
    for (size_t i = 0; i < count; ++i)
    {
        newArray[i] = _DuplicateString(in[i]);
    }
    return newArray;
}

//////////////////////////////////////////////////////////////////////////
CommandLine_t * CreateCommandLine(int const argc, char const * const * const argv)
{
    CommandLine_t * cmd = (CommandLine_t *)malloc(sizeof(CommandLine_t));
    memset(cmd, 0, sizeof(CommandLine_t));

    xo_args_ctx * const ctx = xo_args_create_ctx_advanced(
        argc, argv, "sqlite3", "1.0.0", NULL, NULL, NULL, NULL, NULL);

    // Declare
    //////////////////////////////////////////////////////////////////////////
    xo_args_arg const * const arg_ArchiveArgs =
        xo_args_declare_arg(ctx, "A", "A", XO_ARGS_TYPE_STRING_ARRAY);

    xo_args_arg const * const arg_Append =
        xo_args_declare_arg(ctx, "append", "append", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_ASCII =
        xo_args_declare_arg(ctx, "ascii", "ascii", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Bail =
        xo_args_declare_arg(ctx, "bail", "bail", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Batch =
        xo_args_declare_arg(ctx, "batch", "batch", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Box =
        xo_args_declare_arg(ctx, "box", "box", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Column =
        xo_args_declare_arg(ctx, "column", "column", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Command =
        xo_args_declare_arg(ctx, "cmd", "cmd", XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_CSV =
        xo_args_declare_arg(ctx, "csv", "csv", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Deserialize = xo_args_declare_arg(
        ctx, "deserialize", "deserialize", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Echo =
        xo_args_declare_arg(ctx, "echo", "echo", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_InitFilename =
        xo_args_declare_arg(ctx, "init", "init", XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_Header =
        xo_args_declare_arg(ctx, "header", "header", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_HTML =
        xo_args_declare_arg(ctx, "html", "html", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Interactive = xo_args_declare_arg(
        ctx, "interactive", "interactive", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_JSON =
        xo_args_declare_arg(ctx, "json", "json", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Line =
        xo_args_declare_arg(ctx, "line", "line", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_List =
        xo_args_declare_arg(ctx, "list", "list", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_LookAside = xo_args_declare_arg(
        ctx, "lookaside", "lookaside", XO_ARGS_TYPE_INT_ARRAY);

    xo_args_arg const * const arg_Markdown =
        xo_args_declare_arg(ctx, "markdown", "markdown", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_MaxSize =
        xo_args_declare_arg(ctx, "maxsize", "maxsize", XO_ARGS_TYPE_INT);

    xo_args_arg const * const arg_MemTrace =
        xo_args_declare_arg(ctx, "memtrace", "memtrace", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_MMap =
        xo_args_declare_arg(ctx, "mmap", "mmap", XO_ARGS_TYPE_INT);

    xo_args_arg const * const arg_NewLine =
        xo_args_declare_arg(ctx, "newline", "newline", XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_NoFollow =
        xo_args_declare_arg(ctx, "nofollow", "nofollow", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Nonce =
        xo_args_declare_arg(ctx, "nonce", "nonce", XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_NoRowIDInView = xo_args_declare_arg(
        ctx, "no-rowid-in-view", "no-rowid-in-view", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_NullValue = xo_args_declare_arg(
        ctx, "nullvalues", "nullvalues", XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_PageCache = xo_args_declare_arg(
        ctx, "pagecache", "pagecache", XO_ARGS_TYPE_INT_ARRAY);

    xo_args_arg const * const arg_PageCacheTrace = xo_args_declare_arg(
        ctx, "pcachetrace", "pcachetrace", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Quote =
        xo_args_declare_arg(ctx, "quote", "quote", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Readonly =
        xo_args_declare_arg(ctx, "readonly", "readonly", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Safe =
        xo_args_declare_arg(ctx, "safe", "safe", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Separator =
        xo_args_declare_arg(ctx, "separator", "separator", XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_Stats =
        xo_args_declare_arg(ctx, "stats", "stats", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Table =
        xo_args_declare_arg(ctx, "table", "table", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Tabs =
        xo_args_declare_arg(ctx, "tabs", "tabs", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_UnsafeTesting = xo_args_declare_arg(
        ctx, "unsafe-testing", "unsafe-testing", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_VFSName =
        xo_args_declare_arg(ctx, "vfs", "vfs", XO_ARGS_TYPE_STRING);

    xo_args_arg const * const arg_VFSTrace =
        xo_args_declare_arg(ctx, "vfstrace", "vfstrace", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * const arg_Zip =
        xo_args_declare_arg(ctx, "zip", "zip", XO_ARGS_TYPE_SWITCH);

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
        cmd->OutputType |= OutputType_ASCII;
    }

    xo_args_try_get_bool(arg_Bail, &cmd->Bail);

    xo_args_try_get_bool(arg_Batch, &cmd->Batch);

    bool box;
    if (xo_args_try_get_bool(arg_Box, &box) && box)
    {
        cmd->OutputType |= OutputType_Box;
    }

    bool column;
    if (xo_args_try_get_bool(arg_Column, &column) && column)
    {
        cmd->OutputType |= OutputType_Column;
    }

    xo_args_try_get_string(arg_Command, &cmd->Command);

    bool CSV;
    if (xo_args_try_get_bool(arg_CSV, &CSV) && CSV)
    {
        cmd->OutputType |= OutputType_CSV;
    }

    xo_args_try_get_bool(arg_Deserialize, &cmd->Deserialize);

    xo_args_try_get_bool(arg_Echo, &cmd->Echo);

    xo_args_try_get_string(arg_InitFilename, &cmd->InitFilename);

    xo_args_try_get_bool(arg_Header, &cmd->Header);

    bool HTML;
    if (xo_args_try_get_bool(arg_HTML, &HTML) && HTML)
    {
        cmd->OutputType |= OutputType_HTML;
    }

    xo_args_try_get_bool(arg_Interactive, &cmd->Interactive);

    bool JSON;
    if (xo_args_try_get_bool(arg_JSON, &JSON) && JSON)
    {
        cmd->OutputType |= OutputType_JSON;
    }

    bool line;
    if (xo_args_try_get_bool(arg_Line, &line) && line)
    {
        cmd->OutputType |= OutputType_Line;
    }

    bool list;
    if (xo_args_try_get_bool(arg_List, &list) && list)
    {
        cmd->OutputType |= OutputType_List;
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
        cmd->OutputType |= OutputType_Markdown;
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
        cmd->OutputType |= OutputType_Quote;
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
        cmd->OutputType |= OutputType_Table;
    }

    bool tabs;
    if (xo_args_try_get_bool(arg_Tabs, &tabs) && tabs)
    {
        cmd->OutputType |= OutputType_Tabs;
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
    size_t outputTypeTemp = cmd->OutputType;
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
                free((void*)cmd->ArchiveArgs[i]);
            }
            cmd->ArchiveArgsCount = 0;
            free(cmd->ArchiveArgs);
            cmd->ArchiveArgs = NULL;
        }

        if (NULL != cmd->NewLine)
        {
            free((void*)cmd->NewLine);
            cmd->NewLine = NULL;
        }

        if (NULL != cmd->Nonce)
        {
            free((void*)cmd->Nonce);
            cmd->Nonce = NULL;
        }

        if (NULL != cmd->NullValue)
        {
            free((void*)cmd->NullValue);
            cmd->NullValue = NULL;
        }

        if (NULL != cmd->Separator)
        {
            free((void*)cmd->Separator);
            cmd->Separator = NULL;
        }

        if (NULL != cmd->VFS)
        {
            free((void*)cmd->VFS);
            cmd->VFS = NULL;
        }

        free(cmd);
    }
}

#define XO_ARGS_IMPL
#include <xo-args.h>