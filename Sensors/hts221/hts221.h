#ifndef HTS221_H
#define HTS221_H

#include "stm32l4xx_hal.h"

extern I2C_HandleTypeDef hi2c2;

#ifndef HTS221_I2C
#define HTS221_I2C hi2c2
#endif

typedef enum {
    HTS221_OK = 0,
    HTS221_NOT_READY,
    HTS221_IO_ERROR,
    HTS221_BAD_DEVICE
} HTS221_Status;

typedef struct {
    float temperature_c;
    float humidity_percent;
} HTS221_Sample;

HTS221_Status HTS221_Init(void);
HTS221_Status HTS221_Read(HTS221_Sample *sample);

#endif
