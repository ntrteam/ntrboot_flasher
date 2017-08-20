#include "i2c.h"

//-----------------------------------------------------------------------------

static const struct { uint8_t bus_id, reg_addr; } dev_data[] = {
    {0, 0x4A}, {0, 0x7A}, {0, 0x78},
    {1, 0x4A}, {1, 0x78}, {1, 0x2C},
    {1, 0x2E}, {1, 0x40}, {1, 0x44},
    {2, 0xD6}, {2, 0xD0}, {2, 0xD2},
    {2, 0xA4}, {2, 0x9A}, {2, 0xA0},
};

static inline uint8_t i2cGetDeviceBusId(uint8_t device_id)
{
    return dev_data[device_id].bus_id;
}

static inline uint8_t i2cGetDeviceRegAddr(uint8_t device_id)
{
    return dev_data[device_id].reg_addr;
}

//-----------------------------------------------------------------------------

static vuint8_t *reg_data_addrs[] = {
    (vuint8_t *)(I2C1_REG_OFF + I2C_REG_DATA),
    (vuint8_t *)(I2C2_REG_OFF + I2C_REG_DATA),
    (vuint8_t *)(I2C3_REG_OFF + I2C_REG_DATA),
};

static inline vuint8_t *i2cGetDataReg(uint8_t bus_id)
{
    return reg_data_addrs[bus_id];
}

//-----------------------------------------------------------------------------

static vuint8_t *reg_cnt_addrs[] = {
    (vuint8_t *)(I2C1_REG_OFF + I2C_REG_CNT),
    (vuint8_t *)(I2C2_REG_OFF + I2C_REG_CNT),
    (vuint8_t *)(I2C3_REG_OFF + I2C_REG_CNT),
};

static inline vuint8_t *i2cGetCntReg(uint8_t bus_id)
{
    return reg_cnt_addrs[bus_id];
}

//-----------------------------------------------------------------------------

static inline void i2cWaitBusy(uint8_t bus_id)
{
    while (*i2cGetCntReg(bus_id) & 0x80);
}

static inline bool i2cGetResult(uint8_t bus_id)
{
    i2cWaitBusy(bus_id);

    return (*i2cGetCntReg(bus_id) >> 4) & 1;
}

static void i2cStop(uint8_t bus_id, uint8_t arg0)
{
    *i2cGetCntReg(bus_id) = (arg0 << 5) | 0xC0;
    i2cWaitBusy(bus_id);
    *i2cGetCntReg(bus_id) = 0xC5;
}

//-----------------------------------------------------------------------------

static bool i2cSelectDevice(uint8_t bus_id, uint8_t dev_reg)
{
    i2cWaitBusy(bus_id);
    *i2cGetDataReg(bus_id) = dev_reg;
    *i2cGetCntReg(bus_id) = 0xC2;

    return i2cGetResult(bus_id);
}

static bool i2cSelectRegister(uint8_t bus_id, uint8_t reg)
{
    i2cWaitBusy(bus_id);
    *i2cGetDataReg(bus_id) = reg;
    *i2cGetCntReg(bus_id) = 0xC0;

    return i2cGetResult(bus_id);
}

bool i2cWriteRegister(uint8_t dev_id, uint8_t reg, uint8_t data)
{
    uint8_t bus_id = i2cGetDeviceBusId(dev_id),
       dev_addr = i2cGetDeviceRegAddr(dev_id);

    bool ret = false;

    for(u32 i = 0; i < 8 && !ret; i++)
    {
        if(i2cSelectDevice(bus_id, dev_addr) && i2cSelectRegister(bus_id, reg))
        {
            i2cWaitBusy(bus_id);
            *i2cGetDataReg(bus_id) = data;
            *i2cGetCntReg(bus_id) = 0xC1;
            i2cStop(bus_id, 0);

            if(i2cGetResult(bus_id)) ret = true;
        }
        *i2cGetCntReg(bus_id) = 0xC5;
        i2cWaitBusy(bus_id);
    }


    return ret;
}
