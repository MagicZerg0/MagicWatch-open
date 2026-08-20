// PowerManager.h
#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "stm32u5xx_hal.h"
#include "main.h"   // 包含 LCD_BLK_Pin, LCD_BLK_GPIO_Port 等定义

#ifdef __cplusplus
extern "C" {
#endif

void EnterSleepMode(void);

#ifdef __cplusplus
}
#endif

#endif