#include <iostream>
#include <string>

#include <xo-args.h>

///////////////////////////////////////////////////////////////////////////////
class Example3Args
{
  public:
    Example3Args(xo_argc_t const argc, xo_argv_t const argv)
        : _args_ctx(xo_args_create_ctx(argc, argv)),
          _foo(xo_args_declare_arg(
              _args_ctx,
              "foo",
              "f",
              XO_ARGS_ARG_FLAG(XO_ARGS_TYPE_STRING | XO_ARGS_ARG_REQUIRED)))
    {
    }

    ~Example3Args()
    {
        xo_args_destroy_ctx(_args_ctx);
    }

    bool Submit()
    {
        if (xo_args_submit(_args_ctx))
        {
            char const * foo_val;
            if (xo_args_try_get_string(_foo, &foo_val))
            {
                _foo_value = foo_val;
            }
            return true;
        }
        return false;
    }

    std::string const & GetFoo() const
    {
        return _foo_value;
    }

  private:
    Example3Args()
    {
    }
    Example3Args(const Example3Args &)
    {
    }

    xo_args_ctx * _args_ctx;
    xo_args_arg * _foo;
    std::string _foo_value;
};

///////////////////////////////////////////////////////////////////////////////
// Example3
int main(xo_argc_t const argc, xo_argv_t const argv)
{
    (void)argc; // unused
    (void)argv; // unused
    xo_argc_t const mock_argc = 3;
    char const * mock_argv[mock_argc];
    mock_argv[0] = "/mock/example1";
    mock_argv[1] = "--foo";
    mock_argv[2] = "this is an example.";

    Example3Args args(mock_argc, mock_argv);

    if (false == args.Submit())
    {
        return -1;
    }

    std::cout << "foo value: " << args.GetFoo() << std::endl;

    return 0;
}

#define XO_ARGS_IMPL
#include <xo-args.h>