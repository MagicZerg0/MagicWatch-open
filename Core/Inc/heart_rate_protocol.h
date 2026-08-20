#ifndef __HEART_RATE_PROTOCOL_H
#define __HEART_RATE_PROTOCOL_H

#include <stdint.h>
#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 命令类型 */
typedef enum {
    CMD_START_MEASUREMENT = 1
} HeartRateCmd_t;

/* 测量结果 */
typedef struct {
    int32_t heart_rate;
    int32_t spo2;
    uint8_t hr_valid;
    uint8_t spo2_valid;
} HeartRateResult_t;

/* 队列句柄（由 app_freertos.c 定义） */
extern osMessageQueueId_t heartRateCmdQueueHandle;
extern osMessageQueueId_t heartRateResultQueueHandle;

#ifdef __cplusplus
}
#endif

#endif