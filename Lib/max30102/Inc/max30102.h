#ifndef __MAX30102_H
#define __MAX30102_H

#include "main.h"          /* HAL 总头文件 */
#include "stdbool.h"
#include <string.h>

/* ========================================================
   I2C 句柄：你用的是 I2C2
   ======================================================== */
extern I2C_HandleTypeDef hi2c2;   /* 由 CubeMX 生成 */

/* ========================================================
   MAX30102 7位地址 0x57 → HAL 左移 1 位 = 0xAE
   ======================================================== */
#define MAX30102_ADDR         (0x57 << 1)

/* ========================================================
   寄存器地址（与原版一致）
   ======================================================== */
#define REG_INTR_STATUS_1     0x00
#define REG_INTR_STATUS_2     0x01
#define REG_INTR_ENABLE_1     0x02
#define REG_INTR_ENABLE_2     0x03
#define REG_FIFO_WR_PTR       0x04
#define REG_OVF_COUNTER       0x05
#define REG_FIFO_RD_PTR       0x06
#define REG_FIFO_DATA         0x07
#define REG_FIFO_CONFIG       0x08
#define REG_MODE_CONFIG       0x09
#define REG_SPO2_CONFIG       0x0A
#define REG_LED1_PA           0x0C
#define REG_LED2_PA           0x0D
#define REG_PILOT_PA          0x10
#define REG_MULTI_LED_CTRL1   0x11
#define REG_MULTI_LED_CTRL2   0x12
#define REG_TEMP_INTR         0x1F
#define REG_TEMP_FRAC         0x20
#define REG_TEMP_CONFIG       0x21
#define REG_PROX_INT_THRESH   0x30
#define REG_REV_ID            0xFE
#define REG_PART_ID           0xFF

/* ========================================================
   算法参数
   ======================================================== */
#define FS                   100
#define BUFFER_SIZE          (FS * 3)       /* 500 采样点 */
#define MA4_SIZE             4
#define HAMMING_SIZE         5

/* ========================================================
   函数声明
   ======================================================== */

/* --- HAL I2C 封装（轮询模式）--- */
uint8_t max30102_write_reg(uint8_t reg, uint8_t data);
uint8_t max30102_read_reg(uint8_t reg, uint8_t *data);
uint8_t max30102_read_fifo(uint32_t *red, uint32_t *ir);
uint8_t max30102_init(void);
void    max30102_reset(void);

/* --- 心率血氧算法（100% 保留原版）--- */
void maxim_heart_rate_and_oxygen_saturation(
    uint32_t *pun_ir_buffer,  int32_t n_ir_buffer_length,
    uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid,
    int32_t *pn_heart_rate,   int8_t *pch_hr_valid);

void maxim_find_peaks(int32_t *pn_locs, int32_t *pn_npks,
    int32_t *pn_x, int32_t n_size, int32_t n_min_height,
    int32_t n_min_distance, int32_t n_max_num);

void maxim_peaks_above_min_height(int32_t *pn_locs, int32_t *pn_npks,
    int32_t *pn_x, int32_t n_size, int32_t n_min_height);

void maxim_remove_close_peaks(int32_t *pn_locs, int32_t *pn_npks,
    int32_t *pn_x, int32_t n_min_distance);

void maxim_sort_ascend(int32_t *pn_x, int32_t n_size);
void maxim_sort_indices_descend(int32_t *pn_x, int32_t *pn_indx, int32_t n_size);
uint8_t max30102_Bus_Read(uint8_t reg);

#endif