#ifndef __LINUX_I2C_THREAD_H__
#define __LINUX_I2C_THREAD_H__

//Register Addresses
#define REG_VERSION_ADDR                0x00
#define REG_UINT16_RW_ADDR              0x01
#define REG_INT16_RW_ADDR               0x02
#define REG_BOOL_RW_ADDR                0x03
#define REG_CHAR_RW_ADDR                0x04

#define REG_UINT16_RO_ADDR              0x11
#define REG_INT16_RO_ADDR               0x12
#define REG_BOOL_RO_ADDR                0x13
#define REG_CHAR_RO_ADDR                0x14


extern void I2C_Thread_Init(void);
#endif