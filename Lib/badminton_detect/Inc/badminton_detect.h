/**
 * @file    badminton_detect.h
 * @brief   羽毛球挥拍检测与评估 —— 验证式识别 + 三层指标架构
 *
 * 架构原则:
 *   用户先选动作再练习 → "验证式识别"而非开放多分类
 *   每类动作独立检测阈值、独立评分维度、独立理想参数矩阵
 *
 * 三层指标:
 *   L1 共性指标: 峰值挥速、击球时机t_lag、回转半径、平滑度(所有动作都算)
 *   L2 发力结构指标: 按动作族选用(头顶族:爆发比+内旋占比+倒拍深度;
 *                      低手族:旋转反转比+出手时长+紧凑度;
 *                      静态族:节奏比+一致性+加速单调性)
 *   L3 专属判据: 阈值表,存Flash,按动作查表
 */

#ifndef BADMINTON_DETECT_H
#define BADMINTON_DETECT_H

#include <stdint.h>
#include <stdbool.h>
#include "mpu6050.h"
#include "madgwick.h"

/* ================================================================
 *  动作类型 (动作族分类)
 * ================================================================ */
typedef enum {
    STROKE_CLEAR = 0,   /* 正手高远球   — 头顶族 */
    STROKE_SMASH,       /* 正手杀球     — 头顶族 */
    STROKE_LIFT,        /* 正手挑球     — 低手族 */
    STROKE_COUNT
} stroke_type_t;

typedef enum {
    FAMILY_OVERHEAD = 0,  /* 头顶: 杀球/高远 */
    FAMILY_UNDERHAND,     /* 低手: 挑球 */
} stroke_family_t;

/* 手性 (挑球内部路由, UI不区分) */
typedef enum {
    HAND_FOREHAND = 0,   /* 正手 */
    HAND_BACKHAND = 1,   /* 反手 */
} hand_t;

/* 挑球双阈值表 (正反手独立) */
typedef struct {
    float t_release_good;    /* 良好出手时长(ms), 正手250/反手300 */
    float r_compact_max;     /* 最大紧凑度(m), 正手0.30/反手0.35 */
    float R_reversal_min;    /* 最小旋转反转比, 正手2.0/反手1.6 */
    float theta_launch_lo;   /* 出射仰角下界(°) */
    float theta_launch_hi;   /* 出射仰角上界(°) */
    float dtw_sigma;         /* DTW评分软化系数 */
} lift_thresholds_t;

/* ================================================================
 *  技术水平
 * ================================================================ */
typedef enum {
    SKILL_NOVICE  = 0,
    SKILL_AMATEUR = 1,
    SKILL_PRO     = 2,
    SKILL_LEVELS  = 3
} skill_level_t;

/* ================================================================
 *  检测状态机
 * ================================================================ */
typedef enum {
    BDS_IDLE,       /* 空闲,等待触发 */
    BDS_RISING,     /* 挥拍进行中,持续追踪特征 */
    BDS_DONE,       /* 挥拍结束,输出结果 */
} bds_state_t;

/* ================================================================
 *  每动作独立检测配置 (验证式识别的核心)
 * ================================================================ */
typedef struct {
    float    trigger_a;        /* 触发阈值: 合加速度(m/s²)超过此值进入RISING */
    float    idle_a;           /* 回归阈值: 合加速度低于此值判定挥拍结束 */
    uint32_t min_dur_ms;       /* 最短有效持续(ms),短于此值的触发视为噪声 */
    float    pre_window_ms;    /* 击球前分析窗(ms) */
    float    post_window_ms;   /* 击球后分析窗(ms) */
} stroke_detect_cfg_t;

/* 每动作评分权重 (可动态配置) */
typedef struct {
    float w_force;       /* 力度/峰值挥速 */
    float w_speed;       /* 角速度峰值 */
    float w_arc;         /* 弧线/持续时间 */
    float w_whip;        /* 鞭打/爆发比 */
    float w_rotation;    /* 内旋占比 */
    float w_launch;      /* 出射仰角 (头顶族/低手族) */
    float w_smoothness;  /* 发力平滑度 */
    float w_extra;       /* 动作专项: 头顶=倒拍深度, 低手=紧凑度, 静态=一致性 */
} stroke_weight_cfg_t;

/* ================================================================
 *  L1 共性指标 — 所有动作都计算
 * ================================================================ */
typedef struct {
    float    peak_gyro_mag;      /* 合角速度峰值 (rad/s) */
    float    peak_accel_mag;     /* 合加速度峰值 (m/s²) */
    float    t_lag_ms;           /* 角速度峰值-击球点时差 (<0=峰值先于击球) */
    float    radius_gyration;    /* 回转半径估计 (m), 越小越紧凑 */
    int      smoothness;         /* 平滑度分: 单峰=100, 多峰扣分 */
    uint32_t duration_ms;        /* 挥拍总持续(触发→回归) */
} l1_common_metrics_t;

/* ================================================================
 *  L2 发力结构指标 — 按动作族选用
 * ================================================================ */

/* 头顶族 (Clear/Smash): 鞭打+内旋链 */
typedef struct {
    float whip_ratio;          /* 爆发比 = 击球中后期ω_mag / 早期ω_mag */
    float pronation_ratio;     /* 前臂内旋占比 = |ωx|_peak / ω_mag_peak */
    float drop_depth_deg;      /* 倒拍深度(°): 引拍段pitch最低点-架拍位差 */
    float launch_angle_deg;    /* 出射仰角(°): 世界系速度方向仰角 */
    float follow_ratio;        /* 随挥完整比: 击球后0.25s弧长/击球前0.25s弧长 */
} l2_overhead_metrics_t;

/* 低手族 (Lift): 紧凑快出手 */
typedef struct {
    float release_time_ms;     /* 出手时长: 动作起始→击球 */
    float compactness_m;       /* 紧凑度(回转半径), <0.30m良好 */
    float reversal_ratio;      /* 旋转反转比: ωx爆发/|外旋|, >2有预张-爆发 */
    float launch_angle_deg;    /* 出射仰角(°): 挑球50~70良好 */
    float impact_pitch_deg;    /* 击球点pitch姿态(°) */
} l2_underhand_metrics_t;

/* 静态族 (Serve): 节奏+一致性 */
typedef struct {
    float tempo_ratio;         /* 节奏比 = T_back / T_forward, 2.5~3.5良好 */
    float consistency_idx;     /* 一致性指数CV, <0.08高度稳定 */
    float monotonicity;        /* 加速单调性: dω/dt>0占比, >0.85良好 */
    float t_peak_lag_ms;       /* 峰值-击球时差, |t|<15ms良好 */
    float launch_angle_deg;    /* 出射仰角: 发球35~55良好 */
    float finish_pitch_deg;    /* 收拍pitch, 校验挥拍平面 */
    int   depth_zone;          /* 落点深度: 0=偏浅 1=到位 2=过深 */
} l2_static_metrics_t;

/* ================================================================
 *  L3 理想参数矩阵 (每动作×每水平的判据)
 * ================================================================ */
typedef struct {
    /* 共性 */
    float g_lo, g_hi;           /* 合角速度峰值理想范围 */
    float a_lo, a_hi;           /* 合加速度峰值理想范围 */
    int   dur_lo, dur_hi;       /* 持续时间理想范围 */
    /* 发力结构 */
    float whip_lo, whip_hi;     /* 爆发比理想范围 */
    float rot_lo,  rot_hi;      /* 内旋占比理想范围 */
    float launch_lo, launch_hi; /* 出射仰角理想范围(°) */
    /* 专项 */
    float drop_lo, drop_hi;     /* 倒拍深度(°) — 头顶族 */
    float compact_lo,compact_hi;/* 紧凑度(m)   — 低手族 */
    float tempo_lo, tempo_hi;   /* 节奏比      — 静态族 */
} ideal_params_t;

/* ================================================================
 *  评分结果
 * ================================================================ */
typedef struct {
    /* 总分与子项 */
    int      total_score;        /* 加权总分 0~100 */
    int      force_score;        /* 力度/挥速分 */
    int      speed_score;        /* 角速度分 */
    int      arc_score;          /* 弧线/持续时间分 */
    int      whip_score;         /* 爆发比分 */
    int      rotation_score;     /* 内旋分 */
    int      launch_score;       /* 出射仰角分 */
    int      smoothness_score;   /* 平滑度分 */
    int      extra_score;        /* 专项分 (倒拍/紧凑/一致性) */

    /* 原始特征 */
    float    peak_accel;
    float    peak_gyro;
    uint32_t duration_ms;
    float    whip_ratio;
    float    rotation_ratio;
    float    launch_angle_deg;
    float    extra_value;        /* 倒拍深度/紧凑度/一致性 实际值 */

    /* 矢量指标 */
    struct {
        float plane_normal[3];
        float plane_flatness;
        float plane_tilt_deg;
        float axis_deviation_deg;
        float attitude_score;
    } vec;

    /* 输出 */
    char     suggestion[160];
    int      new_result;         /* 1=有新结果待读取 */
    float    shape_r;            /* 包络形状相关系数 (模板匹配, 调试用) */
} bds_result_t;

/* ================================================================
 *  环形缓冲区 (时间窗特征提取)
 * ================================================================ */
#define RBUF_SIZE  512   /* 200Hz × 2.56s */
#define WA_BUF_SIZE 120  /* 世界加速度样本 (30~200ms窗口) */
#define QUAT_BUF_SIZE 240 /* 四元数序列 (击球前后各0.6s) */

typedef struct {
    float ax[RBUF_SIZE], ay[RBUF_SIZE], az[RBUF_SIZE];
    float gx[RBUF_SIZE], gy[RBUF_SIZE], gz[RBUF_SIZE];
    uint32_t ts_ms[RBUF_SIZE];
    int      head;
    int      count;
} ring_buffer_t;

/* ================================================================
 *  矢量级指标
 * ================================================================ */
typedef struct {
    /* 挥拍平面 */
    float plane_normal[3];     /* 内禀系法向量 (单位) */
    float plane_flatness;      /* λ_min/Σλ, 越小越平面 */
    float plane_tilt_deg;      /* 相对模板法向量偏角 */

    /* 转轴漂移 (简化版: 击球前50ms与击球时转轴夹角) */
    float axis_deviation_deg;  /* 末端转轴收敛度 */

    /* 四元数姿态相似度 (由 quat_dtw 模块计算) */
    float attitude_score;      /* 0~100, 与模板的测地DTW相似度 */
} vector_metrics_t;

/* ================================================================
 *  检测器主结构
 * ================================================================ */
typedef struct {
    bds_state_t      state;
    stroke_type_t    stroke;
    skill_level_t    skill;
    stroke_family_t  family;

    /* 时间追踪 */
    uint32_t         elapsed_ms;    /* 自初始化累计ms */
    uint32_t         rise_time;     /* 触发时刻 */
    uint32_t         impact_time;   /* 击球时刻(=合加速度峰值时刻) */

    /* 峰值追踪 */
    float            accel_mag_max;
    float            gyro_mag_max;
    float            gy_x_peak;     /* |ωx| 峰值,内旋用 */
    float            gy_early_max;  /* 挥拍早期ω_mag最大值 */
    float            gy_late_max;   /* 挥拍中后期ω_mag最大值 */
    float            pitch_min;     /* 引拍段pitch最低(倒拍深度用) */
    float            pitch_setup;   /* 架拍位pitch */

    /* 时序分析 */
    float            t_omega_peak;  /* ω峰值相对触发时刻(ms) */
    int              n_peaks;       /* 加速段峰个数(平滑度用) */
    float            last_gm;       /* 上一帧ω_mag(单调性用) */
    int              dpos_count;    /* dω/dt>0 帧计数 */
    int              total_frames;  /* 前挥段总帧数 */

    /* 反向点追踪(挑球/发球) */
    float            wx_pre_min;    /* 击球前外旋段ωx最小 */
    float            wx_burst_max;  /* 爆发段ωx最大 */
    bool             wx_reversed;   /* ωx是否出现符号反转 */

    /* 环形缓冲 */
    ring_buffer_t    rbuf;

    /* 能量 */
    float            energy_sum;

    /* ── Madgwick 姿态引用 (外部设置) ── */
    const madgwick_t *ahrs;

    /* ── 世界系速度积分 (击球后窗口) ── */
    float            vx_world, vy_world, vz_world;  /* 累积世界系速度 (g·s) */
    int              vel_samples;                    /* 积分样本计数 */

    /* ── 回转半径追踪 ── */
    float            radius_sum;
    int              radius_samples;

    /* ── 矢量分析缓冲 ── */
    /* 世界系线性加速度 (去重力, 单位g) */
    float            wa_buf[WA_BUF_SIZE][3];
    int              wa_count;
    /* 四元数序列 (q0,q1,q2,q3) 用于姿态DTW */
    float            quat_buf[QUAT_BUF_SIZE][4];
    int              quat_count;

    /* ── 转轴漂移 ── */
    float            gyro_axis_sum;
    int              axis_samples;
    float            omega_impact[3];  /* 击球时刻 ω 矢量, 也用于存储上一帧 ω̂ */

    /* ── 正反手判别 ── */
    hand_t           hand;           /* 检测到的手性 */
    int              hand_confident; /* 0=低置信, 1=高置信 */
    int              hand_forced;    /* 1=强制模式,跳过自动判别 */

    /* ── 峰锚定 ── */
    uint32_t         t_gm_peak_ms;   /* gm 峰值时刻 (ms), 模板匹配锚点 */

    /* ── 挑球闪动指标 ── */
    float            burst_conc;      /* 爆发集中度: 末80ms gm均值 / 全窗gm均值 */
    float            snap_ratio;      /* 末端增速比: 峰值 / 全窗均值 */

    /* ── 挑球冲击检测 (双带通能量比) ── */
    float            acc_hp_energy;   /* 高频段能量 (40~90Hz 等效) */
    float            acc_lp_energy;   /* 低频段能量 (5~15Hz 等效) */
    float            acc_hp_state;    /* 高通滤波器状态 */
    float            acc_lp_state;    /* 低通滤波器状态 */
    float            am_prev;         /* 上一帧 am (差分用) */

    /* ── 计数防抖 ── */
    uint32_t         last_count_time;
    int              in_refractory;

    /* ── 陀螺削顶 ── */
    int              clip_count;      /* ≥3帧贴满量程 → 标记削顶 */

    /* ── 建议迟滞 ── */
    int              sugg_counters[8]; /* 每项建议连续触发计数 */

    bds_result_t     result;
} bds_detector_t;

/* ================================================================
 *  API
 * ================================================================ */
void          BDS_Init(bds_detector_t *det, stroke_type_t stroke, skill_level_t lvl);
void          BDS_FeedData(bds_detector_t *det, const mpu6050_data_t *d);
int           BDS_HasResult(bds_detector_t *det);
bds_result_t  BDS_GetResult(bds_detector_t *det);
const char*   BDS_StrokeName(stroke_type_t t);
stroke_family_t BDS_FamilyOf(stroke_type_t t);
const char*   BDS_StrokeAdvice(stroke_type_t t);

/* 供外部查询每动作配置 */
const stroke_detect_cfg_t* BDS_GetDetectCfg(stroke_type_t t);
const stroke_weight_cfg_t* BDS_GetWeightCfg(stroke_type_t t);

/**
 * @brief 提取刚完成的挥拍角速度曲线 (用于 DTW 模板匹配)
 * @param det      检测器实例
 * @param buf_out  输出缓冲区 (调用者分配)
 * @param max_len  缓冲区最大长度
 * @return 实际填充的样本数
 */
int BDS_ExtractGyroCurve(const bds_detector_t *det, float *buf_out, int max_len);

/**
 * @brief 提取刚完成的挥拍四元数序列 (用于姿态 DTW)
 * @return 实际填充的帧数 (每帧4个float: q0,q1,q2,q3)
 */
int BDS_ExtractQuatCurve(const bds_detector_t *det, float (*quat_out)[4], int max_len);

/* 正反手判别 (符号投票, 挑球内部路由用) */
hand_t BDS_DiscriminateHand(const bds_detector_t *det);

/* 查询挑球双阈值表 */
const lift_thresholds_t* BDS_GetLiftThresholds(hand_t hand);

/* 挑球冲击判据: 区分击球冲击 vs 跨步脚冲击 */
int BDS_IsRacketImpact(const bds_detector_t *det);

#endif /* BADMINTON_DETECT_H */
