#include "utest.h"
#include "xo-args-test-funcs.h"
#include <math.h>
#include <xo-args/xo-args.h>

////////////////////////////////////////////////////////////////////////////////
// We are creating a test fixture just so we can get a setup/shutdown for the
// global test helpers. The data itself is unused.
struct getters
{
    xo_args_ctx * context;
};

////////////////////////////////////////////////////////////////////////////////
UTEST_F_SETUP(getters)
{
    test_global_setup();
    EXPECT_TRUE(true);

    char const * argv[1] = {"/mock/test.ext"};
    int const argc = sizeof(argv) / sizeof(argv[0]);
    xo_args_ctx * context = xo_args_create_ctx_advanced(argc,
                                                        (xo_argv_t)argv,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        test_alloc,
                                                        test_realloc,
                                                        test_free,
                                                        test_printf);

    ASSERT_TRUE(xo_args_submit(context));

    size_t allocation_count;
    test_get_allocations(&allocation_count);
    ASSERT_NE_MSG(
        0u,
        allocation_count,
        "The allocator is not tracking xo-args allocations properly.");

    xo_args_destroy_ctx(context);
    context = NULL;

    test_get_allocations(&allocation_count);
    ASSERT_EQ_MSG(0u,
                  allocation_count,
                  "There is a memory leak after xo_args_destroy_ctx or there "
                  "is an issue tracking xo-args allocations");

    ASSERT_EQ_MSG(0u,
                  strlen(test_get_stdout()),
                  "xo-args was not expected to print anything.");
    ASSERT_EQ_MSG(
        0u, test_get_assert_count(), "xo-args was not expected to assert");

    utest_fixture->context = NULL;
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F_TEARDOWN(getters)
{
    ASSERT_EQ(NULL, (void *)utest_fixture->context);

    size_t allocation_count;
    test_get_allocations(&allocation_count);
    ASSERT_EQ_MSG(0u,
                  allocation_count,
                  "There is a memory leak after xo_args_destroy_ctx or there "
                  "is an issue tracking xo-args allocations");

    // Each test is expected to have a clean shutdown.
    // In tests where printing or asserting is expected: that test should check
    // for that output / those asserts and then call test_global_clear
    ASSERT_EQ(0u, strlen(test_get_stdout()));
    ASSERT_EQ(0u, test_get_assert_count());

    test_global_shutdown();
}

////////////////////////////////////////////////////////////////////////////////
void _test_init_context(struct getters * const utest_fixture,
                        int const argc,
                        char const ** argv)
{
    utest_fixture->context = xo_args_create_ctx_advanced(argc,
                                                         (xo_argv_t)argv,
                                                         NULL,
                                                         NULL,
                                                         NULL,
                                                         test_alloc,
                                                         test_realloc,
                                                         test_free,
                                                         test_printf);
}

////////////////////////////////////////////////////////////////////////////////
#define _TEST_INIT_CONTEXT(utest_fixture, argv)                                \
    do                                                                         \
    {                                                                          \
        ASSERT_EQ(NULL, (void *)utest_fixture->context);                       \
        _test_init_context(                                                    \
            utest_fixture, (int)(sizeof(argv) / sizeof(argv[0])), argv);       \
    } while (0)

////////////////////////////////////////////////////////////////////////////////
// A helper to check that a test has concluded while only producing a certain
// string as standard output. This depends on proper test setup and that the
// test context has been shutdown.
#define _TEST_EXPECT_STDOUT(str)                                               \
    do                                                                         \
    {                                                                          \
        size_t allocation_count;                                               \
        test_get_allocations(&allocation_count);                               \
        ASSERT_EQ(0u, allocation_count);                                       \
        ASSERT_NE(NULL, strstr(test_get_stdout(), str));                       \
        ASSERT_EQ(0u, test_get_assert_count());                                \
    } while (0)

////////////////////////////////////////////////////////////////////////////////
// A helper to check that the test has been shutdown with no asserts or output
#define _TEST_EXPECT_CLEAN_SHUTDOWN()                                          \
    do                                                                         \
    {                                                                          \
        size_t allocation_count;                                               \
        test_get_allocations(&allocation_count);                               \
        ASSERT_EQ(0u, allocation_count);                                       \
        ASSERT_EQ(0U, strlen(test_get_stdout()));                              \
        ASSERT_EQ(0u, test_get_assert_count());                                \
    } while (0)

////////////////////////////////////////////////////////////////////////////////
void _test_destroy_context(struct getters * utest_fixture)
{
    xo_args_destroy_ctx(utest_fixture->context);
    utest_fixture->context = NULL;
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_true_values)
{
    char const * positive_values[] = {"true", "True", "TRUE", "1"};
    const size_t positive_values_count =
        sizeof(positive_values) / sizeof(positive_values[0]);

    for (size_t i = 0; i < positive_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", positive_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        bool foo_value = false;
        ASSERT_TRUE(xo_args_try_get_bool(foo, &foo_value));
        ASSERT_TRUE(foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_false_values)
{
    char const * negative_values[] = {"false", "False", "FALSE", "0"};
    size_t const negative_values_count =
        sizeof(negative_values) / sizeof(negative_values[0]);

    for (size_t i = 0; i < negative_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", negative_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        bool foo_value = true;
        ASSERT_TRUE(xo_args_try_get_bool(foo, &foo_value));
        ASSERT_FALSE(foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_short_name)
{
    char const * argv[] = {"/mock/test.ext", "-f", "true"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            "f",
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool foo_value = false;
    ASSERT_TRUE(xo_args_try_get_bool(foo, &foo_value));
    ASSERT_TRUE(foo_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_invalid_values)
{
    char const * invalid_values[] = {"t", "yes", "10", "truet", "", " "};
    size_t const invalid_values_count =
        sizeof(invalid_values) / sizeof(invalid_values[0]);

    for (size_t i = 0; i < invalid_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", invalid_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_FALSE(xo_args_submit(utest_fixture->context));

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_STDOUT("Invalid value provided for --foo");
        _TEST_EXPECT_STDOUT("--help");
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_missing_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("No value provided");
    _TEST_EXPECT_STDOUT("--help");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_missing_value_followed_by_valid)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "--baz", "BAZ"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
    ASSERT_NE(NULL, (void *)foo);

    xo_args_declare_arg(utest_fixture->context,
                        "baz",
                        NULL,
                        NULL,
                        NULL,
                        XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_STRING);

    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("Invalid value provided for --foo");
    _TEST_EXPECT_STDOUT("--help");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_assignment_true_values)
{
    char const * positive_values[] = {"--foo=true",
                                      "--foo=True",
                                      "--foo=TRUE",
                                      "--foo=1",
                                      "-f=true",
                                      "-f=True",
                                      "-f=TRUE",
                                      "-f=1"};
    const size_t positive_values_count =
        sizeof(positive_values) / sizeof(positive_values[0]);

    for (size_t i = 0; i < positive_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", positive_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                "f",
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        bool foo_value = false;
        ASSERT_TRUE(xo_args_try_get_bool(foo, &foo_value));
        ASSERT_TRUE(foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_assignment_false_values)
{
    char const * negative_values[] = {"--foo=false",
                                      "--foo=False",
                                      "--foo=FALSE",
                                      "--foo=0",
                                      "-f=false",
                                      "-f=False",
                                      "-f=FALSE",
                                      "-f=0"};
    size_t const negative_values_count =
        sizeof(negative_values) / sizeof(negative_values[0]);

    for (size_t i = 0; i < negative_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", negative_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                "f",
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        bool foo_value = true;
        ASSERT_TRUE(xo_args_try_get_bool(foo, &foo_value));
        ASSERT_FALSE(foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_assignment_invalid_values)
{
    char const * invalid_values[] = {"--foo=",
                                     "--foo false",
                                     "--foo= false",
                                     "--foo =false",
                                     "--foo=\"false\"",
                                     "--foo='false'",
                                     "--foo=false ",
                                     "--foo==false"};
    size_t const invalid_values_count =
        sizeof(invalid_values) / sizeof(invalid_values[0]);

    for (size_t i = 0; i < invalid_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", invalid_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_FALSE(xo_args_submit(utest_fixture->context));

        _test_destroy_context(utest_fixture);

        // This test will produce both invalid values for foo and unknown
        // arguments
        //_TEST_EXPECT_STDOUT("Invalid value provided for --foo");
        _TEST_EXPECT_STDOUT("--help");
        test_global_clear();
    }

    char const * argv[] = {"/mock/test.ext", "--foo=", "false"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_BOOL);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("Invalid value provided for --foo");
    _TEST_EXPECT_STDOUT("--help");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, switch_present)
{
    char const * argv[] = {"/mock/test.ext", "--foo"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo = xo_args_declare_arg(
        utest_fixture->context, "foo", NULL, NULL, NULL, XO_ARGS_TYPE_SWITCH);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool switch_value = false;
    ASSERT_TRUE(xo_args_try_get_bool(foo, &switch_value));
    ASSERT_TRUE(switch_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, switch_short_name)
{
    char const * argv[] = {"/mock/test.ext", "-f"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo = xo_args_declare_arg(
        utest_fixture->context, "foo", "f", NULL, NULL, XO_ARGS_TYPE_SWITCH);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool switch_value = false;
    ASSERT_TRUE(xo_args_try_get_bool(foo, &switch_value));
    ASSERT_TRUE(switch_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, switch_absent)
{
    char const * argv[] = {"/mock/test.ext"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo = xo_args_declare_arg(
        utest_fixture->context, "foo", NULL, NULL, NULL, XO_ARGS_TYPE_SWITCH);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool switch_value = true;
    ASSERT_TRUE_MSG(xo_args_try_get_bool(foo, &switch_value),
                    "xo_args_try_get_bool should succeed when getting a "
                    "switch value even if it isn't provided. This is "
                    "because switches are implicitly false.");
    ASSERT_EQ(false, switch_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, switch_absent_and_required)
{
    char const * argv[] = {"/mock/test.ext"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_SWITCH);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE_MSG(
        xo_args_submit(utest_fixture->context),
        "Submit should still succeed here even though the required "
        "argument foo is missing. This is because the switch type ignores "
        "the required flag. A required switch doesn't make sense.");

    bool switch_value = true;
    ASSERT_TRUE_MSG(xo_args_try_get_bool(foo, &switch_value),
                    "Getting a switch value that hasn't been set should be "
                    "successful. The value should be set to false since "
                    "the value is implicitly false.");
    ASSERT_EQ(false, switch_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, switch_with_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "false"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo = xo_args_declare_arg(
        utest_fixture->context, "foo", NULL, NULL, NULL, XO_ARGS_TYPE_SWITCH);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_FALSE_MSG(xo_args_submit(utest_fixture->context),
                     "Submit should fail because the argument 'false' is not "
                     "recognized. ");

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("unknown argument");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, switch_set_twice)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "--foo"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo = xo_args_declare_arg(
        utest_fixture->context, "foo", NULL, NULL, NULL, XO_ARGS_TYPE_SWITCH);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_FALSE_MSG(xo_args_submit(utest_fixture->context),
                     "Submit should fail because the argument --foo was passed "
                     "multiple times");

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("provided multiple times");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, switch_before_valid_input)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "--baz", "BAZ"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo = xo_args_declare_arg(
        utest_fixture->context, "foo", NULL, NULL, NULL, XO_ARGS_TYPE_SWITCH);
    ASSERT_NE(NULL, (void *)foo);

    xo_args_declare_arg(utest_fixture->context,
                        "baz",
                        NULL,
                        NULL,
                        NULL,
                        XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_STRING);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool switch_value = false;
    ASSERT_TRUE(xo_args_try_get_bool(foo, &switch_value));
    ASSERT_EQ(true, switch_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_positive_values)
{
    // check a bunch of variants of 0xdead (leading 0 is octal)
    char const * positive_values[] = {
        "57005", "0x0000DEAD", "0157255", "+57005"};
    size_t const positive_values_count =
        sizeof(positive_values) / sizeof(positive_values[0]);

    for (size_t i = 0; i < positive_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", positive_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        int64_t foo_value = 0;
        ASSERT_TRUE(xo_args_try_get_int(foo, &foo_value));
        ASSERT_EQ(57005, foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_negative_values)
{
    // check a bunch of variants of 0xdead (leading 0 is octal)
    char const * negative_values[] = {"-57005", "-0x0000DEAD", "-0157255"};
    size_t const negative_values_count =
        sizeof(negative_values) / sizeof(negative_values[0]);

    for (size_t i = 0; i < negative_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", negative_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        int64_t foo_value = 0;
        ASSERT_TRUE(xo_args_try_get_int(foo, &foo_value));
        ASSERT_EQ(-57005, foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_short_name)
{
    char const * argv[] = {"/mock/test.ext", "-f", "57005"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            "f",
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    int64_t foo_value = 0;
    ASSERT_TRUE(xo_args_try_get_int(foo, &foo_value));
    ASSERT_EQ(57005, foo_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_max)
{
    ASSERT_EQ_MSG(LLONG_MAX,
                  9223372036854775807,
                  "Test assumes that long long max is 9223372036854775807");

    char const * argv[] = {"/mock/test.ext", "--foo", "9223372036854775807"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    int64_t foo_value = 0;
    ASSERT_TRUE(xo_args_try_get_int(foo, &foo_value));
    ASSERT_EQ(9223372036854775807, foo_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_min)
{
    ASSERT_EQ_MSG(LLONG_MIN,
                  (-(int64_t)9223372036854775807 - 1),
                  "Test assumes that int min is -9223372036854775808");

    char const * argv[] = {"/mock/test.ext", "--foo", "-9223372036854775808"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    int64_t foo_value = 0;
    ASSERT_TRUE(xo_args_try_get_int(foo, &foo_value));
    // positive 9223372036854775808 is an unsigned literal so we have to use
    // 2147483647 (an integer literal) with the unary minus operator and
    // subtract one to check against INT_MIN here.
    ASSERT_EQ((-(int64_t)9223372036854775807 - 1), foo_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_zero)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "0"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    int64_t foo_value = 1;
    ASSERT_TRUE(xo_args_try_get_int(foo, &foo_value));

    ASSERT_EQ(0, foo_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_invalid_values)
{
    char const * invalid_values[] = {"9223372036854775808",
                                     "-9223372036854775809",
                                     "",
                                     " ",
                                     "++1",
                                     "1.0",
                                     "1.",
                                     "0xabcdefg",
                                     "o10",
                                     "10o",
                                     "false"};

    size_t const invalid_values_count =
        sizeof(invalid_values) / sizeof(invalid_values[0]);

    for (size_t i = 0; i < invalid_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", invalid_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_FALSE(xo_args_submit(utest_fixture->context));

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_STDOUT("is not a valid integer");
        _TEST_EXPECT_STDOUT("--help");
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_missing_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("No value provided");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_missing_value_followed_by_valid)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "--baz", "BAZ"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
    ASSERT_NE(NULL, (void *)foo);

    xo_args_declare_arg(utest_fixture->context,
                        "baz",
                        NULL,
                        NULL,
                        NULL,
                        XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_STRING);

    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("is not a valid integer");
    _TEST_EXPECT_STDOUT("--help");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_assignment_positive_values)
{
    // check a bunch of variants of 0xdead (leading 0 is octal)
    char const * positive_values[] = {"--foo=57005",
                                      "--foo=0x0000DEAD",
                                      "--foo=0157255",
                                      "--foo=+57005",
                                      "-f=57005",
                                      "-f=0x0000DEAD",
                                      "-f=0157255",
                                      "-f=+57005"};
    size_t const positive_values_count =
        sizeof(positive_values) / sizeof(positive_values[0]);

    for (size_t i = 0; i < positive_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", positive_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                "f",
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        int64_t foo_value = 0;
        ASSERT_TRUE(xo_args_try_get_int(foo, &foo_value));
        ASSERT_EQ(57005, foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_assignment_negative_values)
{
    // check a bunch of variants of 0xdead (leading 0 is octal)
    char const * negative_values[] = {"--foo=-57005",
                                      "--foo=-0x0000DEAD",
                                      "--foo=-0157255",
                                      "-f=-57005",
                                      "-f=-0x0000DEAD",
                                      "-f=-0157255"};
    size_t const negative_values_count =
        sizeof(negative_values) / sizeof(negative_values[0]);

    for (size_t i = 0; i < negative_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", negative_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                "f",
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        int64_t foo_value = 0;
        ASSERT_TRUE(xo_args_try_get_int(foo, &foo_value));
        ASSERT_EQ(-57005, foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_assignment_invalid_values)
{
    char const * invalid_values[] = {"--foo=",
                                     "--foo 1337",
                                     "--foo= 1337",
                                     "--foo =1337",
                                     "--foo=\"1337\"",
                                     "--foo='1337'",
                                     "--foo=1337 ",
                                     "--foo==1337"};

    size_t const invalid_values_count =
        sizeof(invalid_values) / sizeof(invalid_values[0]);

    for (size_t i = 0; i < invalid_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", invalid_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                "f",
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_FALSE(xo_args_submit(utest_fixture->context));

        _test_destroy_context(utest_fixture);

        // This test can produce both invalid values for --foo and also unknown
        // arguments.
        //_TEST_EXPECT_STDOUT("Value for --foo is not a valid integer");
        _TEST_EXPECT_STDOUT("--help");
        test_global_clear();
    }
    char const * argv[] = {"/mock/test.ext", "--foo=", "1337"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            "f",
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_INT);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("Value for --foo is not a valid integer");
    _TEST_EXPECT_STDOUT("--help");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_positive_values)
{
    // check a bunch of variants of positive double values
    char const * positive_values[] = {"57005.0",
                                      "57005",
                                      "5.7005e4",
                                      "5.7005E4",
                                      "+57005.0",
                                      "0.57005e5",
                                      "5.7e1",
                                      "1.23456789"};
    double expected_values[] = {
        57005.0, 57005.0, 57005.0, 57005.0, 57005.0, 57005.0, 57.0, 1.23456789};
    size_t const positive_values_count =
        sizeof(positive_values) / sizeof(positive_values[0]);

    for (size_t i = 0; i < positive_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", positive_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        double foo_value = 0.0;
        ASSERT_TRUE(xo_args_try_get_double(foo, &foo_value));
        ASSERT_EQ(expected_values[i], foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_negative_values)
{
    // check a bunch of variants of negative double values
    char const * negative_values[] = {"-57005.0",
                                      "-5.7005e4",
                                      "-5.7005E4",
                                      "-0.57005e5",
                                      "-5.7e1",
                                      "-1.23456789"};
    double expected_values[] = {
        -57005.0, -57005.0, -57005.0, -57005.0, -57.0, -1.23456789};
    size_t const negative_values_count =
        sizeof(negative_values) / sizeof(negative_values[0]);

    for (size_t i = 0; i < negative_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", negative_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        double foo_value = 0.0;
        ASSERT_TRUE(xo_args_try_get_double(foo, &foo_value));
        ASSERT_EQ(expected_values[i], foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_short_name)
{
    char const * argv[] = {"/mock/test.ext", "-f", "3.14"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            "f",
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    double foo_value = 0.0;
    ASSERT_TRUE(xo_args_try_get_double(foo, &foo_value));
    ASSERT_EQ(3.14, foo_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
#ifdef NAN
UTEST_F(getters, double_nan)
{
    // check a bunch of variants of negative double values
    char const * nan_values[] = {"NaN", "NAN", "+NaN", "-NaN", "NaN(2)"};
    size_t const nan_values_count = sizeof(nan_values) / sizeof(nan_values[0]);

    for (size_t i = 0; i < nan_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", nan_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        double foo_value = 0.0;
        ASSERT_TRUE(xo_args_try_get_double(foo, &foo_value));

        ASSERT_TRUE(isnan(foo_value));

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
#ifdef INFINITY
UTEST_F(getters, double_inf)
{
    // check a bunch of variants of negative double values
    char const * inf_values[] = {
        "inf", "INF", "infinity", "INFINITY", "-inf", "+inf"};
    size_t const inf_values_count = sizeof(inf_values) / sizeof(inf_values[0]);

    for (size_t i = 0; i < inf_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", inf_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        double foo_value = 0.0;
        ASSERT_TRUE(xo_args_try_get_double(foo, &foo_value));

        ASSERT_TRUE(isinf(foo_value));

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_invalid_values)
{
    char const * invalid_values[] = {
        "--3.14", "++3.14", "3.14f", "", " ", "foo"};

    size_t const invalid_values_count =
        sizeof(invalid_values) / sizeof(invalid_values[0]);

    for (size_t i = 0; i < invalid_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", "--foo", invalid_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_FALSE(xo_args_submit(utest_fixture->context));

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_STDOUT("is not a valid number");
        _TEST_EXPECT_STDOUT("--help");
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_missing_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
    ASSERT_NE(NULL, (void *)foo);
    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);
    _TEST_EXPECT_STDOUT("No value provided");
    _TEST_EXPECT_STDOUT("--help");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_assignment_positive_values)
{
    char const * positive_values[] = {"--foo=57005.0",
                                      "--foo=57005",
                                      "--foo=5.7005e4",
                                      "--foo=5.7005E4",
                                      "--foo=+57005.0",
                                      "--foo=0.57005e5",
                                      "--foo=5.7e1",
                                      "--foo=1.23456789",
                                      "-f=57005.0",
                                      "-f=57005",
                                      "-f=5.7005e4",
                                      "-f=5.7005E4",
                                      "-f=+57005.0",
                                      "-f=0.57005e5",
                                      "-f=5.7e1",
                                      "-f=1.23456789"};
    double expected_values[] = {57005.0,
                                57005.0,
                                57005.0,
                                57005.0,
                                57005.0,
                                57005.0,
                                57.0,
                                1.23456789,
                                57005.0,
                                57005.0,
                                57005.0,
                                57005.0,
                                57005.0,
                                57005.0,
                                57.0,
                                1.23456789};
    size_t const positive_values_count =
        sizeof(positive_values) / sizeof(positive_values[0]);

    for (size_t i = 0; i < positive_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", positive_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                "f",
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        double foo_value = 0.0;
        ASSERT_TRUE(xo_args_try_get_double(foo, &foo_value));
        ASSERT_EQ(expected_values[i], foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_assignment_negative_values)
{
    char const * negative_values[] = {"--foo=-57005.0",
                                      "--foo=-5.7005e4",
                                      "--foo=-5.7005E4",
                                      "--foo=-0.57005e5",
                                      "--foo=-5.7e1",
                                      "--foo=-1.23456789",
                                      "-f=-57005.0",
                                      "-f=-5.7005e4",
                                      "-f=-5.7005E4",
                                      "-f=-0.57005e5",
                                      "-f=-5.7e1",
                                      "-f=-1.23456789"};
    double expected_values[] = {-57005.0,
                                -57005.0,
                                -57005.0,
                                -57005.0,
                                -57.0,
                                -1.23456789,
                                -57005.0,
                                -57005.0,
                                -57005.0,
                                -57005.0,
                                -57.0,
                                -1.23456789};
    size_t const negative_values_count =
        sizeof(negative_values) / sizeof(negative_values[0]);

    for (size_t i = 0; i < negative_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", negative_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                "f",
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        double foo_value = 0.0;
        ASSERT_TRUE(xo_args_try_get_double(foo, &foo_value));
        ASSERT_EQ(expected_values[i], foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_assignment_invalid_values)
{
    char const * invalid_values[] = {"--foo=",
                                     "--foo 3.14",
                                     "--foo= 3.14",
                                     "--foo =3.14",
                                     "--foo=\"3.14\"",
                                     "--foo='3.14'",
                                     "--foo=3.14 ",
                                     "--foo==3.14"};
    size_t const invalid_values_count =
        sizeof(invalid_values) / sizeof(invalid_values[0]);

    for (size_t i = 0; i < invalid_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", invalid_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                NULL,
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_DOUBLE);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_FALSE(xo_args_submit(utest_fixture->context));

        _test_destroy_context(utest_fixture);

        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_with_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "FOO"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_STRING | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    char const * string_value = NULL;
    ASSERT_TRUE(xo_args_try_get_string(foo, &string_value));
    ASSERT_STREQ("FOO", string_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_short_name)
{
    char const * argv[] = {"/mock/test.ext", "-f", "FOO"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            "f",
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_STRING | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    char const * string_value = NULL;
    ASSERT_TRUE(xo_args_try_get_string(foo, &string_value));
    ASSERT_STREQ("FOO", string_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_with_no_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_STRING | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("No value provided");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_with_empty_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo", ""};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_STRING | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    char const * string_value = NULL;
    ASSERT_TRUE(xo_args_try_get_string(foo, &string_value));
    ASSERT_STREQ("", string_value);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_assignment_with_value)
{
    char const * input_values[] = {"--foo=FOO",
                                   "--foo= FOO",
                                   "--foo=FOO ",
                                   "-f=FOO",
                                   "-f= FOO",
                                   "-f=FOO "};
    char const * expected_values[] = {
        "FOO", " FOO", "FOO ", "FOO", " FOO", "FOO "};
    size_t const positive_values_count =
        sizeof(input_values) / sizeof(input_values[0]);

    for (size_t i = 0; i < positive_values_count; ++i)
    {
        char const * argv[] = {"/mock/test.ext", input_values[i]};
        _TEST_INIT_CONTEXT(utest_fixture, argv);

        xo_args_arg const * foo =
            xo_args_declare_arg(utest_fixture->context,
                                "foo",
                                "f",
                                NULL,
                                NULL,
                                XO_ARGS_ARG_REQUIRED | XO_ARGS_TYPE_STRING);
        ASSERT_NE(NULL, (void *)foo);
        ASSERT_TRUE(xo_args_submit(utest_fixture->context));

        char const * foo_value = NULL;
        ASSERT_TRUE(xo_args_try_get_string(foo, &foo_value));
        ASSERT_STREQ(expected_values[i], foo_value);

        _test_destroy_context(utest_fixture);

        _TEST_EXPECT_CLEAN_SHUTDOWN();
        test_global_clear();
    }
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_array_one_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "FOO"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_STRING_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    char const ** string_array_value = NULL;
    size_t string_array_count = 0;
    ASSERT_TRUE(xo_args_try_get_string_array(
        foo, &string_array_value, &string_array_count));
    ASSERT_EQ(1u, string_array_count);
    ASSERT_STREQ("FOO", string_array_value[0]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_array_two_values)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "FOO", "BAR"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_STRING_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    char const ** string_array_value = NULL;
    size_t string_array_count = 0;

    ASSERT_TRUE(xo_args_try_get_string_array(
        foo, &string_array_value, &string_array_count));
    ASSERT_EQ(2u, string_array_count);
    ASSERT_STREQ("FOO", string_array_value[0]);
    ASSERT_STREQ("BAR", string_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_array_two_values_separate)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "FOO", "--foo", "BAR"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_STRING_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    char const ** string_array_value = NULL;
    size_t string_array_count = 0;

    ASSERT_TRUE(xo_args_try_get_string_array(
        foo, &string_array_value, &string_array_count));
    ASSERT_EQ(2u, string_array_count);
    ASSERT_STREQ("FOO", string_array_value[0]);
    ASSERT_STREQ("BAR", string_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, string_array_two_values_like_args)
{
    char const * argv[] = {
        "/mock/test.ext",
        "--foo", // < variable
        "--foo", // < value
        "--foo", // < variable
        "--foo"  // < value
    };
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_STRING_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    char const ** string_array_value = NULL;
    size_t string_array_count = 0;

    ASSERT_TRUE(xo_args_try_get_string_array(
        foo, &string_array_value, &string_array_count));
    ASSERT_EQ(2u, string_array_count);
    ASSERT_STREQ("--foo", string_array_value[0]);
    ASSERT_STREQ("--foo", string_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_array_one_value)
{

    char const * argv[] = {"/mock/test.ext", "--foo", "1337"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_INT_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    int64_t const * int_array_value;
    size_t int_array_count;

    ASSERT_TRUE(
        xo_args_try_get_int_array(foo, &int_array_value, &int_array_count));
    ASSERT_EQ(1u, int_array_count);
    ASSERT_EQ(1337, int_array_value[0]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_array_two_values)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "1337", "7331"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_INT_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    int64_t const * int_array_value;
    size_t int_array_count;

    ASSERT_TRUE(
        xo_args_try_get_int_array(foo, &int_array_value, &int_array_count));
    ASSERT_EQ(2u, int_array_count);
    ASSERT_EQ(1337, int_array_value[0]);
    ASSERT_EQ(7331, int_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_array_two_values_separate)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "1337", "--foo", "7331"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_INT_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    int64_t const * int_array_value;
    size_t int_array_count;

    ASSERT_TRUE(
        xo_args_try_get_int_array(foo, &int_array_value, &int_array_count));
    ASSERT_EQ(2u, int_array_count);
    ASSERT_EQ(1337, int_array_value[0]);
    ASSERT_EQ(7331, int_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, int_array_many_values)
{
    char const * argv[] = {"/mock/test.ext",
                           "--foo",
                           "2",
                           "3",
                           "4",
                           "0xff",
                           "6",
                           "7",
                           "8",
                           "9",
                           "--bar",
                           "BAR",
                           "--foo",
                           "13",
                           "14",
                           "--baz"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_INT_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    xo_args_declare_arg(
        utest_fixture->context, "bar", NULL, NULL, NULL, XO_ARGS_TYPE_STRING);
    xo_args_declare_arg(
        utest_fixture->context, "baz", NULL, NULL, NULL, XO_ARGS_TYPE_SWITCH);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    int64_t const * int_array_value;
    size_t int_array_count;

    ASSERT_TRUE(
        xo_args_try_get_int_array(foo, &int_array_value, &int_array_count));
    ASSERT_EQ(10u, int_array_count);
    ASSERT_EQ(2, int_array_value[0]);
    ASSERT_EQ(3, int_array_value[1]);
    ASSERT_EQ(4, int_array_value[2]);
    ASSERT_EQ(0xff, int_array_value[3]);
    ASSERT_EQ(6, int_array_value[4]);
    ASSERT_EQ(7, int_array_value[5]);
    ASSERT_EQ(8, int_array_value[6]);
    ASSERT_EQ(9, int_array_value[7]);
    ASSERT_EQ(13, int_array_value[8]);
    ASSERT_EQ(14, int_array_value[9]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_array_one_value)
{

    char const * argv[] = {"/mock/test.ext", "--foo", "3.14"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_DOUBLE_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    double const * double_array_value;
    size_t double_array_count;

    ASSERT_TRUE(xo_args_try_get_double_array(
        foo, &double_array_value, &double_array_count));
    ASSERT_EQ(1u, double_array_count);
    ASSERT_EQ(3.14, double_array_value[0]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_array_two_values)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "3.14", "1.59"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_DOUBLE_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    double const * double_array_value;
    size_t double_array_count;

    ASSERT_TRUE(xo_args_try_get_double_array(
        foo, &double_array_value, &double_array_count));
    ASSERT_EQ(2u, double_array_count);
    ASSERT_EQ(3.14, double_array_value[0]);
    ASSERT_EQ(1.59, double_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_array_two_values_separate)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "3.14", "--foo", "1.59"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_DOUBLE_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    double const * double_array_value;
    size_t double_array_count;

    ASSERT_TRUE(xo_args_try_get_double_array(
        foo, &double_array_value, &double_array_count));
    ASSERT_EQ(2u, double_array_count);
    ASSERT_EQ(3.14, double_array_value[0]);
    ASSERT_EQ(1.59, double_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, double_array_many_values)
{
    char const * argv[] = {"/mock/test.ext",
                           "--foo",
                           "3.14",
                           "159",
                           "2.65",
                           "NAN",
                           "35.89",
                           "79.3",
                           "23.8",
                           ".462",
                           "--bar",
                           "BAR",
                           "--foo",
                           "INF",
                           "0.0",
                           "--baz"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_DOUBLE_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    xo_args_declare_arg(
        utest_fixture->context, "bar", NULL, NULL, NULL, XO_ARGS_TYPE_STRING);
    xo_args_declare_arg(
        utest_fixture->context, "baz", NULL, NULL, NULL, XO_ARGS_TYPE_SWITCH);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    double const * double_array_value;
    size_t double_array_count;

    ASSERT_TRUE(xo_args_try_get_double_array(
        foo, &double_array_value, &double_array_count));
    ASSERT_EQ(10u, double_array_count);
    ASSERT_EQ(3.14, double_array_value[0]);
    ASSERT_EQ(159.0, double_array_value[1]);
    ASSERT_EQ(2.65, double_array_value[2]);
    ASSERT_TRUE(isnan(double_array_value[3]));
    ASSERT_EQ(35.89, double_array_value[4]);
    ASSERT_EQ(79.3, double_array_value[5]);
    ASSERT_EQ(23.8, double_array_value[6]);
    ASSERT_EQ(0.462, double_array_value[7]);
    ASSERT_TRUE(isinf(double_array_value[8]));
    ASSERT_EQ(0.0, double_array_value[9]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_array_one_value)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "true"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_BOOL_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool const * bool_array_value;
    size_t bool_array_count;

    ASSERT_TRUE(
        xo_args_try_get_bool_array(foo, &bool_array_value, &bool_array_count));
    ASSERT_EQ(1u, bool_array_count);
    ASSERT_TRUE(bool_array_value[0]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_array_one_value_invalid)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "2"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_BOOL_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("Invalid value provided");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_array_two_values)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "false", "true"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_BOOL_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool const * bool_array_value;
    size_t bool_array_count;

    ASSERT_TRUE(
        xo_args_try_get_bool_array(foo, &bool_array_value, &bool_array_count));
    ASSERT_EQ(2u, bool_array_count);
    ASSERT_FALSE(bool_array_value[0]);
    ASSERT_TRUE(bool_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_array_two_values_invalid)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "false", "3"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    argv[0] = "/mock/test.ext";
    argv[1] = "--foo";
    argv[2] = "false";
    argv[3] = "3";

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_BOOL_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_FALSE(xo_args_submit(utest_fixture->context));

    _test_destroy_context(utest_fixture);

    _TEST_EXPECT_STDOUT("Invalid value provided");
    test_global_clear();
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_array_array_two_values_separate)
{
    char const * argv[] = {"/mock/test.ext", "--foo", "true", "--foo", "true"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_BOOL_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool const * bool_array_value;
    size_t bool_array_count;

    ASSERT_TRUE(
        xo_args_try_get_bool_array(foo, &bool_array_value, &bool_array_count));
    ASSERT_EQ(2u, bool_array_count);
    ASSERT_TRUE(bool_array_value[0]);
    ASSERT_TRUE(bool_array_value[1]);

    _test_destroy_context(utest_fixture);
}

////////////////////////////////////////////////////////////////////////////////
UTEST_F(getters, bool_array_array_many_values)
{
    char const * argv[] = {"/mock/test.ext",
                           "--foo",
                           "1",
                           "0",
                           "1",
                           "0",
                           "TRUE",
                           "FALSE",
                           "TRUE",
                           "False",
                           "--bar",
                           "BAR",
                           "--foo",
                           "True",
                           "false",
                           "--baz"};
    _TEST_INIT_CONTEXT(utest_fixture, argv);

    xo_args_arg const * foo =
        xo_args_declare_arg(utest_fixture->context,
                            "foo",
                            NULL,
                            NULL,
                            NULL,
                            XO_ARGS_TYPE_BOOL_ARRAY | XO_ARGS_ARG_REQUIRED);
    ASSERT_NE(NULL, (void *)foo);

    xo_args_declare_arg(
        utest_fixture->context, "bar", NULL, NULL, NULL, XO_ARGS_TYPE_STRING);
    xo_args_declare_arg(
        utest_fixture->context, "baz", NULL, NULL, NULL, XO_ARGS_TYPE_SWITCH);

    ASSERT_TRUE(xo_args_submit(utest_fixture->context));

    bool const * bool_array_value;
    size_t bool_array_count;
    ASSERT_TRUE(
        xo_args_try_get_bool_array(foo, &bool_array_value, &bool_array_count));
    ASSERT_EQ(10u, bool_array_count);
    ASSERT_TRUE(bool_array_value[0]);
    ASSERT_FALSE(bool_array_value[1]);
    ASSERT_TRUE(bool_array_value[2]);
    ASSERT_FALSE(bool_array_value[3]);
    ASSERT_TRUE(bool_array_value[4]);
    ASSERT_FALSE(bool_array_value[5]);
    ASSERT_TRUE(bool_array_value[6]);
    ASSERT_FALSE(bool_array_value[7]);
    ASSERT_TRUE(bool_array_value[8]);
    ASSERT_FALSE(bool_array_value[9]);

    _test_destroy_context(utest_fixture);
}
