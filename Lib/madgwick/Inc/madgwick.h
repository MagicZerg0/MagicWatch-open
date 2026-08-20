/**
 * @file    madgwick.h
 * @brief   Madgwick AHRS 姿态解算滤波器
 *
 * Sebastian Madgwick, 2011 — 专为嵌入式IMU设计:
 *   陀螺仪积分 → 快速跟踪姿态变化
 *   加速度计   → 梯度下降校正重力方向,消除积分漂移
 *   计算量:  ~300次浮点运算/更新, @160MHz ≈ 2-3μs
 *
 * 使用:
 *   MadgwickInit(&m, 200.0f, 0.05f);   // 200Hz, beta=0.05
 *   每次读MPU6050后:
 *     MadgwickUpdate(&m, ax_g, ay_g, az_g, gx_rad, gy_rad, gz_rad);
 *   获取姿态:
 *     pitch = MadgwickGetPitch(&m);   // 度
 *   获取世界系加速度(去重力):
 *     MadgwickWorldAccel(&m, ax, ay, az, &wx, &wy, &wz);
 */

#ifndef MADGWICK_H
#define MADGWICK_H

#include <stdint.h>

typedef struct {
    /* 四元数 (Hamilton约定: q0标量 + q1*i + q2*j + q3*k) */
    float q0, q1, q2, q3;

    /* 参数 */
    float beta;          /* 算法增益 (典型0.03~0.10) */
    float sample_freq;   /* 采样频率 Hz */
    float inv_sample_freq; /* 1/fs */
} madgwick_t;

/**
 * @brief 初始化滤波器
 * @param m        滤波器实例
 * @param freq_hz  采样频率 (如 200.0f)
 * @param beta     算法增益 (默认 0.05f, 运动剧烈可加大)
 */
void MadgwickInit(madgwick_t *m, float freq_hz, float beta);

/**
 * @brief 喂一帧IMU数据,更新姿态四元数
 * @param m    滤波器实例
 * @param ax   加速度 X (单位: g, 即 9.81m/s² = 1.0)
 * @param ay   加速度 Y
 * @param az   加速度 Z
 * @param gx   角速度 X (单位: rad/s)
 * @param gy   角速度 Y
 * @param gz   角速度 Z
 */
void MadgwickUpdate(madgwick_t *m,
                    float ax, float ay, float az,
                    float gx, float gy, float gz);

/**
 * @brief 从四元数提取欧拉角
 * @return pitch (度), 低头为正
 */
float MadgwickGetPitch(const madgwick_t *m);

/**
 * @return roll (度), 右倾为正
 */
float MadgwickGetRoll(const madgwick_t *m);

/**
 * @return yaw (度), 右转为正 (无磁力计会持续漂移)
 */
float MadgwickGetYaw(const madgwick_t *m);

/**
 * @brief 将传感器系加速度转换到世界系,并移除重力
 * @param[in]  ax, ay, az  传感器系加速度 (单位: g)
 * @param[out] wx, wy, wz  世界系线性加速度 (单位: g, 已去重力)
 *
 * 用法: 对击球后短窗口的世界系速度积分 → 出射仰角
 */
void MadgwickWorldAccel(const madgwick_t *m,
                        float ax, float ay, float az,
                        float *wx, float *wy, float *wz);

#endif /* MADGWICK_H */
