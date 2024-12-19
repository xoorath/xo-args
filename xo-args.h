///////////////////////////////////////////////////////////////////////////////
// xo-args.h
//
// xo-args.h is a single header file library for C and C++ designed to help
// process command line arguments in a way that's reasonably consistent with 
// popular standards.
// See: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
// See: https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html 
//
// Author:              Jared Thomson
// Project homepage:    https://git.merveilles.town/xo/xo-args
// License File:        See License.md
// License URL:         https://creativecommons.org/publicdomain/zero/1.0/
///////////////////////////////////////////////////////////////////////////////
// The anatomy of command line arguments (examples):
//
// program.exe --foo FOO -b BAR -z
//               ^   ^    ^ ^    ^ 
//               |   |    | |    'z' is a switch and does not require a value 
//               |   |    | |    to follow. If `-z` is provided then `z` is
//               |   |    | |    true, otherwise it is false.
//               |   |    | |
//               |   |    | 'BAR' is the string value for a variable with a
//               |   |    | short name 'b'
//               |   |    |
//               |   |    'b' is a short name alternative for a variable. 
//               |   |
//               |   'FOO' is the string value for a variable with the name
//               |   'foo'
//               |
//               'foo' is the name of a variable.
//
// program.exe --foo
//               ^
//               Assuming foo has the type 'string' like the last example: then
//               this example would produce an error because foo has no value. 
//               The help text would be printed and the program would exit with
//               an error code.
// 
// program.exe --foo FOO --foo FOO
//                         ^
//                         Foo has already been set so this would also 
//                         produce an error.
///////////////////////////////////////////////////////////////////////////////
// Quick start guide:
//
// 1. In a single translation unit (C or C++ file) define XO_ARGS_IMPL and
// include xo-args.h.
//      #define XO_ARGS_IMPL // only do this in one c or cpp file
//      #include "xo-args.h"
//
// 2. Setup an xo-args context in your entry point and define any arguments.
//      xo_args_ctx * ctx = xo_args_create_ctx(argc, argv);
//      xo_args_arg * foo = xo_args_define_arg("foo", "f", XO_ARGS_TYPE_BOOL);
//
// 3. Conclude setup and check to see if the program arguments were valid based
// on your earlier definition. If the arguments aren't valid you can exit the
// program with an error code. Don't forget to destroy the context before you 
// quit for a clean exit without memory leaks.
//      if (false == xo_args_end_init(ctx))
//      {
//          xo_args_destroy_ctx(ctx);
//          return -1;
//      }
//
//  4. After you've concluded the setup with xo_args_end_init, if it returned 
// true you are free to start using your arguments and destroy the context 
// whenever you're done. Once the context is destroyed all of the args you
// defined earlier will be destroyed as well.
//      if (true == xo_args_has_value(foo))
//      {
//          printf("foo: %i\n", xo_args_get_bool(foo));
//      }
//      // ... the rest of the program
//      xo_args_destroy_ctx(ctx); // foo and ctx are now both freed
///////////////////////////////////////////////////////////////////////////////
// Implementation notes:
// 
// An argument name is not optional but short names are optional. There is no
// enforcement of how long or short a name or short name can or should be.
// Switches are an exception to this rule: a name is not required so long as a
// short name is provided.
//
// There is no chaining of switches. For example -abc is not equivolent to
// -a -b -c.
// 
// Arrays can be defined two ways: by setting a variable multiple times or by
// values continuously until a valid argument is encountered. For example:
// [-foo 1 -foo 2 -foo 3 --bar BAR] will have an array named foo contain the
// values 1, 2 and 3 as well as a variable bar with a value of BAR. 
// [-foo 1 2 3 --bar BAR] is equivolent. If there was no variable named bar
// declared then foo would have the values: 1, 2, 3, --bar, BAR.
// 
///////////////////////////////////////////////////////////////////////////////
#if !defined(__XO_ARGS_H__)
#define __XO_ARGS_H__

#include <inttypes.h>
#if !defined(__cplusplus)
#include <stdbool.h>
#endif // !defined(__cplusplus)
#include <stddef.h>

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

///////////////////////////////////////////////////////////////////////////////
// Types

// entry point argc
typedef int xo_argc_t;

// entry point argv type
typedef char const * const * xo_argv_t;

// typedefs to allow the user to change our allocation and print functions
// the defaults when not provided are: malloc, realloc, free and printf
typedef void*(*xo_args_alloc_fn)(size_t);
typedef void*(*xo_args_realloc_fn)(void*, size_t);
typedef void(*xo_args_free_fn)(void*);
typedef int(*xo_args_print_fn)(char const *, ...);

// An opaque context structure to hold implementation details of xo-args
typedef struct xo_args_ctx xo_args_ctx;

// An opaque structure to hold implementation details of a single argument
typedef struct xo_args_arg xo_args_arg;

typedef enum XO_ARGS_ARG_FLAG
{
    // If no type flag is set the default is string
    XO_ARGS_TYPE_STRING       = 1<<0,
    XO_ARGS_TYPE_SWITCH       = 1<<1,
    XO_ARGS_TYPE_BOOL         = 1<<2,
    XO_ARGS_TYPE_INT          = 1<<3,
    XO_ARGS_TYPE_STRING_ARRAY = 1<<4,
    XO_ARGS_TYPE_BOOL_ARRAY   = 1<<5,
    XO_ARGS_TYPE_INT_ARRAY    = 1<<6,

    XO_ARGS_ARG_OPTIONAL = 0,
    XO_ARGS_ARG_REQUIRED = 1<<7
} XO_ARGS_ARG_FLAG;

///////////////////////////////////////////////////////////////////////////////
xo_args_ctx* xo_args_create_ctx_advanced(xo_argc_t const argc,
                                         xo_argv_t const argv,
                                         char const * const app_name,
                                         char const * const app_version,
                                         char const * const app_documentation,
                                         xo_args_alloc_fn const alloc_fn,
                                         xo_args_realloc_fn const realloc_fn,
                                         xo_args_free_fn const free_fn,
                                         xo_args_print_fn const print_fn);

///////////////////////////////////////////////////////////////////////////////
xo_args_ctx* xo_args_create_ctx(xo_argc_t const argc,
                                xo_argv_t const argv);

///////////////////////////////////////////////////////////////////////////////
// xo_args_submit concludes the setup of xo-args and parses all arguments. 
// If xo_args_submit returns true: the arguments are valid and can be used.
// If xo_args_submit returns false: the arguments are invalid, the help text
// will have been printed, and the program may now exit with an error code.
bool xo_args_submit(xo_args_ctx * const context);

///////////////////////////////////////////////////////////////////////////////
void xo_args_destroy_ctx(xo_args_ctx * const context);

///////////////////////////////////////////////////////////////////////////////
xo_args_arg * xo_args_declare_arg(xo_args_ctx * const context,
                                  char const * const name,
                                  char const * const short_name,
                                  XO_ARGS_ARG_FLAG const flags);

///////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_string(xo_args_arg const * const arg, 
                            char const ** out_string);

///////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_int(xo_args_arg const * const arg, int * out_int);

///////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_bool(xo_args_arg const * const arg, bool * out_bool);

#if defined(__cplusplus)
} // extern "C"
#endif // defined(__cplusplus)
#endif // __XO_ARGS_H__
///////////////////////////////////////////////////////////////////////////////
// XO_ARGS_IMPL should be defined by hand in one C file before including xo-args.h
#if defined(XO_ARGS_IMPL)

#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#define XO_ARGS_XSTR(a) XO_ARGS_STR(a)
#define XO_ARGS_STR(a) #a

// __FUNCTION__ should be fairly reliabe but it's not perfectly portable
// This define is just a fallback in those cases.
#if !defined(__FUNCTION__)
    #define __FUNCTION__  __FILE__ "(" XO_ARGS_XSTR(__LINE__) ")"
#endif

#if !defined(XO_ARGS_ASSERT)
#include <assert.h>
#define XO_ARGS_ASSERT(condition, message) assert(((condition)) && ("xo-args assert " __FUNCTION__ ": " message))
#endif

#if defined(_WIN32)
char const g_xo_args_path_separators[3] = "/\\";
#else
char const g_xo_args_path_separators[2] = "/";
#endif

#if !defined(min)
#define min(x, y) ((x <= y ? x : y))
#endif

///////////////////////////////////////////////////////////////////////////////
typedef struct xo_args_arg
{

    char const * name;
    char const * short_name;

    // The flags tracked so far.
    XO_ARGS_ARG_FLAG flags;

    // has_value is unset until parsed
    bool has_value;
} xo_args_arg;

///////////////////////////////////////////////////////////////////////////////
typedef struct _xo_args_arg_single
{
    xo_args_arg base;
    union
    {
        bool _bool;
        int _int;
        char* _string;
    } value;
} _xo_args_arg_single;

///////////////////////////////////////////////////////////////////////////////
typedef struct _xo_args_arg_array
{
    xo_args_arg base;
    void** array;
    size_t array_reserved;
    size_t array_size;
} _xo_args_arg_array;


///////////////////////////////////////////////////////////////////////////////
typedef struct xo_args_ctx
{
    xo_argc_t argc;
    xo_argv_t argv;
    xo_args_alloc_fn alloc;
    xo_args_realloc_fn realloc;
    xo_args_free_fn free;
    xo_args_print_fn print;
    char const * app_name;
    char const * app_version;
    char const * app_documentation;
    
    // A list of all allocations to free later in xo_args_cleanup
    void** allocations;
    size_t allocations_reserved; // number of allocated elements in allocations
    size_t allocations_size;     // actual size

    // A list of arguments. This is not a tracked allocation but all
    // arguments in this list are tracked allocations.
    xo_args_arg** args;
    size_t args_reserved;
    size_t args_size;
} xo_args_ctx;

///////////////////////////////////////////////////////////////////////////////
void* _xo_args_tracked_alloc(xo_args_ctx * const context, size_t const size)
{
    if (context->allocations_reserved == context->allocations_size)
    {
        context->allocations_reserved *= 2;
        context->allocations = (void**)context->realloc(context->allocations, 
                                                        context->allocations_reserved 
                                                        * sizeof(void*));
    }
    void* const mem = context->alloc(size);
    context->allocations[context->allocations_size++] = mem;
    return mem;
}

///////////////////////////////////////////////////////////////////////////////
void* _xo_args_tracked_realloc(xo_args_ctx * const context, void * const mem, size_t const size)
{
    for (size_t i = 0; i < context->allocations_size; ++i)
    {
        if (mem == context->allocations[i])
        {
            context->allocations[i] = context->realloc(mem, size);
            return context->allocations[i];
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void _xo_args_tracked_free(xo_args_ctx * const context, void * const mem)
{
    // We will free the memory and stop tracking it, but we do this with a last-swap
    // this lets us avoid shuffling elements down since order of our tracked allocation
    // list is not important to us.
    for (size_t i = 0; i < context->allocations_size; ++i)
    {
        if (context->allocations[i] == mem)
        {
            context->free(mem);
            // decrement the size and now allocations_size is the previous last element index
            --context->allocations_size;

            if (i != context->allocations_size)
            {
                context->allocations[i] = context->allocations[context->allocations_size];
            }
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void _xo_args_arg_array_init(xo_args_ctx * const context, 
                            _xo_args_arg_array * const array,
                            size_t const value_size)
{
    array->array_reserved = 2;
    array->array_size = 0;
    array->array = _xo_args_tracked_alloc(context,
                                          value_size
                                          * array->array_reserved);
}

///////////////////////////////////////////////////////////////////////////////
void _xo_args_arg_array_push(xo_args_ctx * const context, 
                            _xo_args_arg_array * const array,
                            void * const value,
                            size_t const value_size)
{
    if (0 == array->array_reserved)
    {
        _xo_args_arg_array_init(context, array, value_size);
    }

    if (array->array_reserved == array->array_size)
    {
        array->array_reserved *= 2;
        array->array = _xo_args_tracked_realloc(context, 
                                                array->array,
                                                array->array_reserved 
                                                * value_size);
    }
    memcpy(((char*)array->array) + (value_size * array->array_size++),
           value,
           value_size);
}

///////////////////////////////////////////////////////////////////////////////
// The basename of a path is the filename with no path or extension(s)
// Examples: 
//      /a/b/c.e -> c
//      /a/b/c -> c
//      /a/b/c.e.f -> c
//      /a/b/c..f -> c
//      /a/b/c.. -> c
//      /a/b/c/ -> c
//      / -> 
//      /a/b/c// ->
//
// This function returns the basename of path newly allocated with the context's
// alloc function.
//
// Example usage:
//      char const *const foo = "a/b/ccc.e";
//      size_t const basenameLen = _xo_args_basename(foo, NULL);
//      char const * bar = malloc(basenameLen+1);
//      _xo_args_basename(foo, bar);
//      printf("foo: %s\nbar: %s\n", foo, bar); // prints "foo: a/b/ccc.e\nbar: ccc\n"
//      free(bar);
char const* _xo_args_basename(xo_args_ctx * const context, char const * const path)
{
    if (NULL == path)
    {
        return NULL;
    }

    size_t const path_len = strlen(path);
    if (0 == path_len)
    {
        return NULL;
    }

    char const * const end = path + path_len;
    char const * it = end;
    char const * basename_start = path;
    while (it != path)
    {
        bool is_path_sep = false;
        for (size_t i = 0; i < sizeof(g_xo_args_path_separators)-1; ++i)
        {
            is_path_sep = is_path_sep || *it == g_xo_args_path_separators[i];
        }

        if(is_path_sep)
        {
            // The min is to handle the edge case where the last character is a path sep
            // we don't accidentally want to advance past the end.
            basename_start = min(it + 1, end);
            break;
        }
        --it;
    }
    it = basename_start;
    char const* basename_end = basename_start;
    while (it != end)
    {
        if (*it == '.')
        {
            break;
        }
        basename_end = it;
        ++it;
    }
    
    // both basename_start and basename_end are inclusive so we need to add 1
    //      "example"
    //       ^     ^
    //       |     basename_end
    //       basename_start
    // basename_end - basename_start == 6
    // strlen("example") == 7
    size_t const basename_length = (basename_end - basename_start) + 1;
    if (basename_length >= 1)
    {
        char* buff = (char*)_xo_args_tracked_alloc(context, basename_length+1); 
        memcpy(buff, basename_start, basename_length);
        buff[basename_length] = '\0'; 
        return buff;
    }
    // There is no string to copy.
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void _xo_print_try_help(xo_args_ctx const * const context)
{
    XO_ARGS_ASSERT(context, "xo_args_ctx must not be null here.");
    context->print("Try: %s --help\n", context->app_name);
}

///////////////////////////////////////////////////////////////////////////////
void _xo_print_help(xo_args_ctx const * const context)
{
    XO_ARGS_ASSERT(context, "xo_args_ctx must not be null here.");

    if (NULL != context->app_version)
    {
        context->print("%s version %s\n", context->app_name, context->app_version);
    }
    else
    {
        context->print("%s\n", context->app_name);
    }

    context->print("Usage: %s", context->app_name);

    bool any_required = false;
    for (size_t i = 0; i < context->args_size; ++i)
    {
        xo_args_arg const * const arg = context->args[i];
        if (arg->flags & XO_ARGS_ARG_REQUIRED)
        {
            context->print(" --%s", arg->name);
            any_required = true;
        }
    }

    bool any_optional = false;
    for (size_t i = 0; i < context->args_size; ++i)
    {
        xo_args_arg const * const arg = context->args[i];
        if (false == !!(arg->flags & XO_ARGS_ARG_REQUIRED))
        {
            any_optional = true;
        }
    }

    if (any_optional)
    {
        context->print(" [OPTION]...\n");
    }

    if (NULL != context->app_documentation)
    {
        context->print("DOCUMENTATION\n%s\n", context->app_documentation);
    }

    if (any_required)
    {
        for (size_t i = 0; i < context->args_size; ++i)
        {
            xo_args_arg const * const arg = context->args[i];
            if (arg->flags & XO_ARGS_ARG_REQUIRED)
            {
                if (arg->short_name != NULL)
                {
                    printf("\t-%s, --%s\n", arg->short_name, arg->name);
                }
                else
                {
                    printf("\t--%s\n", arg->name);
                }

                if (arg->flags & XO_ARGS_TYPE_STRING)
                {
                    printf("\t\tstring\n");
                }
                else if (arg->flags & XO_ARGS_TYPE_INT)
                {
                    printf("\t\tinteger\n");
                }
                else if (arg->flags & XO_ARGS_TYPE_BOOL)
                {
                    printf("\t\ttrue|false\n");
                }
                else if (arg->flags & XO_ARGS_TYPE_SWITCH)
                {
                    printf("\t\tenables this switch\n");
                }
            }
        }
    }

}

///////////////////////////////////////////////////////////////////////////////
xo_args_ctx* xo_args_create_ctx_advanced(xo_argc_t const argc,
                                         xo_argv_t const argv,
                                         char const * const app_name,
                                         char const * const app_version,
                                         char const * const app_documentation,
                                         xo_args_alloc_fn const alloc_fn,
                                         xo_args_realloc_fn const realloc_fn,
                                         xo_args_free_fn const free_fn,
                                         xo_args_print_fn const print_fn)
{
    if (argc < 1)
    {
        XO_ARGS_ASSERT(argc >= 1, "argc is expected to be >= 1");
        (print_fn != NULL ? print_fn : printf)("xo-args error: " __FUNCTION__ 
                                               " argc is expected to be >= 1 but was %i\n", argc);
        return NULL;
    }
    
    if (NULL == argv)
    {
        XO_ARGS_ASSERT(NULL != argv, "argv is required");
        (print_fn != NULL ? print_fn : printf)("xo-args error: " __FUNCTION__ 
                                               " argv is required\n");
        return NULL;
    }

    {
        bool any_arg_is_null = false;
        for (int i = 0; i < argc; ++i)
        {
            if (NULL == argv[i])
            {
                (print_fn != NULL ? print_fn : printf)("xo-args error: " __FUNCTION__ 
                                                       " argv[%i] was NULL\n", i);
                any_arg_is_null = true;
            }
        }

        if (any_arg_is_null)
        {
            XO_ARGS_ASSERT(false == any_arg_is_null, "one or more arguments in argv was null");
            return NULL;
        }
    }

    xo_args_ctx* const context = (xo_args_ctx*)(NULL != alloc_fn 
        ? alloc_fn(sizeof(xo_args_ctx))
        : malloc(sizeof(xo_args_ctx)));

    context->argc = argc;
    context->argv = argv;
    context->print = NULL == print_fn ? printf : print_fn;
    context->alloc = NULL == alloc_fn ? malloc : alloc_fn;
    context->realloc = NULL == realloc_fn ? realloc : realloc_fn;
    context->free = NULL == free_fn ? free : free_fn;
    context->allocations_size = 0;
    context->allocations_reserved = 8;
    context->allocations = (void**)context->alloc(context->allocations_reserved * sizeof(void*));

    // Default app_name is the filename parsed from argv[0]
    if (NULL == app_name)
    {
        context->app_name = _xo_args_basename(context, argv[0]);
        if (NULL == context->app_name)
        {
            // We never free app_name directly so this assignment is safe
            context->app_name = "app";
        }
    }
    else
    {
        size_t const len = strlen(app_name);
        char* buff = (char*)_xo_args_tracked_alloc(context, len+1);
        memcpy(buff, app_name, len+1);
        context->app_name = buff;
    }

    if(NULL == app_version)
    {
        // A NULL app_version is supported. We just won't print the version in the 
        // help text.
        context->app_version = NULL;
    }
    else
    {
        size_t const len = strlen(app_version);
        char* buff = (char*)_xo_args_tracked_alloc(context, len+1);
        memcpy(buff, app_version, len+1);
        context->app_version = buff;
    }

    if (NULL == app_documentation)
    {
        // NULL app_documentation is supported. We just won't print the 
        // documentation in the help text.
        context->app_documentation = NULL;
    }
    else
    {
        size_t const len = strlen(app_documentation);
        char* buff = (char*)_xo_args_tracked_alloc(context, len+1);
        memcpy(buff, app_documentation, len+1);
        context->app_documentation = buff;
    }

    context->args_size = 0;
    context->args_reserved = 4;
    context->args = (xo_args_arg**)context->alloc(context->args_reserved * sizeof(xo_args_arg*));

    return context;
}

///////////////////////////////////////////////////////////////////////////////
xo_args_ctx* xo_args_create_ctx(xo_argc_t const argc,
                                xo_argv_t const argv)
{
    return xo_args_create_ctx_advanced(argc,
                                       argv,
                                       /*app_name*/NULL,
                                       /*app_version*/NULL,
                                       /*app_documentation*/NULL,
                                       /*alloc_fn*/NULL,
                                       /*realloc_fn*/NULL,
                                       /*free_fn*/NULL,
                                       /*print_fn*/NULL);
}

///////////////////////////////////////////////////////////////////////////////
// A helper to try and parse out a single argument.
//
// argv_index should be the index into argv where the variable name was found
// it will be automatically advanced as the value of the argument is parsed.
bool _xo_args_try_parse_arg(xo_args_ctx * const context,
                             size_t * const argv_index,
                             xo_args_arg * const arg)
{

    // Providing an argument multiple times is an error unless
    // the type of that argument is an array
    if (arg->has_value 
        && arg->flags & ~(XO_ARGS_TYPE_STRING_ARRAY 
                        | XO_ARGS_TYPE_BOOL_ARRAY 
                        | XO_ARGS_TYPE_INT_ARRAY))
    {
        // Providing an argument multiple times is only valid for arrays
        context->print("Error: %s was provided multiple times which is "
                        "unsupported.\n", context->argv[*argv_index]);
        return false;
    }
    if (arg->flags & XO_ARGS_TYPE_STRING)
    {
        size_t const next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                            context->argv[*argv_index]);
            return false;
        }
        char const * const next_value = context->argv[next_index];
        size_t const next_value_len = strlen(next_value);
        char * buff = _xo_args_tracked_alloc(context, next_value_len + 1);
        memcpy(buff, next_value, next_value_len + 1);
        ((_xo_args_arg_single*)arg)->value._string = buff;
        arg->has_value = true;
        *argv_index = next_index;
        return true;
    }
    else if (arg->flags & XO_ARGS_TYPE_SWITCH)
    {
        // Reminder: value._bool can be uninitialized for switches
        // because when no value is set it is implicitly false.
        ((_xo_args_arg_single*)arg)->value._bool = true;
        return true;
    }
    else if (arg->flags & XO_ARGS_TYPE_BOOL)
    {
        size_t const next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                            context->argv[*argv_index]);
            return false;
        }
        char const * const next_value = context->argv[next_index];
        if(0 == strcmp(next_value, "0")
            || 0 == strcmp(next_value, "false")
            || 0 == strcmp(next_value, "False")
            || 0 == strcmp(next_value, "FALSE"))
        {
            ((_xo_args_arg_single*)arg)->value._bool = false;
            arg->has_value = true;
            *argv_index = next_index;
            return true;
        }
        else if(0 == strcmp(next_value, "1")
                || 0 == strcmp(next_value, "true")
                || 0 == strcmp(next_value, "True")
                || 0 == strcmp(next_value, "TRUE"))
        {
            ((_xo_args_arg_single*)arg)->value._bool = true;
            arg->has_value = true;
            *argv_index = next_index;
            return true;
        }
        else
        {
            context->print("Error: Invalid value provided for %s\n"
                            "expected true or false.\n",
                            context->argv[*argv_index]);
            return false;
        }
    }
    else if (arg->flags & XO_ARGS_TYPE_INT)
    {
        size_t const next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                            context->argv[*argv_index]);
            return false;
        }
        char const * const next_value = context->argv[next_index];
        int parsed_val = 0;
        int const scanned = sscanf(next_value, "%i", &parsed_val);
        if (1 != scanned)
        {
            context->print("Error: Value for %s is out of range\n",
                            context->argv[*argv_index]);
            return false;
        }
        ((_xo_args_arg_single*)arg)->value._int = parsed_val;
        arg->has_value = true;
        *argv_index = next_index;
        return true;
    }
    // For string arrays we expect and consume the next argument no matter what
    // then we continue to take the following strings until a valid argument is
    // encountered or we are out of arguments.
    // For an obtuse example if we want to have the string array foo with the
    // values in order: ['a', '-b', '--foo', 'bar'] then the user could execute 
    // the program like so:
    //      program --foo a -b --foo --foo bar
    //
    //      argv[0] "program"   - The program we're running.
    //      argv[1] "--foo"     - This is a named argument so we start parsing
    //                            the following values as an array.
    //      argv[2] "a"         - This is the first value in the foo array.
    //      argv[3] "-b"        - This is the second value in the foo array
    //                            **assuming** there is no variable with the
    //                            short name 'b'.
    //      argv[4] "--foo"     - This is a named argument so we start parsing
    //                            the following values as an array.
    //      argv[5] "--foo"     - This is the third element of the foo array. 
    //                            It is not treated as a named argument because
    //                            it is the first value following a named
    //                            argument.
    //      argv[6] "bar"        - This is the fourth element of the foo array.
    else if (arg->flags & XO_ARGS_TYPE_STRING_ARRAY)
    {
        _xo_args_arg_array * const array = (_xo_args_arg_array*)arg;
        size_t const next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                            context->argv[*argv_index]);
            return false;
        }

        char const * next_value = context->argv[next_index];
        size_t next_value_len = strlen(next_value);
        char * buff = _xo_args_tracked_alloc(context, next_value_len + 1);
        memcpy(buff, next_value, next_value_len + 1);

        _xo_args_arg_array_push(context, array, &buff, sizeof(char*));

        arg->has_value = true;
        *argv_index = next_index;

        

        return true;
    }
    else if (arg->flags & XO_ARGS_TYPE_BOOL_ARRAY)
    {
    }
    else if (arg->flags & XO_ARGS_TYPE_INT_ARRAY)
    {
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool xo_args_submit(xo_args_ctx * const context)
{
    XO_ARGS_ASSERT(context, "xo_args_ctx must not be null here");
    _xo_print_help(context);

    for (size_t i = 1; i < (size_t)context->argc; ++i)
    {
        char const * const a = context->argv[i];
        size_t const a_len = strlen(a);
        
        // This is an unexpected case but we will try to ignore it.
        if (a_len == 0)
        {
            continue;
        }
        // All valid variables begin with '-' or '--' so a single
        // character argument at this position is unexpected
        // and so is a string not starting with '-' 
        else if (a_len == 1 || a[0] != '-')
        {
            context->print("Error: unknown argument \"%s\"\n", a);
            _xo_print_try_help(context);
            return false;
        }
        // At this point there is a chance the user has input a valid argument 
        else
        {
            if (a[1] == '-')
            {
                for (size_t j = 0; j < context->args_size; ++j)
                {
                    // a+2 to advance past '--'
                    if (0 == strcmp(a+2, context->args[j]->name))
                    {
                        if (false == _xo_args_try_parse_arg(context, &i, context->args[j]))
                        {
                            return false;
                        }
                        break;
                    }
                }
            }
            else
            {
                for (size_t j = 0; j < context->args_size; ++j)
                {
                    // a+1 to advance past '-'
                    if ((NULL != context->args[j]->short_name) 
                        && 0 == strcmp(a+1, context->args[j]->short_name))
                    {
                        if (false == _xo_args_try_parse_arg(context, &i, context->args[j]))
                        {
                            return false;
                        }
                        break;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < context->args_size; ++i)
    {
        xo_args_arg const * const arg = context->args[i];
        if ((arg->flags & XO_ARGS_ARG_REQUIRED)
            && (false == arg->has_value))
        {
            if (NULL == arg->short_name)
            {
                context->print("Error: argument --%s is required.\n",
                               arg->name);
            }
            else
            {
                context->print("Error: argument --%s / -%s is required.\n", 
                               arg->name, 
                               arg->short_name);
            }
            _xo_print_try_help(context);
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
void xo_args_destroy_ctx(xo_args_ctx* context)
{
    XO_ARGS_ASSERT(context, "xo_args_ctx must not be null here");
    // There is no need to use the tracked delete here.
    // It performs extra work such as swapping elements which is intended to
    // keep the list valid after freeing each element. We don't care about that.
    for (size_t i = 0; i < context->allocations_size; ++i)
    {
        context->free(context->allocations[i]);
    }
    context->free(context->allocations);
    context->free(context->args);
    context->free(context);
}

///////////////////////////////////////////////////////////////////////////////
xo_args_arg * xo_args_declare_arg(xo_args_ctx * const context,
                                  char const * const name,
                                  char const * const short_name,
                                  XO_ARGS_ARG_FLAG const flags)
{
    XO_ARGS_ASSERT(NULL != context, "xo_args_ctx must not be null here");
    XO_ARGS_ASSERT(NULL != name, "name must not be null here");

    size_t const name_len = strlen(name);
    XO_ARGS_ASSERT(name_len != 0, "name must be a valid string with a length >= 1");

    size_t const short_name_len = NULL != short_name ? strlen(short_name) : 0;
    XO_ARGS_ASSERT(short_name == NULL || short_name_len != 0, "if a short name is provided it must have a length >= 1");

    for (size_t i = 0; i < name_len; ++i)
    {
        XO_ARGS_ASSERT(isalnum(name[i]), "argument names must be alphanumeric");
    }

    for (size_t i = 0; i < short_name_len; ++i)
    {
        XO_ARGS_ASSERT(isalnum(short_name[i]), "argument short names must be alphanumeric");
    }


    XO_ARGS_ARG_FLAG const all_types = 
        XO_ARGS_TYPE_STRING
        | XO_ARGS_TYPE_SWITCH
        | XO_ARGS_TYPE_BOOL
        | XO_ARGS_TYPE_INT
        | XO_ARGS_TYPE_BOOL_ARRAY
        | XO_ARGS_TYPE_INT_ARRAY
        | XO_ARGS_TYPE_STRING_ARRAY;
    {
        // Extract the type from the provided flags and count the set bits
        // if there is more than one type bit set: the argument declaration is invalid.
        XO_ARGS_ARG_FLAG const type_flag = flags & all_types;
        size_t type_flag_temp = type_flag;
        size_t bits = 0;
        for(;type_flag_temp;++bits)
        {
            type_flag_temp &= type_flag_temp-1;
        }
        
        XO_ARGS_ASSERT(bits <= 1, "arguments must only have one or zero types set");
    }

    // Look for conflicts with existing arguments first.
    for (size_t i = 0; i < context->args_size; ++i)
    {
        xo_args_arg * const existing_arg = context->args[i];
        if (strcmp(existing_arg->name, name) == 0)
        {
            context->print("xo-args error: " __FUNCTION__ 
                           " argument name conflict. name: %s\n", name);
            return NULL;
        }
        if (NULL != short_name 
            && strcmp(existing_arg->short_name, short_name) == 0)
        {
            context->print("xo-args error: " __FUNCTION__
                           " argument short_name conflict. short_name: %s\n", 
                           short_name);
            return NULL;
        }
    }

    // We're going to create a concrete argument and use it polymorphically
    // throughout this function through an 'arg' variable.
    _xo_args_arg_single * arg_single = NULL;
    _xo_args_arg_array * arg_array = NULL;

    if (flags & (XO_ARGS_TYPE_STRING_ARRAY
                | XO_ARGS_TYPE_INT_ARRAY
                | XO_ARGS_TYPE_BOOL_ARRAY))
    {
        arg_array = (_xo_args_arg_array*)_xo_args_tracked_alloc(context,
                                                                sizeof(_xo_args_arg_array));
        // We will allocate the array on first push
        arg_array->array_size = 0;
        arg_array->array_reserved = 0;
        arg_array->array = NULL;
    }
    else
    {
        arg_single = (_xo_args_arg_single*)_xo_args_tracked_alloc(context,
                                                                 sizeof(_xo_args_arg_single));
    }

    xo_args_arg * const arg = NULL != arg_array
                              ? (xo_args_arg*)arg_array 
                              : (xo_args_arg*)arg_single;
    if (context->args_reserved == context->args_size)
    {
        context->args_reserved *= 2;
        context->args = (xo_args_arg**)_xo_args_tracked_realloc(context,
                                                                context->args, 
                                                                context->args_reserved 
                                                                * sizeof(xo_args_arg*));
    }

    context->args[context->args_size++] = arg;
    
    // If any type flag is set: use the flags as is
    // Otherwise take the provided flags and assign the default type of string
    arg->flags = (all_types & flags) ? flags : flags | XO_ARGS_TYPE_STRING;

    {
        char * const buff = (char*)_xo_args_tracked_alloc(context, name_len+1);
        memcpy(buff, name, name_len+1);
        arg->name = buff;
    }

    if (NULL != short_name)
    {
        char * const buff = (char*)_xo_args_tracked_alloc(context, short_name_len+1);
        memcpy(buff, short_name, short_name_len+1);
        arg->short_name = buff;
    }

    arg->has_value = false;
    return arg;
}

///////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_string(xo_args_arg const * const arg, 
                            char const ** out_string)
{
    XO_ARGS_ASSERT(NULL != arg, "argument is null");
    XO_ARGS_ASSERT(NULL != out_string, "out param is null");
    XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_STRING, "incorrect argument type");
    if (arg->has_value)
    {
        *out_string = ((_xo_args_arg_single*)arg)->value._string;
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_int(xo_args_arg const * const arg, int * out_int)
{
    XO_ARGS_ASSERT(NULL != arg, "argument is null");
    XO_ARGS_ASSERT(NULL != out_int, "out param is null");
    XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_INT, "incorrect argument type");
    if (arg->has_value)
    {
        *out_int = ((_xo_args_arg_single*)arg)->value._int;
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_bool(xo_args_arg const * const arg, bool * out_bool)
{
    XO_ARGS_ASSERT(NULL != arg, "argument is null");
    XO_ARGS_ASSERT(NULL != out_bool, "out param is null");
    XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_BOOL, "incorrect argument type");
    if (arg->has_value)
    {
        *out_bool = ((_xo_args_arg_single*)arg)->value._bool;
        return true;
    }
    return false;
}
#endif