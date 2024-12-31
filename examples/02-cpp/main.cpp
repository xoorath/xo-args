#include <iostream>

#include "./CommandLine.h"

int main(int argc, char ** argv)
{
    // We'll use mock data for argc/argv in this example
    (void)argc; // unused
    (void)argv; // unused
    try
    {
        char const * const mock_argv[] = {"/mock/cpp.exe",
                                          "-m",
                                          "Hello World!",
                                          "-r=5",
                                          "-V"};
        int const mock_argc = sizeof(mock_argv) / sizeof(mock_argv[0]);

        CommandLine cmd(mock_argc, mock_argv);

        for (int64_t i = 0; i < cmd.GetRepeat(); ++i)
        {
            std::cout << cmd.GetMessage() << std::endl;
        }
    }
    catch (std::exception*)
    {
        return -1;
    }
    return 0;
}