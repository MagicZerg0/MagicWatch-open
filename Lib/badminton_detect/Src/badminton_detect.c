/**
 * @file    badminton_detect.c
 * @brief   羽毛球挥拍检测与评估实现
 *
 * 核心变化(v2.0):
 *   - 每动作独立触发阈值(验证式识别)
 *   - 鞭打比 bug 修复: gy_early_max 改为取最大值
 *   - 三层指标架构: L1共性 → L2发力结构 → L3专属判据
 *   - 平滑度/倒拍深度/出射仰角/随挥完整比/旋转反转比/紧凑度 等新指标
 */

#include "badminton_detect.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* ================================================================
 *  每动作独立检测配置 (验证式识别核心)
 * ================================================================ */
static const stroke_detect_cfg_t DETECT_CFG[STROKE_COUNT] = {
    /*                   触发A    回归A   最小ms  击球前窗 击球后窗 */
    [STROKE_CLEAR] = {   40.0f,  15.0f,  120,     400,     300 },
    [STROKE_SMASH] = {   45.0f,  18.0f,   50,     300,     200 },
    [STROKE_LIFT]  = {   30.0f,  12.0f,   55,     250,     150 },
};

/* 每动作评分权重 */
static const stroke_weight_cfg_t WEIGHT_CFG[STROKE_COUNT] = {
    /*                   力度   速度   弧线   鞭打   内旋   仰角   平滑  专项 */
    [STROKE_CLEAR] = {  0.50f, 0.08f, 0.08f, 0.10f, 0.06f, 0.06f, 0.05f, 0.07f },
    [STROKE_SMASH] = {  0.60f, 0.08f, 0.05f, 0.10f, 0.05f, 0.05f, 0.05f, 0.02f },
    [STROKE_LIFT]  = {  0.80f, 0.02f, 0.02f, 0.00f, 0.00f, 0.03f, 0.03f, 0.10f },
};

/* ================================================================
 *  L3 理想参数矩阵 IDEAL[stroke][skill]
 *  结构: {g_lo,g_hi, a_lo,a_hi, dur_lo,dur_hi,
 *          whip_lo,whip_hi, rot_lo,rot_hi,
 *          launch_lo,launch_hi, drop_lo,drop_hi,
 *          compact_lo,compact_hi, tempo_lo,tempo_hi}
 * ================================================================ */
static const ideal_params_t IDEAL[STROKE_COUNT] = {
    /* ── 高远球 ── */
    [STROKE_CLEAR] = { 1100,2100, 48,120, 170,330,  0.85f,4.2f, 0.55f,0.95f, 18,35, 55,85,   0,0, 0,0 },
    /* ── 杀球 ── */
    [STROKE_SMASH] = { 1750,2450, 90,185, 160,225,  1.4f,5.0f, 0.65f,0.95f, -20,-8, 50,85,  0,0, 0,0 },
    /* ── 挑球 ── */
    [STROKE_LIFT]  = {  500,1700, 65,90,   45,170,  0,0, 0,0, 50,68, 0,0, 0.15f,0.40f, 0,0 },
};

/* ================================================================
 *  模板匹配评分体系 (高远球 & 挑球)
 *  以自录标准动作为模板, 评包络形状相似度 → 稳定复现 + 强区分
 * ================================================================ */
#define ENV_N  32          /* 包络重采样点数 */

typedef struct {
    float env[ENV_N];      /* 归一化包络模板 (峰值=1.0) */
    float gm_peak_ref;     /* 标准动作 gm 峰值参考 (°/s) */
    float pre_ms, post_ms; /* 峰锚定窗口 (ms) */
} stroke_template_t;

/* 初始模板: 从用户CSV标准数据近似合成, 后续可现场录制替换 */
static stroke_template_t TPL[STROKE_COUNT] = {
    [STROKE_CLEAR] = {
        .pre_ms = 150, .post_ms = 300,
        .gm_peak_ref = 1650.0f,
        .env = { 0.15f,0.22f,0.30f,0.40f,0.52f,0.65f,0.77f,0.88f,
                 0.95f,1.00f,0.97f,0.91f,0.84f,0.76f,0.68f,0.60f,
                 0.53f,0.47f,0.41f,0.36f,0.31f,0.27f,0.23f,0.20f,
                 0.17f,0.14f,0.12f,0.10f,0.09f,0.08f,0.07f,0.06f }
    },
    [STROKE_LIFT] = {
        .pre_ms = 120, .post_ms = 120,
        .gm_peak_ref = 1200.0f,
        .env = { 0.04f,0.07f,0.12f,0.20f,0.32f,0.48f,0.66f,0.84f,
                 0.95f,1.00f,0.93f,0.78f,0.58f,0.38f,0.23f,0.14f,
                 0.09f,0.05f,0.03f,0.02f,0.01f,0.01f,0.01f,0.00f,
                 0.00f,0.00f,0.00f,0.00f,0.00f,0.00f,0.00f,0.00f }
    },
};

/* ================================================================
 *  挑球双阈值表 (正手/反手独立)
 * ================================================================ */
static const lift_thresholds_t LIFT_TH[2] = {
    [HAND_FOREHAND] = { 250.0f, 0.30f, 2.0f, 50.0f, 70.0f, 0.50f },
    [HAND_BACKHAND] = { 300.0f, 0.35f, 1.6f, 50.0f, 70.0f, 0.65f },
};

const lift_thresholds_t* BDS_GetLiftThresholds(hand_t hand) {
    return (hand <= HAND_BACKHAND) ? &LIFT_TH[hand] : &LIFT_TH[0];
}

/* ================================================================
 *  正反手判别器 (三特征符号投票)
 *
 *  右手佩戴 + 右手持拍前提下:
 *    特征1: ωx 击球前符号 — 正手内旋(+), 反手外旋(-)
 *    特征2: 击球时刻 roll  — 反手掌心方向差90°+
 *    特征3: 挥拍平面法向量 Y分量 — 正反手镜像面相反
 *
 *  投票: s1+s2+s3 >= 1 → 正手, 否则反手
 * ================================================================ */
hand_t BDS_DiscriminateHand(const bds_detector_t *det) {
    int s1 = 0, s2 = 0, s3 = 0;

    /* 特征1: ωx 击球前符号 (最强特征) — 从 gy_x_peak 和 wx_reversed 推断 */
    /* 正手挑球: 击球段 ωx 正向爆发(内旋); 反手: ωx 负向(外旋) */
    if (det->wx_burst_max > 30.0f && det->wx_burst_max > fabsf(det->wx_pre_min))
        s1 = +1;   /* 正手: ωx正向占优 */
    else if (det->wx_pre_min < -30.0f && fabsf(det->wx_pre_min) > det->wx_burst_max)
        s1 = -1;   /* 反手: ωx负向占优 */
    else
        s1 = (det->wx_burst_max > 0.0f) ? +1 : -1;  /* 弱信号: 取符号 */

    /* 特征2: 击球时刻 roll — Madgwick roll, 反手掌心翻转 */
    if (det->ahrs) {
        float roll = MadgwickGetRoll(det->ahrs);
        /* 正手挑球击球时前臂自然位 roll≈-20~+30°; 反手击球掌心内转 roll<-40°或>+60° */
        s2 = (roll > -40.0f && roll < 50.0f) ? +1 : -1;
    }

    /* 特征3: 挥拍平面法向量 Y分量符号 (镜像面相反) */
    float ny = det->result.vec.plane_normal[1];
    s3 = (ny >= 0.0f) ? +1 : -1;

    int vote = s1 + s2 + s3;
    return (vote >= 1) ? HAND_FOREHAND : HAND_BACKHAND;
}

/* ================================================================
 *  挑球冲击判据: 双带通能量比区分击球 vs 跨步
 *
 *  简化为MCU友好版: 用一阶差分近似高频, 用滑动均值近似低频
 *  高频能量 ≈ am 帧间差分方差 (30ms窗)
 *  低频能量 ≈ am 滑动均值方差
 *  判据: E_high/E_low > 2.0 && gm > 5.24 rad/s (300°/s)
 * ================================================================ */
int BDS_IsRacketImpact(const bds_detector_t *det) {
    /* 简化版: 用已存储的 acc_hp_energy / acc_lp_energy */
    float ratio = (det->acc_lp_energy > 0.01f)
                  ? det->acc_hp_energy / det->acc_lp_energy : 0.0f;
    return (ratio > 2.0f) ? 1 : 0;
}

/* ================================================================
 *  辅助函数
 * ================================================================ */
static inline float vec3_mag(float x, float y, float z) {
    return sqrtf(x*x + y*y + z*z);
}

static inline float vec3_dot(const float *a, const float *b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static inline void vec3_cross(const float *a, const float *b, float *c) {
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}

static inline void vec3_normalize(float *v) {
    float m = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (m > 1e-9f) { v[0]/=m; v[1]/=m; v[2]/=m; }
}

/* ── 3×3 对称矩阵特征分解 (解析法) ──
 * 输入: M = [[a,b,c],[b,d,e],[c,e,f]]
 * 输出: eigenvals[3] (升序), eigenvecs[3][3] (每行一个特征向量)
 */
static void sym_eigen3(float a, float b, float c,
                       float d, float e, float f,
                       float *evals, float (*evecs)[3])
{
    /* 特征多项式系数: λ³ - p·λ² + q·λ - r = 0 */
    float p = a + d + f;
    float q = a*d + a*f + d*f - b*b - c*c - e*e;
    float r = a*d*f + 2.0f*b*c*e - a*e*e - d*c*c - f*b*b;

    /* 化为 depressed cubic: t³ + pt + q = 0, t = λ - p/3 */
    float p3 = p / 3.0f;
    float pp = q - p3*p3*3.0f;       /* p of depressed */
    float qq = 2.0f*p3*p3*p3 - p3*q + r;  /* -q of depressed (flip sign) */

    /* 三个实根的三角解法 */
    float phi;
    float R = sqrtf((pp < 0.0f) ? -pp/3.0f : 0.0f);
    float Q = (R > 1e-12f) ? qq / (2.0f * R*R*R) : 0.0f;
    if      (Q >  1.0f) Q =  1.0f;
    else if (Q < -1.0f) Q = -1.0f;
    phi = acosf(Q) / 3.0f;
    float pi23 = 2.0943951f; /* 2π/3 */

    float t0 = 2.0f * R * cosf(phi);
    float t1 = 2.0f * R * cosf(phi + pi23);
    float t2 = 2.0f * R * cosf(phi - pi23);

    evals[0] = t0 + p3;
    evals[1] = t1 + p3;
    evals[2] = t2 + p3;

    /* 排序 → 升序 (简单冒泡) */
    for (int i = 0; i < 2; i++) {
        for (int j = i+1; j < 3; j++) {
            if (evals[i] > evals[j]) {
                float tmp = evals[i]; evals[i] = evals[j]; evals[j] = tmp;
            }
        }
    }

    /* 每个特征值对应的特征向量: (M-λI)奇异 → 取两行叉积得零空间 */
    for (int k = 0; k < 3; k++) {
        float lam = evals[k];
        float r0[3] = {a-lam, b,     c};
        float r1[3] = {b,     d-lam, e};
        float r2[3] = {c,     e,     f-lam};

        /* 叉积 r0×r1 */
        float v[3];
        v[0] = r0[1]*r1[2] - r0[2]*r1[1];
        v[1] = r0[2]*r1[0] - r0[0]*r1[2];
        v[2] = r0[0]*r1[1] - r0[1]*r1[0];
        float m = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        if (m < 1e-9f) {
            /* r0×r1 退化 → 试 r0×r2 */
            v[0] = r0[1]*r2[2] - r0[2]*r2[1];
            v[1] = r0[2]*r2[0] - r0[0]*r2[2];
            v[2] = r0[0]*r2[1] - r0[1]*r2[0];
            m = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        }
        if (m > 1e-9f) { evecs[k][0]=v[0]/m; evecs[k][1]=v[1]/m; evecs[k][2]=v[2]/m; }
        else          { evecs[k][0]=1.0f; evecs[k][1]=0.0f; evecs[k][2]=0.0f; }
    }
}

/**
 * @brief 软评分: smoothstep 映射, 消除悬崖效应
 *        x_good=完美, x_bad=不可接受, 中间平滑过渡
 */
static float soft_score(float x, float x_good, float x_bad) {
    if (x_good >= x_bad) {
        /* 越大越好: 如爆发比 */
        if (x >= x_good) return 100.0f;
        if (x <= x_bad) return 10.0f;
        float t = (x_good - x) / (x_good - x_bad);  /* 0~1, 越大越差 */
        return 100.0f - 90.0f * t * t * (3.0f - 2.0f*t); /* smoothstep 10~100 */
    } else {
        /* 越小越好: 如出手时长 */
        if (x <= x_good) return 100.0f;
        if (x >= x_bad) return 10.0f;
        float t = (x - x_good) / (x_bad - x_good);
        return 100.0f - 90.0f * t * t * (3.0f - 2.0f*t);
    }
}

/* ── 环形缓冲提取 gm 包络 (以 gm 峰值时刻为锚, 按时间戳重采样) ── */
static int extract_envelope(ring_buffer_t *rb, uint32_t t_peak,
                            float pre_ms, float post_ms,
                            float *env_out, float *gm_peak_out)
{
    float raw[160]; uint32_t ts[160]; int n = 0;
    uint32_t t0 = t_peak - (uint32_t)pre_ms;
    uint32_t t1 = t_peak + (uint32_t)post_ms;

    for (int k = 0; k < rb->count && n < 160; k++) {
        int idx = (rb->head - 1 - k + RBUF_SIZE) % RBUF_SIZE;
        if (rb->ts_ms[idx] < t0) break;
        if (rb->ts_ms[idx] > t1) continue;
        raw[n] = sqrtf(rb->gx[idx]*rb->gx[idx]
                     + rb->gy[idx]*rb->gy[idx]
                     + rb->gz[idx]*rb->gz[idx]);
        ts[n]  = rb->ts_ms[idx];
        n++;
    }
    if (n < ENV_N/2) return -1;
    for (int i = 0; i < n/2; i++) {
        float tf = raw[i]; raw[i] = raw[n-1-i]; raw[n-1-i] = tf;
        uint32_t tt = ts[i]; ts[i] = ts[n-1-i]; ts[n-1-i] = tt;
    }
    float peak = 1e-6f;
    float span = (float)(ts[n-1] - ts[0]);
    for (int i = 0; i < ENV_N; i++) {
        float t_want = ts[0] + span * i / (ENV_N - 1);
        int j = 0;
        while (j < n-2 && ts[j+1] < t_want) j++;
        float dt = (float)(ts[j+1] - ts[j]);
        float a  = (dt > 0.5f) ? (t_want - ts[j]) / dt : 0.0f;
        env_out[i] = raw[j] + a * (raw[j+1] - raw[j]);
        if (env_out[i] > peak) peak = env_out[i];
    }
    for (int i = 0; i < ENV_N; i++) env_out[i] /= peak;
    *gm_peak_out = peak;
    return 0;
}

/* ── Pearson 相关系数 ── */
static float pearson(const float *a, const float *b, int n) {
    float ma=0, mb=0;
    for (int i=0;i<n;i++){ ma+=a[i]; mb+=b[i]; }
    ma/=n; mb/=n;
    float sab=0, saa=0, sbb=0;
    for (int i=0;i<n;i++){
        float da=a[i]-ma, db=b[i]-mb;
        sab+=da*db; saa+=da*da; sbb+=db*db;
    }
    float d = sqrtf(saa*sbb);
    return (d>1e-9f)? sab/d : 0.0f;
}

/* ── 结构分: 高远球峰后单调衰减占比 ── */
static float clear_decay_score(const float *env) {
    int i_pk = 0;
    for (int i=1;i<ENV_N;i++) if (env[i]>env[i_pk]) i_pk=i;
    int falling = 0, total = 0;
    for (int i=i_pk+1;i<ENV_N;i++,total++)
        if (env[i] <= env[i-1] + 0.03f) falling++;
    float mono = (total>0)? (float)falling/total : 0.f;
    return soft_score(mono, 0.85f, 0.45f);
}

/* ── 结构分: 挑球单尖峰性 ── */
static float lift_singlepeak_score(const float *env) {
    int i_pk = 0;
    for (int i=1;i<ENV_N;i++) if (env[i]>env[i_pk]) i_pk=i;
    float second = 0.0f;
    for (int i=1;i<ENV_N-1;i++) {
        if (i>=i_pk-3 && i<=i_pk+3) continue;
        if (env[i]>env[i-1] && env[i]>env[i+1] && env[i]>second)
            second = env[i];
    }
    return soft_score(second, 0.45f, 0.80f);
}

/* ── 模板匹配评分: 高远球/挑球用 ── */
static int score_by_template(bds_detector_t *det) {
    stroke_template_t *tpl = &TPL[det->stroke];
    float env[ENV_N], gm_peak;
    if (extract_envelope(&det->rbuf, det->t_gm_peak_ms,
                         tpl->pre_ms, tpl->post_ms, env, &gm_peak) < 0)
        return 40;

    float r  = pearson(env, tpl->env, ENV_N);
    float s1 = soft_score(r, 0.92f, 0.55f);
    float ratio = gm_peak / tpl->gm_peak_ref;
    float s2;
    if (ratio >= 0.80f && ratio <= 1.35f) s2 = 100.0f;
    else if (ratio > 1.35f)               s2 = 95.0f;
    else s2 = soft_score(ratio, 0.80f, 0.45f);
    float s3 = (det->stroke == STROKE_CLEAR)
               ? clear_decay_score(env) : lift_singlepeak_score(env);

    float w[3] = {0.5f, 0.3f, 0.2f}, sc[3] = {s1, s2, s3};
    float acc = 0;
    for (int i=0;i<3;i++) {
        float s = sc[i]; if (s < 5.0f) s = 5.0f;
        acc += w[i] * logf(s / 100.0f);
    }
    int total = (int)(100.0f * expf(acc));

    if (r < 0.60f)                          total = (total < 45) ? total : 45;
    if (gm_peak < 0.50f * tpl->gm_peak_ref) total = (total < 50) ? total : 50;

    det->result.shape_r = r;
    return total;
}

/** @brief 旧版硬阈值保留作回退 */
static int score_item_hard(float v, float lo, float hi) {
    if (v >= lo && v <= hi) return 100;
    if (v < lo) { int s = (int)(v / lo * 100.0f); return s > 0 ? s : 0; }
    int s = (int)(hi / v * 100.0f); return s > 0 ? s : 0;
}

/** @brief 单向评分: 理想区间内100, 超过上限满分, 低于下限soft过渡 */
static int score_item_soft(float v, float lo, float hi) {
    if (v >= lo) return 100;
    float half = (hi - lo) * 0.25f;
    if (half < 1.0f) half = 1.0f;
    return (int)soft_score(v, lo - half, lo - half*2.0f);
}

/** @brief 双向评分: 理想区间内100, 两侧都扣分 (给高远球dur这种不能太快不能太慢的) */
static int score_item_bilateral(float v, float lo, float hi) {
    if (v >= lo && v <= hi) return 100;
    float half = (hi - lo) * 0.25f;
    if (half < 1.0f) half = 1.0f;
    if (v < lo) return (int)soft_score(v, lo - half, lo - half*2.0f);
    return (int)soft_score(v, hi + half, hi + half*2.0f);
}

/* ================================================================
 *  环形缓冲区
 * ================================================================ */
static void rbuf_push(ring_buffer_t *rb,
                      float ax, float ay, float az,
                      float gx, float gy, float gz,
                      uint32_t ts)
{
    int i = rb->head;
    rb->ax[i] = ax; rb->ay[i] = ay; rb->az[i] = az;
    rb->gx[i] = gx; rb->gy[i] = gy; rb->gz[i] = gz;
    rb->ts_ms[i] = ts;
    rb->head = (i + 1) % RBUF_SIZE;
    if (rb->count < RBUF_SIZE) rb->count++;
}

/* 读环形缓冲区中相对时间偏移 [t_start_ms, t_end_ms] 范围内的数据 */
static int rbuf_slice(ring_buffer_t *rb, uint32_t now_ms,
                      int t_start_offset, int t_end_offset,
                      float *gx_out, float *gy_out, float *gz_out,
                      int max_samples)
{
    int n = 0;
    uint32_t t_start = (uint32_t)((int)now_ms + t_start_offset);
    uint32_t t_end   = (uint32_t)((int)now_ms + t_end_offset);
    for (int k = 0; k < rb->count && n < max_samples; k++) {
        int idx = (rb->head - 1 - k + RBUF_SIZE) % RBUF_SIZE;
        if (rb->ts_ms[idx] >= t_start && rb->ts_ms[idx] <= t_end) {
            if (gx_out) gx_out[n] = rb->gx[idx];
            if (gy_out) gy_out[n] = rb->gy[idx];
            if (gz_out) gz_out[n] = rb->gz[idx];
            n++;
        }
    }
    return n;
}

/* ================================================================
 *  简单峰检测 (平滑度分析)
 *  在窗口中找 prominence > 0.25*peak 的峰个数
 * ================================================================ */
static int count_peaks(float *data, int n, float peak_val) {
    if (n < 3) return 1;
    float threshold = peak_val * 0.50f;  /* 低于主峰50%的波动不算独立峰 */
    int npeaks = 0;
    for (int i = 1; i < n - 1; i++) {
        if (data[i] > data[i-1] && data[i] > data[i+1] && data[i] > threshold) {
            npeaks++;
        }
    }
    return npeaks > 0 ? npeaks : 1;
}

/* ================================================================
 *  姿态获取 (Madgwick → 真实世界系姿态)
 *  若 ahrs 未设置则返回 0
 * ================================================================ */
static float get_pitch_deg(const bds_detector_t *det) {
    if (det->ahrs) return MadgwickGetPitch(det->ahrs);
    return 0.0f;
}

/* ================================================================
 *  BDS_Init — 初始化检测器
 * ================================================================ */
void BDS_Init(bds_detector_t *det, stroke_type_t stroke, skill_level_t lvl) {
    /* 保留 ahrs 指针 (由外部在 Training_Start 前设置) */
    const madgwick_t *saved_ahrs = det->ahrs;
    memset(det, 0, sizeof(*det));
    det->ahrs   = saved_ahrs;
    det->state  = BDS_IDLE;
    det->stroke = stroke;
    det->skill  = lvl;  /* 保留兼容, IDEAL 表已统一不再区分 */
    det->family = BDS_FamilyOf(stroke);
}

/* ================================================================
 *  BDS_FeedData — 每5ms喂一帧数据
 * ================================================================ */
void BDS_FeedData(bds_detector_t *det, const mpu6050_data_t *d) {
    float am = vec3_mag(d->accel_x, d->accel_y, d->accel_z);
    float gm = vec3_mag(d->gyro_x, d->gyro_y, d->gyro_z);

    /* 重复帧防御: 传感器未更新时丢弃 (DATA_READY 未触发导致) */
    {
        static mpu6050_data_t s_prev;
        if (memcmp(&s_prev, d, sizeof(*d)) == 0) return;
        s_prev = *d;
    }

    det->elapsed_ms += 5;

    /* 环形缓冲记录 */
    rbuf_push(&det->rbuf, d->accel_x, d->accel_y, d->accel_z,
              d->gyro_x, d->gyro_y, d->gyro_z, det->elapsed_ms);

    const stroke_detect_cfg_t *cfg = &DETECT_CFG[det->stroke];
    float trigger = cfg->trigger_a;
    float idle    = cfg->idle_a;
    uint32_t min_dur = cfg->min_dur_ms;

    switch (det->state) {

    case BDS_IDLE: {
        if (am > trigger) {
            det->state         = BDS_RISING;
            det->rise_time     = det->elapsed_ms;
            det->accel_mag_max = am;
            det->gyro_mag_max  = gm;
            det->t_gm_peak_ms  = det->elapsed_ms;
            det->gy_x_peak     = fabsf(d->gyro_x);
            det->pitch_min     = get_pitch_deg(det);
            det->pitch_setup   = det->pitch_min;
            det->energy_sum    = 0.0f;
            det->n_peaks       = 0;
            det->dpos_count    = 0;
            det->total_frames  = 0;
            det->last_gm       = 0.0f;
            det->t_omega_peak  = 0.0f;
            det->wx_pre_min    = 0.0f;
            det->wx_burst_max  = 0.0f;
            det->wx_reversed   = false;
            /* 矢量分析复位 */
            det->vx_world = det->vy_world = det->vz_world = 0.0f;
            det->vel_samples = 0;
            det->radius_sum = 0.0f;
            det->radius_samples = 0;
            det->wa_count = 0;
            det->quat_count = 0;
            det->gyro_axis_sum = 0.0f;
            det->axis_samples = 0;
            det->omega_impact[0] = det->omega_impact[1] = det->omega_impact[2] = 0.0f;
            /* 冲击检测复位 */
            det->acc_hp_energy = 0.0f;
            det->acc_lp_energy = 0.0f;
            det->acc_hp_state = 0.0f;
            det->acc_lp_state = 0.0f;
            det->am_prev = am;
            /* 手性复位 */
            det->hand = HAND_FOREHAND;
            det->hand_confident = 0;
            /* 记录触发帧世界加速度 */
            if (det->ahrs) {
                float wx, wy, wz;
                MadgwickWorldAccel(det->ahrs,
                    d->accel_x/9.81f, d->accel_y/9.81f, d->accel_z/9.81f,
                    &wx, &wy, &wz);
                if (det->wa_count < WA_BUF_SIZE) {
                    det->wa_buf[det->wa_count][0] = wx;
                    det->wa_buf[det->wa_count][1] = wy;
                    det->wa_buf[det->wa_count][2] = wz;
                    det->wa_count++;
                }
                /* 记录四元数 */
                if (det->quat_count < QUAT_BUF_SIZE) {
                    det->quat_buf[det->quat_count][0] = det->ahrs->q0;
                    det->quat_buf[det->quat_count][1] = det->ahrs->q1;
                    det->quat_buf[det->quat_count][2] = det->ahrs->q2;
                    det->quat_buf[det->quat_count][3] = det->ahrs->q3;
                    det->quat_count++;
                }
            }
        }
        break;
    }

    case BDS_RISING: {
        /* ── 陀螺削顶检测: 连续≥3帧任轴贴满 ±2000°/s ── */
        {
            float g_abs_max = fabsf(d->gyro_x);
            if (fabsf(d->gyro_y) > g_abs_max) g_abs_max = fabsf(d->gyro_y);
            if (fabsf(d->gyro_z) > g_abs_max) g_abs_max = fabsf(d->gyro_z);
            if (g_abs_max > 1950.0f) det->clip_count++;
            else det->clip_count = 0;  /* 不连续则复位 */
        }

        /* ── 峰值追踪 ── */
        if (am > det->accel_mag_max) {
            det->accel_mag_max = am;
            det->impact_time   = det->elapsed_ms;
        }
        if (gm > det->gyro_mag_max) {
            det->gyro_mag_max   = gm;
            det->t_omega_peak   = (float)(det->elapsed_ms - det->rise_time);
            det->t_gm_peak_ms   = det->elapsed_ms;  /* gm 峰值时刻, 模板匹配用 */
        }
        if (fabsf(d->gyro_x) > det->gy_x_peak)
            det->gy_x_peak = fabsf(d->gyro_x);

        /* ── 鞭打比：不再按固定时间窗追踪 (改由 DONE 阶段根据击球点回放计算) ── */

        /* ── 倒拍深度: Madgwick pitch 最低点 ── */
        {
            float pitch = get_pitch_deg(det);
            if (pitch < det->pitch_min) det->pitch_min = pitch;
        }

        /* ── 世界系速度积分 + 世界加速度存储 + 四元数存储 ── */
        {
            uint32_t dsr = det->elapsed_ms - det->rise_time;
            if (dsr > 30 && dsr < 350 && det->ahrs) {  /* 350ms 覆盖最长挥拍(高远入门 310ms) */
                float wx, wy, wz;
                MadgwickWorldAccel(det->ahrs,
                    d->accel_x/9.81f, d->accel_y/9.81f, d->accel_z/9.81f,
                    &wx, &wy, &wz);
                float dt_g = 0.005f;
                det->vx_world += wx * 9.81f * dt_g;
                det->vy_world += wy * 9.81f * dt_g;
                det->vz_world += wz * 9.81f * dt_g;
                det->vel_samples++;
                /* 存储世界加速度 (PCA用) */
                if (det->wa_count < WA_BUF_SIZE) {
                    det->wa_buf[det->wa_count][0] = wx;
                    det->wa_buf[det->wa_count][1] = wy;
                    det->wa_buf[det->wa_count][2] = wz;
                    det->wa_count++;
                }
            }
            /* 存储四元数序列 (姿态DTW用) */
            if (det->ahrs && det->quat_count < QUAT_BUF_SIZE) {
                det->quat_buf[det->quat_count][0] = det->ahrs->q0;
                det->quat_buf[det->quat_count][1] = det->ahrs->q1;
                det->quat_buf[det->quat_count][2] = det->ahrs->q2;
                det->quat_buf[det->quat_count][3] = det->ahrs->q3;
                det->quat_count++;
            }
        }

        /* ── 转轴漂移: 高ω时追踪 ω̂ 相邻夹角累积 ── */
        {
            if (gm > 3.49f) {  /* >200°/s */
                float nx = d->gyro_x / gm;
                float ny = d->gyro_y / gm;
                float nz = d->gyro_z / gm;
                if (det->axis_samples > 0) {
                    float *prev = det->omega_impact; /* 复用存储上一帧ω̂ */
                    float dot = prev[0]*nx + prev[1]*ny + prev[2]*nz;
                    if (dot > 0.9999f) dot = 0.9999f;
                    if (dot < -0.9999f) dot = -0.9999f;
                    det->gyro_axis_sum += acosf(dot) * 57.29578f; /* 度 */
                }
                det->omega_impact[0] = nx;
                det->omega_impact[1] = ny;
                det->omega_impact[2] = nz;
                det->axis_samples++;
            }
        }

        /* ── 回转半径追踪: r ≈ |a|/ω² (ω高时精确) ── */
        {
            if (gm > 6.0f) {  /* ω > 6 rad/s (~344°/s) 时离心占主导 */
                float r_est = am / (gm * gm);   /* |a_total| / ω² */
                if (r_est < 2.0f) {  /* 滤除异常值 (传感器噪声) */
                    det->radius_sum += r_est;
                    det->radius_samples++;
                }
            }
        }

        /* ── 前臂旋转反转(ωx符号反转) — 改由 DONE 阶段回放计算 ── */

        /* ── 能量 ── */
        if (am > 10.0f) det->energy_sum += (am - 10.0f);

        /* ── 单调性追踪 (前挥段) ── */
        {
            uint32_t dsr = det->elapsed_ms - det->rise_time;
            if (dsr > 30) {  /* 跳过初始段 */
                det->total_frames++;
                if (gm > det->last_gm) det->dpos_count++;
                det->last_gm = gm;
            }
        }

        /* ── 双带通能量追踪 (挑球冲击 vs 跨步区分) ── */
        {
            /* 一阶高通 (fc≈35Hz, α=exp(-2π·35/200)≈0.33) 跟踪am的快速变化 */
            float am_diff = am - det->am_prev;
            det->acc_hp_state = 0.33f * (det->acc_hp_state + am_diff);
            det->acc_hp_energy = 0.9f * det->acc_hp_energy
                               + 0.1f * (det->acc_hp_state * det->acc_hp_state);
            /* 一阶低通 (fc≈12Hz, α=1-exp(-2π·12/200)≈0.31) 跟踪am的慢变 */
            det->acc_lp_state += 0.31f * (am - det->acc_lp_state);
            det->acc_lp_energy = 0.9f * det->acc_lp_energy
                               + 0.1f * (det->acc_lp_state * det->acc_lp_state);
            det->am_prev = am;
        }

        /* ── 回归检测 ── */
        if (am < idle) {
            uint32_t dur = det->elapsed_ms - det->rise_time;
            if (dur >= min_dur) {
                det->state = BDS_DONE;
            } else {
                /* 太短,当作噪声丢弃 */
                det->state = BDS_IDLE;
            }
        }
        break;
    }

    case BDS_DONE: {
        /* ── 基础特征 ── */
        uint32_t dur     = det->elapsed_ms - det->rise_time;
        float    pa      = det->accel_mag_max;
        float    pg      = det->gyro_mag_max;
        /* ── 鞭打比 + 内旋占比: 回放环形缓冲区, 以 impact_time 自适应分界 ── */
        float    gy_early_max = 1.0f;
        float    gy_late_max  = 1.0f;
        float    gx_burst_peak = 0.0f;   /* |ωx| 在爆发窗口内的峰值 */
        float    gm_burst_mean = 1.0f;   /* ω_mag 在爆发窗口内的均值 */
        float    peak_dur_ms  = 0.0f;    /* 尖峰持续时长(ms): gm>70%峰值的连续帧 */
        {
            float gm_buf[200];           /* 合角速度, 800 字节 */
            float ax_buf[200];           /* |ωx|, 用于内旋占比 */
            float raw_gx[200];           /* 带符号 ωx, 用于旋转反转 */
            int   gm_ts[200];            /* 时间戳 (ms) */
            int   n = 0;
            uint32_t t_start = det->rise_time;
            uint32_t t_end   = det->elapsed_ms;

            /* 从环形缓冲区提取 gm 和 |gx|, 按时间升序填充 */
            for (int k = det->rbuf.count - 1; k >= 0 && n < 200; k--) {
                int idx = (det->rbuf.head - 1 - k + RBUF_SIZE) % RBUF_SIZE;
                if (det->rbuf.ts_ms[idx] >= t_start
                    && det->rbuf.ts_ms[idx] <= t_end) {
                    float gx = det->rbuf.gx[idx];
                    float gy = det->rbuf.gy[idx];
                    float gz = det->rbuf.gz[idx];
                    gm_buf[n] = sqrtf(gx*gx + gy*gy + gz*gz);
                    ax_buf[n] = fabsf(gx);   /* |ωx|, 用于内旋占比 */
                    raw_gx[n] = gx;          /* 带符号, 用于旋转反转 */
                    gm_ts[n]  = (int)det->rbuf.ts_ms[idx];
                    n++;
                }
            }

            if (n >= 6) {
                /* 找最接近 impact_time(合加速度峰值) 的索引 */
                int impact_idx = n / 2;
                {
                    int best_d = 0x7FFFFFFF;
                    for (int i = 0; i < n; i++) {
                        int d = gm_ts[i] - (int)det->impact_time;
                        if (d < 0) d = -d;
                        if (d < best_d) { best_d = d; impact_idx = i; }
                    }
                }

                /* 早期 = [0, impact_idx - 8帧]: 纯粹引拍段 (与晚期不重叠) */
                {
                    int end = (impact_idx >= 8) ? (impact_idx - 8) : 0;
                    float best = 0.0f;
                    for (int i = 0; i <= end; i++)
                        if (gm_buf[i] > best) best = gm_buf[i];
                    gy_early_max = (best > 1.0f) ? best : 1.0f;
                }

                /* 晚期 = [impact_idx - 8帧, impact_idx + 6帧]: 爆发窗口 */
                {
                    int lo = (impact_idx >= 8) ? (impact_idx - 8) : 0;
                    int hi = (impact_idx + 6 < n) ? (impact_idx + 6) : (n - 1);
                    float best_gm = 0.0f;
                    float best_ax = 0.0f;
                    float sum_gm  = 0.0f;
                    int   count   = hi - lo + 1;
                    for (int i = lo; i <= hi; i++) {
                        if (gm_buf[i] > best_gm) best_gm = gm_buf[i];
                        if (ax_buf[i] > best_ax) best_ax = ax_buf[i];
                        sum_gm += gm_buf[i];
                    }
                    gy_late_max  = (best_gm > 1.0f) ? best_gm : 1.0f;
                    gx_burst_peak = best_ax;
                    gm_burst_mean = (count > 0) ? (sum_gm / (float)count) : 1.0f;

                    /* 尖峰持续时长: gm 持续高于峰值70%的最大连续帧数 */
                    {
                        float thr = gy_late_max * 0.7f;
                        int run = 0, best_run = 0;
                        for (int i = lo; i <= hi; i++) {
                            if (gm_buf[i] >= thr) {
                                run++;
                                if (run > best_run) best_run = run;
                            } else {
                                run = 0;
                            }
                        }
                        peak_dur_ms = (float)best_run * 5.0f;
                    }

                    /* 爆发集中度 C_burst: 末80ms角冲量占整窗比例 (挑球闪动指标) */
                    {
                        int burst16 = (impact_idx >= 16) ? (impact_idx - 16) : 0;
                        float sum_burst = 0.0f, sum_all = 0.0f;
                        int   n_burst = 0, n_all = 0;
                        for (int i = burst16; i <= impact_idx && i < n; i++)
                            { sum_burst += gm_buf[i]; n_burst++; }
                        for (int i = 0; i < n; i++)
                            { sum_all += gm_buf[i]; n_all++; }
                        det->burst_conc = (n_burst > 0 && n_all > 0 && sum_all > 0.01f)
                            ? (sum_burst / (float)n_burst) / (sum_all / (float)n_all) : 1.0f;
                        det->snap_ratio = (gm_burst_mean > 0.01f)
                            ? gy_late_max / gm_burst_mean : 1.0f;
                    }

                    /* 前臂旋转反转: 引拍段 ωx 最小(外旋) vs 爆发段 ωx 最大(内旋) */
                    {
                        int pre_end = (impact_idx >= 5) ? (impact_idx - 5) : 0;
                        float wx_min = 0.0f, wx_max = 0.0f;
                        for (int i = 0; i <= pre_end; i++)
                            if (raw_gx[i] < wx_min) wx_min = raw_gx[i];
                        int lo2 = (impact_idx >= 8) ? (impact_idx - 8) : 0;
                        int hi2 = (impact_idx + 6 < n) ? (impact_idx + 6) : (n - 1);
                        for (int i = lo2; i <= hi2; i++)
                            if (raw_gx[i] > wx_max) wx_max = raw_gx[i];
                        det->wx_pre_min   = wx_min;
                        det->wx_burst_max = wx_max;
                        det->wx_reversed  = (wx_min < -30.0f && wx_max > 60.0f);
                    }
                }
            }
        }
        float    wr      = (gy_early_max > 1.0f)
                           ? gy_late_max / gy_early_max : 1.0f;
        float    rr      = (gm_burst_mean > 0.01f)
                           ? gx_burst_peak / gm_burst_mean : 0.5f;
        /* 辅助诊断: 内旋峰值占全局合角速度峰值的比例 (区分分子小 vs 分母大) */
        float    gx_ratio = (pg > 0.0f) ? gx_burst_peak / pg : 0.0f;
        float    t_lag   = det->t_omega_peak - (float)dur * 0.5f;
        (void)t_lag;  /* 暂存,后续一致性评估使用 */
        float    smooth  = 100.0f;
        float    launch  = 0.0f;
        float    extra   = 0.0f;
        float    radius  = (det->radius_samples > 5)
                           ? det->radius_sum / (float)det->radius_samples : 0.50f;

        const ideal_params_t *id = &IDEAL[det->stroke];
        const stroke_weight_cfg_t *w = &WEIGHT_CFG[det->stroke];

        /* ════════════════════════════════════════════════════════
         *  矢量分析: 内禀坐标系 + 挥拍平面 PCA
         * ════════════════════════════════════════════════════════ */
        memset(&det->result.vec, 0, sizeof(det->result.vec));

        if (det->wa_count >= 10) {
            /* ── 1. 构建内禀坐标系 ──
             * Z = 重力方向 [0,0,1] (世界系, Madgwick 已对齐)
             * X = 水平面内 PCA 主方向
             * Y = Z × X
             */
            /* 计算世界加速度协方差矩阵的水平分量 (只用 x,y) */
            float sxx = 0.0f, sxy = 0.0f, syy = 0.0f;
            float mx = 0.0f, my = 0.0f;
            for (int i = 0; i < det->wa_count; i++) {
                mx += det->wa_buf[i][0];
                my += det->wa_buf[i][1];
            }
            mx /= (float)det->wa_count;
            my /= (float)det->wa_count;
            for (int i = 0; i < det->wa_count; i++) {
                float dx = det->wa_buf[i][0] - mx;
                float dy = det->wa_buf[i][1] - my;
                sxx += dx*dx;
                sxy += dx*dy;
                syy += dy*dy;
            }
            /* 2×2 PCA: 找主方向 (最大特征值对应特征向量) */
            float trace_2d  = sxx + syy;
            float det_2d    = sxx*syy - sxy*sxy;
            float disc_2d   = trace_2d*trace_2d - 4.0f*det_2d;
            if (disc_2d < 0.0f) disc_2d = 0.0f;
            float lambda1   = (trace_2d + sqrtf(disc_2d)) * 0.5f;
            /* 主特征向量 (X轴在水平面的方向) */
            float ix[3], iy[3], iz[3];
            iz[0] = 0.0f; iz[1] = 0.0f; iz[2] = 1.0f;  /* 重力=Z */
            float ev_x = sxy;
            float ev_y = lambda1 - sxx;
            float ev_m = sqrtf(ev_x*ev_x + ev_y*ev_y);
            if (ev_m > 0.01f) {
                ix[0] = ev_x / ev_m;
                ix[1] = ev_y / ev_m;
            } else {
                ix[0] = 1.0f; ix[1] = 0.0f;
            }
            ix[2] = 0.0f;
            /* Y = Z × X */
            vec3_cross(iz, ix, iy);

            /* ── 2. 挥拍平面 PCA (全3D世界加速度) ── */
            {
                float m3[3] = {0,0,0};
                for (int i = 0; i < det->wa_count; i++)
                    for (int j = 0; j < 3; j++) m3[j] += det->wa_buf[i][j];
                for (int j = 0; j < 3; j++) m3[j] /= (float)det->wa_count;

                float ca=0, cb=0, cc=0, cd=0, ce=0, cf=0;
                for (int i = 0; i < det->wa_count; i++) {
                    float d0 = det->wa_buf[i][0] - m3[0];
                    float d1 = det->wa_buf[i][1] - m3[1];
                    float d2 = det->wa_buf[i][2] - m3[2];
                    ca += d0*d0; cb += d0*d1; cc += d0*d2;
                    cd += d1*d1; ce += d1*d2; cf += d2*d2;
                }
                float evals[3], evecs[3][3];
                sym_eigen3(ca,cb,cc, cd,ce, cf, evals, evecs);

                /* 最小特征值对应法向量 */
                float n_raw[3] = {evecs[0][0], evecs[0][1], evecs[0][2]};
                float n_mag = sqrtf(n_raw[0]*n_raw[0]+n_raw[1]*n_raw[1]+n_raw[2]*n_raw[2]);
                if (n_mag > 1e-9f) {
                    det->result.vec.plane_normal[0] = n_raw[0]/n_mag;
                    det->result.vec.plane_normal[1] = n_raw[1]/n_mag;
                    det->result.vec.plane_normal[2] = n_raw[2]/n_mag;
                }
                /* 平面度 */
                float sum_evals = evals[0]+evals[1]+evals[2];
                det->result.vec.plane_flatness = (sum_evals > 0.01f)
                    ? evals[0] / sum_evals : 0.0f;

                /* ── 3. 内禀系法向量模板比较 ──
                 * 头顶族理想平面: 法向量 ≈ (±1,0,0) (拍面在YZ平面运动)
                 * 低手族理想平面: 法向量 ≈ (0,±1,0) (拍面在XZ平面运动)
                 */
                float tmpl_n[3] = {0,0,0};
                if (det->family == FAMILY_OVERHEAD)
                    { tmpl_n[0]=1.0f; tmpl_n[1]=0.0f; tmpl_n[2]=0.0f; }
                else
                    { tmpl_n[0]=0.0f; tmpl_n[1]=1.0f; tmpl_n[2]=0.0f; }
                float dot_n = det->result.vec.plane_normal[0]*tmpl_n[0]
                            + det->result.vec.plane_normal[1]*tmpl_n[1]
                            + det->result.vec.plane_normal[2]*tmpl_n[2];
                if (dot_n >  1.0f) dot_n =  1.0f;
                if (dot_n < -1.0f) dot_n = -1.0f;
                det->result.vec.plane_tilt_deg = acosf(dot_n) * 57.29578f;
            }
        }

        /* ── 转轴漂移 ── */
        if (det->axis_samples > 1) {
            det->result.vec.axis_deviation_deg =
                det->gyro_axis_sum / (float)det->axis_samples;
        }

        /* ── 挑球正反手判别 (内部路由, 强制模式跳过) ── */
        if (det->stroke == STROKE_LIFT && !det->hand_forced) {
            det->hand = BDS_DiscriminateHand(det);
            /* 镜像归一化: 反手时翻转 ωx 符号, 统一按"先反向预张→后正向爆发"计算 */
            if (det->hand == HAND_BACKHAND) {
                float tmp = det->wx_burst_max;
                det->wx_burst_max = -det->wx_pre_min;  /* 外旋深度→爆发势 */
                det->wx_pre_min  = -tmp;               /* 符号翻转 */
            }
        }

        /* ── 平滑度分析: 对完整挥拍的合角速度曲线数峰 ── */
        {
            float gm_full[200];
            int   n = 0;
            for (int k = det->rbuf.count - 1; k >= 0 && n < 200; k--) {
                int idx = (det->rbuf.head - 1 - k + RBUF_SIZE) % RBUF_SIZE;
                if (det->rbuf.ts_ms[idx] >= det->rise_time
                    && det->rbuf.ts_ms[idx] <= det->elapsed_ms) {
                    float gx = det->rbuf.gx[idx];
                    float gy = det->rbuf.gy[idx];
                    float gz = det->rbuf.gz[idx];
                    gm_full[n] = sqrtf(gx*gx + gy*gy + gz*gz);
                    n++;
                }
            }
            if (n >= 3) {
                int peaks = count_peaks(gm_full, n, pg);
                smooth = (peaks == 1) ? 100.0f :
                         (peaks == 2) ? 70.0f  :
                         (peaks == 3) ? 50.0f  : 35.0f;
            }
        }

        /* ── L2指标: 按动作族计算 ── */
        switch (det->family) {
        case FAMILY_OVERHEAD: {
            /* 出射仰角: 世界系速度方向 */
            if (det->vel_samples > 5) {
                float vh = sqrtf(det->vx_world*det->vx_world +
                                 det->vy_world*det->vy_world);
                launch = atan2f(det->vz_world, vh) * 57.29578f;
            } else {
                launch = get_pitch_deg(det);  /* 回退: 用当前pitch近似 */
            }
            /* 倒拍深度 */
            extra = det->pitch_setup - det->pitch_min;
            break;
        }
        case FAMILY_UNDERHAND: {
            extra = (float)(det->impact_time - det->rise_time);
            if (det->vel_samples > 5) {
                float vh = sqrtf(det->vx_world*det->vx_world +
                                 det->vy_world*det->vy_world);
                launch = atan2f(det->vz_world, vh) * 57.29578f;
            } else {
                launch = get_pitch_deg(det);
            }
            break;
        }
        }

        /* ── 软评分 (smoothstep, 无悬崖) ── */
        int s1;
        if (det->stroke == STROKE_LIFT) {
            /* 挑球力度线性映射: 正手满分75, 反手满分100 */
            float full = (det->hand == HAND_FOREHAND) ? 75.0f : 100.0f;
            float r = (pa - 33.0f) / (full - 33.0f);
            if (r >= 1.0f)      s1 = 100;
            else if (r <= 0.0f) s1 = 10;
            else                s1 = (int)(10.0f + 90.0f * r);
        } else {
            s1 = score_item_soft(pa, id->a_lo, id->a_hi);
        }
        int s2 = score_item_soft(pg,    id->g_lo,  id->g_hi);
        /* s3: 杀球/挑球越快越好, 高远球不能太快也不能太慢 */
        int s3;
        if (det->stroke == STROKE_CLEAR)
            s3 = score_item_bilateral((float)dur, (float)id->dur_lo, (float)id->dur_hi);
        else
            s3 = score_item_soft(-(float)dur, -(float)id->dur_hi, -(float)id->dur_lo);
        int s4 = (w->w_whip  > 0.0f) ? score_item_soft(wr,  id->whip_lo, id->whip_hi):50;
        int s5 = (w->w_rotation>0.0f) ? score_item_soft(rr,  id->rot_lo,  id->rot_hi) :50;
        /* s6: 杀球越陡越好(取反), 高远/挑球越高越好 */
        int s6 = 50;
        if (w->w_launch > 0.0f) {
            if (det->family == FAMILY_OVERHEAD && det->stroke == STROKE_SMASH)
                s6 = score_item_soft(-launch, -(id->launch_hi), -(id->launch_lo));
            else
                s6 = score_item_soft(launch, id->launch_lo, id->launch_hi);
        }
        int s7 = (int)smooth;
        int s8 = 50;
        switch (det->family) {
        case FAMILY_OVERHEAD:
            s8 = score_item_soft(extra, id->drop_lo, id->drop_hi); break;
        case FAMILY_UNDERHAND: {
            /* 发力峰值: 爆发窗口内合角速度最大值 */
            int s8_peak  = score_item_soft(gy_late_max, 500.0f, 1700.0f);
            /* 尖峰持续: gm>70%峰值的连续时长, 越小越好 */
            int s8_dur   = (int)soft_score(peak_dur_ms, 30.0f, 60.0f);
            /* 爆发集中度: 末80ms gm均值/全窗均值, 越大越集中 */
            int s8_burst = (int)soft_score(det->burst_conc, 1.3f, 0.8f);
            /* 末端增速: 峰值/全窗均值, 越大说明击球瞬间越爆发 */
            int s8_snap  = (int)soft_score(det->snap_ratio, 1.2f, 0.9f);
            s8 = (s8_peak * 25 + s8_dur * 25 + s8_burst * 25 + s8_snap * 25) / 100;
            extra = peak_dur_ms;
            break;
        }
        }

        /* ── 总分: 高远球用模板匹配, 杀球/挑球用算术平均 ── */
        int total;
        if (det->stroke == STROKE_CLEAR) {
            total = score_by_template(det);
        } else {
            total = (int)(s1*w->w_force + s2*w->w_speed + s3*w->w_arc +
                          s4*w->w_whip  + s5*w->w_rotation +
                          s6*w->w_launch + s7*w->w_smoothness + s8*w->w_extra);
        }

        /* ── 关键门控: 核心技术缺失 → 总分封顶 ── */
        if (det->family == FAMILY_OVERHEAD && s4 < 30)
            total = (total < 55) ? total : 55;  /* 无鞭打 → 最多55 */
        if (det->family == FAMILY_OVERHEAD && s5 < 30)
            total = (total < 50) ? total : 50;  /* 无内旋 → 最多50 */

        /* ── 头顶族错打检测: 选了杀球但打成高远? (评分之后、建议之前) ── */
        int mis_hit_override = 0;
        if (det->family == FAMILY_OVERHEAD) {
            int vote_clear = 0;
            if (launch > 18.0f) vote_clear++;
            if (get_pitch_deg(det) > -5.0f) vote_clear++;
            if (wr < 1.8f) vote_clear++;

            if (det->stroke == STROKE_SMASH && vote_clear >= 2) {
                mis_hit_override = 1;
                total = (int)(total * 0.6f);
            } else if (det->stroke == STROKE_CLEAR && vote_clear <= 0) {
                mis_hit_override = 1;
                total = (int)(total * 0.6f);
            }
        }

        /* ── 削顶标记 ── */
        if (det->clip_count >= 3) total = (int)(total * 0.85f);

        /* ── 建议生成 ── */
        char sug[160] = "";
        if (mis_hit_override) {
            if (det->stroke == STROKE_SMASH)
                strcpy(sug, "出球向上,更像高远球。杀球需击球点更前、手腕下压。");
            else
                strcpy(sug, "出球向下,更像杀球。高远需拍面向前上方送。");
        } else if (det->stroke == STROKE_CLEAR && total >= 85) {
            strcpy(sug, "动作标准,继续保持!");
        } else {
        /* ── 核心创新指标: 鞭打比 + 前臂内旋 ── */
        if (s4 < 40 && w->w_whip > 0.0f) {
            if (radius > 0.50f)
                strcat(sug, "鞭打比不足且有甩大臂倾向:发力来自肩膀画大圈,缺少前臂爆发。先慢速感受肘先走-前臂滞后-前臂超过肘-手腕超过前臂的四步鞭打,让发力从肩膀下沉到前臂和手腕;");
            else
                strcat(sug, "鞭打比不足:先慢速感受肘先走-前臂滞后-前臂超过肘-手腕超过前臂的四步鞭打;");
        }

        if (w->w_rotation > 0.0f) {
            if (s5 < 40 && gx_ratio < 0.30f)
                strcat(sug, "前臂内旋不足:击球瞬间前臂没有主动向内旋转,力量全来自甩大臂。慢速练习握拍放松、击球前手掌从朝上翻到朝下;");
            else if (s5 < 40)
                strcat(sug, "小臂发力不纯:击球瞬间手腕有多余晃动,力量分散。练习时注意击球前后手腕保持稳定,只做内旋不做额外翻腕;");
            else if (s5 < 60)
                strcat(sug, "前臂内旋偏弱:有一定内旋发力但不够突出,尝试击球瞬间更集中地加速前臂旋转;");
        }
        /* ── 基础指标 ── */
        if (s1 < 50)      strcat(sug, "加大力度;");
        else if (s1 < 70) strcat(sug, "力度稍弱;");
        if (s2 < 40)      strcat(sug, "手腕旋转速度偏慢:尝试加快前臂挥动,提高击球瞬间的角速度;");
        else if (s2 < 60) strcat(sug, "角速度接近标准但还能更快:击球瞬间手腕加速再果断一些;");
        if (s7 < 50) strcat(sug, "存在二次发力情况:引拍完了立刻出手,中间不要有调整动作,保证发力的平滑度;");
        if (det->family == FAMILY_OVERHEAD) {
            if (s8 < 40)
                strcat(sug, "引拍不足:没有倒拍蓄力,架拍完就直接往前挥了。引拍时肘先向上抬起,前臂放松,让拍头自然向后垂落到后背位置;");
            else if (s8 < 60)
                strcat(sug, "倒拍偏浅:引拍时拍头下垂不够深,加速距离不足。肘抬高后前臂完全放松,感受拍头靠重力自己掉到后背;");
            else if (s8 < 80)
                strcat(sug, "倒拍接近标准但还能更深:引拍时身体充分侧转,让拍头垂落到位后再启动前挥;");
        }
        if (det->family == FAMILY_UNDERHAND) {
            if (s1 < 30)
                strcat(sug, "挑球力度严重不足:没有用拇指顶住拍柄宽面,引拍时前臂外旋蓄力,击球瞬间前臂内旋把球弹出去;");
            else if (s1 < 70)
                strcat(sug, "挑球力度偏弱:加速前臂挥动,触球瞬间手指收紧加力,加深外旋预张幅度,让手腕弹出更大的力量;");
            else if (s1 < 85)
                strcat(sug, "挑球发力可更集中:缩短击球时间,让发力更短促。想象拍面触球瞬间手指收紧、手腕快弹,触球后立刻放松;");
            else
                strcat(sug, "动作标准,继续保持!");
        }

        } /* !mis_hit_override */
        if (strlen(sug) == 0) {
            if (total >= 85)
                strcpy(sug, "动作标准,继续保持!");
            else
                strcpy(sug, "动作标准!");
        }

        /* ── 填充结果 ── */
        det->result.total_score    = total;
        det->result.force_score    = s1;
        det->result.speed_score    = s2;
        det->result.arc_score      = s3;
        det->result.whip_score     = s4;
        det->result.rotation_score = s5;
        det->result.launch_score   = s6;
        det->result.smoothness_score = s7;
        det->result.extra_score    = s8;
        det->result.peak_accel     = pa;
        det->result.peak_gyro      = pg;
        det->result.duration_ms    = dur;
        det->result.whip_ratio     = wr;
        det->result.rotation_ratio = rr;
        det->result.launch_angle_deg = launch;
        det->result.extra_value    = extra;
        strncpy(det->result.suggestion, sug, sizeof(det->result.suggestion)-1);
        det->result.new_result     = 1;

        det->state = BDS_IDLE;
        break;
    }
    }
}

/* ================================================================
 *  结果接口
 * ================================================================ */
int BDS_HasResult(bds_detector_t *det) {
    return det->result.new_result;
}

bds_result_t BDS_GetResult(bds_detector_t *det) {
    det->result.new_result = 0;
    return det->result;
}

/* ================================================================
 *  工具函数
 * ================================================================ */
const char* BDS_StrokeName(stroke_type_t t) {
    switch (t) {
    case STROKE_CLEAR: return "高远球";
    case STROKE_SMASH: return "杀球";
    case STROKE_LIFT:  return "挑球";
    default:           return "未知";
    }
}

stroke_family_t BDS_FamilyOf(stroke_type_t t) {
    switch (t) {
    case STROKE_CLEAR:
    case STROKE_SMASH: return FAMILY_OVERHEAD;
    case STROKE_LIFT:  return FAMILY_UNDERHAND;
    default:           return FAMILY_OVERHEAD;
    }
}

const char* BDS_StrokeAdvice(stroke_type_t t) {
    switch (t) {
    case STROKE_CLEAR: return "侧身引拍→转体鞭打→手腕内旋→收拍至左下方";
    case STROKE_SMASH: return "架拍高于肩→转体下压→手腕闪动→收拍至身体左侧";
    case STROKE_LIFT:  return "低手位引拍→前臂手腕发力→拍面快速上挑";
    default:           return "";
    }
}

const stroke_detect_cfg_t* BDS_GetDetectCfg(stroke_type_t t) {
    return (t < STROKE_COUNT) ? &DETECT_CFG[t] : &DETECT_CFG[0];
}

const stroke_weight_cfg_t* BDS_GetWeightCfg(stroke_type_t t) {
    return (t < STROKE_COUNT) ? &WEIGHT_CFG[t] : &WEIGHT_CFG[0];
}

/* ================================================================
 *  提取角速度曲线 (供 DTW 使用)
 * ================================================================ */
int BDS_ExtractGyroCurve(const bds_detector_t *det, float *buf_out, int max_len) {
    int n = 0;
    uint32_t t_start = det->rise_time;
    uint32_t t_end   = det->elapsed_ms;
    for (int k = det->rbuf.count - 1; k >= 0 && n < max_len; k--) {
        int idx = (det->rbuf.head - 1 - k + RBUF_SIZE) % RBUF_SIZE;
        if (det->rbuf.ts_ms[idx] >= t_start && det->rbuf.ts_ms[idx] <= t_end) {
            float gx = det->rbuf.gx[idx];
            float gy = det->rbuf.gy[idx];
            float gz = det->rbuf.gz[idx];
            buf_out[n++] = sqrtf(gx*gx + gy*gy + gz*gz);
        }
        if (det->rbuf.ts_ms[idx] < t_start) break;
    }
    return n;
}

int BDS_ExtractQuatCurve(const bds_detector_t *det, float (*quat_out)[4], int max_len) {
    int n = det->quat_count;
    if (n > max_len) n = max_len;
    for (int i = 0; i < n; i++) {
        quat_out[i][0] = det->quat_buf[i][0];
        quat_out[i][1] = det->quat_buf[i][1];
        quat_out[i][2] = det->quat_buf[i][2];
        quat_out[i][3] = det->quat_buf[i][3];
    }
    return n;
}
