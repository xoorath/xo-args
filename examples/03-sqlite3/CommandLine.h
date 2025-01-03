#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "OutputType.h"

////////////////////////////////////////////////////////////////////////////////
typedef struct CommandLine_s
{
    char const ** ArchiveArgs;
    size_t ArchiveArgsCount;

    bool Append;
    OutputMode_t OutputMode;
    bool Bail;
    bool Batch;
    char const * Command;
    bool Deserialize;
    bool Echo;
    char const * InitFilename;
    bool Header;
    bool Interactive;
    uint32_t LookasideEntries;
    uint32_t LookasideBytes;
    uint32_t MaxSize;
    bool MemTrace;
    uint32_t MMap;
    char const * NewLine;
    bool NoFollow;
    char const * Nonce;
    bool NoRowIDInView;
    char const * NullValue;
    uint32_t PageCacheSize;
    uint32_t PageCacheBytes;
    bool PageCacheTrace;
    bool Readonly;
    bool Safe;
    char const * Separator;
    bool Stats;
    bool UnsafeTesting;
    char const * VFS;
    bool VFSTrace;
    bool Zip;
} CommandLine_t;

//////////////////////////////////////////////////////////////////////////
CommandLine_t * CreateCommandLine(int const argc,
                                  char const * const * const argv);

//////////////////////////////////////////////////////////////////////////
void DestroyCommandLine(CommandLine_t * const cmd);