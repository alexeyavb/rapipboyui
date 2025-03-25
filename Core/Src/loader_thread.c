#include <stdatomic.h>                      // C11 atomic data types
#include <time.h>                           // Required for: clock()
#include <raylib.h>
#include <pthread.h>

#include "global_variables.h"
#include "loader_thread.h"


pthread_t threadId = { 0 };
static atomic_bool dataLoaded = false;
static atomic_int dataProgress = 0; 
static void* LoadDataThread(void *arg);

void checkThreadState(){
    switch (loader_ruller_state)
    {
        case STATE_WAITING:
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                int error = pthread_create(&threadId, NULL, &LoadDataThread, NULL);
                if (error != 0) TraceLog(LOG_ERROR, "Error creating loading thread");
                else TraceLog(LOG_INFO, "Loading thread initialized successfully");

                loader_ruller_state = STATE_LOADING;
            }
        } break;
        case STATE_LOADING:
        {
            framesCounter++;
            if (atomic_load_explicit(&dataLoaded, memory_order_relaxed))
            {
                framesCounter = 0;
                int error = pthread_join(threadId, NULL);
                if (error != 0) TraceLog(LOG_ERROR, "Error joining loading thread");
                else TraceLog(LOG_INFO, "Loading thread terminated successfully");

                loader_ruller_state = STATE_FINISHED;
            }
        } break;
        case STATE_FINISHED:
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                // Reset everything to launch again
                atomic_store_explicit(&dataLoaded, false, memory_order_relaxed);
                atomic_store_explicit(&dataProgress, 0, memory_order_relaxed);
                loader_ruller_state = STATE_WAITING;
            }
        } break;
        default: break;
    }    
}

static void* LoadDataThread(void *arg){
    int timeCounter = 0;            // Time counted in ms
    clock_t prevTime = clock();     // Previous time

    // We simulate data loading with a time counter for (RULLER_WIDTH/100) seconds
    while (timeCounter < RULLER_WIDTH * RULLER_PAINT_DELTA_X)
    {
        clock_t currentTime = clock() - prevTime;
        timeCounter = currentTime*1000/CLOCKS_PER_SEC;

        // We accumulate time over a global variable to be used in
        // main thread as a progress bar
        atomic_store_explicit(&dataProgress, timeCounter/10, memory_order_relaxed);
    }

    // When data has finished loading, we set global variable
    atomic_store_explicit(&dataLoaded, true, memory_order_relaxed);

    return NULL;
}

void drawRuller(){
    ClearBackground(RAYWHITE);
    switch (loader_ruller_state)
    {
        case STATE_WAITING: DrawText("PRESS ENTER TO START", RULLER_PAINT_DELTA_X, RULLER_POS_Y, RULLER_HEIGHT - RULLER_PAINT_DELTA_Y, DARKGRAY); break;
        case STATE_LOADING:
        {
            DrawRectangle(RULLER_POS_X, RULLER_POS_Y, atomic_load_explicit(&dataProgress, memory_order_relaxed), RULLER_HEIGHT, GREEN);
            if ((framesCounter/15)%2) DrawText("LOADING DATA...", RULLER_PAINT_DELTA_X, RULLER_POS_Y, RULLER_HEIGHT - RULLER_PAINT_DELTA_Y, YELLOW);
        } break;
        case STATE_FINISHED:
        {
            DrawRectangle(RULLER_POS_X, RULLER_POS_Y, RULLER_WIDTH, RULLER_HEIGHT, DARKGREEN);
            DrawText("DATA LOADED!", RULLER_PAINT_DELTA_X, RULLER_POS_Y, - RULLER_PAINT_DELTA_Y, GREEN);
        } break;
        default: break;
    }
    DrawRectangleLines(RULLER_POS_X-1, RULLER_POS_Y-1, RULLER_WIDTH+2, RULLER_HEIGHT+2, DARKGRAY);
}
