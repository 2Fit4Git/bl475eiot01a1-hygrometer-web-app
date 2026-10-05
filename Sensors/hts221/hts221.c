#include "hts221.h"

#include <stddef.h>

#define HTS221_ADDRESS          (0x5FU << 1)
#define HTS221_REG_WHO_AM_I     0x0FU
#define HTS221_REG_AV_CONF      0x10U
#define HTS221_REG_CTRL_REG1    0x20U
#define HTS221_REG_STATUS       0x27U
#define HTS221_REG_HUMIDITY_OUT 0x28U
#define HTS221_REG_CALIB_START  0x30U
#define HTS221_WHO_AM_I_VALUE   0xBCU
#define HTS221_REG_AUTO_INC     0x80U
#define HTS221_STATUS_H_DA      0x02U
#define HTS221_STATUS_T_DA      0x01U

typedef struct {
    float humidity_0;
    float humidity_1;
    float temperature_0;
    float temperature_1;
    int16_t humidity_out_0;
    int16_t humidity_out_1;
    int16_t temperature_out_0;
    int16_t temperature_out_1;
} HTS221_Calibration;

static HTS221_Calibration calibration;

static HTS221_Status read_registers(uint8_t reg, uint8_t *data, uint16_t length)
{
    if (HAL_I2C_Mem_Read(&HTS221_I2C, HTS221_ADDRESS,
                         (uint16_t)(reg | (length > 1U ? HTS221_REG_AUTO_INC : 0U)),
                         I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY) != HAL_OK) {
        return HTS221_IO_ERROR;
    }

    return HTS221_OK;
}

static HTS221_Status write_register(uint8_t reg, uint8_t value)
{
    if (HAL_I2C_Mem_Write(&HTS221_I2C, HTS221_ADDRESS, reg,
                          I2C_MEMADD_SIZE_8BIT, &value, 1U, HAL_MAX_DELAY) != HAL_OK) {
        return HTS221_IO_ERROR;
    }

    return HTS221_OK;
}

static int16_t signed_word(const uint8_t *bytes)
{
    return (int16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

HTS221_Status HTS221_Init(void)
{
    uint8_t id;
    uint8_t raw[16];
    uint8_t temperature_msb;
    HTS221_Status status;

    status = read_registers(HTS221_REG_WHO_AM_I, &id, 1U);
    if (status != HTS221_OK) {
        return status;
    }
    if (id != HTS221_WHO_AM_I_VALUE) {
        return HTS221_BAD_DEVICE;
    }

    status = read_registers(HTS221_REG_CALIB_START, raw, sizeof(raw));
    if (status != HTS221_OK) {
        return status;
    }

    calibration.humidity_0 = (float)raw[0] / 2.0f;
    calibration.humidity_1 = (float)raw[1] / 2.0f;
    temperature_msb = raw[5];
    calibration.temperature_0 =
        (float)((uint16_t)raw[2] | (((uint16_t)temperature_msb & 0x03U) << 8)) / 8.0f;
    calibration.temperature_1 =
        (float)((uint16_t)raw[3] | (((uint16_t)temperature_msb & 0x0CU) << 6)) / 8.0f;
    calibration.humidity_out_0 = signed_word(&raw[6]);
    calibration.humidity_out_1 = signed_word(&raw[10]);
    calibration.temperature_out_0 = signed_word(&raw[12]);
    calibration.temperature_out_1 = signed_word(&raw[14]);

    if ((calibration.humidity_out_0 == calibration.humidity_out_1) ||
        (calibration.temperature_out_0 == calibration.temperature_out_1)) {
        return HTS221_BAD_DEVICE;
    }

    /* Maximize sensor averaging, enable BDU and continuous 12.5 Hz output. */
    status = write_register(HTS221_REG_AV_CONF, 0x3FU);
    if (status != HTS221_OK) {
        return status;
    }
    return write_register(HTS221_REG_CTRL_REG1, 0x87U);
}

HTS221_Status HTS221_Read(HTS221_Sample *sample)
{
    uint8_t status_reg;
    uint8_t raw[4];
    int16_t humidity_raw;
    int16_t temperature_raw;
    float humidity;
    float temperature;
    HTS221_Status status;

    if ((sample == NULL) || (&HTS221_I2C == NULL)) {
        return HTS221_IO_ERROR;
    }

    status = read_registers(HTS221_REG_STATUS, &status_reg, 1U);
    if (status != HTS221_OK) {
        return status;
    }
    if ((status_reg & (HTS221_STATUS_H_DA | HTS221_STATUS_T_DA)) !=
        (HTS221_STATUS_H_DA | HTS221_STATUS_T_DA)) {
        return HTS221_NOT_READY;
    }

    status = read_registers(HTS221_REG_HUMIDITY_OUT, raw, sizeof(raw));
    if (status != HTS221_OK) {
        return status;
    }

    humidity_raw = signed_word(&raw[0]);
    temperature_raw = signed_word(&raw[2]);

    humidity = calibration.humidity_0 +
        ((float)(humidity_raw - calibration.humidity_out_0) *
         (calibration.humidity_1 - calibration.humidity_0)) /
        (float)(calibration.humidity_out_1 - calibration.humidity_out_0);
    temperature = calibration.temperature_0 +
        ((float)(temperature_raw - calibration.temperature_out_0) *
         (calibration.temperature_1 - calibration.temperature_0)) /
        (float)(calibration.temperature_out_1 - calibration.temperature_out_0);

    if (humidity < 0.0f) {
        humidity = 0.0f;
    } else if (humidity > 100.0f) {
        humidity = 100.0f;
    }

    sample->temperature_c = temperature;
    sample->humidity_percent = humidity;
    return HTS221_OK;
}
