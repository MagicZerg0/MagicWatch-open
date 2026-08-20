#ifndef MPU6050_H
#define MPU6050_H

#include "stm32u5xx_hal.h"
#include <stdint.h>

/* MPU6050 I2C address (AD0 = GND) */
#define MPU6050_ADDR            0x68

/* Register map */
#define MPU6050_REG_WHO_AM_I    0x75
#define MPU6050_REG_PWR_MGMT_1  0x6B
#define MPU6050_REG_PWR_MGMT_2  0x6C
#define MPU6050_REG_SMPLRT_DIV  0x19
#define MPU6050_REG_CONFIG      0x1A
#define MPU6050_REG_GYRO_CONFIG 0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C
#define MPU6050_REG_INT_ENABLE  0x38
#define MPU6050_REG_INT_PIN_CFG 0x37
#define MPU6050_REG_INT_STATUS  0x3A
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_TEMP_OUT_H  0x41
#define MPU6050_REG_GYRO_XOUT_H 0x43
#define MPU6050_REG_SIGNAL_PATH_RESET 0x68
#define MPU6050_REG_USER_CTRL   0x6A

/* Gyroscope full-scale range */
typedef enum {
    MPU6050_GYRO_250  = 0x00,  /* ±250 °/s  */
    MPU6050_GYRO_500  = 0x01,  /* ±500 °/s  */
    MPU6050_GYRO_1000 = 0x02,  /* ±1000 °/s */
    MPU6050_GYRO_2000 = 0x03,  /* ±2000 °/s */
} mpu6050_gyro_range_t;

/* Accelerometer full-scale range */
typedef enum {
    MPU6050_ACCEL_2G  = 0x00,  /* ±2g  */
    MPU6050_ACCEL_4G  = 0x01,  /* ±4g  */
    MPU6050_ACCEL_8G  = 0x02,  /* ±8g  */
    MPU6050_ACCEL_16G = 0x03,  /* ±16g */
} mpu6050_accel_range_t;

/* Digital Low-Pass Filter bandwidth */
typedef enum {
    MPU6050_DLPF_256HZ = 0x00,
    MPU6050_DLPF_188HZ = 0x01,
    MPU6050_DLPF_98HZ  = 0x02,
    MPU6050_DLPF_42HZ  = 0x03,
    MPU6050_DLPF_20HZ  = 0x04,
    MPU6050_DLPF_10HZ  = 0x05,
    MPU6050_DLPF_5HZ   = 0x06,
} mpu6050_dlpf_t;

/* Raw sensor data */
typedef struct {
    int16_t accel_x, accel_y, accel_z;
    int16_t temp;
    int16_t gyro_x,  gyro_y,  gyro_z;
} mpu6050_raw_t;

/* Scaled sensor data (real units) */
typedef struct {
    float accel_x, accel_y, accel_z;  /* m/s² */
    float gyro_x,  gyro_y,  gyro_z;  /* °/s   */
    float temp;                       /* °C    */
} mpu6050_data_t;

/* Configuration structure */
typedef struct {
    mpu6050_gyro_range_t  gyro_range;
    mpu6050_accel_range_t accel_range;
    mpu6050_dlpf_t        dlpf;
    uint8_t               sample_rate_div;  /* Rate = 1kHz / (1 + div) */
} mpu6050_config_t;

/* API */
HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c, const mpu6050_config_t *cfg);
HAL_StatusTypeDef MPU6050_ReadRaw(I2C_HandleTypeDef *hi2c, mpu6050_raw_t *raw);
void              MPU6050_Convert(const mpu6050_raw_t *raw, const mpu6050_config_t *cfg, mpu6050_data_t *out);
uint8_t           MPU6050_WhoAmI(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef MPU6050_Reset(I2C_HandleTypeDef *hi2c);

#endif
