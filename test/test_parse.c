#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "test_parse.h"

typedef struct {
    enum { TC_SUCC, TC_ERR } type;
    int line;
    const char *src;
    // TC_SUCC
    CfgEntry *expected_entries;
    int expected_count;
    // TC_ERR
    const char *expected_error;
} TestCase;

static const TestCase test_cases[] = {
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "",
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = " ",
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "#",
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "#\n",
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: \"hello, world!\"",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key",
                 .type = CFG_TYPE_STRING,
                 .val.string = "hello, world!"},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 10 # Inline comment",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_INT, .val.integer = 10},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: -1",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_INT, .val.integer = -1},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: true",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_BOOL, .val.boolean = true},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: false",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_BOOL, .val.boolean = false},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 0.5)",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key",
                 .type = CFG_TYPE_COLOR,
                 .val.color =
                     (CfgColor){.r = 255, .g = 255, .b = 255, .a = 127}},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 0.5",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_FLOAT, .val.floating = 0.5},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key_: true",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key_", .type = CFG_TYPE_BOOL, .val.boolean = true},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key.: true",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key.", .type = CFG_TYPE_BOOL, .val.boolean = true},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "a: true\nb:true",
        .expected_entries =
            (CfgEntry[]){
                {.key = "b", .type = CFG_TYPE_BOOL, .val.boolean = true},
                {.key = "a", .type = CFG_TYPE_BOOL, .val.boolean = true},
            },
        .expected_count = 2,
    },
    {
        .type = TC_SUCC,
        .line = __LINE__,
        .src = "key: 1.",
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_FLOAT, .val.floating = 1.0},
            },
        .expected_count = 1,
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "!",
        .expected_error = "missing key",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key",
        .expected_error = "':' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key!",
        .expected_error = "':' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key  :",
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:",
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:  ",
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:\n",
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key:  \n",
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: @\n",
        .expected_error = "invalid value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"",
        .expected_error = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"\x80",
        .expected_error = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"hello",
        .expected_error = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: \"hello\x80",
        .expected_error = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: 10x",
        .expected_error = "unexpected character 'x'",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: -",
        .expected_error = "invalid value",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: t",
        .expected_error = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: f",
        .expected_error = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: x",
        .expected_error = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(",
        .expected_error = "',' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba",
        .expected_error = "'(' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba x",
        .expected_error = "'(' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: r",
        .expected_error = "invalid literal",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(0.5",
        .expected_error =
            "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(-1",
        .expected_error =
            "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(-1",
        .expected_error =
            "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(256",
        .expected_error =
            "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, -1)",
        .expected_error = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 2)",
        .expected_error = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1",
        .expected_error = "')' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, x",
        .expected_error = "number expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(x",
        .expected_error = "number expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255 x",
        .expected_error = "',' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1 x",
        .expected_error = "')' expected",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: 2147483648",
        .expected_error = "number too large",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: 0.0000000000",
        .expected_error = "number too large",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: 1.33333333333333333",
        .expected_error = "number too large",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, 1.5)",
        .expected_error = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .line = __LINE__,
        .src = "key: rgba(255, 255, 255, -0.5)",
        .expected_error = "alpha must be in range (0, 1)",
    },
};

static TestResult
assert_eq_entry(const CfgEntry *expected, const CfgEntry *actual)
{
    ASSERT(expected->type == actual->type);
    ASSERT(0 == strcmp(expected->key, actual->key));

    switch (expected->type) {
    case CFG_TYPE_STRING:
        ASSERT(0 == strcmp(expected->val.string, actual->val.string));
        break;

    case CFG_TYPE_INT:
        ASSERT(expected->val.integer == actual->val.integer);
        break;

    case CFG_TYPE_FLOAT:
        ASSERT(expected->val.floating == actual->val.floating);
        break;

    case CFG_TYPE_BOOL:
        ASSERT(expected->val.boolean == actual->val.boolean);
        break;

    case CFG_TYPE_COLOR:
        ASSERT(0 == memcmp(&expected->val.color, &actual->val.color,
                           sizeof(CfgColor)));
        break;
    }

    return OK;
}

static TestResult
assert_eq_entries(TestCase tc, CfgEntry *actual_entries)
{
    int i = 0;
    for (CfgEntry *entry = actual_entries; entry; entry = entry->next) {
        ASSERT(i < tc.expected_count);
        TestResult result = assert_eq_entry(&tc.expected_entries[i], entry);
        if (result.type != TEST_PASSED)
            return result;
        i++;
    }

    return OK;
}

static TestResult
run_test_case(TestCase tc)
{
    char *arena = malloc(TEST_CAPACITY);
    ASSERT(arena != NULL);

    CfgError err;
    Cfg cfg = {.arena = arena, .capacity = TEST_CAPACITY};
    int res = cfg_parse(tc.src, strlen(tc.src), &cfg, &err);

    printf("%s:%d\n", __FILE__, tc.line);
    cfg_fprint(stdout, &cfg);
    cfg_fprint_error(stdout, &err);

    switch (tc.type) {
    case TC_SUCC:
        ASSERT(res == 0);
        return assert_eq_entries(tc, cfg.entries);

    case TC_ERR:
        ASSERT(res != 0);
        ASSERT(0 == strcmp(tc.expected_error, err.msg));
        break;
    }

    return OK;
}

void
run_parse_tests(Scoreboard *sb, FILE *stream)
{
    TestResult result;
    int total = COUNT_OF(test_cases);

    for (int i = 0; i < total; i++) {
        result = run_test_case(test_cases[i]);
        update_scoreboard(sb, result);
        log_result(result, stream);
    }
}
