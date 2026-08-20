/**
 * @file    madgwick.c
 * @brief   Madgwick AHRS 实现
 *
 * 参考: S.O.H. Madgwick, "An efficient orientation filter for
 *       inertial and inertial/magnetic sensor arrays", 2011.
 *
 * 算法核心:
 *   陀螺仪 + 加速度计 → 四元数姿态
 *   无磁力计时 yaw 不可观测(会漂移), pitch/roll 稳定
 */

#include "madgwick.h"
#include <math.h>

/* ── 快速平方根倒数 (Quake III 经典) ── */
static float inv_sqrt(float x) {
    float halfx = 0.5f * x;
    float y     = x;
    long  i     = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

/* ================================================================
 *  MadgwickInit
 * ================================================================ */
void MadgwickInit(madgwick_t *m, float freq_hz, float beta) {
    /* 初始姿态: 水平放置, 航向任意 */
    m->q0 = 1.0f;
    m->q1 = 0.0f;
    m->q2 = 0.0f;
    m->q3 = 0.0f;

    m->beta             = beta;
    m->sample_freq      = freq_hz;
    m->inv_sample_freq  = 1.0f / freq_hz;
}

/* ================================================================
 *  MadgwickUpdate — 核心更新函数 (AHRS, 无磁力计)
 * ================================================================ */
void MadgwickUpdate(madgwick_t *m,
                    float ax, float ay, float az,
                    float gx, float gy, float gz)
{
    float q0 = m->q0, q1 = m->q1, q2 = m->q2, q3 = m->q3;
    float recip_norm;
    float s0, s1, s2, s3;
    float q_dot0, q_dot1, q_dot2, q_dot3;

    /* ── 归一化加速度计测量 ── */
    recip_norm = inv_sqrt(ax*ax + ay*ay + az*az);
    ax *= recip_norm;
    ay *= recip_norm;
    az *= recip_norm;

    /* ── 梯度下降校正步 ── */
    /* 目标函数 f = [2(q1*q3 - q0*q2) - ax,
     *               2(q0*q1 + q2*q3) - ay,
     *               2(0.5 - q1² - q2²) - az]
     * 梯度 = Jacobianᵀ · f
     * Jacobian = [-2q2,   2q3,  -2q0,  2q1
     *              2q1,   2q0,   2q3,  2q2
     *               0,   -4q1,  -4q2,   0 ]
     */
    {
        float _2q0 = 2.0f * q0, _2q1 = 2.0f * q1;
        float _2q2 = 2.0f * q2, _2q3 = 2.0f * q3;
        float _4q1 = 4.0f * q1, _4q2 = 4.0f * q2;

        /* f 向量 */
        float f0 = _2q1*q3 - _2q0*q2 - ax;
        float f1 = _2q0*q1 + _2q2*q3 - ay;
        float f2 = 1.0f - _2q1*q1 - _2q2*q2 - az;  /* 2*(0.5-q1²-q2²) */

        /* ∇f = Jᵀ · f 的四个分量 */
        s0 = -_2q2*f0 + _2q1*f1;
        s1 =  _2q3*f0 + _2q0*f1 - _4q1*f2;
        s2 = -_2q0*f0 + _2q3*f1 - _4q2*f2;
        s3 =  _2q1*f0 + _2q2*f1;
    }

    /* 归一化梯度 */
    recip_norm = inv_sqrt(s0*s0 + s1*s1 + s2*s2 + s3*s3);
    s0 *= recip_norm;
    s1 *= recip_norm;
    s2 *= recip_norm;
    s3 *= recip_norm;

    /* ── 陀螺仪四元数导数: q_dot = 0.5 * q ⊗ ω ── */
    q_dot0 = 0.5f * (-q1*gx - q2*gy - q3*gz);
    q_dot1 = 0.5f * ( q0*gx + q2*gz - q3*gy);
    q_dot2 = 0.5f * ( q0*gy - q1*gz + q3*gx);
    q_dot3 = 0.5f * ( q0*gz + q1*gy - q2*gx);

    /* ── 梯度下降修正 ── */
    q_dot0 -= m->beta * s0;
    q_dot1 -= m->beta * s1;
    q_dot2 -= m->beta * s2;
    q_dot3 -= m->beta * s3;

    /* ── 一阶积分 ── */
    float dt = m->inv_sample_freq;
    m->q0 = q0 + q_dot0 * dt;
    m->q1 = q1 + q_dot1 * dt;
    m->q2 = q2 + q_dot2 * dt;
    m->q3 = q3 + q_dot3 * dt;

    /* ── 四元数归一化 ── */
    recip_norm = inv_sqrt(m->q0*m->q0 + m->q1*m->q1 +
                          m->q2*m->q2 + m->q3*m->q3);
    m->q0 *= recip_norm;
    m->q1 *= recip_norm;
    m->q2 *= recip_norm;
    m->q3 *= recip_norm;
}

/* ================================================================
 *  欧拉角提取
 * ================================================================ */
float MadgwickGetPitch(const madgwick_t *m) {
    /* pitch = asin(2*(q0*q2 - q3*q1)) */
    float val = 2.0f * (m->q0 * m->q2 - m->q3 * m->q1);
    if (val >  1.0f) val =  1.0f;
    if (val < -1.0f) val = -1.0f;
    return asinf(val) * 57.2957795f;  /* rad → deg */
}

float MadgwickGetRoll(const madgwick_t *m) {
    /* roll = atan2(2*(q0*q1 + q2*q3), 1 - 2*(q1² + q2²)) */
    float val = 2.0f * (m->q0 * m->q1 + m->q2 * m->q3);
    float den = 1.0f - 2.0f * (m->q1*m->q1 + m->q2*m->q2);
    return atan2f(val, den) * 57.2957795f;
}

float MadgwickGetYaw(const madgwick_t *m) {
    /* yaw = atan2(2*(q0*q3 + q1*q2), 1 - 2*(q2² + q3²)) */
    float val = 2.0f * (m->q0 * m->q3 + m->q1 * m->q2);
    float den = 1.0f - 2.0f * (m->q2*m->q2 + m->q3*m->q3);
    return atan2f(val, den) * 57.2957795f;
}

/* ================================================================
 *  世界系加速度 (去重力)
 *
 *  原理: 用四元数旋转传感器系加速度 → 世界系, 再减去 [0,0,1]
 *
 *  旋转矩阵 R (四元数 → 旋转):
 *    [1-2(q2²+q3²),  2(q1q2-q0q3),  2(q1q3+q0q2)]
 *    [2(q1q2+q0q3),  1-2(q1²+q3²),  2(q2q3-q0q1)]
 *    [2(q1q3-q0q2),  2(q2q3+q0q1),  1-2(q1²+q2²)]
 * ================================================================ */
void MadgwickWorldAccel(const madgwick_t *m,
                        float ax, float ay, float az,
                        float *wx, float *wy, float *wz)
{
    float q0 = m->q0, q1 = m->q1, q2 = m->q2, q3 = m->q3;

    float q0q1 = q0*q1, q0q2 = q0*q2, q0q3 = q0*q3;
    float q1q2 = q1*q2, q1q3 = q1*q3, q2q3 = q2*q3;
    float q1q1 = q1*q1, q2q2 = q2*q2, q3q3 = q3*q3;

    /* R * [ax, ay, az]^T */
    *wx = (1.0f - 2.0f*(q2q2+q3q3))*ax + 2.0f*(q1q2 - q0q3)*ay       + 2.0f*(q1q3 + q0q2)*az;
    *wy = 2.0f*(q1q2 + q0q3)*ax       + (1.0f - 2.0f*(q1q1+q3q3))*ay + 2.0f*(q2q3 - q0q1)*az;
    *wz = 2.0f*(q1q3 - q0q2)*ax       + 2.0f*(q2q3 + q0q1)*ay       + (1.0f - 2.0f*(q1q1+q2q2))*az;

    /* 减重力: 世界系Z轴向上,重力 = [0,0,-1g] → 加回来得到线性加速度 */
    *wz -= 1.0f;  /* 去掉重力分量 (单位: g) */
}
