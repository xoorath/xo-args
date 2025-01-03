////////////////////////////////////////////////////////////////////////////////
// xo-args.h - pre-release - public domain
// authored from 2024-2025 by Jared Thomson
//
// xo-args provides a way to declare named arguments for a program's argv
// in a portable manner with a syntax that will be familiar to many users.
// xo-args allows for typed arguments, arrays of values, and will generate help
// text. It is designed to work with C99 or C++98 or newer.
//
// LICENSE
//
//  See the end of file for license information.
//
// MAIN FEATURES
//
//      1. Parsing of arguments with error checking for the following types:
//         string, integer, double, and boolean. There is also an array type
//         for each of these data types.
//      2. Switches are special arguments with no value. A switch is a boolean
//         that is implicitly false and becomes true when present.
//      3. Each argument has a name and optionally a short-name. This let's
//         users set variables with two syntaxes: --name and -n.
//      4. Help text is generated for the program based on the initial setup of
//         the xo-args context and the declaration of each argument. This help
//         text can be accessed by users with '--help' or '-h' on the command
//         line.
//      5. xo-args can be configured with a custom print function and allocator.
//
// USAGE
//
//  Include this file in whatever places need to refer to it. In ONE C/C++ file,
//  write:
//      #define XO_ARGS_IMPL
//  before the #include of this file. This expands out the actual implementation
//  into that C/C++ file.
//
//  Creating a context:
//      To use xo-args, you must create a context, declare arguments using that
//      context, submit the arguments when finished, and destroy the context
//      when it is no longer needed. The necessary functions are:
//      all of that:
//          xo_args_create_ctx          -- Creates the context
//          xo_args_create_ctx_advanced -- A more feature-rich alternative to
//                                         xo_args_create_ctx
//          xo_args_declare_arg         -- Declares an argument
//          xo_args_submit              -- Begins argument parsing
//          xo_args_destroy_ctx         -- Cleans up the context
//
//  Declaring arguments:
//      Every argument must have a name. That name is specified by users on the
//      command line with two dashes (example: if the name is "key-name", users
//      will type "--key-name"). A short name is optional and can be specified
//      by users with a single dash (example: if the short name is "k", users
//      will type "-k"). All names are case sensitive, and there us nothing
//      preventing you from using the same string for a name and short-name.
//      Names and short-names must not conflict with existing arguments.
//
//      A value tip and description are used only for generating help text,
//      helping users understand what to expect from a given argument.
//
//      Arguments are optional by default. Use the XO_ARGS_ARG_REQUIRED flag to
//      indicate than an argument is required.
//
//  Data types:
//      xo-args supports strings, integers, doubles and booleans. There are also
//      array types for each of those data types. The integer type is backed by
//      int64_t with all the limitations that implies; similarly, doubles are
//      backed by the double type.
//
//  User experience:
//      Suppose you declare an application 'foo.exe' that takes a "verbose"/"V"
//      switch, a double "timeout"/"t", a string array "input"/"i" and a string
//      "output"/"o". Here are examples of how the program could be invoked:
//
//          foo.exe --input input1.txt input2.txt --output out.txt -V -t=10.0
//          foo.exe -i input1.txt input2.txt -o out.txt --timeout 10
//          foo.exe -i input1.txt -i input2.txt -o out.txt --timeout infinity
//          foo.exe -i input1.txt --verbose -o out.txt --input input2.txt -t 10
//
// IMPLEMENTATION NOTES
//
//      xo-args is developed in a manner similar to other single-header
//      libraries and directly references and copies style notes from
//      Sean Barrett's stb project: https://github.com/nothings/stb
////////////////////////////////////////////////////////////////////////////////
#if !defined(__XO_ARGS_H__)
#define __XO_ARGS_H__

#include <stdint.h>
#if !defined(__cplusplus)
#include <stdbool.h>
#endif // !defined(__cplusplus)
#include <stddef.h>

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

    ////////////////////////////////////////////////////////////////////////////
    // Types

    // entry point argc
    typedef int xo_argc_t;

    // entry point argv type
    typedef char const * const * xo_argv_t;

    // Function pointer typedefs for customizing the print function and
    // allocator.
    typedef void * (*xo_args_alloc_fn)(size_t);
    typedef void * (*xo_args_realloc_fn)(void *, size_t);
    typedef void (*xo_args_free_fn)(void *);
    typedef int (*xo_args_print_fn)(char const *, ...);

    // An opaque context structure to hold implementation details of xo-args.
    typedef struct xo_args_ctx xo_args_ctx;

    // An opaque structure to hold implementation details of a single argument.
    typedef struct xo_args_arg xo_args_arg;

    // Bit-flags for declaring an argument.
    // A valid XO_ARGS_ARG_FLAG value is any one type value with or without
    // XO_ARGS_ARG_REQUIRED.
    //
    // Examples:
    //      XO_ARGS_TYPE_STRING                         // valid
    //      XO_ARGS_TYPE_STRING|XO_ARGS_TYPE_BOOL       // INVALID
    //      XO_ARGS_TYPE_STRING|XO_ARGS_ARG_REQUIRED    // valid
    typedef enum XO_ARGS_ARG_FLAG
    {
        XO_ARGS_TYPE_STRING = 1 << 0,
        // To get the value of a switch: use xo_args_try_get_bool
        XO_ARGS_TYPE_SWITCH = 1 << 1,
        XO_ARGS_TYPE_BOOL = 1 << 2,
        XO_ARGS_TYPE_INT = 1 << 3,
        XO_ARGS_TYPE_DOUBLE = 1 << 4,
        XO_ARGS_TYPE_STRING_ARRAY = 1 << 5,
        XO_ARGS_TYPE_BOOL_ARRAY = 1 << 6,
        XO_ARGS_TYPE_INT_ARRAY = 1 << 7,
        XO_ARGS_TYPE_DOUBLE_ARRAY = 1 << 8,

        XO_ARGS_ARG_OPTIONAL = 0,
        XO_ARGS_ARG_REQUIRED = 1 << 9
    } XO_ARGS_ARG_FLAG;

    ////////////////////////////////////////////////////////////////////////////
    // Creates an xo-args context to be used with other API functions.
    // argc, argv: required program arguments
    //
    // app_name: optional name for the executable. If not provided: xo-args will
    // parse the application filename from argv[0].
    //
    // app_version: optional version string to be printed when --help/-h or
    // --version/-v are supplied by the user.
    //
    // app_documentation: optional additional documentation string to be printed
    // at the beginning of the generated help text.
    //
    // alloc_fn, realloc_fn, free_fn: optional allocation functions.
    //
    // print_fn: optional free function.
    xo_args_ctx * xo_args_create_ctx_advanced(
        xo_argc_t const argc,
        xo_argv_t const argv,
        char const * const app_name,
        char const * const app_version,
        char const * const app_documentation,
        xo_args_alloc_fn const alloc_fn,
        xo_args_realloc_fn const realloc_fn,
        xo_args_free_fn const free_fn,
        xo_args_print_fn const print_fn);

    ////////////////////////////////////////////////////////////////////////////
    // Creates an xo-args context to be used with other API functions.
    // argc, argv: required program arguments
    xo_args_ctx * xo_args_create_ctx(xo_argc_t const argc,
                                     xo_argv_t const argv);

    ////////////////////////////////////////////////////////////////////////////
    // xo_args_submit concludes the setup of xo-args and parses all arguments.
    // If xo_args_submit returns true: the arguments are valid and can be used.
    // If xo_args_submit returns false: the arguments are invalid or the
    // --help/-h or --version/-v arguments were provided.
    bool xo_args_submit(xo_args_ctx * const context);

    ////////////////////////////////////////////////////////////////////////////
    // Destroys the xo-args context and all memory tracked by xo-args.
    void xo_args_destroy_ctx(xo_args_ctx * const context);

    ////////////////////////////////////////////////////////////////////////////
    // Prints the generated help text. This is done automatically during submit
    // if the program arguments contain --help (as a switch, not a string value
    // which would be consumed / ignored)
    //
    // This function should only be called after xo_args_submit has returned
    // true.
    void xo_args_print_help(xo_args_ctx const * const context);

    ////////////////////////////////////////////////////////////////////////////
    // Prints the version text specified when creating the xo-args context.
    // Version text can only be printed if a version string was provided via
    // xo_args_create_ctx_advanced.
    void xo_args_print_version(xo_args_ctx const * const context);

    ////////////////////////////////////////////////////////////////////////////
    // Declares a program argument.
    //
    // context: The xo-args context
    //
    // name: The name of this argument. ie: "foo" if the program takes "--foo"
    // as an argument.
    //
    // short_name (optional): An alternate name for this argument. ie: "f" if
    // the program can take "-f" as an argument. If the short name and name are
    // the same then the generated help text will show only the short name
    // syntax.
    //
    // value_tip (optional): For arguments that take a value (anything
    // but switches) this value tip is printed next to the name.
    // ie: "--foo, -f <integer>". You may wish to fill this out for a more user
    // friendly input suggestion that just a data type. For example "<file>" or
    // "[files]" may be more useful to your users than "<text>" or "[text]".
    //
    // description (optional): Text to be printed next to the argument in the
    // help text.
    //
    // flags: bit-flags for combining an argument data type and optionally a
    // required flag.
    xo_args_arg * xo_args_declare_arg(xo_args_ctx * const context,
                                      char const * const name,
                                      char const * const short_name,
                                      char const * const value_tip,
                                      char const * const description,
                                      XO_ARGS_ARG_FLAG const flags);

    ////////////////////////////////////////////////////////////////////////////
    bool xo_args_try_get_string(xo_args_arg const * const arg,
                                char const ** out_string);

    ////////////////////////////////////////////////////////////////////////////
    bool xo_args_try_get_int(xo_args_arg const * const arg, int64_t * out_int);

    ////////////////////////////////////////////////////////////////////////////
    bool xo_args_try_get_double(xo_args_arg const * const arg,
                                double * out_int);

    ////////////////////////////////////////////////////////////////////////////
    bool xo_args_try_get_bool(xo_args_arg const * const arg, bool * out_bool);

    ////////////////////////////////////////////////////////////////////////////
    bool xo_args_try_get_string_array(xo_args_arg const * const arg,
                                      char const *** out_string_array,
                                      size_t * out_array_count);

    ////////////////////////////////////////////////////////////////////////////
    bool xo_args_try_get_int_array(xo_args_arg const * const arg,
                                   int64_t const ** out_int_array,
                                   size_t * out_array_count);

    ////////////////////////////////////////////////////////////////////////////
    bool xo_args_try_get_double_array(xo_args_arg const * const arg,
                                      double const ** out_double_array,
                                      size_t * out_array_count);

    ////////////////////////////////////////////////////////////////////////////
    bool xo_args_try_get_bool_array(xo_args_arg const * const arg,
                                    bool const ** out_bool_array,
                                    size_t * out_array_count);

#if defined(__cplusplus)
} // extern "C"
#endif // defined(__cplusplus)
#endif // __XO_ARGS_H__
////////////////////////////////////////////////////////////////////////////////
// XO_ARGS_IMPL should be defined by hand in one C file before including
// xo-args.h
#if defined(XO_ARGS_IMPL)

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XO_ARGS_XSTR(a) XO_ARGS_STR(a)
#define XO_ARGS_STR(a) #a

#if !defined(XO_ARGS_ASSERT)
#include <assert.h>
#define XO_ARGS_ASSERT(condition, message)                                     \
    assert(((condition)) && ("xo-args assert: " message))
#endif

#if defined(_WIN32)
char const g_xo_args_path_separators[3] = "/\\";
#else
char const g_xo_args_path_separators[2] = "/";
#endif

#if !defined(min)
#define min(x, y) ((x <= y ? x : y))
#endif

////////////////////////////////////////////////////////////////////////////////
struct xo_args_arg
{
    char const * name; // ie: "foo"
    size_t name_length;

    char const * short_name; // ie: "f"
    size_t short_name_length;

    char const * value_tip; // ie: "filename"
    size_t value_tip_length;

    char const * description; // ie: "loads the foo file"
    size_t description_length;

    // The flags tracked so far.
    XO_ARGS_ARG_FLAG flags;

    // has_value is unset until parsed
    bool has_value;
};

////////////////////////////////////////////////////////////////////////////////
typedef struct _xo_args_arg_single
{
    xo_args_arg base;
    union
    {
        bool _bool;
        char * _string;
        int64_t _int;
        double _double;
    } value;
} _xo_args_arg_single;

////////////////////////////////////////////////////////////////////////////////
typedef struct _xo_args_arg_array
{
    xo_args_arg base;
    void ** array;
    size_t array_reserved;
    size_t array_size;
} _xo_args_arg_array;

////////////////////////////////////////////////////////////////////////////////
struct xo_args_ctx
{
    xo_argc_t argc;
    xo_argv_t argv;
    xo_args_alloc_fn alloc;
    xo_args_realloc_fn realloc;
    xo_args_free_fn free;
    xo_args_print_fn print;
    char const * app_name;
    size_t app_name_length;

    char const * app_version;
    size_t app_version_length;

    char const * app_documentation;
    size_t app_documentation_length;

    // A list of all allocations to free later in xo_args_cleanup
    void ** allocations;
    size_t allocations_reserved; // number of allocated elements in allocations
    size_t allocations_size;     // actual size

    // A list of arguments. This is not a tracked allocation but all
    // arguments in this list are tracked allocations.
    xo_args_arg ** args;
    size_t args_reserved;
    size_t args_size;

    bool submitted;
};

////////////////////////////////////////////////////////////////////////////////
bool _xo_args_arg_flag_is_array(XO_ARGS_ARG_FLAG const flags)
{
    return !!(flags
              & (XO_ARGS_TYPE_STRING_ARRAY | XO_ARGS_TYPE_INT_ARRAY
                 | XO_ARGS_TYPE_DOUBLE_ARRAY | XO_ARGS_TYPE_BOOL_ARRAY));
}

////////////////////////////////////////////////////////////////////////////////
typedef enum _xo_args_arg_match_type
{
    _XO_ARGS_ARG_MATCH_TYPE_NAME,
    _XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME,
    _XO_ARGS_ARG_MATCH_TYPE_SHORT_NAME,
    _XO_ARGS_ARG_MATCH_TYPE_ASSIGN_SHORT_NAME
} _xo_args_arg_match_type;

////////////////////////////////////////////////////////////////////////////////
typedef struct _xo_args_arg_match
{
    _xo_args_arg_match_type match_type;
    // May represent a name or short_name
    char const * matched_name;
    size_t matched_name_length;
} _xo_args_arg_match;

////////////////////////////////////////////////////////////////////////////////
bool _xo_args_arg_matches_input(xo_args_arg const * const arg,
                                char const * const str,
                                _xo_args_arg_match * out_match)
{
    size_t const str_len = strlen(str);
    if (0 == str_len)
    {
        return false;
    }
    if ('-' == str[0])
    {
        if ((str_len > 2) && ('-' == str[1])
            && (0 == strncmp(&str[2], arg->name, arg->name_length)))
        {
            if (str_len - 2 == arg->name_length)
            {
                if (NULL != out_match)
                {
                    out_match->match_type = _XO_ARGS_ARG_MATCH_TYPE_NAME;
                    out_match->matched_name = arg->name;
                    out_match->matched_name_length = arg->name_length;
                }
                return true;
            }
            if ('=' == str[arg->name_length + 2])
            {
                if (NULL != out_match)
                {
                    out_match->match_type = _XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME;
                    out_match->matched_name = arg->name;
                    out_match->matched_name_length = arg->name_length;
                }
                return true;
            }
            return false;
        }

        if ((NULL != arg->short_name)
            && (0 == strncmp(&str[1], arg->short_name, arg->short_name_length)))
        {
            if (str_len - 1 == arg->short_name_length)
            {
                if (NULL != out_match)
                {
                    out_match->match_type = _XO_ARGS_ARG_MATCH_TYPE_SHORT_NAME;
                    out_match->matched_name = arg->short_name;
                    out_match->matched_name_length = arg->short_name_length;
                }
                return true;
            }
            if ('=' == str[arg->short_name_length + 1])
            {
                if (NULL != out_match)
                {
                    out_match->match_type =
                        _XO_ARGS_ARG_MATCH_TYPE_ASSIGN_SHORT_NAME;
                    out_match->matched_name = arg->short_name;
                    out_match->matched_name_length = arg->short_name_length;
                }
                return true;
            }
            return false;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
void * _xo_args_tracked_alloc(xo_args_ctx * const context, size_t const size)
{
    if (context->allocations_reserved == context->allocations_size)
    {
        context->allocations_reserved *= 2;
        context->allocations = (void **)context->realloc(
            context->allocations,
            context->allocations_reserved * sizeof(void *));
    }
    void * const mem = context->alloc(size);
    context->allocations[context->allocations_size++] = mem;
    return mem;
}

////////////////////////////////////////////////////////////////////////////////
void * _xo_args_tracked_realloc(xo_args_ctx * const context,
                                void * const mem,
                                size_t const size)
{
    for (size_t i = 0; i < context->allocations_size; ++i)
    {
        if (mem == context->allocations[i])
        {
            context->allocations[i] = context->realloc(mem, size);
            return context->allocations[i];
        }
    }
    XO_ARGS_ASSERT(false, "Failed to find allocation for realloc");
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void _xo_args_tracked_free(xo_args_ctx * const context, void * const mem)
{
    // We will free the memory and stop tracking it, but we do this with a
    // last-swap this lets us avoid shuffling elements down since order of our
    // tracked allocation list is not important to us.
    for (size_t i = 0; i < context->allocations_size; ++i)
    {
        if (context->allocations[i] == mem)
        {
            context->free(mem);
            // decrement the size and now allocations_size is the previous last
            // element index
            --context->allocations_size;

            if (i != context->allocations_size)
            {
                context->allocations[i] =
                    context->allocations[context->allocations_size];
            }
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void _xo_args_arg_array_init(xo_args_ctx * const context,
                             _xo_args_arg_array * const array,
                             size_t const value_size)
{
    array->array_reserved = 2;
    array->array_size = 0;
    array->array = (void **)_xo_args_tracked_alloc(
        context, value_size * array->array_reserved);
}

////////////////////////////////////////////////////////////////////////////////
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
        array->array = (void **)_xo_args_tracked_realloc(
            context, array->array, array->array_reserved * value_size);
    }
    memcpy(((char *)array->array) + (value_size * array->array_size++),
           value,
           value_size);
}

////////////////////////////////////////////////////////////////////////////////
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
//      printf("foo: %s\nbar: %s\n", foo, bar); // prints "foo: a/b/ccc.e\nbar:
//      ccc\n" free(bar);
char const * _xo_args_basename(xo_args_ctx * const context,
                               char const * const path)
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
        for (size_t i = 0; i < sizeof(g_xo_args_path_separators) - 1; ++i)
        {
            is_path_sep = is_path_sep || *it == g_xo_args_path_separators[i];
        }

        if (is_path_sep)
        {
            // The min is to handle the edge case where the last character is a
            // path sep we don't accidentally want to advance past the end.
            basename_start = min(it + 1, end);
            break;
        }
        --it;
    }
    it = basename_start;
    char const * basename_end = basename_start;
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
        char * buff =
            (char *)_xo_args_tracked_alloc(context, basename_length + 1);
        memcpy(buff, basename_start, basename_length);
        buff[basename_length] = '\0';
        return buff;
    }
    // There is no string to copy.
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
bool _xo_isalnum(char const c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
           || (c >= '0' && c <= '9') || (c == '-');
}

////////////////////////////////////////////////////////////////////////////////
bool _xo_isalnum_str(char const * const start, char const * const end)
{
    char const * const last = (NULL != end ? end : (start + strlen(start))) - 1;
    char const * curr = start;
    while (curr != last)
    {
        if (false == _xo_isalnum(*curr))
        {
            return false;
        }
        ++curr;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void _xo_print_try_help(xo_args_ctx const * const context)
{
    context->print("Try: %s --help\n", context->app_name);
}

////////////////////////////////////////////////////////////////////////////////
void _xo_args_print_arg_help(xo_args_ctx const * const context,
                             xo_args_arg const * const arg,
                             size_t const left_column_width)
{
    char left_buffer[128] = {0};
    if (arg->short_name != NULL)
    {
        if ((arg->name_length == arg->short_name_length)
            && (0 == strcmp(arg->name, arg->short_name)))
        {
            if (arg->value_tip_length != 0)
            {
                snprintf(left_buffer,
                         sizeof(left_buffer),
                         "  -%s %s",
                         arg->short_name,
                         arg->value_tip);
            }
            else
            {
                snprintf(
                    left_buffer, sizeof(left_buffer), "  -%s", arg->short_name);
            }
        }
        else
        {
            if (arg->value_tip_length != 0)
            {
                snprintf(left_buffer,
                         sizeof(left_buffer),
                         "  --%s, -%s %s",
                         arg->name,
                         arg->short_name,
                         arg->value_tip);
            }
            else
            {
                snprintf(left_buffer,
                         sizeof(left_buffer),
                         "  --%s, -%s",
                         arg->name,
                         arg->short_name);
            }
        }
    }
    else
    {
        if (arg->value_tip_length != 0)
        {
            snprintf(left_buffer,
                     sizeof(left_buffer),
                     "  --%s %s",
                     arg->name,
                     arg->value_tip);
        }
        else
        {
            snprintf(left_buffer, sizeof(left_buffer), "  --%s", arg->name);
        }
    }

    size_t const left_buffer_len = strlen(left_buffer);
    size_t const whitespace_needed = left_buffer_len > left_column_width
                                         ? 0
                                         : left_column_width - left_buffer_len;
    context->print("%s%*s", left_buffer, whitespace_needed, "");

    if (NULL != arg->description)
    {
        // todo: wrap after some width
        context->print(arg->description);
    }
    context->print("\n");
}

////////////////////////////////////////////////////////////////////////////////
void xo_args_print_help(xo_args_ctx const * const context)
{
    if (NULL != context->app_version)
    {
        context->print(
            "%s version %s\n", context->app_name, context->app_version);
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

    size_t left_column_width = 0;
    for (size_t i = 0; i < context->args_size; ++i)
    {
        xo_args_arg const * const arg = context->args[i];
        // 8 for an indent of 2 spaces, "--" before the name and a buffer of 4
        // after everything
        size_t arg_column_space_needed = 8;
        arg_column_space_needed += arg->name_length;
        arg_column_space_needed +=
            (arg->value_tip_length != 0) ? arg->value_tip_length + 1 : 0;

        if (NULL != arg->short_name)
        {
            // As a special case if the name and short name are the same:
            // we just print the short name.
            if ((arg->name_length == arg->short_name_length)
                && (0 == strcmp(arg->short_name, arg->name)))
            {
                arg_column_space_needed = 7;
                arg_column_space_needed += arg->short_name_length;
                arg_column_space_needed += (arg->value_tip_length != 0)
                                               ? arg->value_tip_length + 1
                                               : 0;
            }
            else
            {
                // 2 for ", " between the name and short name
                arg_column_space_needed += 2 + arg->short_name_length;
            }
        }
        left_column_width = (left_column_width < arg_column_space_needed)
                                ? arg_column_space_needed
                                : left_column_width;
    }

    if (any_required)
    {
        if (any_optional)
        {
            context->print("REQUIRED ARGUMENTS:\n");
        }
        for (size_t i = 0; i < context->args_size; ++i)
        {
            xo_args_arg const * const arg = context->args[i];
            if (arg->flags & XO_ARGS_ARG_REQUIRED)
            {
                _xo_args_print_arg_help(context, arg, left_column_width);
            }
        }
    }

    if (any_optional)
    {
        if (any_required)
        {
            context->print("OPTIONAL ARGUMENTS:\n");
        }
        for (size_t i = 0; i < context->args_size; ++i)
        {
            xo_args_arg const * const arg = context->args[i];
            if (XO_ARGS_ARG_REQUIRED != (arg->flags & XO_ARGS_ARG_REQUIRED))
            {
                _xo_args_print_arg_help(context, arg, left_column_width);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void xo_args_print_version(xo_args_ctx const * const context)
{
    if (NULL != context->app_version)
    {
        context->print(
            "%s version %s\n", context->app_name, context->app_version);
    }
    else
    {
        XO_ARGS_ASSERT(NULL != context->app_version,
                       "No version string was "
                       "provided but xo_args_print_version was called.");
    }
}

////////////////////////////////////////////////////////////////////////////////
xo_args_ctx * xo_args_create_ctx_advanced(xo_argc_t const argc,
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
        (print_fn != NULL ? print_fn : printf)("xo-args error: %s argc is"
                                               " expected to be >= 1 but was"
                                               "%i\n",
                                               __func__,
                                               argc);
        return NULL;
    }

    if (NULL == argv)
    {
        XO_ARGS_ASSERT(NULL != argv, "argv is required");
        (print_fn != NULL ? print_fn : printf)("xo-args error: %s argv is"
                                               " required\n",
                                               __func__);
        return NULL;
    }

    {
        bool any_arg_is_null = false;
        for (int i = 0; i < argc; ++i)
        {
            if (NULL == argv[i])
            {
                (print_fn != NULL ? print_fn : printf)("xo-args error: %s"
                                                       " argv[%i] was NULL\n",
                                                       __func__,
                                                       i);
                any_arg_is_null = true;
            }
        }

        if (any_arg_is_null)
        {
            XO_ARGS_ASSERT(false == any_arg_is_null,
                           "one or more arguments in argv was null");
            return NULL;
        }
    }

    xo_args_ctx * const context =
        (xo_args_ctx *)(NULL != alloc_fn ? alloc_fn(sizeof(xo_args_ctx))
                                         : malloc(sizeof(xo_args_ctx)));

    context->argc = argc;
    context->argv = argv;
    context->print = NULL == print_fn ? printf : print_fn;
    context->alloc = NULL == alloc_fn ? malloc : alloc_fn;
    context->realloc = NULL == realloc_fn ? realloc : realloc_fn;
    context->free = NULL == free_fn ? free : free_fn;
    context->allocations_size = 0;
    context->allocations_reserved = 8;
    context->allocations =
        (void **)context->alloc(context->allocations_reserved * sizeof(void *));
    context->submitted = false;

    // Default app_name is the filename parsed from argv[0]
    if (NULL == app_name)
    {
        context->app_name = _xo_args_basename(context, argv[0]);
        if (NULL == context->app_name)
        {
            // We never free app_name directly so this assignment is safe
            context->app_name = "app";
        }

        context->app_name_length = strlen(context->app_name);
    }
    else
    {
        size_t const len = strlen(app_name);
        char * buff = (char *)_xo_args_tracked_alloc(context, len + 1);
        memcpy(buff, app_name, len + 1);
        context->app_name = buff;
        context->app_name_length = len;
    }

    if (NULL == app_version)
    {
        // A NULL app_version is supported. We just won't print the version in
        // the help text.
        context->app_version = NULL;
        context->app_version_length = 0;
    }
    else
    {
        size_t const len = strlen(app_version);
        char * buff = (char *)_xo_args_tracked_alloc(context, len + 1);
        memcpy(buff, app_version, len + 1);
        context->app_version = buff;
        context->app_version_length = 0;
    }

    if (NULL == app_documentation)
    {
        // NULL app_documentation is supported. We just won't print the
        // documentation in the help text.
        context->app_documentation = NULL;
        context->app_documentation_length = 0;
    }
    else
    {
        size_t const len = strlen(app_documentation);
        char * buff = (char *)_xo_args_tracked_alloc(context, len + 1);
        memcpy(buff, app_documentation, len + 1);
        context->app_documentation = buff;
        context->app_documentation_length = 0;
    }

    context->args_size = 0;
    context->args_reserved = 4;
    context->args = (xo_args_arg **)_xo_args_tracked_alloc(
        context, context->args_reserved * sizeof(xo_args_arg *));

    return context;
}

////////////////////////////////////////////////////////////////////////////////
xo_args_ctx * xo_args_create_ctx(xo_argc_t const argc, xo_argv_t const argv)
{
    return xo_args_create_ctx_advanced(argc,
                                       argv,
                                       /*app_name*/ NULL,
                                       /*app_version*/ NULL,
                                       /*app_documentation*/ NULL,
                                       /*alloc_fn*/ NULL,
                                       /*realloc_fn*/ NULL,
                                       /*free_fn*/ NULL,
                                       /*print_fn*/ NULL);
}

////////////////////////////////////////////////////////////////////////////////
bool _xo_args_try_parse_int(char const * const input, int64_t * out_int)
{
    // strtoll will discard any leading whitespace.
    // I would prefer we only accept integers with no leading whitespace so we
    // can accomplish this by just checking the first character here:
    if ('\0' == input[0] || isspace(input[0]))
    {
        return false;
    }

    errno = 0;
    char * end_ptr;
    long long const long_val = strtoll(input, &end_ptr, 0);
    int64_t const parsed_val = (int64_t)long_val;

    if (0 != errno || '\0' != *end_ptr || long_val != (long long)parsed_val
        || input == end_ptr)
    {
        return false;
    }

    *out_int = parsed_val;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool _xo_args_try_parse_bool(char const * const input, bool * out_bool)
{
    if ((0 == strcmp(input, "0")) || (0 == strcmp(input, "false"))
        || (0 == strcmp(input, "False")) || (0 == strcmp(input, "FALSE")))
    {
        *out_bool = false;
        return true;
    }
    else if ((0 == strcmp(input, "1")) || (0 == strcmp(input, "true"))
             || (0 == strcmp(input, "True")) || (0 == strcmp(input, "TRUE")))
    {
        *out_bool = true;
        return true;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
bool _xo_args_try_parse_double(char const * const input, double * out_double)
{
    // strtod will discard any leading whitespace.
    // I would prefer we only accept integers with no leading whitespace so we
    // can accomplish this by just checking the first character here:
    if ('\0' == input[0] || isspace(input[0]))
    {
        return false;
    }

    errno = 0;
    char * end_ptr;
    *out_double = strtod(input, &end_ptr);

    if (0 != errno || '\0' != *end_ptr || input == end_ptr)
    {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// A helper to try and parse out a single argument.
//
// argv_index should be the index into argv where the variable name was found
// it will be automatically advanced as the value of the argument is parsed.
bool _xo_args_try_parse_arg(xo_args_ctx * const context,
                            size_t * const argv_index,
                            xo_args_arg * const arg,
                            _xo_args_arg_match const * const match)
{

    // Providing an argument multiple times is an error unless
    // the type of that argument is an array
    if (arg->has_value && false == _xo_args_arg_flag_is_array(arg->flags))
    {
        // Providing an argument multiple times is only valid for arrays
        context->print("Error: %s was provided multiple times which is "
                       "unsupported.\n",
                       context->argv[*argv_index]);
        return false;
    }
    if (arg->flags & XO_ARGS_TYPE_STRING)
    {
        if (_XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME == match->match_type
            || _XO_ARGS_ARG_MATCH_TYPE_ASSIGN_SHORT_NAME == match->match_type)
        {
            // This is an offset from the start of the user-input argument until
            // the value begins (after the assignment operator). The value 3
            // accounts for "--" and "=" in a name based assignment.. The value
            // 2 accounts for a single "-" and "=" in a short name assignment.
            size_t const offset =
                ((_XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME == match->match_type) ? 3
                                                                            : 2)
                + match->matched_name_length;

            char const * const argv_value = context->argv[*argv_index];
            size_t const argv_value_length = strlen(argv_value);

            char * const buff = (char *)_xo_args_tracked_alloc(
                context, (argv_value_length - offset) + 1);
            // +1 here will copy the null terminator from argv_value
            memcpy(buff, &argv_value[offset], argv_value_length - offset + 1);
            ((_xo_args_arg_single *)arg)->value._string = buff;
            arg->has_value = true;
            return true;
        }

        size_t const next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                           context->argv[*argv_index]);
            return false;
        }
        char const * const next_value = context->argv[next_index];
        size_t const next_value_len = strlen(next_value);
        char * const buff =
            (char *)_xo_args_tracked_alloc(context, next_value_len + 1);
        memcpy(buff, next_value, next_value_len + 1);
        ((_xo_args_arg_single *)arg)->value._string = buff;
        arg->has_value = true;
        *argv_index = next_index;
        return true;
    }
    else if (arg->flags & XO_ARGS_TYPE_SWITCH)
    {
        // Reminder: value._bool can be uninitialized for switches because when
        // no value is set it is implicitly false.
        ((_xo_args_arg_single *)arg)->base.has_value = true;
        return true;
    }
    else if (arg->flags & XO_ARGS_TYPE_BOOL)
    {
        if (_XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME == match->match_type
            || _XO_ARGS_ARG_MATCH_TYPE_ASSIGN_SHORT_NAME == match->match_type)
        {
            // This is an offset from the start of the user-input argument until
            // the value begins (after the assignment operator). The value 3
            // accounts for "--" and "=" in a name based assignment.. The value
            // 2 accounts for a single "-" and "=" in a short name assignment.
            size_t const offset =
                ((_XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME == match->match_type) ? 3
                                                                            : 2)
                + match->matched_name_length;

            char const * const argv_value = context->argv[*argv_index];

            if (_xo_args_try_parse_bool(
                    &argv_value[offset],
                    &((_xo_args_arg_single *)arg)->value._bool))
            {
                arg->has_value = true;
                return true;
            }

            context->print("Error: Invalid value provided for %s\n"
                           "expected true or false.\n",
                           context->argv[*argv_index]);
            return false;
        }

        size_t const next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                           context->argv[*argv_index]);
            return false;
        }
        char const * const next_value = context->argv[next_index];

        if (_xo_args_try_parse_bool(next_value,
                                    &((_xo_args_arg_single *)arg)->value._bool))
        {
            arg->has_value = true;
            *argv_index = next_index;
            return true;
        }

        context->print("Error: Invalid value provided for %s\n"
                       "expected true or false.\n",
                       context->argv[*argv_index]);
        return false;
    }
    else if (arg->flags & XO_ARGS_TYPE_INT)
    {
        if (_XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME == match->match_type
            || _XO_ARGS_ARG_MATCH_TYPE_ASSIGN_SHORT_NAME == match->match_type)
        {
            // This is an offset from the start of the user-input argument until
            // the value begins (after the assignment operator). The value 3
            // accounts for "--" and "=" in a name based assignment.. The value
            // 2 accounts for a single "-" and "=" in a short name assignment.
            size_t const offset =
                ((_XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME == match->match_type) ? 3
                                                                            : 2)
                + match->matched_name_length;

            char const * const argv_value = context->argv[*argv_index];

            int64_t parsed_value;
            if (true
                == _xo_args_try_parse_int(&argv_value[offset], &parsed_value))
            {
                ((_xo_args_arg_single *)arg)->value._int = parsed_value;
                arg->has_value = true;
                return true;
            }

            context->print("Error: Value for %.*s is not a valid integer or is "
                           "out of range\n",
                           offset - 1u,
                           context->argv[*argv_index]);

            return false;
        }
        char const * argv_name = context->argv[*argv_index];
        size_t const next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                           context->argv[*argv_index]);
            return false;
        }

        int64_t parsed_value;
        if (true
            == _xo_args_try_parse_int(context->argv[next_index], &parsed_value))
        {
            ((_xo_args_arg_single *)arg)->value._int = parsed_value;
            arg->has_value = true;
            *argv_index = next_index;
            return true;
        }

        context->print("Error: Value for %s is not a valid integer or is "
                       "out of range\n",
                       argv_name);

        return false;
    }
    else if (arg->flags & XO_ARGS_TYPE_DOUBLE)
    {
        if (_XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME == match->match_type
            || _XO_ARGS_ARG_MATCH_TYPE_ASSIGN_SHORT_NAME == match->match_type)
        {
            // This is an offset from the start of the user-input argument until
            // the value begins (after the assignment operator). The value 3
            // accounts for "--" and "=" in a name based assignment.. The value
            // 2 accounts for a single "-" and "=" in a short name assignment.
            size_t const offset =
                ((_XO_ARGS_ARG_MATCH_TYPE_ASSIGN_NAME == match->match_type) ? 3
                                                                            : 2)
                + match->matched_name_length;

            char const * const argv_value = context->argv[*argv_index];

            double parsed_value;
            if (true
                == _xo_args_try_parse_double(&argv_value[offset],
                                             &parsed_value))
            {
                ((_xo_args_arg_single *)arg)->value._double = parsed_value;
                arg->has_value = true;
                return true;
            }

            context->print("Error: Value for %.*s is not a valid number or is "
                           "out of range\n",
                           offset - 1u,
                           context->argv[*argv_index]);

            return false;
        }

        char const * argv_name = context->argv[*argv_index];
        size_t const next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                           context->argv[*argv_index]);
            return false;
        }

        double parsed_value;
        if (true
            == _xo_args_try_parse_double(context->argv[next_index],
                                         &parsed_value))
        {
            ((_xo_args_arg_single *)arg)->value._double = parsed_value;
            arg->has_value = true;
            *argv_index = next_index;
            return true;
        }

        context->print("Error: Value for %s is not a valid number or is "
                       "out of range\n",
                       argv_name);

        return false;
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
        _xo_args_arg_array * const array = (_xo_args_arg_array *)arg;
        size_t next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                           context->argv[*argv_index]);
            return false;
        }

        char const * next_value = context->argv[next_index];
        size_t next_value_len = strlen(next_value);
        char * buff =
            (char *)_xo_args_tracked_alloc(context, next_value_len + 1);
        memcpy(buff, next_value, next_value_len + 1);
        _xo_args_arg_array_push(context, array, (void *)&buff, sizeof(char *));
        arg->has_value = true;
        *argv_index = next_index;

        // Consume every following value until we see a valid argument
        for (++next_index; next_index < (size_t)context->argc; ++next_index)
        {
            next_value = context->argv[next_index];
            next_value_len = strlen(next_value);

            for (size_t j = 0; j < context->args_size; ++j)
            {
                xo_args_arg const * const other_arg = context->args[j];
                if (_xo_args_arg_matches_input(other_arg, next_value, NULL))
                {
                    // The next argument is a valid arg so don't parse
                    // that as a value of this string array
                    return true;
                }
            }

            buff = (char *)_xo_args_tracked_alloc(context, next_value_len + 1);
            memcpy(buff, next_value, next_value_len + 1);
            _xo_args_arg_array_push(
                context, array, (void *)&buff, sizeof(char *));
            *argv_index = next_index;
        }
        return true;
    }
    else if (arg->flags & XO_ARGS_TYPE_INT_ARRAY)
    {
        char const * argv_name = context->argv[*argv_index];
        _xo_args_arg_array * const array = (_xo_args_arg_array *)arg;
        size_t next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                           context->argv[*argv_index]);
            return false;
        }

        char const * next_value = context->argv[next_index];
        int64_t parsed_value;

        if (true
            == _xo_args_try_parse_int(context->argv[next_index], &parsed_value))
        {
            _xo_args_arg_array_push(
                context, array, &parsed_value, sizeof(int64_t));
            arg->has_value = true;
            *argv_index = next_index;
        }
        else
        {
            context->print("Error: Value for %s is not a valid integer or is "
                           "out of range\n",
                           argv_name);
            return false;
        }

        // Consume every following value until we see a valid argument
        for (++next_index; next_index < (size_t)context->argc; ++next_index)
        {
            next_value = context->argv[next_index];

            for (size_t j = 0; j < context->args_size; ++j)
            {
                xo_args_arg const * const other_arg = context->args[j];
                if (_xo_args_arg_matches_input(other_arg, next_value, NULL))
                {
                    // The next argument is a valid arg so don't parse
                    // that as a value of this string array
                    return true;
                }
            }

            if (true
                == _xo_args_try_parse_int(context->argv[next_index],
                                          &parsed_value))
            {
                _xo_args_arg_array_push(
                    context, array, &parsed_value, sizeof(int64_t));
                arg->has_value = true;
                *argv_index = next_index;
            }
            else
            {
                context->print(
                    "Error: Value for %s is not a valid integer or is "
                    "out of range\n",
                    argv_name);
                return false;
            }
        }

        return true;
    }
    else if (arg->flags & XO_ARGS_TYPE_DOUBLE_ARRAY)
    {
        char const * argv_name = context->argv[*argv_index];
        _xo_args_arg_array * const array = (_xo_args_arg_array *)arg;
        size_t next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n",
                           context->argv[*argv_index]);
            return false;
        }

        char const * next_value = context->argv[next_index];
        double parsed_value;

        if (true
            == _xo_args_try_parse_double(context->argv[next_index],
                                         &parsed_value))
        {
            _xo_args_arg_array_push(
                context, array, &parsed_value, sizeof(double));
            arg->has_value = true;
            *argv_index = next_index;
        }
        else
        {
            context->print("Error: Value for %s is not a valid number or is "
                           "out of range\n",
                           argv_name);
            return false;
        }

        // Consume every following value until we see a valid argument
        for (++next_index; next_index < (size_t)context->argc; ++next_index)
        {
            next_value = context->argv[next_index];

            for (size_t j = 0; j < context->args_size; ++j)
            {
                xo_args_arg const * const other_arg = context->args[j];
                if (_xo_args_arg_matches_input(other_arg, next_value, NULL))
                {
                    // The next argument is a valid arg so don't parse
                    // that as a value of this string array
                    return true;
                }
            }

            if (true
                == _xo_args_try_parse_double(context->argv[next_index],
                                             &parsed_value))
            {
                _xo_args_arg_array_push(
                    context, array, &parsed_value, sizeof(double));
                arg->has_value = true;
                *argv_index = next_index;
            }
            else
            {
                context->print(
                    "Error: Value for %s is not a valid number or is "
                    "out of range\n",
                    argv_name);
                return false;
            }
        }

        return true;
    }
    else if (arg->flags & XO_ARGS_TYPE_BOOL_ARRAY)
    {
        char const * argv_name = context->argv[*argv_index];
        _xo_args_arg_array * const array = (_xo_args_arg_array *)arg;
        size_t next_index = (*argv_index) + 1;
        if (next_index >= (size_t)context->argc)
        {
            context->print("Error: No value provided for %s\n", argv_name);
            return false;
        }
        char const * next_value = context->argv[next_index];

        bool parsed_value;
        if (_xo_args_try_parse_bool(next_value, &parsed_value))
        {
            _xo_args_arg_array_push(
                context, array, &parsed_value, sizeof(bool));
            arg->has_value = true;
            *argv_index = next_index;
        }
        else
        {
            context->print("Error: Invalid value provided for %s\n"
                           "expected true or false.\n",
                           argv_name);
            return false;
        }

        // Consume every following value until we see a valid argument
        for (++next_index; next_index < (size_t)context->argc; ++next_index)
        {
            next_value = context->argv[next_index];

            for (size_t j = 0; j < context->args_size; ++j)
            {
                xo_args_arg const * const other_arg = context->args[j];
                if (_xo_args_arg_matches_input(other_arg, next_value, NULL))
                {
                    // The next argument is a valid arg so don't parse
                    // that as a value of this string array
                    return true;
                }
            }

            if (_xo_args_try_parse_bool(next_value, &parsed_value))
            {
                _xo_args_arg_array_push(
                    context, array, &parsed_value, sizeof(bool));
                arg->has_value = true;
                *argv_index = next_index;
            }
            else
            {
                context->print("Error: Invalid value provided for %s\n"
                               "expected true or false.\n",
                               argv_name);
                return false;
            }
        }

        return true;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_submit(xo_args_ctx * const context)
{
    if (NULL == context)
    {
        XO_ARGS_ASSERT(NULL != context, "xo_args_ctx must not be null here.");
        return false;
    }

    xo_args_arg const * const arg_help = xo_args_declare_arg(
        context, "help", "h", NULL, "show this message", XO_ARGS_TYPE_SWITCH);

    xo_args_arg const * arg_version = NULL;
    if (NULL != context->app_version)
    {
        arg_version = xo_args_declare_arg(context,
                                          "version",
                                          "v",
                                          NULL,
                                          "shows the program version",
                                          XO_ARGS_TYPE_SWITCH);
    }

    for (size_t i = 1; i < (size_t)context->argc; ++i)
    {
        char const * const argv_arg = context->argv[i];
        size_t const argv_arg_len = strlen(argv_arg);

        // This is an unexpected case but we will try to ignore it.
        if (argv_arg_len == 0)
        {
            continue;
        }
        // All valid variables begin with '-' or '--' so a single
        // character argument at this position is unexpected
        // and so is a string not starting with '-'
        else if (argv_arg_len == 1 || argv_arg[0] != '-')
        {
            context->print("Error: unknown argument \"%s\"\n", argv_arg);
            _xo_print_try_help(context);
            return false;
        }
        // At this point there is a chance the user has input a valid argument
        else
        {
            bool parsed_arg = false;
            for (size_t j = 0; j < context->args_size; ++j)
            {
                _xo_args_arg_match match;
                if (_xo_args_arg_matches_input(
                        context->args[j], argv_arg, &match))
                {
                    if (false
                        == _xo_args_try_parse_arg(
                            context, &i, context->args[j], &match))
                    {
                        _xo_print_try_help(context);
                        return false;
                    }
                    parsed_arg = true;
                    break;
                }
            }
            if (parsed_arg)
            {
                continue;
            }
            else
            {
                // the argv_arg looks like an argument but didn't match any
                // known arguments.
                context->print("Error: unknown argument \"%s\"\n", argv_arg);
                _xo_print_try_help(context);
                return false;
            }
        }
    }

    bool help = false;
    if (xo_args_try_get_bool(arg_help, &help) && true == help)
    {
        xo_args_print_help(context);
        return false;
    }

    bool version = false;
    if ((NULL != context->app_version)
        && xo_args_try_get_bool(arg_version, &version) && (true == version))
    {
        xo_args_print_help(context);
        return false;
    }

    for (size_t i = 0; i < context->args_size; ++i)
    {
        xo_args_arg const * const arg = context->args[i];
        if ((arg->flags & XO_ARGS_ARG_REQUIRED) && (false == arg->has_value))
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

////////////////////////////////////////////////////////////////////////////////
void xo_args_destroy_ctx(xo_args_ctx * context)
{
    if (NULL == context)
    {
        XO_ARGS_ASSERT(NULL != context, "xo_args_ctx must not be null here.");
        return;
    }
    // There is no need to use the tracked delete here.
    // It performs extra work such as swapping elements which is intended to
    // keep the list valid after freeing each element. We don't care about that.
    for (size_t i = 0; i < context->allocations_size; ++i)
    {
        context->free(context->allocations[i]);
    }
    context->free(context->allocations);
    context->free(context);
}

////////////////////////////////////////////////////////////////////////////////
xo_args_arg * xo_args_declare_arg(xo_args_ctx * const context,
                                  char const * const name,
                                  char const * const short_name,
                                  char const * const value_tip,
                                  char const * const description,
                                  XO_ARGS_ARG_FLAG const flags)
{
    (void)value_tip;
    (void)description;
    if (NULL == context)
    {
        XO_ARGS_ASSERT(NULL != context, "xo_args_ctx must not be null here.");
        return NULL;
    }
    if (NULL == name)
    {
        XO_ARGS_ASSERT(NULL != name, "name must not be null here");
        return NULL;
    }

    size_t const name_len = strlen(name);
    XO_ARGS_ASSERT(name_len != 0,
                   "name must be a valid string with a length >= 1");

    size_t const short_name_len = NULL != short_name ? strlen(short_name) : 0;
    XO_ARGS_ASSERT(short_name == NULL || short_name_len != 0,
                   "if a short name is provided it must have a length >= 1");

    bool const name_is_alnum = _xo_isalnum_str(name, name + name_len);
    if (false == name_is_alnum)
    {
        XO_ARGS_ASSERT(true == name_is_alnum,
                       "argument names must be alphanumeric");
        return NULL;
    }

    if (short_name_len > 0)
    {
        bool const short_name_is_alnum =
            _xo_isalnum_str(short_name, short_name + short_name_len);
        if (false == short_name_is_alnum)
        {
            XO_ARGS_ASSERT(true == short_name_is_alnum,
                           "argument short names must be alphanumeric");
            return NULL;
        }
    }

    XO_ARGS_ARG_FLAG const all_types =
        (XO_ARGS_ARG_FLAG)(XO_ARGS_TYPE_STRING | XO_ARGS_TYPE_SWITCH
                           | XO_ARGS_TYPE_BOOL | XO_ARGS_TYPE_INT
                           | XO_ARGS_TYPE_DOUBLE | XO_ARGS_TYPE_BOOL_ARRAY
                           | XO_ARGS_TYPE_INT_ARRAY | XO_ARGS_TYPE_DOUBLE_ARRAY
                           | XO_ARGS_TYPE_STRING_ARRAY);
    {
        // Extract the type from the provided flags and count the set bits
        // if there is more than one type bit set: the argument declaration is
        // invalid.
        XO_ARGS_ARG_FLAG const type_flag =
            (XO_ARGS_ARG_FLAG)(flags & all_types);
        size_t type_flag_temp = type_flag;
        size_t bits = 0;
        for (; type_flag_temp; ++bits)
        {
            type_flag_temp &= type_flag_temp - 1;
        }

        if (bits > 1)
        {
            XO_ARGS_ASSERT(bits <= 1,
                           "arguments must only have one or zero types set");
            return NULL;
        }
    }

    // Look for conflicts with existing arguments first.
    for (size_t i = 0; i < context->args_size; ++i)
    {
        xo_args_arg * const existing_arg = context->args[i];
        if ((name_len == existing_arg->name_length)
            && (0 == strcmp(existing_arg->name, name)))
        {
            context->print("xo-args error: %s argument name conflict. name:"
                           " %s\n",
                           __func__,
                           name);
            return NULL;
        }
        if ((NULL != short_name) && (NULL != existing_arg->short_name)
            && (short_name_len == existing_arg->short_name_length)
            && (0 == strcmp(existing_arg->short_name, short_name)))
        {
            context->print("xo-args error: %s argument short_name conflict."
                           " short_name: %s\n",
                           __func__,
                           short_name);
            return NULL;
        }
    }

    // We're going to create a concrete argument and use it polymorphically
    // throughout this function through an 'arg' variable.
    _xo_args_arg_single * arg_single = NULL;
    _xo_args_arg_array * arg_array = NULL;

    if (_xo_args_arg_flag_is_array(flags))
    {
        arg_array = (_xo_args_arg_array *)_xo_args_tracked_alloc(
            context, sizeof(_xo_args_arg_array));
        // We will allocate the array on first push
        arg_array->array_size = 0;
        arg_array->array_reserved = 0;
        arg_array->array = NULL;
    }
    else
    {
        arg_single = (_xo_args_arg_single *)_xo_args_tracked_alloc(
            context, sizeof(_xo_args_arg_single));
    }

    xo_args_arg * const arg = NULL != arg_array ? (xo_args_arg *)arg_array
                                                : (xo_args_arg *)arg_single;
    if (context->args_reserved == context->args_size)
    {
        context->args_reserved *= 2;
        context->args = (xo_args_arg **)_xo_args_tracked_realloc(
            context,
            context->args,
            context->args_reserved * sizeof(xo_args_arg *));
    }

    context->args[context->args_size++] = arg;

    // If any type flag is set: use the flags as is otherwise take the provided
    // flags and assign the default type of string
    arg->flags =
        (XO_ARGS_ARG_FLAG)((all_types & flags) ? flags
                                               : flags | XO_ARGS_TYPE_STRING);

    // A required switch doesn't make much sense so we will just assume the dev
    // meant the switch should behave normally (optionally).
    if ((XO_ARGS_TYPE_SWITCH | XO_ARGS_ARG_REQUIRED)
        == (flags & (XO_ARGS_TYPE_SWITCH | XO_ARGS_ARG_REQUIRED)))
    {
        arg->flags = (XO_ARGS_ARG_FLAG)(arg->flags & ~XO_ARGS_ARG_REQUIRED);
    }

    {
        char * const buff =
            (char *)_xo_args_tracked_alloc(context, name_len + 1);
        memcpy(buff, name, name_len + 1);
        arg->name = buff;
        arg->name_length = name_len;
    }

    if (NULL != short_name)
    {
        char * const buff =
            (char *)_xo_args_tracked_alloc(context, short_name_len + 1);
        memcpy(buff, short_name, short_name_len + 1);
        arg->short_name = buff;
        arg->short_name_length = short_name_len;
    }
    else
    {
        arg->short_name = NULL;
        arg->short_name_length = 0;
    }

    if (NULL != description)
    {
        size_t const description_length = strlen(description);
        char * const buff =
            (char *)_xo_args_tracked_alloc(context, description_length + 1);
        memcpy(buff, description, description_length + 1);
        arg->description = buff;
        arg->description_length = description_length;
    }
    else
    {
        arg->description = NULL;
        arg->description_length = 0;
    }

    if (NULL != value_tip)
    {
        size_t const value_tip_length = strlen(value_tip);
        char * const buff =
            (char *)_xo_args_tracked_alloc(context, value_tip_length + 1);
        memcpy(buff, value_tip, value_tip_length + 1);
        arg->value_tip = buff;
        arg->value_tip_length = value_tip_length;
    }
    else if (arg->flags & XO_ARGS_TYPE_STRING)
    {
        arg->value_tip = "<text>";
        arg->value_tip_length = 6;
    }
    else if (arg->flags & XO_ARGS_TYPE_INT)
    {
        arg->value_tip = "<integer>";
        arg->value_tip_length = 9;
    }
    else if (arg->flags & XO_ARGS_TYPE_DOUBLE)
    {
        arg->value_tip = "<number>";
        arg->value_tip_length = 8;
    }
    else if (arg->flags & XO_ARGS_TYPE_BOOL)
    {
        arg->value_tip = "<true|false>";
        arg->value_tip_length = 12;
    }
    else if (arg->flags & XO_ARGS_TYPE_STRING_ARRAY)
    {
        arg->value_tip = "[text]";
        arg->value_tip_length = 6;
    }
    else if (arg->flags & XO_ARGS_TYPE_INT_ARRAY)
    {
        arg->value_tip = "[integer]";
        arg->value_tip_length = 9;
    }
    else if (arg->flags & XO_ARGS_TYPE_DOUBLE_ARRAY)
    {
        arg->value_tip = "[number]";
        arg->value_tip_length = 8;
    }
    else if (arg->flags & XO_ARGS_TYPE_BOOL_ARRAY)
    {
        arg->value_tip = "[true|false]";
        arg->value_tip_length = 12;
    }
    else
    {
        // Switches don't get a tip because they don't have a value that follows
        arg->value_tip = NULL;
        arg->value_tip_length = 0;
    }

    arg->has_value = false;
    return arg;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_string(xo_args_arg const * const arg,
                            char const ** out_string)
{
    if (NULL == arg)
    {
        XO_ARGS_ASSERT(NULL != arg, "argument is null");
        return false;
    }
    if (NULL == out_string)
    {
        XO_ARGS_ASSERT(NULL != out_string, "out param is null");
        return false;
    }
    if (XO_ARGS_TYPE_STRING != (arg->flags & XO_ARGS_TYPE_STRING))
    {
        XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_STRING,
                       "incorrect argument type");
        return false;
    }
    if (arg->has_value)
    {
        *out_string = ((_xo_args_arg_single *)arg)->value._string;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_int(xo_args_arg const * const arg, int64_t * out_int)
{
    if (NULL == arg)
    {
        XO_ARGS_ASSERT(NULL != arg, "argument is null");
        return false;
    }
    if (NULL == out_int)
    {
        XO_ARGS_ASSERT(NULL != out_int, "out param is null");
        return false;
    }
    if (XO_ARGS_TYPE_INT != (arg->flags & XO_ARGS_TYPE_INT))
    {
        XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_INT,
                       "incorrect argument type");
        return false;
    }
    if (arg->has_value)
    {
        *out_int = ((_xo_args_arg_single *)arg)->value._int;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_double(xo_args_arg const * const arg, double * out_double)
{
    if (NULL == arg)
    {
        XO_ARGS_ASSERT(NULL != arg, "argument is null");
        return false;
    }
    if (NULL == out_double)
    {
        XO_ARGS_ASSERT(NULL != out_double, "out param is null");
        return false;
    }
    if (XO_ARGS_TYPE_DOUBLE != (arg->flags & XO_ARGS_TYPE_DOUBLE))
    {
        XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_DOUBLE,
                       "incorrect argument type");
        return false;
    }
    if (arg->has_value)
    {
        *out_double = ((_xo_args_arg_single *)arg)->value._double;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_bool(xo_args_arg const * const arg, bool * out_bool)
{
    if (NULL == arg)
    {
        XO_ARGS_ASSERT(NULL != arg, "argument is null");
        return false;
    }
    if (NULL == out_bool)
    {
        XO_ARGS_ASSERT(NULL != out_bool, "out param is null");
        return false;
    }
    bool const type_is_bool =
        XO_ARGS_TYPE_BOOL == (arg->flags & XO_ARGS_TYPE_BOOL);
    bool const type_is_switch =
        XO_ARGS_TYPE_SWITCH == (arg->flags & XO_ARGS_TYPE_SWITCH);
    if (false == type_is_bool && false == type_is_switch)
    {
        XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_BOOL,
                       "incorrect argument type");
        return false;
    }

    if (type_is_bool)
    {
        if (arg->has_value)
        {
            *out_bool = ((_xo_args_arg_single *)arg)->value._bool;
            return true;
        }
        return false;
    }
    // the type is switch here
    // switches are implicitly false until set - once set they are true
    *out_bool = arg->has_value;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_string_array(xo_args_arg const * const arg,
                                  char const *** out_string_array,
                                  size_t * out_array_count)
{
    if (NULL == arg)
    {
        XO_ARGS_ASSERT(NULL != arg, "argument is null");
        return false;
    }
    if (NULL == out_string_array)
    {
        XO_ARGS_ASSERT(NULL != out_string_array, "out param is null");
        return false;
    }
    if (NULL == out_array_count)
    {
        XO_ARGS_ASSERT(NULL != out_array_count, "out param is null");
        return false;
    }
    if (XO_ARGS_TYPE_STRING_ARRAY != (arg->flags & XO_ARGS_TYPE_STRING_ARRAY))
    {
        XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_STRING_ARRAY,
                       "incorrect argument type");
        return false;
    }
    if (true == arg->has_value)
    {
        *out_array_count = ((_xo_args_arg_array *)arg)->array_size;
        *out_string_array = (char const **)((_xo_args_arg_array *)arg)->array;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_int_array(xo_args_arg const * const arg,
                               int64_t const ** out_int_array,
                               size_t * out_array_count)
{
    if (NULL == arg)
    {
        XO_ARGS_ASSERT(NULL != arg, "argument is null");
        return false;
    }
    if (NULL == out_int_array)
    {
        XO_ARGS_ASSERT(NULL != out_int_array, "out param is null");
        return false;
    }
    if (NULL == out_array_count)
    {
        XO_ARGS_ASSERT(NULL != out_array_count, "out param is null");
        return false;
    }
    if (XO_ARGS_TYPE_INT_ARRAY != (arg->flags & XO_ARGS_TYPE_INT_ARRAY))
    {
        XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_INT_ARRAY,
                       "incorrect argument type");
        return false;
    }
    if (true == arg->has_value)
    {
        *out_array_count = ((_xo_args_arg_array *)arg)->array_size;
        *out_int_array = (int64_t const *)((_xo_args_arg_array *)arg)->array;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_double_array(xo_args_arg const * const arg,
                                  double const ** out_double_array,
                                  size_t * out_array_count)
{
    if (NULL == arg)
    {
        XO_ARGS_ASSERT(NULL != arg, "argument is null");
        return false;
    }
    if (NULL == out_double_array)
    {
        XO_ARGS_ASSERT(NULL != out_double_array, "out param is null");
        return false;
    }
    if (NULL == out_array_count)
    {
        XO_ARGS_ASSERT(NULL != out_array_count, "out param is null");
        return false;
    }
    if (XO_ARGS_TYPE_DOUBLE_ARRAY != (arg->flags & XO_ARGS_TYPE_DOUBLE_ARRAY))
    {
        XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_INT_ARRAY,
                       "incorrect argument type");
        return false;
    }
    if (true == arg->has_value)
    {
        *out_array_count = ((_xo_args_arg_array *)arg)->array_size;
        *out_double_array = (double const *)((_xo_args_arg_array *)arg)->array;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool xo_args_try_get_bool_array(xo_args_arg const * const arg,
                                bool const ** out_bool_array,
                                size_t * out_array_count)
{
    if (NULL == arg)
    {
        XO_ARGS_ASSERT(NULL != arg, "argument is null");
        return false;
    }
    if (NULL == out_bool_array)
    {
        XO_ARGS_ASSERT(NULL != out_bool_array, "out param is null");
        return false;
    }
    if (NULL == out_array_count)
    {
        XO_ARGS_ASSERT(NULL != out_array_count, "out param is null");
        return false;
    }
    if (XO_ARGS_TYPE_BOOL_ARRAY != (arg->flags & XO_ARGS_TYPE_BOOL_ARRAY))
    {
        XO_ARGS_ASSERT(arg->flags & XO_ARGS_TYPE_BOOL_ARRAY,
                       "incorrect argument type");
        return false;
    }
    if (true == arg->has_value)
    {
        *out_array_count = ((_xo_args_arg_array *)arg)->array_size;
        *out_bool_array = (bool const *)((_xo_args_arg_array *)arg)->array;
        return true;
    }
    return false;
}
#endif
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <https://unlicense.org/>