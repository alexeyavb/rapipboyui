#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <poll.h>
#include <limits.h>
#include <sys/time.h>
#include <pthread.h>
#include "local_timers.h"
#include "linux_evthread.h"

#include <raylib.h>

#define MSG_DELAY_MSEC 6000
static long next_message = 0;
extern void* threadproc(void *data);


pthread_mutex_t poll_mutex;
pthread_t poll_thread;

typedef enum AXIS{
    AXIS_X = 0, AXIS_Y
} AXIS, *PAXIS, **LPAXIS;

typedef struct input_event{
    struct timeval time;
    unsigned short type;
    unsigned short code;
    int value;
} *PINPUT_EVENT, **LPINPUT_EVENT;


bool event_thread_running = false;
static struct pollfd axis[2];

#define X_AXIS_DEVICE "/dev/input/event0\0"
#define Y_AXIS_DEVICE "/dev/input/event1\0"



void EventThread_Init(void){
    int result;
    pthread_mutex_init(&poll_mutex, NULL);
    event_thread_running = true;

    result = pthread_create(&poll_thread, NULL, threadproc, NULL);
    if (result != 0) {
        TraceLog(LOG_ERROR, "Error create event poll threads");
        perror("pthread_create");        
    }
    TraceLog(LOG_INFO, "Event thread running");
    return;
}

int opendevices(void){
    axis[AXIS_X].fd = open(X_AXIS_DEVICE, O_RDONLY | O_NONBLOCK);
    axis[AXIS_Y].fd = open(Y_AXIS_DEVICE, O_RDONLY | O_NONBLOCK);

    if(axis[AXIS_X].fd < 0)
    {
        TraceLog(LOG_ERROR, "Unable open device '%s' for reading, thread initialization break", X_AXIS_DEVICE);
        return(1);
    }

    if(axis[AXIS_Y].fd < 0)
    {
        TraceLog(LOG_ERROR, "Unable open device '%s' for reading, thread initialization break", Y_AXIS_DEVICE);
        return(2);
    }

    axis[AXIS_X].events = POLLIN; axis[AXIS_Y].events = POLLIN;

    return(0);
}

int closedevices(void){
    if(axis[AXIS_X].fd >= 0)
    {
        TraceLog(LOG_INFO, "Close device '%s' ", X_AXIS_DEVICE);        
        close(axis[AXIS_X].fd);

    }

    if(axis[AXIS_Y].fd >= 0)
    {
        TraceLog(LOG_INFO, "Close device '%s' ", Y_AXIS_DEVICE);
        close(axis[AXIS_Y].fd);
    }

    return 0;
}


int polldevices(struct input_event* inp_data, int inp_size){

    int timeout_ms = 500;
    int retX = poll(&(axis[AXIS_X]), 1, timeout_ms);
    int retY = poll(&(axis[AXIS_Y]), 1, timeout_ms);

    if(retX > 0){
        if(axis[AXIS_X].revents){
            memset(inp_data,0,inp_size);
            ssize_t rX = read(axis[AXIS_X].fd, inp_data, inp_size);
            if(rX < 0) {
                // printf(" error read x axis event data %d\n", (int)rX);
                TraceLog(LOG_WARNING, "Unable to read x axis event data from '%s', data %d ", X_AXIS_DEVICE, (int)rX);
            }
            else {
                // printf("X AXIS_EVENT time=%ld.%06d type=%hu code=%hu value=%d\n", inp_data->time.tv_sec, inp_data->time.tv_usec, inp_data->type, inp_data->code, inp_data->value);
                TraceLog(LOG_INFO, "Received X AXIS_EVENT time=%ld.%06d type=%hu code=%hu value=%d ", inp_data->time.tv_sec, inp_data->time.tv_usec, inp_data->type, inp_data->code, inp_data->value);
                memset(inp_data,0,inp_size);                
            }
        }        
    }

    if(retY > 0){
        if(axis[AXIS_Y].revents){
            memset(inp_data,0,inp_size);
            ssize_t rY = read(axis[AXIS_Y].fd, inp_data, inp_size);
            if(rY < 0) {
                // printf(" error read Y axis event data %d\n", (int)rY);                
                TraceLog(LOG_WARNING, "Unable to read y axis event data from '%s', data %d ", Y_AXIS_DEVICE, (int)rY);
            }
            else {
                TraceLog(LOG_INFO, "Received Y AXIS_EVENT time=%ld.%06d type=%hu code=%hu value=%d ", inp_data->time.tv_sec, inp_data->time.tv_usec, inp_data->type, inp_data->code, inp_data->value);
                memset(inp_data,0,inp_size);                
            }
        }        
    }
    return 0;
}

void* threadproc(void *data){    
    // printf("Enter Thread proc callback\n");
    TraceLog(LOG_INFO, "Entering Thread proc callback ");
    opendevices();
    static const int inp_size = sizeof(struct input_event);
    TraceLog(LOG_DEBUG, "input structure size=%d ", inp_size);
    // printf("input_size=%d\n", inp_size);
    static struct input_event input_data;
    static struct input_event* inp_data = &input_data;
    memset(inp_data,0,inp_size);

    event_thread_running = true;

    while (event_thread_running){
        if(GetTickCountMs() >= next_message){
            // printf("Event Thread monitor proc callback\n");
            TraceLog(LOG_INFO, "Event thread monitor proc callback ");
            next_message = GetTickCountMs() + (MSG_DELAY_MSEC);
        }
        pthread_mutex_lock(&poll_mutex);
        polldevices(inp_data, inp_size);
        pthread_mutex_unlock(&poll_mutex);
    }
    closedevices();
    // printf("Exit from Thread proc callback\n");
    TraceLog(LOG_INFO, "Exit from Event thread proc callback ");
    return 0;
}
