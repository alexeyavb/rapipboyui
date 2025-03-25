#ifndef __LOCAL_TIMERS_H__
#define __LOCAL_TIMERS_H__
#include <stdint.h>
extern uint64_t GetTickCountMs(void);
inline uint32_t TimeElapsedMs(const struct timespec tStartTime, const struct timespec tEndTime){
    return 1000*(tEndTime.tv_sec - tStartTime.tv_sec) +
           (tEndTime.tv_nsec - tStartTime.tv_nsec)/1000000;
 }

#endif
