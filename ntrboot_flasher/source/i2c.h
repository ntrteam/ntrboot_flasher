#ifndef I2C_ARM9_INCLUDE
#define I2C_ARM9_INCLUDE

#include "3dstypes.h"

#include <stddef.h>
#include <stdint.h>

#define I2C1_REG_OFF 0x10161000
#define I2C2_REG_OFF 0x10144000
#define I2C3_REG_OFF 0x10148000

#define I2C_REG_DATA  0
#define I2C_REG_CNT   1
#define I2C_REG_CNTEX 2
#define I2C_REG_SCL   4

#define I2C_DEV_MCU  3
#define I2C_DEV_GYRO 10
#define I2C_DEV_IR   13

// Registers for Power Management (I2C_PM)
#define I2CREGPM_BATUNK		0x00
#define I2CREGPM_PWRIF		0x10
#define I2CREGPM_PWRCNT		0x11
#define I2CREGPM_MMCPWR		0x12
#define I2CREGPM_BATTERY	0x20
#define I2CREGPM_RTC		0x30
#define I2CREGPM_CAMLED		0x31
#define I2CREGPM_VOL		0x40
#define I2CREGPM_RESETFLAG	0x70

const uint8_t i2cGetDeviceBusId(uint8_t device_id);
const uint8_t i2cGetDeviceRegAddr(uint8_t device_id);

volatile uint8_t* const i2cGetDataReg(uint8_t bus_id);
volatile uint8_t* const i2cGetCntReg(uint8_t bus_id);

void i2cWaitBusy(uint8_t bus_id);
bool i2cGetResult(uint8_t bus_id);
uint8_t i2cGetData(uint8_t bus_id);
void i2cStop(uint8_t bus_id, uint8_t arg0);

bool i2cSelectDevice(uint8_t bus_id, uint8_t dev_reg);
bool i2cSelectRegister(uint8_t bus_id, uint8_t reg);

uint8_t i2cReadRegister(uint8_t dev_id, uint8_t reg);
bool i2cWriteRegister(uint8_t dev_id, uint8_t reg, uint8_t data);
bool i2cWriteRegisterNDMA(uint8_t dev_id, uint8_t reg, uint8_t data);

bool i2cReadRegisterBuffer(unsigned int dev_id, int reg, uint8_t* buffer, size_t buf_size);

#endif // I2C_ARM7_INCLUDE