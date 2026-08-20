/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "max30102.h"
#include "heart_rate_protocol.h"
#include "swing_protocol.h"
#include "swing_task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* 命令类型 */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for TGFXTask */
osThreadId_t TGFXTaskHandle;
const osThreadAttr_t TGFXTask_attributes = {
  .name = "TGFXTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 3048 * 4
};
/* Definitions for heartRateTask */
osThreadId_t heartRateTaskHandle;
const osThreadAttr_t heartRateTask_attributes = {
  .name = "heartRateTask",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 1024 * 4
};
/* Definitions for swingTask */
osThreadId_t swingTaskHandle;
const osThreadAttr_t swingTask_attributes = {
  .name = "swingTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 3072 * 4
};
/* Definitions for gui_msg */
osMessageQueueId_t gui_msgHandle;
const osMessageQueueAttr_t gui_msg_attributes = {
  .name = "gui_msg"
};
/* Definitions for heartRateCmdQueue */
osMessageQueueId_t heartRateCmdQueueHandle;
const osMessageQueueAttr_t heartRateCmdQueue_attributes = {
  .name = "heartRateCmdQueue"
};
/* Definitions for heartRateResultQueue */
osMessageQueueId_t heartRateResultQueueHandle;
const osMessageQueueAttr_t heartRateResultQueue_attributes = {
  .name = "heartRateResultQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of gui_msg */
  gui_msgHandle = osMessageQueueNew (16, sizeof(uint16_t), &gui_msg_attributes);
  /* creation of heartRateCmdQueue */
  heartRateCmdQueueHandle = osMessageQueueNew (4, sizeof(uint32_t), &heartRateCmdQueue_attributes);
  /* creation of heartRateResultQueue */
  heartRateResultQueueHandle = osMessageQueueNew (1, sizeof(uint32_t), &heartRateResultQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  /* 重新创建结果队列，使用正确的结构体大小（CubeMX 生成的会被覆盖） */
  osMessageQueueDelete(heartRateResultQueueHandle);
  heartRateResultQueueHandle = osMessageQueueNew(1, sizeof(HeartRateResult_t), &heartRateResultQueue_attributes);
  
  // 命令队列
  swingCmdQueueHandle = osMessageQueueNew(4, sizeof(SwingCmdArgs_t), NULL);
  // 单次挥拍结果队列
  swingResultQueueHandle = osMessageQueueNew(4, sizeof(SwingResult_t), NULL);
  // 训练完成队列
  trainingDoneQueueHandle = osMessageQueueNew(2, sizeof(TrainingDone_t), NULL);
  /* USER CODE END RTOS_QUEUES */
  /* creation of TGFXTask */
  TGFXTaskHandle = osThreadNew(StartDefaultTask, NULL, &TGFXTask_attributes);

  /* creation of heartRateTask */
  heartRateTaskHandle = osThreadNew(vHeartRateTask, NULL, &heartRateTask_attributes);

  /* creation of swingTask */
  swingTaskHandle = osThreadNew(SwingTask_Main, NULL, &swingTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the TGFXTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN TGFXTask */
  extern void touchgfxTaskEntry(void);
  touchgfxTaskEntry();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END TGFXTask */
}

/* USER CODE BEGIN Header_vHeartRateTask */

static int32_t collect_samples(uint32_t *red_buf, uint32_t *ir_buf, uint32_t timeout_ms)
{
    int32_t count = 0;
    uint32_t red, ir;
    uint32_t last_red = 0, last_ir = 0;
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);

    while (count < BUFFER_SIZE)
    {
        if (xTaskGetTickCount() > deadline)
            break;

        /* read FIFO directly; do NOT depend on the INT pin or wr/rd pointers.
           When the FIFO is empty the read returns the last sample unchanged,
           so we use "data changed" as the new-sample indicator. */
        max30102_read_fifo(&red, &ir);
        if (red != last_red || ir != last_ir)
        {
            last_red = red;
            last_ir = ir;
            if (red > 100 && ir > 100)   /* relaxed threshold for weak signal */
            {
                red_buf[count] = red;
                ir_buf[count]  = ir;
                count++;
            }
        }
        osDelay(2);   /* throttle I2C accesses */
    }

    return count;
}


/**
* @brief Function implementing the heartRateTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vHeartRateTask */
__weak void vHeartRateTask(void *argument)
{
  /* USER CODE BEGIN heartRateTask */
  HeartRateCmd_t cmd;
  HeartRateResult_t result;
  static uint32_t red_buf[BUFFER_SIZE];
  static uint32_t ir_buf[BUFFER_SIZE];
  int32_t sample_count = 0;
  int32_t hr, spo2;
  int8_t  hr_valid, spo2_valid;

  osDelay(100);

  if (!max30102_init()) {
      result.hr_valid = 0;
      result.heart_rate = -999;
      result.spo2 = -999;
      result.spo2_valid = 0;
      osMessageQueuePut(heartRateResultQueueHandle, &result, 0, 0);
      vTaskSuspend(NULL);
  }

  for(;;)
  {
      if (osMessageQueueGet(heartRateCmdQueueHandle, &cmd, NULL, osWaitForever) == osOK)
      {
          if (cmd == CMD_START_MEASUREMENT)
          {
              #define MAX_ROUNDS       3
              #define MIN_VALID_CNT    1
              int32_t hr_vals[MAX_ROUNDS];
              int32_t valid_cnt = 0;
              int32_t spo2_final = -1;

              for (int r = 0; r < MAX_ROUNDS; r++)
              {
                  sample_count = collect_samples(red_buf, ir_buf, 6000);

                  if (sample_count >= BUFFER_SIZE)
                  {
                      maxim_heart_rate_and_oxygen_saturation(ir_buf, BUFFER_SIZE, red_buf,
                          &spo2, &spo2_valid, &hr, &hr_valid);

                      if (hr_valid && hr >= 45 && hr <= 130)
                      {
                          hr_vals[valid_cnt] = hr;
                          valid_cnt++;
                      }
                      if (spo2_valid && spo2 >= 80 && spo2 <= 100)
                          spo2_final = spo2;
                  }
                  osDelay(50);
              }

              if (valid_cnt >= MIN_VALID_CNT)
              {
                  for (int i = 0; i < valid_cnt - 1; i++)
                      for (int j = i + 1; j < valid_cnt; j++)
                          if (hr_vals[i] > hr_vals[j])
                          {
                              int32_t tmp = hr_vals[i];
                              hr_vals[i] = hr_vals[j];
                              hr_vals[j] = tmp;
                          }

                  result.heart_rate = hr_vals[valid_cnt / 2];
                  result.hr_valid = 1;
                  result.spo2 = spo2_final;
              }
              else
              {
                  result.heart_rate = -1;
                  result.hr_valid = 0;
                  result.spo2 = -1;
                  result.spo2_valid = 0;
              }

              osMessageQueuePut(heartRateResultQueueHandle, &result, 0, 0);
          }
      }
  }
  /* USER CODE END heartRateTask */
}

/* USER CODE BEGIN Header_SwingTask_Main */
/**
* @brief Function implementing the swingTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SwingTask_Main */
void SwingTask_Main(void *argument)
{
  /* USER CODE BEGIN swingTask */
  extern void vSwingTaskMain(void *argument);
  vSwingTaskMain(argument);
  /* USER CODE END swingTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

