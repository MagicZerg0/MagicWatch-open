/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_KEY_Pin GPIO_PIN_0
#define USER_KEY_GPIO_Port GPIOA
#define USER_KEY_EXTI_IRQn EXTI0_IRQn
#define CST816_SDA_Pin GPIO_PIN_1
#define CST816_SDA_GPIO_Port GPIOA
#define CST816_SCL_Pin GPIO_PIN_2
#define CST816_SCL_GPIO_Port GPIOA
#define CST816_RST_Pin GPIO_PIN_3
#define CST816_RST_GPIO_Port GPIOA
#define ST7789_DC_Pin GPIO_PIN_0
#define ST7789_DC_GPIO_Port GPIOB
#define ST7789_CS_Pin GPIO_PIN_1
#define ST7789_CS_GPIO_Port GPIOB
#define ST7789_RST_Pin GPIO_PIN_2
#define ST7789_RST_GPIO_Port GPIOB
#define ST7789_PWR_Pin GPIO_PIN_10
#define ST7789_PWR_GPIO_Port GPIOB
#define MAX30102_INT_Pin GPIO_PIN_12
#define MAX30102_INT_GPIO_Port GPIOB
#define MAX30102_SCL_Pin GPIO_PIN_13
#define MAX30102_SCL_GPIO_Port GPIOB
#define MAX30102_SDA_Pin GPIO_PIN_14
#define MAX30102_SDA_GPIO_Port GPIOB
#define CST816_INT_Pin GPIO_PIN_15
#define CST816_INT_GPIO_Port GPIOB
#define CST816_INT_EXTI_IRQn EXTI15_IRQn
#define MPU6050_INT_Pin GPIO_PIN_5
#define MPU6050_INT_GPIO_Port GPIOB
#define MPU6050_SCL_Pin GPIO_PIN_6
#define MPU6050_SCL_GPIO_Port GPIOB
#define MPU6050_SDA_Pin GPIO_PIN_7
#define MPU6050_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
