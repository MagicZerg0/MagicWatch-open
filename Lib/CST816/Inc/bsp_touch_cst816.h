#ifndef __CST816_H__
#define __CST816_H__


#include "main.h"
#ifdef __cplusplus
extern "C" {
#endif


#define TP_PRES_DOWN 0x81
#define TP_COORD_UD  0x40

/* IO 引脚定义 */
#define CST816_RST_PORT    GPIOA
#define CST816_RST_PIN     GPIO_PIN_3
#define CST816_SCL_PORT    GPIOA
#define CST816_SCL_PIN     GPIO_PIN_2
#define CST816_SDA_PORT    GPIOA
#define CST816_SDA_PIN     GPIO_PIN_1
#define CST816_INT_PORT    GPIOA
#define CST816_INT_PIN     GPIO_PIN_4

/* HAL 封装的 IO 操作 */
#define CST816_RST_Set()    HAL_GPIO_WritePin(CST816_RST_PORT, CST816_RST_PIN, GPIO_PIN_SET)
#define CST816_RST_Clr()    HAL_GPIO_WritePin(CST816_RST_PORT, CST816_RST_PIN, GPIO_PIN_RESET)
#define CST816_SCL_Set()    HAL_GPIO_WritePin(CST816_SCL_PORT, CST816_SCL_PIN, GPIO_PIN_SET)
#define CST816_SCL_Clr()    HAL_GPIO_WritePin(CST816_SCL_PORT, CST816_SCL_PIN, GPIO_PIN_RESET)
#define CST816_SDA_Set()    HAL_GPIO_WritePin(CST816_SDA_PORT, CST816_SDA_PIN, GPIO_PIN_SET)
#define CST816_SDA_Clr()    HAL_GPIO_WritePin(CST816_SDA_PORT, CST816_SDA_PIN, GPIO_PIN_RESET)
#define CST816_SDA_Read()   HAL_GPIO_ReadPin(CST816_SDA_PORT, CST816_SDA_PIN)
#define CST816_INT_Read()   HAL_GPIO_ReadPin(CST816_INT_PORT, CST816_INT_PIN)

/* I2C 读写命令 */
#define FT_CMD_WR 0x2A
#define FT_CMD_RD 0x2B

/* CST816 寄存器 */
#define FT_DEVIDE_MODE       0x00
#define FT_REG_NUM_FINGER    0x02
#define FT_TP1_REG           0x03
#define FT_TP2_REG           0x09
#define FT_TP3_REG           0x0F
#define FT_TP4_REG           0x15
#define FT_TP5_REG           0x1B
#define FT_ID_G_LIB_VERSION  0xA1
#define FT_ID_G_MODE         0xA4
#define FT_ID_G_THGROUP      0x80
#define FT_ID_G_PERIODACTIVE 0x88
#define Chip_Vendor_ID       0xA7
#define ID_G_CST816ID        0xA8

/* 触摸点数据结构 */
typedef struct {
    uint8_t TouchSta;   // b7: 按下标记；b6: 更新标记；低5位: 有效触摸点
    uint16_t x[5];
    uint16_t y[5];
} TouchPointRefTypeDef;

extern TouchPointRefTypeDef TPR_Structure;

/* 驱动接口 */
uint8_t CST816_WR_Reg(uint16_t reg, uint8_t *buf, uint8_t len);
void    CST816_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len);
void    DWT_Init(void);
void    CST816_Init(void);
void    CST816_Scan(void);

#ifdef __cplusplus
}
#endif

#endif