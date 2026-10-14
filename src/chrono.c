#include "chrono.h"

#include "debug.h"
#include "macros.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "native/thread.h"

atomic_bool chrono_thread_init = false;

static void chrono_thread(void *args) {
    LOG_INFO("Chono", "Thread starting");

    CHRONO_INFO *info = args;
    atomic_store_explicit(&chrono_thread_init, true, memory_order_release);
    /* ptr/target are integer counters stored as pointers (see tests). Do not
     * use pointer arithmetic from NULL — that is undefined behavior (UBSan). */
    while (info->ptr != info->target) {
        info->ptr = (uint8_t *)((uintptr_t)info->ptr + (uintptr_t)(intptr_t)info->step);
        yieldcpu(info->interval_ms);
    }

    if (info->callback) {
        info->callback(info->cb_data);
    }

    /* Clear after callback so chrono_end does not return while we still touch info. */
    atomic_store_explicit(&chrono_thread_init, false, memory_order_release);

    LOG_INFO("Chrono", "Thread exited cleanly");
}

bool chrono_start(CHRONO_INFO *info) {
    if (!info) {
        LOG_ERR("Chrono", "Chrono info structure is null.");
        return false;
    }

    thread(chrono_thread, info);

    return true;
}

bool chrono_end(CHRONO_INFO *info) {
    if (!info) {
        LOG_ERR("Chrono", "Chrono info is null");
        return false;
    }

    (*info).finished = true;

    while (atomic_load_explicit(&chrono_thread_init, memory_order_acquire)) {
        yieldcpu(1);
    }

    return true;
}

void chrono_callback(uint32_t ms, void func(void *), void *funcargs) {
    yieldcpu(ms);
    func(funcargs);
}
