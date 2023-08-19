#include <stdint.h>
#include <stdlib.h>

#include "../config.h"

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    int capacity = 512;
    CfgEntry *entries = malloc(capacity * sizeof(CfgEntry));
    if (entries == NULL)
        return 1;

    CfgError err;
    Cfg cfg = {.entries = entries, .capacity = capacity};
    int res = cfg_parse(Data, Size, &cfg, &err);

    free(entries);
    return 0;
}

// Docs: https://llvm.org/docs/LibFuzzer.html
