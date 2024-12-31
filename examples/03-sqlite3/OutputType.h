#pragma once

typedef enum OutputType_e
{
    OutputType_Default = 0,

    OutputType_ASCII = 1 << 0,
    OutputType_Box = 1 << 1,
    OutputType_Column = 1 << 2,
    OutputType_CSV = 1 << 3,
    OutputType_HTML = 1 << 4,
    OutputType_JSON = 1 << 5,
    OutputType_Line = 1 << 6,
    OutputType_List = 1 << 7,
    OutputType_Markdown = 1 << 8,
    OutputType_Quote = 1 << 9,
    OutputType_Table = 1 << 10,
    OutputType_Tabs = 1 << 11
} OutputType_t;
