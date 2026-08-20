/**
 * @file    dtw_matcher.c
 * @brief   DTW 模板匹配实现
 *
 * 算法:
 *   - 输入曲线重采样到 N=100 点 (线性插值)
 *   - 峰值归一化 (除以最大值)
 *   - 前3条平均建模板
 *   - Sakoe-Chiba 带约束 (w=20) + 两行滚动数组
 */

#include "dtw_matcher.h"
#include <string.h>
#include <math.h>

/* ================================================================
 *  重采样: 将 length 点曲线线性插值到 DTW_TMPL_LEN 点
 * ================================================================ */
static void resample(const float *src, int src_len, float *dst, int dst_len) {
    if (src_len < 2) {
        float v = (src_len > 0) ? src[0] : 0.0f;
        for (int i = 0; i < dst_len; i++) dst[i] = v;
        return;
    }
    for (int i = 0; i < dst_len; i++) {
        float pos = (float)i / (float)(dst_len - 1) * (float)(src_len - 1);
        int   idx = (int)pos;
        float frac = pos - (float)idx;
        if (idx >= src_len - 1) {
            dst[i] = src[src_len - 1];
        } else {
            dst[i] = src[idx] * (1.0f - frac) + src[idx + 1] * frac;
        }
    }
}

/* ================================================================
 *  峰值归一化
 * ================================================================ */
static void normalize_peak(float *curve, int len) {
    float maxv = 0.0f;
    for (int i = 0; i < len; i++) {
        if (curve[i] > maxv) maxv = curve[i];
    }
    if (maxv > 0.001f) {
        float inv = 1.0f / maxv;
        for (int i = 0; i < len; i++) curve[i] *= inv;
    }
}

/* ================================================================
 *  DTW 距离 (Sakoe-Chiba 带 + 两行滚动)
 *  N = 模板长度, M = 输入长度 (均已归一化)
 *
 *  因为模板和输入都是 DTW_TMPL_LEN (100), N == M
 *  简化: i 和 j 的索引范围相同
 * ================================================================ */
static float dtw_distance(const float *tmpl, const float *curve, int len,
                          int bandwidth,
                          float *row0, float *row1)
{
    /* 初始化第一行 */
    row0[0] = fabsf(tmpl[0] - curve[0]);
    for (int j = 1; j < len && j <= bandwidth; j++) {
        row0[j] = row0[j-1] + fabsf(tmpl[0] - curve[j]);
    }

    int path_len = 1;

    for (int i = 1; i < len; i++) {
        int j_start = (i - bandwidth > 0) ? i - bandwidth : 0;
        int j_end   = (i + bandwidth < len) ? i + bandwidth : len - 1;

        for (int j = j_start; j <= j_end; j++) {
            float cost = fabsf(tmpl[i] - curve[j]);
            float best;

            if (j == j_start) {
                /* 只能从上方来 */
                best = row0[j];
            } else if (j == j_start || i - 1 < j_start) {
                /* 只能从左侧来或对角 */
                float up   = (j <= i - 1 + bandwidth) ? row0[j] : 1e9f;
                float left = row1[j-1];
                float diag = (j > 0) ? row0[j-1] : 1e9f;
                best = up;
                if (left < best) best = left;
                if (diag < best) best = diag;
            } else {
                float up   = row0[j];
                float left = row1[j-1];
                float diag = row0[j-1];
                best = up;
                if (left < best) best = left;
                if (diag < best) best = diag;
            }

            row1[j] = cost + best;
        }

        /* 交换行 */
        float *tmp = row0; row0 = row1; row1 = tmp;
        path_len++;
    }

    /* 归一化距离: 除以路径长度 */
    float dist = row0[len - 1];
    if (path_len > 0) dist /= (float)path_len;
    return dist;
}

/* ================================================================
 *  DTW_Init
 * ================================================================ */
void DTW_Init(dtw_matcher_t *dtw) {
    memset(dtw, 0, sizeof(*dtw));
}

/* ================================================================
 *  DTW_Feed
 * ================================================================ */
float DTW_Feed(dtw_matcher_t *dtw, const float *curve, int length) {
    /* 重采样+归一化 */
    float buf[DTW_TMPL_LEN];
    resample(curve, length, buf, DTW_TMPL_LEN);
    normalize_peak(buf, DTW_TMPL_LEN);

    /* 模板未建立: 存储原始曲线 */
    if (dtw->tmpl_built < 3) {
        int idx = dtw->raw_count;
        if (idx < 3) {
            memcpy(dtw->raw_buf[idx], buf, sizeof(buf));
            dtw->raw_count++;
        }

        /* 收集满3条 → 建模板 (逐点平均) */
        if (dtw->raw_count >= 3) {
            for (int i = 0; i < DTW_TMPL_LEN; i++) {
                dtw->tmpl[i] = (dtw->raw_buf[0][i] +
                                dtw->raw_buf[1][i] +
                                dtw->raw_buf[2][i]) / 3.0f;
            }
            normalize_peak(dtw->tmpl, DTW_TMPL_LEN);
            dtw->tmpl_built = 1;
        }
        return -1.0f;  /* 模板未就绪 */
    }

    /* 模板已就绪: 计算 DTW 距离 */
    float dist = dtw_distance(dtw->tmpl, buf, DTW_TMPL_LEN,
                              DTW_BANDWIDTH, dtw->row0, dtw->row1);
    dtw->best_distance = dist;
    return dist;
}

/* ================================================================
 *  查询接口
 * ================================================================ */
bool DTW_TemplateReady(const dtw_matcher_t *dtw) {
    return dtw->tmpl_built != 0;
}

float DTW_GetLastDistance(const dtw_matcher_t *dtw) {
    return dtw->best_distance;
}

bool DTW_IsMatch(const dtw_matcher_t *dtw, float threshold) {
    if (!dtw->tmpl_built) return true;
    return dtw->best_distance <= threshold;
}

/* ================================================================
 *  四元数 DTW (测地距离 + Sakoe-Chiba 带 + 两行滚动)
 *
 *  距离度量: d(qa,qb) = 2*acos(|qa·qb|)
 *    四元数表示同一旋转有两种符号 (q 和 -q)，取绝对值消除歧义
 *
 *  评分: score = 100 * exp(-dist / sigma)   sigma=0.5 (经验值)
 * ================================================================ */

static float quat_geodesic(const float *qa, const float *qb) {
    float dot = qa[0]*qb[0] + qa[1]*qb[1] + qa[2]*qb[2] + qa[3]*qb[3];
    if (dot >  1.0f) dot =  1.0f;
    if (dot < -1.0f) dot = -1.0f;
    return 2.0f * acosf(fabsf(dot));  /* 取绝对值消除符号歧义 */
}

static float quat_dtw_distance(const float (*tmpl)[4], int tlen,
                               const float (*input)[4], int ilen,
                               int bandwidth,
                               float *row0, float *row1)
{
    /* 初始化第一行 */
    row0[0] = quat_geodesic(tmpl[0], input[0]);
    for (int j = 1; j < ilen && j <= bandwidth; j++)
        row0[j] = row0[j-1] + quat_geodesic(tmpl[0], input[j]);

    for (int i = 1; i < tlen; i++) {
        int j_start = (i - bandwidth > 0) ? i - bandwidth : 0;
        int j_end   = (i + bandwidth < ilen) ? i + bandwidth : ilen - 1;

        for (int j = j_start; j <= j_end; j++) {
            float cost = quat_geodesic(tmpl[i], input[j]);
            float best;

            if (j == j_start) {
                best = row0[j];  /* 只能从上方来 */
            } else {
                float up   = row0[j];
                float left = row1[j-1];
                float diag = row0[j-1];
                best = up;
                if (left < best) best = left;
                if (diag < best) best = diag;
            }
            row1[j] = cost + best;
        }
        /* 交换行 */
        float *tmp = row0; row0 = row1; row1 = tmp;
    }

    float dist = row0[ilen - 1];
    int path_len = tlen + ilen;  /* 近似路径长度 */
    if (path_len > 0) dist /= (float)path_len;
    return dist;
}

/* ── 重采样四元数序列到 QUAT_DTW_LEN ── */
static void quat_resample(const float (*src)[4], int src_len,
                          float (*dst)[4], int dst_len)
{
    if (src_len < 2) {
        float q0=src[0][0], q1=src[0][1], q2=src[0][2], q3=src[0][3];
        for (int i = 0; i < dst_len; i++) {
            dst[i][0]=q0; dst[i][1]=q1; dst[i][2]=q2; dst[i][3]=q3;
        }
        return;
    }
    for (int i = 0; i < dst_len; i++) {
        float pos = (float)i / (float)(dst_len - 1) * (float)(src_len - 1);
        int   idx = (int)pos;
        float frac = pos - (float)idx;
        if (idx >= src_len - 1) {
            dst[i][0]=src[src_len-1][0]; dst[i][1]=src[src_len-1][1];
            dst[i][2]=src[src_len-1][2]; dst[i][3]=src[src_len-1][3];
        } else {
            /* SLERP 近似: 线性插值 + 归一化 */
            float a = 1.0f - frac, b = frac;
            float q0 = a*src[idx][0] + b*src[idx+1][0];
            float q1 = a*src[idx][1] + b*src[idx+1][1];
            float q2 = a*src[idx][2] + b*src[idx+1][2];
            float q3 = a*src[idx][3] + b*src[idx+1][3];
            float m = sqrtf(q0*q0+q1*q1+q2*q2+q3*q3);
            if (m > 1e-9f) { dst[i][0]=q0/m; dst[i][1]=q1/m;
                             dst[i][2]=q2/m; dst[i][3]=q3/m; }
            else { dst[i][0]=src[idx][0]; dst[i][1]=src[idx][1];
                   dst[i][2]=src[idx][2]; dst[i][3]=src[idx][3]; }
        }
    }
}

#define QUAT_RAW_MAX  5

void QuatDTW_Init(quat_dtw_t *qdtw) {
    memset(qdtw, 0, sizeof(*qdtw));
}

float QuatDTW_Feed(quat_dtw_t *qdtw, const float (*quat)[4], int len) {
    if (len < 3) return -1.0f;

    /* 重采样到固定长度 */
    float buf[QUAT_DTW_LEN][4];
    quat_resample(quat, len, buf, QUAT_DTW_LEN);

    /* 模板未建立: 暂存, 第一拍直接作为模板 */
    if (!qdtw->tmpl_built) {
        memcpy(qdtw->tmpl, buf, sizeof(buf));
        qdtw->tmpl_len = QUAT_DTW_LEN;
        qdtw->tmpl_built = 1;
        qdtw->raw_count = 1;
        return -1.0f;
    }

    /* 模板已建立: 计算 DTW 距离 */
    float dist = quat_dtw_distance(qdtw->tmpl, qdtw->tmpl_len,
                                   buf, QUAT_DTW_LEN,
                                   QUAT_BANDWIDTH, qdtw->row0, qdtw->row1);
    qdtw->best_distance = dist;
    /* 评分: exp 映射, sigma=0.5 → dist≈0→100, dist≈0.5→37, dist≈1.0→14 */
    qdtw->score = 100.0f * expf(-dist / 0.5f);
    return dist;
}

bool QuatDTW_Ready(const quat_dtw_t *qdtw) {
    return qdtw->tmpl_built != 0;
}

float QuatDTW_GetScore(const quat_dtw_t *qdtw) {
    return qdtw->score;
}
