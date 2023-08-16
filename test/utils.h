#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>

// #define LOGFILE

#ifdef LOGFILE
#define RED ""
#define GREEN ""
#define RESET ""
#else
#define RED "\e[1;31m"
#define GREEN "\e[1;32m"
#define RESET "\e[0m"
#endif

#define TEST_CAPACITY 32

typedef struct {
    int passed;
    int failed;
    int aborted;
} Scoreboard;

typedef enum {
    TEST_PASSED,
    TEST_FAILED,
    TEST_ABORTED,
} TestResultType;

typedef struct {
    TestResultType type;
    const char *file;
    int line;
} TestResult;

// clang-format off
#define ASSERT(X)              \
    if (!(X))                  \
        return (TestResult) {  \
            .type=TEST_FAILED, \
            .file=__FILE__,    \
            .line=__LINE__     \
        }

#define ABORT (TestResult) {.type=TEST_ABORTED, .file=__FILE__, .line=__LINE__}
#define OK (TestResult) {.type=TEST_PASSED, .file=__FILE__, .line=__LINE__}

#define COUNT_OF(X) (sizeof(X) / sizeof((X)[0]))

#define WRAP(E) (Cfg) {        \
        .entries=(E),          \
        .capacity=COUNT_OF(E), \
        .count=COUNT_OF(E)     \
    }
// clang-format on

void log_result(TestResult result, FILE *stream);
void update_scoreboard(Scoreboard *sb, TestResult result);

#endif
