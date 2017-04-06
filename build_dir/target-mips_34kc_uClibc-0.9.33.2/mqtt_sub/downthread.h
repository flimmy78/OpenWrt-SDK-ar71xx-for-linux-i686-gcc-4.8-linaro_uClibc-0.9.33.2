#include <pthread.h>
#define THREAD_SLEEP_SPAN 1
int thread_run;
pthread_t thread_down;
pthread_mutex_t down_work_mutex;
int down_thread_init();
