/**
 * @file    training_session.c
 * @brief   训练会话管理 v2.0
 *
 * 新增:
 *   - 前3拍建立个人基线模板
 *   - 一致性追踪 (CV统计, 发球用, 暂保留)
 *   - 多动作类型支持
 */

#include "training_session.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ── 全局状态(UI直接读取) ── */
train_state_t  g_train_state       = TRAIN_IDLE;
uint8_t        g_train_count       = 0;
uint8_t        g_train_target      = 20;
float          g_train_last_peak   = 0.0f;
int            g_train_last_score  = 0;
float          g_train_heart_rate  = 0.0f;
uint8_t        g_train_template_built = 0;
SwingResult_t  g_swing_result;
bds_result_t   g_last_full_result;

/* ── 内部 ── */
static bds_detector_t detector;
static int      total_scores;
static float    score_sum;
static float    peak_max;
static int      error_count;
static char g_last_detailed_advice[300] = "";

/* 疲劳检测 + 模板质检 */
#define FATIGUE_WIN  10
static float    speed_hist[FATIGUE_WIN*2];
static float    release_hist[FATIGUE_WIN*2];
static int      speed_hist_idx;
static float    user_scores[50];
static int      user_score_cnt;
static float    user_p70;

/* DTW 模板匹配 */
static dtw_matcher_t dtw;
static quat_dtw_t    qdtw;          /* 四元数姿态 DTW */
float   g_train_dtw_dist    = 0.0f;
uint8_t g_train_dtw_ready   = 0;
float   g_train_quat_score  = 0.0f;

/* 一致性追踪 (发球用) */
#define MAX_TEMPLATE_SAMPLES  20
static float template_whip[MAX_TEMPLATE_SAMPLES];
static float template_peak[MAX_TEMPLATE_SAMPLES];
static float template_dur[MAX_TEMPLATE_SAMPLES];
static float template_launch[MAX_TEMPLATE_SAMPLES];
static int   template_count;

/* ── 个人模板: 前几拍取平均建立基线 ── */
#define TEMPLATE_BUILD_COUNT  3

static float baseline_whip;
static float baseline_peak;
static float baseline_dur;
static float baseline_launch;

void Training_Start(int type, uint8_t target, skill_level_t lvl) {
    BDS_Init(&detector, (stroke_type_t)type, lvl);
    DTW_Init(&dtw);
    QuatDTW_Init(&qdtw);  /* 每组训练重建四元数DTW模板 */
    g_train_state          = TRAIN_RUNNING;
    g_train_count          = 0;
    g_train_target         = target;
    g_train_last_peak      = 0.0f;
    g_train_last_score     = 0;
    g_train_template_built = 0;
    g_train_dtw_dist       = 0.0f;
    g_train_dtw_ready      = 0;
    total_scores           = 0;
    score_sum              = 0.0f;
    peak_max               = 0.0f;
    error_count            = 0;
    template_count         = 0;
    baseline_whip          = 0.0f;
    baseline_peak          = 0.0f;
    baseline_dur           = 0.0f;
    baseline_launch        = 0.0f;
}

void Training_Update(mpu6050_data_t *d) {
    if (g_train_state != TRAIN_RUNNING) return;

    BDS_FeedData(&detector, d);

    if (BDS_HasResult(&detector)) {
        bds_result_t r = BDS_GetResult(&detector);

        /* ── 挑球三证据计数 ── */
        int do_count = 1;  /* 默认计数 */
        int is_impact = 1; /* 默认有击球 */

        // if (detector.stroke == STROKE_LIFT) {
        //     /* 证据A: 挥拍轮廓 — BDS_HasResult 已确认 */
        //     /* 证据B: 击球冲击 — 双带通能量比 */
        //     is_impact = BDS_IsRacketImpact(&detector);
        //     /* 证据C: DTW门控 — 下面计算 */

        //     /* 不应期检查: 600ms内不再触发 */
        //     uint32_t now = detector.elapsed_ms;
        //     if (now - detector.last_count_time < 600) {
        //         do_count = 0;
        //     }
        // }

        /* ── DTW 模板匹配 (标量角速度 + 四元数姿态) ── */
        {
            float gyro_curve[200];
            int clen = BDS_ExtractGyroCurve(&detector, gyro_curve, 200);
            float dtw_dist = DTW_Feed(&dtw, gyro_curve, clen);
            g_train_dtw_dist = (dtw_dist >= 0.0f) ? dtw_dist : 0.0f;
            g_train_dtw_ready = DTW_TemplateReady(&dtw) ? 1 : 0;

            float quat_buf[QUAT_DTW_LEN][4];
            int qlen = BDS_ExtractQuatCurve(&detector, quat_buf, QUAT_DTW_LEN);
            QuatDTW_Feed(&qdtw, quat_buf, qlen);
            g_train_quat_score = QuatDTW_GetScore(&qdtw);

            /* 证据C 门控 */
            int dtw_ok = (!g_train_dtw_ready) || (dtw_dist <= 0.30f);
            int quat_ok = (!QuatDTW_Ready(&qdtw)) || (g_train_quat_score > 30.0f);

            /* 挑球三证据: A(轮廓OK) + B(冲击?) + C(DTW OK) */
            if (detector.stroke == STROKE_LIFT) {
                if (!is_impact && dtw_ok) {
                    /* A+C,无B: 空挥, 计数但不评分 */
                    r.total_score = 0;
                    strcpy(r.suggestion, "空挥(未击中球)");
                } else if (!dtw_ok && !quat_ok) {
                    /* DTW门控失败: 不计数 */
                    do_count = 0;
                }
                /* B无A: DONE状态本身确保A已通过, 不会出现 */
            } else {
                /* 非挑球: 沿用原门控 */
                if (g_train_dtw_ready && dtw_dist > 0.25f) {
                    r.total_score = r.total_score * 7 / 10;
                    strcat(r.suggestion, "|动作与模板差异大,检查姿势");
                }
            }

            if (QuatDTW_Ready(&qdtw) && g_train_quat_score > 0.0f) {
                r.vec.attitude_score = g_train_quat_score;
            }
        }

        if (do_count) {
            g_train_last_peak  = r.peak_accel;
            g_train_last_score = r.total_score;
            strncpy(g_last_detailed_advice, r.suggestion, sizeof(g_last_detailed_advice) - 1);
            g_last_full_result = r;  /* 保存完整结果供串口输出 */
            g_train_count++;
            detector.last_count_time = detector.elapsed_ms;

            /* ── 填充 SwingResult_t (合同1: 供 TouchGFX 画波形) ── */
            g_swing_result.swing_index = g_train_count;
            g_swing_result.peak_accel  = r.peak_accel;
            g_swing_result.score       = r.total_score;
            {
                float gyro_buf[SWING_WAVEFORM_MAX];
                int wlen = BDS_ExtractGyroCurve(&detector, gyro_buf, SWING_WAVEFORM_MAX);
                g_swing_result.waveform_len = (uint16_t)(wlen < SWING_WAVEFORM_MAX ? wlen : SWING_WAVEFORM_MAX);
                for (int wi = 0; wi < (int)g_swing_result.waveform_len; wi++)
                    g_swing_result.waveform[wi] = gyro_buf[wi];
            }

            total_scores++;
            score_sum += (float)r.total_score;
            if (r.total_score < 50 && r.total_score > 0) error_count++;
            if (r.peak_accel > peak_max) peak_max = r.peak_accel;

            /* ── 疲劳检测: 近10拍 vs 前10拍 ── */
            if (speed_hist_idx < FATIGUE_WIN*2) {
                speed_hist[speed_hist_idx]   = r.peak_gyro;
                release_hist[speed_hist_idx] = (float)r.duration_ms;
                speed_hist_idx++;
            } else {
                /* 滑动: 移位 */
                for (int fi = 0; fi < FATIGUE_WIN*2-1; fi++) {
                    speed_hist[fi]   = speed_hist[fi+1];
                    release_hist[fi] = release_hist[fi+1];
                }
                speed_hist[FATIGUE_WIN*2-1]   = r.peak_gyro;
                release_hist[FATIGUE_WIN*2-1] = (float)r.duration_ms;

                if (speed_hist_idx >= FATIGUE_WIN*2) {
                    float early_spd=0, late_spd=0;
                    float early_rel=0, late_rel=0;
                    for (int fi = 0; fi < FATIGUE_WIN; fi++) {
                        early_spd += speed_hist[fi];
                        early_rel += release_hist[fi];
                        late_spd  += speed_hist[FATIGUE_WIN+fi];
                        late_rel  += release_hist[FATIGUE_WIN+fi];
                    }
                    early_spd /= FATIGUE_WIN; late_spd /= FATIGUE_WIN;
                    early_rel /= FATIGUE_WIN; late_rel /= FATIGUE_WIN;
                    float spd_drop = (early_spd > 0) ? (early_spd-late_spd)/early_spd : 0;
                    float rel_rise = (early_rel > 0) ? (late_rel-early_rel)/early_rel : 0;
                    if (spd_drop > 0.12f && rel_rise > 0.15f) {
                        strcat(r.suggestion, "|挥速下降,建议休息2分钟再练");
                    }
                }
            }

            /* ── 用户P70阈值更新 (模板质检用) ── */
            if (user_score_cnt < 50) {
                user_scores[user_score_cnt++] = (float)r.total_score;
            } else {
                /* 简单P70: 排序取70%分位 */
                float sorted[50];
                memcpy(sorted, user_scores, sizeof(sorted));
                for (int si=0;si<49;si++) for(int sj=si+1;sj<50;sj++)
                    if(sorted[si]>sorted[sj]){float t=sorted[si];sorted[si]=sorted[sj];sorted[sj]=t;}
                user_p70 = sorted[35]; /* 70%分位 */
                /* 移位 */
                for (int fi=0;fi<49;fi++) user_scores[fi]=user_scores[fi+1];
                user_scores[49] = (float)r.total_score;
            }

            /* 前 TEMPLATE_BUILD_COUNT 拍建立个人基线 (仅高分拍参与) */
            if (template_count < TEMPLATE_BUILD_COUNT
                && r.total_score >= (int)user_p70) {
                template_whip[template_count]   = r.whip_ratio;
                template_peak[template_count]   = r.peak_accel;
                template_dur[template_count]    = (float)r.duration_ms;
                template_launch[template_count] = r.launch_angle_deg;
                template_count++;
                if (template_count == TEMPLATE_BUILD_COUNT) {
                    for (int i = 0; i < TEMPLATE_BUILD_COUNT; i++) {
                        baseline_whip   += template_whip[i];
                        baseline_peak   += template_peak[i];
                        baseline_dur    += template_dur[i];
                        baseline_launch += template_launch[i];
                    }
                    baseline_whip   /= (float)TEMPLATE_BUILD_COUNT;
                    baseline_peak   /= (float)TEMPLATE_BUILD_COUNT;
                    baseline_dur    /= (float)TEMPLATE_BUILD_COUNT;
                    baseline_launch /= (float)TEMPLATE_BUILD_COUNT;
                    g_train_template_built = 1;
                }
            }
        }

        if (g_train_count >= g_train_target) {
            g_train_state = TRAIN_DONE;
        }
    }
}

TrainingDone_t Training_Stop(void) {
    TrainingDone_t r;
    memset(&r, 0, sizeof(r));
    r.type      = (int)detector.stroke;
    r.total     = g_train_count;
    r.max_peak  = peak_max;
    r.avg_score = total_scores > 0 ? (score_sum / (float)total_scores) : 0.0f;

    /* 稳定性: 误差次数占比 */
    r.stability = total_scores > 0
                  ? (uint8_t)(100 - error_count * 100 / total_scores) : 100;
    if (r.stability > 100) r.stability = 100;

    /* 一致性指数 (发球用) */
    if (template_count >= TEMPLATE_BUILD_COUNT) {
        float cv_sum = 0.0f;
        int   n_cv   = 0;
        for (int i = 0; i < template_count; i++) {
            float cv;
            if (baseline_peak > 0.0f) {
                cv = fabsf(template_peak[i] - baseline_peak) / baseline_peak;
                cv_sum += cv; n_cv++;
            }
            if (baseline_dur > 0.0f) {
                cv = fabsf(template_dur[i] - baseline_dur) / baseline_dur;
                cv_sum += cv; n_cv++;
            }
        }
        float ci = n_cv > 0 ? cv_sum / (float)n_cv : 0.0f;
        /* ci < 0.08 高度稳定, ci > 0.20 差异大 */
        r.stability = ci < 0.08f ? 95 : ci < 0.15f ? 75 : ci < 0.25f ? 50 : 25;
    }

    /* 显示最后一拍的详细建议 */
    strncpy(r.advice, g_last_detailed_advice, sizeof(r.advice) - 1);
    r.advice[sizeof(r.advice) - 1] = '\0';
    if (strlen(r.advice) == 0) {
    /* 兜底 */
    if (r.avg_score >= 85)
        snprintf(r.advice, sizeof(r.advice), "表现优秀,发力不错,记住这个挥拍鞭甩发力的感觉,保持节奏!");
    else if (r.avg_score >= 70)
        snprintf(r.advice, sizeof(r.advice), "良好,继续加油!");
    else if (r.avg_score >= 30)
        snprintf(r.advice, sizeof(r.advice), "力度偏弱,注意发力集中度");
    else
        snprintf(r.advice, sizeof(r.advice), "动作错误,多练习基本功,放慢节奏注重动作完整性");
}
    g_train_state = TRAIN_IDLE;
    return r;
}

void Training_SetAHRS(const madgwick_t *ahrs) {
    detector.ahrs = ahrs;
}

static int g_hand_pref = 0;
void Training_SetHandPreference(int pref) {
    g_hand_pref = pref;
    if (pref == 1) { detector.hand = HAND_FOREHAND; detector.hand_forced = 1; }
    else if (pref == 2) { detector.hand = HAND_BACKHAND; detector.hand_forced = 1; }
    else { detector.hand_forced = 0; }
}

uint8_t Training_GetTodayTotal(void) {
    /* TODO: Flash存储完成后实现 */
    return 0;
}
