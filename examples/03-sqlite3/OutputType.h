#pragma once

typedef enum OutputMode_e
{
    OutputMode_Default = 0,

    OutputMode_ASCII = 1 << 0,
    OutputMode_Box = 1 << 1,
    OutputMode_Column = 1 << 2,
    OutputMode_CSV = 1 << 3,
    OutputMode_HTML = 1 << 4,
    OutputMode_JSON = 1 << 5,
    OutputMode_Line = 1 << 6,
    OutputMode_List = 1 << 7,
    OutputMode_Markdown = 1 << 8,
    OutputMode_Quote = 1 << 9,
    OutputMode_Table = 1 << 10,
    OutputMode_Tabs = 1 << 11
} OutputMode_t;
