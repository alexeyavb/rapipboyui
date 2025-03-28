#ifndef __GLOBAL_VARIABLES_H__
#define __GLOBAL_VARIABLES_H__

#include <stdbool.h>
#include <stdint.h>

// screen definitions
#define SCRREN_WIDTH    800u
#define SCREEN_HEIGHT   480u

//ruller coords
#define RULLER_POS_X    150u
#define RULLER_POS_Y    465u
#define RULLER_WIDTH    600u
#define RULLER_HEIGHT   8u
#define RULLER_PAINT_DELTA_X 10u
#define RULLER_PAINT_DELTA_Y 2u

//ruller states
typedef enum { STATE_WAITING, STATE_LOADING, STATE_FINISHED } ruller_estate;
extern ruller_estate loader_ruller_state;

//fps indicator position
#define FPS_X_POS 710u
#define FPS_Y_POS 0u
extern int framesCounter;

// Global threads loop flags 
extern bool event_thread_running;
extern bool simple_thread_running;
extern bool i2c_thread_running;


#endif // !__GLOBAL_VARIABLES_H__
