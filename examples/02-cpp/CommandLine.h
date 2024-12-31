#pragma once

#include <inttypes.h>
#include <string>

class CommandLineImpl;

class CommandLine
{
  public:
    CommandLine(int argc, char const * const * argv);
    ~CommandLine();

    bool GetVerbose() const;
    const std::string & GetMessage() const;
    int64_t GetRepeat() const;

  private:
    CommandLine();
    CommandLine(const CommandLine &);
    CommandLine & operator=(CommandLine &);
    CommandLineImpl * m_Impl;
};