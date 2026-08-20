#include "mpu6050.h"

#define I2C_TIMEOUT 100

static HAL_StatusTypeDef WriteReg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return HAL_I2C_Master_Transmit(hi2c, MPU6050_ADDR << 1, buf, 2, I2C_TIMEOUT);
}

static HAL_StatusTypeDef ReadRegs(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *data, uint8_t len)
{
    return HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, I2C_TIMEOUT);
}

uint8_t MPU6050_WhoAmI(I2C_HandleTypeDef *hi2c)
{
    uint8_t val = 0;
    if (ReadRegs(hi2c, MPU6050_REG_WHO_AM_I, &val, 1) != HAL_OK) return 0;
    return val;
}

HAL_StatusTypeDef MPU6050_Reset(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef rc;

    /* Device reset: bit 7 = 1 */
    rc = WriteReg(hi2c, MPU6050_REG_PWR_MGMT_1, 0x80);
    if (rc != HAL_OK) return rc;
    HAL_Delay(100);

    /* Signal path reset */
    rc = WriteReg(hi2c, MPU6050_REG_SIGNAL_PATH_RESET, 0x07);
    if (rc != HAL_OK) return rc;
    HAL_Delay(100);

    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c, const mpu6050_config_t *cfg)
{
    HAL_StatusTypeDef rc;

    /* Reset device */
    rc = MPU6050_Reset(hi2c);
    if (rc != HAL_OK) return rc;

    /* Verify identity */
    uint8_t id = MPU6050_WhoAmI(hi2c);
    if (id != 0x68 && id != 0x70) return HAL_ERROR;

    /* Wake up: clear sleep bit, set clock to PLL gyro X */
    rc = WriteReg(hi2c, MPU6050_REG_PWR_MGMT_1, 0x01);
    if (rc != HAL_OK) return rc;
    HAL_Delay(10);

    /* All sensors on */
    rc = WriteReg(hi2c, MPU6050_REG_PWR_MGMT_2, 0x00);
    if (rc != HAL_OK) return rc;

    /* Sample rate divider */
    rc = WriteReg(hi2c, MPU6050_REG_SMPLRT_DIV, cfg->sample_rate_div);
    if (rc != HAL_OK) return rc;

    /* DLPF config */
    rc = WriteReg(hi2c, MPU6050_REG_CONFIG, cfg->dlpf);
    if (rc != HAL_OK) return rc;

    /* Gyro range */
    rc = WriteReg(hi2c, MPU6050_REG_GYRO_CONFIG, cfg->gyro_range << 3);
    if (rc != HAL_OK) return rc;

    /* Accel range */
    rc = WriteReg(hi2c, MPU6050_REG_ACCEL_CONFIG, cfg->accel_range << 3);
    if (rc != HAL_OK) return rc;

    /* INT pin: active low, open-drain, latch until cleared */
    rc = WriteReg(hi2c, MPU6050_REG_INT_PIN_CFG, 0x30);
    if (rc != HAL_OK) return rc;

    /* Enable data-ready interrupt */
    rc = WriteReg(hi2c, MPU6050_REG_INT_ENABLE, 0x01);
    if (rc != HAL_OK) return rc;

    /* Disable I2C bypass (aux I2C off, XDA/XCL unused) */
    rc = WriteReg(hi2c, MPU6050_REG_USER_CTRL, 0x00);
    if (rc != HAL_OK) return rc;

    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_ReadRaw(I2C_HandleTypeDef *hi2c, mpu6050_raw_t *raw)
{
    uint8_t buf[14];

    if (ReadRegs(hi2c, MPU6050_REG_ACCEL_XOUT_H, buf, 14) != HAL_OK)
        return HAL_ERROR;

    raw->accel_x = (int16_t)((buf[0]  << 8) | buf[1]);
    raw->accel_y = (int16_t)((buf[2]  << 8) | buf[3]);
    raw->accel_z = (int16_t)((buf[4]  << 8) | buf[5]);
    raw->temp    = (int16_t)((buf[6]  << 8) | buf[7]);
    raw->gyro_x  = (int16_t)((buf[8]  << 8) | buf[9]);
    raw->gyro_y  = (int16_t)((buf[10] << 8) | buf[11]);
    raw->gyro_z  = (int16_t)((buf[12] << 8) | buf[13]);

    return HAL_OK;
}

void MPU6050_Convert(const mpu6050_raw_t *raw, const mpu6050_config_t *cfg, mpu6050_data_t *out)
{
    static const float accel_lsb[] = {
        [MPU6050_ACCEL_2G]  = 16384.0f,
        [MPU6050_ACCEL_4G]  =  8192.0f,
        [MPU6050_ACCEL_8G]  =  4096.0f,
        [MPU6050_ACCEL_16G] =  2048.0f,
    };

    static const float gyro_lsb[] = {
        [MPU6050_GYRO_250]  = 131.0f,
        [MPU6050_GYRO_500]  =  65.5f,
        [MPU6050_GYRO_1000] =  32.8f,
        [MPU6050_GYRO_2000] =  16.4f,
    };

    float a_lsb = accel_lsb[cfg->accel_range];
    float g_lsb = gyro_lsb[cfg->gyro_range];

    out->accel_x = raw->accel_x / a_lsb * 9.80665f;
    out->accel_y = raw->accel_y / a_lsb * 9.80665f;
    out->accel_z = raw->accel_z / a_lsb * 9.80665f;
    out->gyro_x  = raw->gyro_x  / g_lsb;
    out->gyro_y  = raw->gyro_y  / g_lsb;
    out->gyro_z  = raw->gyro_z  / g_lsb;
    out->temp    = raw->temp / 340.0f + 36.53f;
}
