#include "test_get.h"
#include "test_load.h"
#include "test_parse.h"
#include "test_print.h"

int
main(void)
{
#ifdef LOGFILE
    FILE *stream = fopen("log.txt", "w");
    if (!stream) {
        fprintf(stderr, "FATAL: Failed to open log file\n");
        return 1;
    }
#else
    FILE *stream = stdout;
#endif

    Scoreboard sb = {0};
    run_parse_tests(&sb, stream);
    run_load_tests(&sb, stream);
    run_get_tests(&sb, stream);
    run_print_tests(&sb, stream);

    int total = sb.passed + sb.failed + sb.aborted;
    fprintf(stream, "Total: %d Passed: %d Failed: %d Aborted: %d\n", total,
            sb.passed, sb.failed, sb.aborted);

#ifdef LOGFILE
    fclose(stream);
#endif
    return 0;
}
