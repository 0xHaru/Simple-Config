#include <string.h>

#include "../config.h"
#include "test_load.h"

static TestResult
run_load_test(void)
{
    CfgError err;
    CfgEntry entries[TEST_CAPACITY];
    Cfg cfg = {.entries = entries, .capacity = TEST_CAPACITY};

    ASSERT(0 == cfg_parse_file("sample.cfg", &cfg, &err));

    ASSERT(-1 == cfg_parse_file("sample.txt", &cfg, &err));
    ASSERT(0 == strcmp("invalid file extension", err.msg));

    ASSERT(-1 == cfg_parse_file("", &cfg, &err));
    ASSERT(0 == strcmp("invalid filename", err.msg));

    ASSERT(-1 == cfg_parse_file("sample2.cfg", &cfg, &err));
    ASSERT(0 == strcmp("failed to open file", err.msg));

    return OK;
}

void
run_load_tests(Scoreboard *sb, FILE *stream)
{
    TestResult result = run_load_test();
    update_scoreboard(sb, result);
    log_result(result, stream);
}
