#ifndef TRAINING_SESSION_H
#define TRAINING_SESSION_H

#include "badminton_detect.h"
#include "dtw_matcher.h"
#include "swing_protocol.h"

/* 训练状态 */
typedef enum {
    TRAIN_IDLE,      /* 空闲 */
    TRAIN_READY,     /* 准备(倒计时) */
    TRAIN_RUNNING,   /* 训练中 */
    TRAIN_DONE,      /* 完成 */
} train_state_t;

/* ---- 供UI读取的全局状态 (合同3: 不可改名字) ---- */
extern train_state_t  g_train_state;
extern uint8_t        g_train_count;     /* 已完成次数 */
extern uint8_t        g_train_target;    /* 目标次数     */
extern float          g_train_last_peak; /* 上拍峰值     */
extern int            g_train_last_score;/* 上拍评分     */
extern float          g_train_heart_rate;/* 心率(0=未测)*/
extern float          g_train_dtw_dist;    /* 最近标量DTW 距离 (新增, 可选读) */
extern uint8_t        g_train_dtw_ready;   /* 标量DTW 模板是否就绪 (新增, 可选读) */
extern float          g_train_quat_score;  /* 四元数DTW 姿态相似度 0~100 (新增, 可选读) */

/* 最新一拍波形结果 (每拍更新, UI读取用于画波形图) */
extern SwingResult_t  g_swing_result;

/* 最新一拍完整结果 (含八维评分+矢量指标+建议, 供串口/蓝牙输出) */
extern bds_result_t   g_last_full_result;

/* ---- 供UI调用的函数 ---- */

/* 开始训练: UI点「开始练习」时调用
 * type: 0=高远球 1=杀球 2=挑球 (与 StrokeType 枚举值一致) */
void Training_Start(int type, uint8_t target, skill_level_t lvl);

/* 喂数据: main循环每5ms调一次 */
void Training_Update(mpu6050_data_t *d);

/* 停止训练: UI点「停止」或自动完成时调用, 返回训练报告 */
TrainingDone_t Training_Stop(void);

/* 获取今日累计训练次数 */
uint8_t Training_GetTodayTotal(void);

/* 设置 Madgwick AHRS 实例指针 (必须在 Training_Start 之前调用) */
void Training_SetAHRS(const madgwick_t *ahrs);

/* 强制手性偏好 (0=自动判别, 1=强制正手, 2=强制反手) */
void Training_SetHandPreference(int pref);

#endif
