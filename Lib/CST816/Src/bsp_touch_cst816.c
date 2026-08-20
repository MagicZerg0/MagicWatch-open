#include "bsp_touch_cst816.h"


TouchPointRefTypeDef TPR_Structure;

/* 初始化DWT */
void DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* 简单循环延时 - STM32U585 适用 */
void delay_us(uint32_t us)
{
    while (us--)
    {
        /* 160MHz 下约 40 次循环 ≈ 1µs，可根据实际振荡微调 */
        volatile uint32_t i = 40;
        while (i--) { __NOP(); }
    }
}

// /* 微秒延时 (假设主频 = 84 MHz) */  //这个会因为U5系列的trustzone机制导致失效 因此改用上面那个
// void delay_us(uint32_t us) {
//     uint32_t start = DWT->CYCCNT;
//     uint32_t ticks = us * (SystemCoreClock / 1000000);
//     while ((DWT->CYCCNT - start) < ticks);
// }

/* 模拟 I2C 时用到的短暂延时 (约 1~2 us) */
static void cst816_delay(void) {
    delay_us(2);   // 根据实际时序微调，保证 SCL 高电平 >0.6 us，低电平 >1.2 us
}

/* 产生起始信号 */
static void CST816_Start(void) {
    CST816_SDA_Set();
    CST816_SCL_Set();
    cst816_delay();
    CST816_SDA_Clr();
    cst816_delay();
    CST816_SCL_Clr();
    cst816_delay();
}

/* 产生停止信号 */
static void CST816_Stop(void) {
    CST816_SDA_Clr();
    CST816_SCL_Set();
    cst816_delay();
    CST816_SDA_Set();
    cst816_delay();
}

/* 发送一个字节，高位在前 */
static void CST816_WriteByte(uint8_t dat) {
    for (uint8_t i = 0; i < 8; i++) {
        if (dat & 0x80)
            CST816_SDA_Set();
        else
            CST816_SDA_Clr();
        cst816_delay();
        CST816_SCL_Set();
        cst816_delay();
        CST816_SCL_Clr();
        dat <<= 1;
    }
    CST816_SDA_Set();  // 释放总线
    cst816_delay();
}

/* 接收一个字节并发送 ACK */
static uint8_t CST816_ReadByte(uint8_t ack) {
    uint8_t byte = 0;
    CST816_SDA_Set();  // 释放总线
    for (uint8_t i = 0; i < 8; i++) {
        CST816_SCL_Set();
        cst816_delay();
        byte <<= 1;
        if (CST816_SDA_Read())
            byte |= 0x01;
        CST816_SCL_Clr();
        cst816_delay();
    }
    // 发送 ACK (0) 或 NACK (1)
    if (ack)
        CST816_SDA_Set();
    else
        CST816_SDA_Clr();
    cst816_delay();
    CST816_SCL_Set();
    cst816_delay();
    CST816_SCL_Clr();
    CST816_SDA_Set();
    cst816_delay();
    return byte;
}

/* 等待从机应答，返回 0 表示成功，1 表示失败 */
static uint8_t CST816_WaitAck(void) {
    uint8_t timeout = 0;
    CST816_SDA_Set();
    cst816_delay();
    CST816_SCL_Set();
    cst816_delay();
    while (CST816_SDA_Read()) {
        if (++timeout > 200) {
            CST816_Stop();
            return 1;
        }
    }
    CST816_SCL_Clr();
    cst816_delay();
    return 0;
}

/* 写寄存器序列 */
uint8_t CST816_WR_Reg(uint16_t reg, uint8_t *buf, uint8_t len) {
    uint8_t ret = 0;
    CST816_Start();
    CST816_WriteByte(FT_CMD_WR);
    if (CST816_WaitAck()) { ret = 1; goto exit; }
    CST816_WriteByte(reg & 0xFF);
    if (CST816_WaitAck()) { ret = 2; goto exit; }
    for (uint8_t i = 0; i < len; i++) {
        CST816_WriteByte(buf[i]);
        if (CST816_WaitAck()) { ret = 3; break; }
    }
exit:
    CST816_Stop();
    return ret;
}

/* 读寄存器序列 */
void CST816_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len) {
    CST816_Start();
    CST816_WriteByte(FT_CMD_WR);
    CST816_WaitAck();
    CST816_WriteByte(reg & 0xFF);
    CST816_WaitAck();
    CST816_Start();
    CST816_WriteByte(FT_CMD_RD);
    CST816_WaitAck();
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = CST816_ReadByte(i == (len - 1) ? 1 : 0);  // 最后一个字节发 NACK
    }
    CST816_Stop();
}

/* 初始化触摸芯片：复位 + 寄存器配置 */
void CST816_Init(void) {
    // 硬件复位时序
    CST816_RST_Clr();
    HAL_Delay(50);
    CST816_RST_Set();
    HAL_Delay(100);

    // 确保 I2C 初始态
    CST816_SDA_Set();
    CST816_SCL_Set();
    HAL_Delay(10);

    // ====== 读取芯片 ID ======
    uint8_t chip_id = 0;
    CST816_RD_Reg(0xA7, &chip_id, 1);
    // 在这里设置断点，或者用串口打印 chip_id
    //printf("CST816 Chip ID: 0x%02X\r\n", chip_id);

    uint8_t temp;

    // 进入正常操作模式
    temp = 0;
    CST816_WR_Reg(FT_DEVIDE_MODE, &temp, 1);

    // 触摸阈值 (厂商推荐 21，实际可调)
    temp = 21;
    CST816_WR_Reg(FT_ID_G_THGROUP, &temp, 1);

    // 激活周期 (不能小于 12)
    temp = 12;
    CST816_WR_Reg(FT_ID_G_PERIODACTIVE, &temp, 1);
}

/* 扫描触摸坐标 (轮询方式) */
//static const uint16_t reg_list[] = {FT_TP1_REG, FT_TP2_REG, FT_TP3_REG, FT_TP4_REG, FT_TP5_REG};

//以下是无旋转版本
// void CST816_Scan(void)
// {
//     uint8_t sta = 0;
//     CST816_RD_Reg(0x02, &sta, 1);          // 读触摸状态
//     if (sta & 0x0F)                         // 有手指按下
//     {
//         uint8_t buf[6];
//         CST816_RD_Reg(FT_TP1_REG, buf, 6); // 读第一个触摸点
//         TPR_Structure.x[0] = buf[1];        // X 坐标
//         TPR_Structure.y[0] = buf[3];        // Y 坐标
//         TPR_Structure.TouchSta = TP_PRES_DOWN;
//     }
//     else                                    // 无手指
//     {
//         TPR_Structure.TouchSta = 0;
//         TPR_Structure.x[0] = 0;
//         TPR_Structure.y[0] = 0;
//     }
// }

//以下是顺时针旋转90°版本
void CST816_Scan(void)
{
    uint8_t sta = 0;
    CST816_RD_Reg(0x02, &sta, 1);          // 读触摸状态
    if (sta & 0x0F)                         // 有手指按下
    {
        uint8_t buf[6];
        CST816_RD_Reg(FT_TP1_REG, buf, 6); // 读第一个触摸点
        // 原始值
        uint16_t rawX = buf[1];
        uint16_t rawY = buf[3];
        // ===== 尝试 2：XY 互换 + Y 翻转（可能性最大）=====
        TPR_Structure.x[0] = rawY;
        TPR_Structure.y[0] = 239 - rawX;
        
        TPR_Structure.TouchSta = TP_PRES_DOWN;
    }
    else                                    // 无手指
    {
        TPR_Structure.TouchSta = 0;
        TPR_Structure.x[0] = 0;
        TPR_Structure.y[0] = 0;
    }
}