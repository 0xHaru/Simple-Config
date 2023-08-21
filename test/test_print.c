#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "test_print.h"

TestResult
run_test(const char *expected, void *arg, bool is_error)
{
    char buffer[256];
    FILE *stream = fmemopen(buffer, sizeof(buffer), "w");
    if (stream == NULL)
        return ABORT;

    if (!is_error)
        cfg_fprint(stream, (Cfg *) arg);
    else
        cfg_fprint_error(stream, (CfgError *) arg);

    fclose(stream);

    ASSERT(0 == strncmp(expected, buffer, strlen(expected)));
    return OK;
}

TestResult
run_print_test()
{
    CfgError err;
    CfgEntry entries[TEST_CAPACITY];
    Cfg cfg = {.entries = entries, .capacity = TEST_CAPACITY};

    static const char expected[] = "font: \"JetBrainsMono Nerd Font\"\n"
                                   "font.size: 14\n"
                                   "zoom: 1.500000\n"
                                   "line_numbers: true\n"
                                   "ruler: false\n"
                                   "bg.color: rgba(255, 255, 255, 255)";

    static const char src[] = "font: \"JetBrainsMono Nerd Font\"\n"
                              "font.size: 14\n"
                              "zoom: 1.5\n"
                              "line_numbers: true\n"
                              "ruler: false\n"
                              "bg.color: rgba(255, 255, 255, 1)";

    int res = cfg_parse(src, strlen(src), &cfg, &err);
    if (res != 0)
        return ABORT;

    return run_test(expected, &cfg, false);
}

TestResult
run_error_test_1()
{
    CfgError err;
    CfgEntry entries[TEST_CAPACITY];
    Cfg cfg = {.entries = entries, .capacity = TEST_CAPACITY};

    int res = cfg_parse_file("", &cfg, &err);
    if (res == 0)
        return ABORT;

    return run_test("Error: invalid filename\n", &err, true);
}

TestResult
run_error_test_2()
{
    CfgError err;
    CfgEntry entries[TEST_CAPACITY];
    Cfg cfg = {.entries = entries, .capacity = TEST_CAPACITY};

    static const char *src = "a:true\nb:";

    int res = cfg_parse(src, strlen(src), &cfg, &err);
    if (res == 0)
        return ABORT;

    return run_test("Error at 2:3 :: missing value\n", &err, true);
}

void
run_print_tests(Scoreboard *sb, FILE *stream)
{
    TestResult result;

    result = run_print_test();
    update_scoreboard(sb, result);
    log_result(result, stream);

    result = run_error_test_1();
    update_scoreboard(sb, result);
    log_result(result, stream);

    result = run_error_test_2();
    update_scoreboard(sb, result);
    log_result(result, stream);
}
