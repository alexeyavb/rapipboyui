#include "linux_i2c_thread.h"
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <poll.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include "local_timers.h"
#include "linux_evthread.h"

#include <raylib.h>

#define MSG_DELAY_MSEC 6000
static long next_message = 0;
extern void* I2C_threadproc(void *data);


pthread_mutex_t I2C_poll_mutex;
pthread_t I2C_poll_thread;

typedef enum AXIS{
    AXIS_X = 0, AXIS_Y
} AXIS, *PAXIS, **LPAXIS;

typedef struct input_event{
    struct timeval time;
    unsigned short type;
    unsigned short code;
    int value;
} *PINPUT_EVENT, **LPINPUT_EVENT;


bool i2c_thread_running = false;
int i2c_dev = -1;

#define MODULE_INFO "I2C_THREAD"
#define I2C_DEVICE "/dev/i2c-2"
#define I2C_DEVICE_ADDR 0x2c

static const char* i2c_devname = (char*)I2C_DEVICE;
/*
**	@brief		:	Open i2c bus
**	#bus_name	:	i2c bus name such as: /dev/i2c-1
**	@return		:	failed return -1, success return i2c bus fd
*/
int i2c_open(const char *bus_name)
{
    int fd;
    /* Open i2c-bus devcice */
    if ((fd = open(bus_name, O_RDWR)) == -1) {
        TraceLog(LOG_ERROR, "%s Error opening i2c device %s in RW mode", MODULE_INFO, I2C_DEVICE);
        return fd;
    }

    TraceLog(LOG_INFO, "%s device %s opened in RW mode is NOW", MODULE_INFO, I2C_DEVICE);
    return fd;
}

void i2c_close(int bus)
{
    close(bus);
}

void I2C_Thread_Init(void){
    int result;
    pthread_mutex_init(&I2C_poll_mutex, NULL);
    i2c_thread_running = true;

    result = pthread_create(&I2C_poll_thread, NULL, I2C_threadproc, NULL);
    if (result != 0) {
        TraceLog(LOG_ERROR, "%s Error create event poll threads", MODULE_INFO);
        perror("pthread_create");        
    }
    TraceLog(LOG_INFO, "%s i2c poll thread running", MODULE_INFO);
    return;
}

int I2C_opendevices(void){

    if( (i2c_dev = i2c_open( i2c_devname )) < 0 ){
        TraceLog(LOG_ERROR, "%s Error opening i2c device %s", MODULE_INFO, I2C_DEVICE);
        i2c_thread_running = false;
        return i2c_dev;
    }

    if (ioctl(i2c_dev, I2C_SLAVE, I2C_DEVICE_ADDR) < 0) 
	{
        TraceLog(LOG_ERROR, "%s Error opening i2c device %s on address %#02x", MODULE_INFO, I2C_DEVICE, I2C_DEVICE_ADDR);
		close(i2c_dev);
        i2c_thread_running = false;
		return -21;
	}    
    TraceLog(LOG_INFO, "%s i2c device %s on address %#02x already opened", MODULE_INFO, I2C_DEVICE, I2C_DEVICE_ADDR);
    return 0;
}

int I2C_closedevices(void){
    TraceLog(LOG_INFO, "%s Closing i2c device %s", MODULE_INFO, I2C_DEVICE);
    if(i2c_dev > 0)
        i2c_close(i2c_dev);
        
    return 0;
}


int I2C_polldevices(struct input_event* inp_data, int inp_size){
    __u8 version_register = 0x00; /* Device register to access */
    __s32 res;
    // char buf[10];
    static const uint8_t min_protocol_version = 0x1a;
    /* Using SMBus commands */
    res = i2c_smbus_read_byte_data(i2c_dev, version_register);

    if (res < 0) {
        /* ERROR HANDLING: i2c transaction failed */
        TraceLog(LOG_ERROR, "%s Error while receive protocol version", MODULE_INFO);
        i2c_thread_running = false; // break loop
        return 2;

    } else {
        if(min_protocol_version > res){
            TraceLog(LOG_ERROR, "%s Protocol version rcvd, wait >= %#02x, recv %#02x", MODULE_INFO, min_protocol_version, res);
            i2c_thread_running = false; // break loop
            return 3;
        }        
    }
    UNUSED(inp_data); UNUSED(inp_size);

    // /* Using I2C Write, equivalent of 
    //          i2c_smbus_write_word_data(file,register,0x6543) */
    // buf[0] = register;
    // buf[1] = 0x43;
    // buf[2] = 0x65;

    // if ( write(file,buf,3) != 3) {
    //   /* ERROR HANDLING: i2c transaction failed */
    // }

    // /* Using I2C Read, equivalent of i2c_smbus_read_byte(file) */
    // if (read(file,buf,1) != 1) {
    //   /* ERROR HANDLING: i2c transaction failed */
    // } else {
    //   /* buf[0] contains the read byte */
    // }
    
    
    
    
    /*
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
*/
    return 0;
}

void* I2C_threadproc(void *data){
    
    i2c_thread_running = true;
    
    TraceLog(LOG_INFO, "%s Entering i2c thread proc callback", MODULE_INFO);
    
    I2C_opendevices();
    static const int inp_size = sizeof(struct input_event);
    TraceLog(LOG_DEBUG, "%s Input structure size=%d ", MODULE_INFO, inp_size);
    // printf("input_size=%d\n", inp_size);
    static struct input_event input_data;
    static struct input_event* inp_data = &input_data;
    memset(inp_data,0,inp_size);

    while (i2c_thread_running){
        if(GetTickCountMs() >= next_message){
            TraceLog(LOG_INFO, "%s ~ %s addr %#02x thread monitor proc callback ", MODULE_INFO, I2C_DEVICE, I2C_DEVICE_ADDR);
            next_message = GetTickCountMs() + (MSG_DELAY_MSEC);
        }
        pthread_mutex_lock(&I2C_poll_mutex);
        I2C_polldevices(inp_data, inp_size);
        pthread_mutex_unlock(&I2C_poll_mutex);
    }

    I2C_closedevices();
    TraceLog(LOG_INFO, "%s Exiting from i2c thread proc ", MODULE_INFO);
    return 0;
}
