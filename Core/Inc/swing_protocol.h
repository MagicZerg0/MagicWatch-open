#ifndef SWING_PROTOCOL_H
#define SWING_PROTOCOL_H

#include <stdint.h>
#include "cmsis_os2.h"

/* ═══════════════════════════════════════════════
 *  合同1: 每拍结果 (算法层 → TouchGFX)
 * ═══════════════════════════════════════════════ */
#define SWING_WAVEFORM_MAX  240

typedef struct {
    uint8_t  swing_index;
    float    peak_accel;
    int      score;
    float    waveform[SWING_WAVEFORM_MAX];
    uint16_t waveform_len;
} SwingResult_t;

/* ═══════════════════════════════════════════════
 *  合同2: 训练完成报告 (算法层 → TouchGFX)
 * ═══════════════════════════════════════════════ */
typedef struct {
    uint8_t  total;
    float    max_peak;
    float    avg_score;
    uint8_t  stability;
    int      type;
    char     advice[300];
} TrainingDone_t;

/* ═══════════════════════════════════════════════
 *  FreeRTOS 通信层 (TouchGFX ↔ swing_task)
 *  (不属于算法合同，是传输通道)
 * ═══════════════════════════════════════════════ */
typedef enum {
    SWING_CMD_START = 1,
    SWING_CMD_STOP  = 2,
} SwingCmd_t;

typedef struct {
    SwingCmd_t cmd;
    int        stroke;
    uint8_t    target;
    int        skill;
} SwingCmdArgs_t;

extern osMessageQueueId_t swingCmdQueueHandle;
extern osMessageQueueId_t swingResultQueueHandle;
extern osMessageQueueId_t trainingDoneQueueHandle;

#endif /* SWING_PROTOCOL_H */