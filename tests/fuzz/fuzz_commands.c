#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/commands.c"
#include "../../src/command_funcs.c"
#include "../../src/text.c"
#include "../mock/mock_domain.h"
#include "../../src/settings.h"

static FRIEND cmd_friend;
static GROUPCHAT cmd_group;
static FRIEND *cmd_found;

void friend_set_alias(FRIEND *f, uint8_t *alias, uint16_t length) {
    (void)f;
    (void)alias;
    (void)length;
}

void utox_write_metadata(FRIEND *f) {
    (void)f;
}

FRIEND *find_friend_by_name(uint8_t *name) {
    (void)name;
    return cmd_found;
}

bool string_to_id(uint8_t *dest, char *src) {
    if (!dest || !src) {
        return false;
    }
    memset(dest, 0x11, TOX_ADDRESS_SIZE);
    return *src != 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    settings.portable_mode = true;
    settings.verbose       = LOG_LVL_FATAL;
    mock_domain_reset();
    memset(&cmd_friend, 0, sizeof cmd_friend);
    memset(&cmd_group, 0, sizeof cmd_group);
    cmd_friend.number = 1;
    cmd_friend.online = true;
    cmd_group.number  = 1;
    cmd_found         = &cmd_friend;
    mock_sel_friend   = &cmd_friend;
    mock_sel_group    = &cmd_group;
    tox_thread_init   = UTOX_TOX_THREAD_INIT_SUCCESS;

    if (size == 0) {
        do_tox_url(NULL, 0);
        return 0;
    }

    char *buf = malloc(size + 1);
    if (!buf) {
        return 0;
    }
    memcpy(buf, data, size);
    buf[size] = 0;

    if (size >= 4 && memcmp(buf, "tox:", 4) == 0) {
        do_tox_url((uint8_t *)buf, (int)size);
    } else {
        char *cmd = NULL;
        char *arg = NULL;
        uint16_t n = size > 0xffff ? 0xffff : (uint16_t)size;
        if (n && buf[0] != '/') {
            /* Force a command-shaped input so the parser walks the table. */
            buf[0] = '/';
        }
        (void)utox_run_command(buf, n, &cmd, &arg, 1);
    }

    free(buf);
    return 0;
}
