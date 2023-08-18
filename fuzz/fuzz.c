#include <stdint.h>
#include <stdlib.h>

#include "../config.h"

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    int capacity = 8192;
    char *arena = malloc(capacity);
    if (arena == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return 1;
    }

    CfgError err;
    Cfg cfg = {.arena = arena, .capacity = capacity};
    int res = cfg_parse(Data, Size, &cfg, &err);

    free(arena);
    return 0;
}

// Docs: https://llvm.org/docs/LibFuzzer.html
