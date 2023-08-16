#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "test_parse.h"

typedef struct {
    enum { TC_SUCC, TC_ERR } type;
    int line;
    const char *src;
    int capacity;
    // TC_SUCC
    CfgEntry *expected_entries;
    int expected_count;
    // TC_ERR
    const char *expected_error;
} TestCase;

static const TestCase test_cases[] = {
    {
        .type = TC_SUCC,
        .src = "",
        .capacity = TEST_CAPACITY,
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .src = " ",
        .capacity = TEST_CAPACITY,
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .src = "#",
        .capacity = TEST_CAPACITY,
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .src = "#\n",
        .capacity = TEST_CAPACITY,
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .src = "x",
        .capacity = 0,
        .expected_entries = (CfgEntry[]){},
        .expected_count = 0,
    },
    {
        .type = TC_SUCC,
        .src = "key: \"hello, world!\"",
        .capacity = TEST_CAPACITY,
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
        .src = "key: 10 # Inline comment",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_INT, .val.integer = 10},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .src = "key: -1",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_INT, .val.integer = -1},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .src = "key: true",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_BOOL, .val.boolean = true},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .src = "key: false",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_BOOL, .val.boolean = false},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .src = "key: rgba(255, 255, 255, 1)",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key",
                 .type = CFG_TYPE_COLOR,
                 .val.color =
                     (CfgColor){.r = 255, .g = 255, .b = 255, .a = 255}},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .src = "key: 0.5",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_FLOAT, .val.floating = 0.5},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .src = "key_: true",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key_", .type = CFG_TYPE_BOOL, .val.boolean = true},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .src = "key.: true",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key.", .type = CFG_TYPE_BOOL, .val.boolean = true},
            },
        .expected_count = 1,
    },
    {
        .type = TC_SUCC,
        .src = "a: true\nb:true",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "a", .type = CFG_TYPE_BOOL, .val.boolean = true},
                {.key = "b", .type = CFG_TYPE_BOOL, .val.boolean = true},
            },
        .expected_count = 2,
    },
    {
        .type = TC_SUCC,
        .src = "key: 1.",
        .capacity = TEST_CAPACITY,
        .expected_entries =
            (CfgEntry[]){
                {.key = "key", .type = CFG_TYPE_FLOAT, .val.floating = 1.0},
            },
        .expected_count = 1,
    },
    {
        .type = TC_ERR,
        .src = "!",
        .capacity = TEST_CAPACITY,
        .expected_error = "missing key",
    },
    {
        .type = TC_ERR,
        // The key must be longer than CFG_MAX_KEY
        .src = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
        .capacity = TEST_CAPACITY,
        .expected_error = "key too long",
    },
    {
        .type = TC_ERR,
        .src = "key",
        .capacity = TEST_CAPACITY,
        .expected_error = "':' expected",
    },
    {
        .type = TC_ERR,
        .src = "key!",
        .capacity = TEST_CAPACITY,
        .expected_error = "':' expected",
    },
    {
        .type = TC_ERR,
        .src = "key  :",
        .capacity = TEST_CAPACITY,
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .src = "key:",
        .capacity = TEST_CAPACITY,
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .src = "key:  ",
        .capacity = TEST_CAPACITY,
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .src = "key:\n",
        .capacity = TEST_CAPACITY,
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .src = "key:  \n",
        .capacity = TEST_CAPACITY,
        .expected_error = "missing value",
    },
    {
        .type = TC_ERR,
        .src = "key: @\n",
        .capacity = TEST_CAPACITY,
        .expected_error = "invalid value",
    },
    {
        .type = TC_ERR,
        .src = "key: \"",
        .capacity = TEST_CAPACITY,
        .expected_error = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .src = "key: \"\x80",
        .capacity = TEST_CAPACITY,
        .expected_error = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .src = "key: \"hello",
        .capacity = TEST_CAPACITY,
        .expected_error = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        .src = "key: \"hello\x80",
        .capacity = TEST_CAPACITY,
        .expected_error = "closing '\"' expected",
    },
    {
        .type = TC_ERR,
        // The value must be longer than CFG_MAX_VAL
        .src = "key: \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"",
        .capacity = TEST_CAPACITY,
        .expected_error = "value too long",
    },
    {
        .type = TC_ERR,
        .src = "key: 10x",
        .capacity = TEST_CAPACITY,
        .expected_error = "unexpected character 'x'",
    },
    {
        .type = TC_ERR,
        .src = "key: -",
        .capacity = TEST_CAPACITY,
        .expected_error = "invalid value",
    },
    {
        .type = TC_ERR,
        .src = "key: t",
        .capacity = TEST_CAPACITY,
        .expected_error = "invalid literal",
    },
    {
        .type = TC_ERR,
        .src = "key: f",
        .capacity = TEST_CAPACITY,
        .expected_error = "invalid literal",
    },
    {
        .type = TC_ERR,
        .src = "key: x",
        .capacity = TEST_CAPACITY,
        .expected_error = "invalid literal",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(",
        .capacity = TEST_CAPACITY,
        .expected_error = "',' expected",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba",
        .capacity = TEST_CAPACITY,
        .expected_error = "'(' expected",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba x",
        .capacity = TEST_CAPACITY,
        .expected_error = "'(' expected",
    },
    {
        .type = TC_ERR,
        .src = "key: r",
        .capacity = TEST_CAPACITY,
        .expected_error = "invalid literal",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(0.5",
        .capacity = TEST_CAPACITY,
        .expected_error =
            "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(-1",
        .capacity = TEST_CAPACITY,
        .expected_error =
            "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(-1",
        .capacity = TEST_CAPACITY,
        .expected_error =
            "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(256",
        .capacity = TEST_CAPACITY,
        .expected_error =
            "red, blue and green must be integers in range (0, 255)",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(255, 255, 255, -1)",
        .capacity = TEST_CAPACITY,
        .expected_error = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(255, 255, 255, 2)",
        .capacity = TEST_CAPACITY,
        .expected_error = "alpha must be in range (0, 1)",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(255, 255, 255, 1",
        .capacity = TEST_CAPACITY,
        .expected_error = "')' expected",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(255, 255, 255, x",
        .capacity = TEST_CAPACITY,
        .expected_error = "number expected",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(x",
        .capacity = TEST_CAPACITY,
        .expected_error = "number expected",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(255 x",
        .capacity = TEST_CAPACITY,
        .expected_error = "',' expected",
    },
    {
        .type = TC_ERR,
        .src = "key: rgba(255, 255, 255, 1 x",
        .capacity = TEST_CAPACITY,
        .expected_error = "')' expected",
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
assert_eq_entries(const TestCase tc, const CfgEntry *actual_entries)
{
    for (int i = 0; i < tc.expected_count; i++) {
        TestResult result =
            assert_eq_entry(&tc.expected_entries[i], &actual_entries[i]);
        if (result.type != TEST_PASSED)
            return result;
    }
    return OK;
}

static TestResult
run_test_case(TestCase tc)
{
    assert(tc.capacity <= TEST_CAPACITY);

    CfgError err;
    CfgEntry entries[TEST_CAPACITY];
    Cfg cfg = {.entries = entries, .capacity = tc.capacity};

    int res = cfg_parse(tc.src, strlen(tc.src), &cfg, &err);

    switch (tc.type) {
    case TC_SUCC:
        ASSERT(res == 0);
        ASSERT(tc.expected_count == cfg.count);
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
    int total = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < total; i++) {
        result = run_test_case(test_cases[i]);
        update_scoreboard(sb, result);
        log_result(result, stream);
    }
}
