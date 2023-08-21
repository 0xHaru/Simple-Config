#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Error: missing config file\n");
        return 1;
    }

    int capacity = 64;
    CfgEntry *entries = malloc(capacity * sizeof(CfgEntry));
    if (entries == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return 1;
    }

    CfgError err;
    Cfg cfg = {.entries = entries, .capacity = capacity};
    int res = cfg_parse_file(argv[1], &cfg, &err);

    if (res != 0) {
        cfg_fprint_error(stderr, &err);
        free(entries);
        return 1;
    }

    char *font = cfg_get_string(&cfg, "font", "Noto Sans Mono");
    int font_size = cfg_get_int(&cfg, "font.size", 12);
    float zoom = cfg_get_float(&cfg, "zoom", 1.0);
    bool line_num = cfg_get_bool(&cfg, "lineNumbers", true);
    CfgColor bg = cfg_get_color(&cfg, "bg.color",
                                (CfgColor){
                                    .r = 255,
                                    .g = 255,
                                    .b = 255,
                                    .a = 1,
                                });

    fprintf(stdout, "font: %s\n", font);
    fprintf(stdout, "font.size: %d\n", font_size);
    fprintf(stdout, "zoom: %f\n", zoom);
    fprintf(stdout, "lineNumbers: %s\n", line_num ? "true" : "false");
    fprintf(stdout, "bg.color: rgba(%d, %d, %d, %d)\n", bg.r, bg.g, bg.b, bg.a);

    // cfg_fprint(stdout, &cfg);

    free(entries);
    return 0;
}
