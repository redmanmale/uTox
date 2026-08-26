#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/macros.h"
#include "../../src/messages.h"
#include "../../src/chatlog.c"
#include "../../src/text.c"
#include "../../src/filesys.h"
#include "../../src/native/filesys.h"
#include "../../src/settings.h"

#define FUZZ_FRIEND_ID "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

void message_free(MSG_HEADER *msg) {
    if (!msg) {
        return;
    }
    free(msg->via.txt.msg);
    free(msg);
}

void native_export_chatlog_init(uint32_t friend_number) {
    (void)friend_number;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    settings.portable_mode = true;
    settings.verbose       = LOG_LVL_FATAL;
    native_create_dir((uint8_t *)"./tox/");

    char path[UTOX_FILE_NAME_LENGTH];
    snprintf(path, sizeof path, "./tox/%s.new.txt", FUZZ_FRIEND_ID);
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (size) {
        fwrite(data, 1, size, fp);
    }
    fclose(fp);

    (void)utox_count_chatlog((char *)FUZZ_FRIEND_ID);
    (void)utox_count_unsent_chatlog((char *)FUZZ_FRIEND_ID);
    size_t n = 0;
    MSG_HEADER **list = utox_load_chatlog((char *)FUZZ_FRIEND_ID, &n, 32, 0);
    if (list) {
        for (size_t i = 0; i < n; i++) {
            message_free(list[i]);
        }
        free(list);
    }
    remove(path);
    return 0;
}
