#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include <raylib.h>
#include "local_timers.h"
#include "simple_thread.h"

pthread_mutex_t mutex;
pthread_t simple_thread;
#define MSG_DELAY_MSEC 6000
static long next_message = 0;
bool simple_thread_running = false;

void *thread_function(void* arg) {
    pthread_t selfthr = pthread_self();
    TraceLog(LOG_INFO, "Entering Simple thread proc callback ");

    // pthread_self()->__sig
    #ifdef LINUX
        // printf("Starting loop, Thread ID: %lu\n", selfthr);
        TraceLog(LOG_INFO, "Starting simple thread loop, Thread ID: %lu", selfthr);
    #else
        TraceLog(LOG_INFO, "Starting simple thread loop, Thread ID: %lu", selfthr->__sig);
    #endif

    // int counter = 0, in = 0;
    while(simple_thread_running){
        
        pthread_mutex_lock(&mutex);
        if(GetTickCountMs() >= next_message){
            // printf("Event Thread monitor proc callback\n");
            TraceLog(LOG_INFO, "Simple thread monitor proc callback ");
            next_message = GetTickCountMs() + (MSG_DELAY_MSEC);
        }        
        pthread_mutex_unlock(&mutex);
    }
    simple_thread_deinit();
    TraceLog(LOG_INFO, "Simple thread exit");
    return NULL;
}

void simple_thread_init(void){
    simple_thread_running = true;
    TraceLog(LOG_INFO, "Simple thread initialize mutexes");
    pthread_mutex_init(&mutex, NULL);
}

void simple_thread_deinit(void){
    TraceLog(LOG_INFO, "Simple thread deinitialize mutexes");
    pthread_mutex_destroy(&mutex);
}

int simple_thread_run(void) {
    int result;
    result = pthread_create(&simple_thread, NULL, thread_function, NULL);

    if (result != 0) {
        TraceLog(LOG_ERROR, "Error create Simple thread");
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    TraceLog(LOG_INFO, "Simple thread running");
    // pthread_join(thread, NULL); // it`s a lock main call
    return 0;
}

