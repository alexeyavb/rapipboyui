#include <stdint.h>
#ifdef LINUX

/*
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
*/
    #define _DEFAULT_SOURCE 1    
    #include <stdint.h>
    #include <linux/types.h>
    #include <linux/time.h>
    #include <time.h>
#else
    #include <stdint.h>
    #include <sys/_types.h>
    #include <time.h>    
#endif

#include "local_timers.h"
extern void clock_gettime(int, void*);

uint64_t GetTickCountMs(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);
}
