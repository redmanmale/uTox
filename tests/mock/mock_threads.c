#include <pthread.h>

void thread(void * (*func)(void *), void *args) {
    pthread_t      thread_temp;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1 << 20);
    pthread_create(&thread_temp, &attr, func, args);
    /* Detach: fire-and-forget; without this TSan reports a thread leak. */
    pthread_detach(thread_temp);
    pthread_attr_destroy(&attr);
}
