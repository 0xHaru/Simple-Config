#include <string.h>

#include "../config.h"
#include "test_get.h"

static TestResult
run_get_str_test(void)
{
    CfgEntry entries[] = {
        {.key = "keyA", .type = CFG_TYPE_STRING, .val.string = "foobar"},
        {.key = "keyC", .type = CFG_TYPE_BOOL, .val.boolean = true},
    };
    Cfg cfg = WRAP(entries);

    ASSERT(0 == strcmp("foobar", cfg_get_string(&cfg, "keyA", "barfoo")));
    ASSERT(0 == strcmp("barfoo", cfg_get_string(&cfg, "keyB", "barfoo")));
    ASSERT(0 == strcmp("barfoo", cfg_get_string(&cfg, "keyC", "barfoo")));

    return OK;
}

static TestResult
run_get_bool_test(void)
{
    CfgEntry entries[] = {
        {.key = "key", .type = CFG_TYPE_BOOL, .val.boolean = true},
    };
    Cfg cfg = WRAP(entries);

    ASSERT(true == cfg_get_bool(&cfg, "key", false));

    return OK;
}

static TestResult
run_get_int_test(void)
{
    CfgEntry entries[] = {
        {.key = "key", .type = CFG_TYPE_INT, .val.integer = 16},
    };
    Cfg cfg = WRAP(entries);

    ASSERT(16 == cfg_get_int(&cfg, "key", 64));

    ASSERT(16 == cfg_get_int_min(&cfg, "key", 64, 8));
    ASSERT(64 == cfg_get_int_min(&cfg, "key", 64, 32));

    ASSERT(16 == cfg_get_int_max(&cfg, "key", 64, 32));
    ASSERT(4 == cfg_get_int_max(&cfg, "key", 4, 8));

    ASSERT(16 == cfg_get_int_range(&cfg, "key", 64, 8, 32));
    ASSERT(4 == cfg_get_int_range(&cfg, "key", 4, 4, 8));
    ASSERT(32 == cfg_get_int_range(&cfg, "key", 32, 32, 64));

    return OK;
}

static TestResult
run_get_float_test(void)
{
    CfgEntry entries[] = {
        {.key = "key", .type = CFG_TYPE_FLOAT, .val.floating = 16},
    };
    Cfg cfg = WRAP(entries);

    ASSERT(16 == cfg_get_float(&cfg, "key", 64));

    ASSERT(16 == cfg_get_float_min(&cfg, "key", 64, 8));
    ASSERT(64 == cfg_get_float_min(&cfg, "key", 64, 32));

    ASSERT(16 == cfg_get_float_max(&cfg, "key", 64, 32));
    ASSERT(4 == cfg_get_float_max(&cfg, "key", 4, 8));

    ASSERT(16 == cfg_get_float_range(&cfg, "key", 64, 8, 32));
    ASSERT(4 == cfg_get_float_range(&cfg, "key", 4, 4, 8));
    ASSERT(32 == cfg_get_float_range(&cfg, "key", 32, 32, 64));

    return OK;
}

static TestResult
run_get_color_test(void)
{
    CfgColor c1 = {.r = 255, .g = 255, .b = 255, .a = 255};
    CfgColor c2 = {.r = 0, .g = 0, .b = 0, .a = 0};

    CfgEntry entries[] = {
        {.key = "key", .type = CFG_TYPE_COLOR, .val.color = c1},
    };
    Cfg cfg = WRAP(entries);

    CfgColor actual = cfg_get_color(&cfg, "key", c2);
    ASSERT(0 == memcmp(&c1, &actual, sizeof(CfgColor)));

    return OK;
}

void
run_get_tests(Scoreboard *sb, FILE *stream)
{
    TestResult result;

    result = run_get_str_test();
    update_scoreboard(sb, result);
    log_result(result, stream);

    result = run_get_bool_test();
    update_scoreboard(sb, result);
    log_result(result, stream);

    result = run_get_int_test();
    update_scoreboard(sb, result);
    log_result(result, stream);

    result = run_get_float_test();
    update_scoreboard(sb, result);
    log_result(result, stream);

    result = run_get_color_test();
    update_scoreboard(sb, result);
    log_result(result, stream);
}
