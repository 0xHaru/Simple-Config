#include "utils.h"

void
log_result(TestResult result, FILE *stream)
{
    char *res;
    char *color;

    switch (result.type) {
    case TEST_PASSED:
        res = "PASSED";
        color = GREEN;
        break;

    case TEST_FAILED:
        res = "FAILED";
        color = RED;
        break;

    case TEST_ABORTED:
        res = "ABORTED";
        color = RED;
        break;
    }

    fprintf(stream, "%s%s" RESET " - %s:%d\n", color, res, result.file,
            result.line);
}

void
update_scoreboard(Scoreboard *sb, TestResult result)
{
    switch (result.type) {
    case TEST_PASSED:
        sb->passed++;
        break;
    case TEST_FAILED:
        sb->failed++;
        break;
    case TEST_ABORTED:
        sb->aborted++;
        break;
    }
}
