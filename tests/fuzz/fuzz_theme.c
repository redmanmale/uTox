#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/theme.c"
#include "../../src/theme_tables.c"
#include "../../src/settings.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    settings.portable_mode = true;
    settings.verbose       = LOG_LVL_FATAL;

    if (size == 0) {
        read_custom_theme(NULL, 0);
        bool err = false;
        (void)try_parse_hex_colour(NULL, &err);
        return 0;
    }

    char color[64];
    size_t n = size < sizeof color - 1 ? size : sizeof color - 1;
    memcpy(color, data, n);
    color[n] = 0;
    bool err = false;
    (void)try_parse_hex_colour(color, &err);

    read_custom_theme(data, size);
    return 0;
}
