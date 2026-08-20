/**
 * @file    dtw_matcher.h
 * @brief   DTW 模板匹配 — Sakoe-Chiba 带约束 + 两行滚动数组
 *
 * 用途: 验证式识别门控
 *   - 前3拍建立个人角速度模板
 *   - 后续每拍与模板做 DTW 比对
 *   - 距离过大 → "未识别到有效动作" (而非硬评分)
 *
 * 内存: 模板100点 + 两行滚动数组 ≈ 1.2KB
 */

#ifndef DTW_MATCHER_H
#define DTW_MATCHER_H

#include <stdint.h>
#include <stdbool.h>

#define DTW_TMPL_LEN    100    /* 标量DTW模板长度 */
#define DTW_BANDWIDTH    20    /* Sakoe-Chiba 约束带宽 */
#define QUAT_DTW_LEN    240   /* 四元数DTW模板长度 (击球前后各0.6s @200Hz) */
#define QUAT_BANDWIDTH   20   /* 四元数DTW带宽 */

/* ── 标量 DTW (角速度曲线, 保留) ── */
typedef struct {
    float   tmpl[DTW_TMPL_LEN];
    float   row0[DTW_TMPL_LEN];
    float   row1[DTW_TMPL_LEN];
    int     tmpl_built;
    int     raw_count;
    float   raw_buf[3][DTW_TMPL_LEN];
    float   best_distance;
} dtw_matcher_t;

/* ── 四元数 DTW (姿态序列匹配) ── */
typedef struct {
    float   tmpl[QUAT_DTW_LEN][4];  /* 模板四元数序列 [q0,q1,q2,q3] */
    int     tmpl_len;               /* 模板实际长度 */
    float   row0[QUAT_DTW_LEN];     /* DTW 滚动行 */
    float   row1[QUAT_DTW_LEN];
    int     tmpl_built;
    int     raw_count;
    float   best_distance;
    float   score;                  /* 0~100, 100*exp(-dist/sigma) */
} quat_dtw_t;

/**
 * @brief 初始化匹配器
 */
void DTW_Init(dtw_matcher_t *dtw);

/**
 * @brief 喂入一条原始角速度曲线 (自动重采样到 DTW_TMPL_LEN)
 * @param curve    原始 ω_mag 序列
 * @param length   曲线长度
 *
 * 前3条曲线用于建立模板; 模板建立后每条曲线都会与模板比对。
 *
 * @return 模板建立前返回 -1.0;
 *         模板建立后返回归一化 DTW 距离 (0=完全相同, 越大越不相似)
 */
float DTW_Feed(dtw_matcher_t *dtw, const float *curve, int length);

/**
 * @brief 模板是否已建立
 */
bool DTW_TemplateReady(const dtw_matcher_t *dtw);

/**
 * @brief 获取最近一次 DTW 距离
 */
float DTW_GetLastDistance(const dtw_matcher_t *dtw);

/**
 * @brief 判断曲线是否匹配模板
 * @param threshold  DTW 距离阈值 (建议 0.15~0.25, 越小越严格)
 */
bool DTW_IsMatch(const dtw_matcher_t *dtw, float threshold);

/* ── 四元数 DTW API ── */

/**
 * @brief 初始化四元数DTW匹配器
 */
void QuatDTW_Init(quat_dtw_t *qdtw);

/**
 * @brief 喂入一条四元数序列 (每次挥拍完成时调用)
 * @param quat   四元数数组 [q0,q1,q2,q3] × N
 * @param len    序列长度
 * @return 模板建立前返回 -1.0; 建立后返回归一化测地DTW距离
 */
float QuatDTW_Feed(quat_dtw_t *qdtw, const float (*quat)[4], int len);

/**
 * @brief 模板是否已建立
 */
bool QuatDTW_Ready(const quat_dtw_t *qdtw);

/**
 * @brief 获取姿态相似度评分 (0~100)
 */
float QuatDTW_GetScore(const quat_dtw_t *qdtw);

#endif /* DTW_MATCHER_H */
