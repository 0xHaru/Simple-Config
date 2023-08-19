#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "test_load.h"

static TestResult
run_file_size_test(void)
{
    CfgError err;
    ASSERT(163 == cfg_file_size("sample.cfg", &err));
    return OK;
}

static TestResult
run_load_test(void)
{
    char *arena = malloc(TEST_CAPACITY);
    ASSERT(arena != 0);

    CfgError err;
    Cfg cfg = {.arena = arena, .capacity = TEST_CAPACITY};

    ASSERT(0 == cfg_load("sample.cfg", &cfg, &err));

    ASSERT(-1 == cfg_load("sample.txt", &cfg, &err));
    ASSERT(0 == strcmp("invalid file extension", err.msg));

    ASSERT(-1 == cfg_load("", &cfg, &err));
    ASSERT(0 == strcmp("invalid filename", err.msg));

    ASSERT(-1 == cfg_load("sample2.cfg", &cfg, &err));
    ASSERT(0 == strcmp("failed to open file", err.msg));

    return OK;
}

void
run_load_tests(Scoreboard *sb, FILE *stream)
{
    TestResult result;

    result = run_load_test();
    update_scoreboard(sb, result);
    log_result(result, stream);

    result = run_file_size_test();
    update_scoreboard(sb, result);
    log_result(result, stream);
}
