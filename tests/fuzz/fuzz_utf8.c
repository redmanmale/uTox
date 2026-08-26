#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/text.c"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        (void)utf8_validate(NULL, 0);
        return 0;
    }

    uint8_t *buf = malloc(size + 8);
    if (!buf) {
        return 0;
    }
    memcpy(buf, data, size);
    memset(buf + size, 0, 8);

    (void)utf8_validate(buf, (int)size);
    (void)utf8_len((const char *)buf);

    int n = utf8_validate(buf, (int)size);
    if (n < 0) {
        n = 0;
    }
    char *html = tohtml((char *)buf, (uint16_t)n);
    free(html);
    free(buf);
    return 0;
}
