#include "swing_task.h"
#include "swing_protocol.h"
#include "mpu6050.h"
#include "training_session.h"
#include "madgwick.h"
#include "main.h"
#include "cmsis_os2.h"
#include <math.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

osMessageQueueId_t swingCmdQueueHandle;
osMessageQueueId_t swingResultQueueHandle;
osMessageQueueId_t trainingDoneQueueHandle;

static mpu6050_config_t mpu_cfg;
static madgwick_t       ahrs;
static int              last_swing_count = 0;

// 自捕获波形
static float    self_wf[SWING_WAVEFORM_MAX];
static uint16_t self_wf_len = 0;
static uint8_t  self_capturing = 0;
static uint8_t  self_idle = 0;

static void I2C_BusReset(void) {
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        osDelay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        osDelay(1);
    }
    osDelay(10);
}

static void InitMPU6050(void) {
    I2C_BusReset();

    mpu_cfg.gyro_range      = MPU6050_GYRO_2000;
    mpu_cfg.accel_range     = MPU6050_ACCEL_16G;
    mpu_cfg.dlpf            = MPU6050_DLPF_42HZ;
    mpu_cfg.sample_rate_div = 4;

    MPU6050_Init(&hi2c1, &mpu_cfg);
}

void vSwingTaskMain(void *argument) {
    InitMPU6050();

    MadgwickInit(&ahrs, 200.0f, 0.05f);
    Training_SetAHRS(&ahrs);

    last_swing_count = 0;

    while (1) {
        // 1. 检查命令
        SwingCmdArgs_t cmd;
        if (osMessageQueueGet(swingCmdQueueHandle, &cmd, NULL, 0) == osOK) {
            if (cmd.cmd == SWING_CMD_START) {
                Training_Start(cmd.stroke, cmd.target, (skill_level_t)cmd.skill);
                last_swing_count = 0;
                self_capturing   = 0;
                self_wf_len      = 0;
                self_idle        = 0;
            }
            else if (cmd.cmd == SWING_CMD_STOP) {
                if (g_train_state == TRAIN_RUNNING) {
                    TrainingDone_t done = Training_Stop();
                    osMessageQueuePut(trainingDoneQueueHandle, &done, 0, 0);
                }
            }
        }

        // 2. 训练中
        if (g_train_state == TRAIN_RUNNING) {
            mpu6050_raw_t raw;
            mpu6050_data_t d;

            if (MPU6050_ReadRaw(&hi2c1, &raw) == HAL_OK) {
                MPU6050_Convert(&raw, &mpu_cfg, &d);

                // 合角速度 (°/s)
                float gm = sqrtf(d.gyro_x*d.gyro_x +
                                 d.gyro_y*d.gyro_y +
                                 d.gyro_z*d.gyro_z);

                // ── 自捕获波形 ──
                // 触发：gm > 500 °/s
                if (gm > 500.0f && !self_capturing) {
                    self_capturing = 1;
                    self_wf_len    = 0;
                    self_idle      = 0;
                }
                // 收集中
                if (self_capturing && self_wf_len < SWING_WAVEFORM_MAX) {
                    self_wf[self_wf_len++] = gm;
                }
                // 结束判定：gm < 200 °/s 持续
                if (gm < 200.0f && self_capturing) {
                    self_idle++;
                    if (self_idle > 15) {   // 75ms 空闲 → 挥拍结束
                        self_capturing = 0;
                        self_idle      = 0;
                    }
                } else if (gm >= 200.0f) {
                    self_idle = 0;
                }

                // Madgwick 姿态更新
                MadgwickUpdate(&ahrs,
                    d.accel_x / 9.81f, d.accel_y / 9.81f, d.accel_z / 9.81f,
                    d.gyro_x * 0.0174533f, d.gyro_y * 0.0174533f, d.gyro_z * 0.0174533f);

                // 喂检测算法
                Training_Update(&d);

                // 检测到新挥拍
                if (g_train_count != last_swing_count) {
                    last_swing_count = g_train_count;

                    SwingResult_t res;
                    res.swing_index  = g_train_count;
                    res.peak_accel   = g_train_last_peak;
                    res.score        = g_train_last_score;

                    // 用自己的波形
                    if (self_wf_len > 0) {
                        res.waveform_len = self_wf_len;
                        memcpy(res.waveform, self_wf,
                               self_wf_len * sizeof(float));
                    } else {
                        // 兜底：用队友的波形
                        res = g_swing_result;
                    }

                    osMessageQueuePut(swingResultQueueHandle, &res, 0, 0);

                    // 复位，准备下一拍
                    self_capturing = 0;
                    self_wf_len    = 0;
                    self_idle      = 0;
                }
            }

            // 3. 训练完成
            if (g_train_state == TRAIN_DONE) {
                TrainingDone_t done = Training_Stop();
                osMessageQueuePut(trainingDoneQueueHandle, &done, 0, 0);
            }
        }

        osDelay(5);
    }
}