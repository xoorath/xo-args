#include "CommandLine.h"

#include <exception>
#include <iostream>
#include <string>

#include <xo-args/xo-args.h>

class CommandLineImpl
{
  public:
    CommandLineImpl(int argc, char const * const * argv)
        : m_Message(), m_Repeat(10), m_Verbose(false)
    {
        xo_args_ctx * const ctx = xo_args_create_ctx_advanced(
            argc,
            argv,
            "02-cpp",
            "1.0.0",
            "This app is an example demonstration using xo-args.",
            NULL,
            NULL,
            NULL,
            NULL);

        xo_args_arg const * const argMessage = xo_args_declare_arg(
            ctx,
            "message",
            "m",
            "MSG",
            "a message to print to stdout some number of times (see: --repeat)",
            (XO_ARGS_ARG_FLAG)(XO_ARGS_TYPE_STRING | XO_ARGS_ARG_REQUIRED));

        xo_args_arg const * const argRepeat =
            xo_args_declare_arg(ctx,
                                "repeat",
                                "r",
                                "COUNT",
                                "the number of times to print the message",
                                (XO_ARGS_ARG_FLAG)XO_ARGS_TYPE_INT);

        xo_args_arg const * const argVerbose =
            xo_args_declare_arg(ctx,
                                "verbose",
                                "V",
                                NULL,
                                "print additional info",
                                (XO_ARGS_ARG_FLAG)XO_ARGS_TYPE_SWITCH);

        if (!xo_args_submit(ctx))
        {
            // This is a user error: we should catch this exception and
            // exit the program.
            throw new std::exception();
        }

        char const * message_value;
        // This try_get won't return false because the argument is marked as
        // required.
        xo_args_try_get_string(argMessage, &message_value);
        m_Message = message_value;

        // We don't care if these try_get calls return false. In that case we
        // will use the default values.
        xo_args_try_get_int(argRepeat, &m_Repeat);
        xo_args_try_get_bool(argVerbose, &m_Verbose);

        if (m_Verbose)
        {
            std::cout << "verbose = true" << std::endl
                      << "message = \"" << m_Message << "\"" << std::endl
                      << "repeat = " << m_Repeat << std::endl;
        }

        xo_args_destroy_ctx(ctx);
    }

    bool GetVerbose() const
    {
        return m_Verbose;
    }

    const std::string & GetMessage() const
    {
        return m_Message;
    }

    int64_t GetRepeat() const
    {
        return m_Repeat;
    }

  private:
    std::string m_Message;
    int64_t m_Repeat;
    bool m_Verbose;
};

CommandLine::CommandLine(int argc, char const * const * argv)
    : m_Impl(new CommandLineImpl(argc, argv))
{
}

CommandLine::~CommandLine()
{
    delete m_Impl;
}

bool CommandLine::GetVerbose() const
{
    return m_Impl->GetVerbose();
}

const std::string & CommandLine::GetMessage() const
{
    return m_Impl->GetMessage();
}

int64_t CommandLine::GetRepeat() const
{
    return m_Impl->GetRepeat();
}

/*private*/ CommandLine::CommandLine()
{
}
/*private*/ CommandLine::CommandLine(const CommandLine &)
{
}
/*private*/ CommandLine & CommandLine::operator=(CommandLine &)
{
    return *this;
}

#define XO_ARGS_IMPL
#include <xo-args/xo-args.h>