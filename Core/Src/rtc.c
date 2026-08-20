/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"
#include "stm32u5xx_hal_rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_PrivilegeStateTypeDef privilegeState = {0};
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */
  uint32_t rtc_was_set = 0;
  RTC_TimeTypeDef savedTime = {0};
  RTC_DateTypeDef savedDate = {0};
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) == 0x32F2) {
      rtc_was_set = 1;
      // 直接读 RTC 寄存器（HAL_RTC_Init 还没调用，不能用 HAL API）
      uint32_t tr = RTC->TR;
      uint32_t dr = RTC->DR;
      // BCD 解码 → 存入 HAL 结构体
      savedTime.Hours   = ((tr >> 16) & 0x0F) + ((tr >> 20) & 0x03) * 10;
      savedTime.Minutes = ((tr >> 8)  & 0x0F) + ((tr >> 12) & 0x07) * 10;
      savedTime.Seconds = (tr & 0x0F) + ((tr >> 4) & 0x07) * 10;
      savedTime.TimeFormat = RTC_HOURFORMAT_24;
      savedTime.SubSeconds = 0;
      savedTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
      savedTime.StoreOperation = RTC_STOREOPERATION_RESET;
      savedDate.Year    = ((dr >> 16) & 0x0F) + ((dr >> 20) & 0x0F) * 10;
      savedDate.Month   = ((dr >> 8)  & 0x0F) + ((dr >> 12) & 0x01) * 10;
      savedDate.Date    = (dr & 0x0F) + ((dr >> 4) & 0x03) * 10;
      savedDate.WeekDay = ((dr >> 13) & 0x07);
  }
  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
  privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
  privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
  privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
  if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  // 提升特权
  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_YES;
  HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState);
  if (rtc_was_set) {
      // CubeMX 已经写了 00:00:00，用 HAL 恢复之前保存的时间
      HAL_RTC_SetTime(&hrtc, &savedTime, RTC_FORMAT_BIN);
      HAL_RTC_SetDate(&hrtc, &savedDate, RTC_FORMAT_BIN);
  } else {
      // 首次上电：用 HAL 设置初始时间 12:00:00
      sTime.Hours   = 13;
      sTime.Minutes = 30;
      sTime.Seconds = 0;
      sTime.TimeFormat = RTC_HOURFORMAT_24;
      sTime.SubSeconds = 0;
      sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
      sTime.StoreOperation = RTC_STOREOPERATION_RESET;
      sDate.WeekDay = RTC_WEEKDAY_MONDAY;
      sDate.Month   = RTC_MONTH_AUGUST;
      sDate.Date    = 12;
      sDate.Year    = 26;
      HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);
  }
  // 恢复特权
  //privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
  //HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState);
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
    __HAL_RCC_RTCAPB_CLKAM_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
    __HAL_RCC_RTCAPB_CLK_DISABLE();
    __HAL_RCC_RTCAPB_CLKAM_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

