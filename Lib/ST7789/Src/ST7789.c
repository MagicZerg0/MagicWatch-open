
#include "ST7789.h"
#include "main.h"
#include "spi.h"
//#include "stm32u5xx.h"


/* ========== 全局变量 ========== */
uint16_t BACK_COLOR  = WHITE;
uint16_t POINT_COLOR = BLACK;

extern void DisplayDriver_TransferCompleteCallback(void);


//------------------------------------DMA相关的写在这里了------------------------------------
/* ================================================================
 * DMA 发送基础设施
 * ================================================================ */

/* DMA 忙标志（ISR 里清零，任务里轮询） */
static volatile uint8_t spi_dma_busy = 0;

/* 行缓冲区：240 像素 × 2 字节 = 480 字节，能装下 */
static uint8_t dma_line_buf[240 * 2];

/* 公开查询 */
uint8_t ST7789_DMA_IsBusy(void)
{
    return spi_dma_busy;
}

/* ── 内部：启动 DMA 发送 ── */
static void ST7789_SPI_WriteDMA(uint8_t *data, uint32_t size)
{
    //SCB_CleanDCache_by_Addr((uint32_t *)data, size);
    spi_dma_busy = 1;
    HAL_SPI_Transmit_DMA(&hspi1, data, size);
}

/* ── 内部：等 DMA 完成 ── */
static void ST7789_DMA_WaitDone(void)
{
    while (spi_dma_busy) {
        /* 空等；如果你的工程以后上了 RTOS，这里可以改成 taskYIELD() */
    }
}

/* ── HAL SPI DMA 完成回调（中断上下文） ── */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) {
        spi_dma_busy = 0;
        DisplayDriver_TransferCompleteCallback();
    }
}

/* ── HAL SPI DMA 出错回调 ── */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) {
        spi_dma_busy = 0;
    }
}

/**
 * ST7789_DMA_FillScreen —— DMA 逐行填充全屏纯色
 * 效果：整个屏幕瞬间变成 color 颜色
 * 参数：color = RGB565，如 RED, BLUE, WHITE
 */
void ST7789_DMA_FillScreen(uint16_t color)
{
    /* 准备一行数据 */
    for (uint16_t i = 0; i < ST7789_WIDTH; i++) {
        dma_line_buf[i * 2]     = color >> 8;      // 高字节
        dma_line_buf[i * 2 + 1] = color & 0xFF;    // 低字节
    }

    /* 设置全屏窗口 */
    ST7789_Cursor(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

    /* 逐行 DMA */
    for (uint16_t row = 0; row < ST7789_HEIGHT; row++) {
        ST7789_DMA_WaitDone();                    // etc. 上一行发完
        ST7789_SPI_WriteDMA(dma_line_buf, ST7789_WIDTH * 2);
    }
    ST7789_DMA_WaitDone();  // 等最后一行
}

/* ================================================================
 * DrawBitmap —— DMA 逐行版本（给 TouchGFX）
 * 内部逐行交换字节序 + DMA 发送，最后等全部完成才返回
 * ================================================================ */
void ST7789_DMA_DrawBitmap(uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h,
                       const uint8_t *pixels)
{
    if (w == 0 || h == 0) return;
    uint16_t row_bytes = w * 2;  // 一行 = 宽 × 2 字节（RGB565）
    /* 1. 设置显示窗口 */
    ST7789_Cursor(x, y, x + w - 1, y + h - 1);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    /* 2. 逐行处理 */
    for (uint16_t row = 0; row < h; row++)
    {
        const uint8_t *src = pixels + row * row_bytes;
        /* 字节序交换：TouchGFX 小端序 → ST7789 大端序 */
        for (uint16_t col = 0; col < row_bytes; col += 2)
        {
            dma_line_buf[col]     = src[col + 1];  // 高字节在前
            dma_line_buf[col + 1] = src[col];
        }
        /* 等待上一行 DMA 完成，然后发下一行 */
        ST7789_DMA_WaitDone();
        ST7789_SPI_WriteDMA(dma_line_buf, row_bytes);
    }
    /* 3. 等最后一行发完 */
    ST7789_DMA_WaitDone();
}


/* ========== DMA 块缓冲区（20000 字节，刚好装下一个 Block） ========== */
static uint8_t dma_block_buf[20000];  

/**
 * ST7789_DrawBitmap_NoWait
 * 
 * 将像素数据字节序交换后拷贝到 dma_block_buf，
 * 设置 ST7789 窗口，启动 DMA，立刻返回。
 * 最大支持 w*h*2 ≤ 4800 字节。
 */
void ST7789_DrawBitmap_NoWait(uint16_t x, uint16_t y,
                               uint16_t w, uint16_t h,
                               const uint8_t *pixels)
{
    if (w == 0 || h == 0) return;

    uint32_t total = (uint32_t)w * h * 2;

    /* 安全检查 */
    if (total > sizeof(dma_block_buf))
    {
        while (1);  // 块太大，调小 TouchGFX Block size
    }

    /* 1. 字节序交换 + 拷贝：TouchGFX 小端 → ST7789 大端 */
    for (uint32_t i = 0; i < total; i += 2)
    {
        dma_block_buf[i]     = pixels[i + 1];
        dma_block_buf[i + 1] = pixels[i];
    }

    /* 2. 设置窗口 */
    ST7789_Cursor(x, y, x + w - 1, y + h - 1);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

    /* 3. 启动 DMA，立刻返回 */
    spi_dma_busy = 1;
    HAL_SPI_Transmit_DMA(&hspi1, dma_block_buf, total);
}

//---------------------------------------------------------------------------------------



/* ========== 内部辅助：SPI 发送一个字节 ========== */
static void ST7789_SPI_WriteByte(uint8_t byte)
{
    HAL_SPI_Transmit(&hspi1, &byte, 1, HAL_MAX_DELAY);
}

/* ========== 内部辅助：发送命令（DC=0） ========== */
static void ST7789_SendCmd(uint8_t cmd)
{
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);  // DC = 0
    ST7789_SPI_WriteByte(cmd);
}

/* ========== 内部辅助：发送数据（DC=1） ========== */
static void ST7789_SendData8(uint8_t data)
{
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);    // DC = 1
    ST7789_SPI_WriteByte(data);
}

/* ========== 内部辅助：发送 16 位数据（RGB565 像素） ========== */
static void ST7789_SendData16(uint16_t data)
{
    uint8_t buf[2];
    buf[0] = data >> 8;      // 高字节在前（MSB）
    buf[1] = data & 0xFF;
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
}

/* ================================================================
 * 设置光标窗口（告诉 ST7789 接下来的像素数据画在哪个矩形区域）
 * ================================================================ */
void ST7789_Cursor(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    ST7789_SendCmd(0x2A);           // 列地址设置
    ST7789_SendData16(x1);
    ST7789_SendData16(x2);

    ST7789_SendCmd(0x2B);           // 行地址设置
    ST7789_SendData16(y1);
    ST7789_SendData16(y2);

    ST7789_SendCmd(0x2C);           // 内存写入命令
}

/* ================================================================
 * ST7789 初始化
 * ================================================================ */
void ST7789_Init(uint16_t Back_color, uint16_t Pen_color)
{
    BACK_COLOR  = Back_color;
    POINT_COLOR = Pen_color;

    /* -------- 硬件复位 -------- */
    HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(120);

    /* -------- 初始化序列（来自原代码，不要改动） -------- */

    ST7789_SendCmd(0x36);
    ST7789_SendData8(0x00);         // MADCTL：显示方向

    ST7789_SendCmd(0x3A);
    ST7789_SendData8(0x05);         // 像素格式：16位（RGB565）

    ST7789_SendCmd(0xB2);           // 帧速率控制
    ST7789_SendData8(0x0C);
    ST7789_SendData8(0x0C);
    ST7789_SendData8(0x00);
    ST7789_SendData8(0x33);
    ST7789_SendData8(0x33);

    ST7789_SendCmd(0xB7);
    ST7789_SendData8(0x35);

    ST7789_SendCmd(0xBB);
    ST7789_SendData8(0x19);

    ST7789_SendCmd(0xC0);
    ST7789_SendData8(0x2C);

    ST7789_SendCmd(0xC2);
    ST7789_SendData8(0x01);

    ST7789_SendCmd(0xC3);
    ST7789_SendData8(0x12);

    ST7789_SendCmd(0xC4);
    ST7789_SendData8(0x20);

    ST7789_SendCmd(0xC6);
    ST7789_SendData8(0x0F);

    ST7789_SendCmd(0xD0);
    ST7789_SendData8(0xA4);
    ST7789_SendData8(0xA1);

    /* 正电压 Gamma 校正 */
    ST7789_SendCmd(0xE0);
    ST7789_SendData8(0xD0);
    ST7789_SendData8(0x04);
    ST7789_SendData8(0x0D);
    ST7789_SendData8(0x11);
    ST7789_SendData8(0x13);
    ST7789_SendData8(0x2B);
    ST7789_SendData8(0x3F);
    ST7789_SendData8(0x54);
    ST7789_SendData8(0x4C);
    ST7789_SendData8(0x18);
    ST7789_SendData8(0x0D);
    ST7789_SendData8(0x0B);
    ST7789_SendData8(0x1F);
    ST7789_SendData8(0x23);

    /* 负电压 Gamma 校正 */
    ST7789_SendCmd(0xE1);
    ST7789_SendData8(0xD0);
    ST7789_SendData8(0x04);
    ST7789_SendData8(0x0C);
    ST7789_SendData8(0x11);
    ST7789_SendData8(0x13);
    ST7789_SendData8(0x2C);
    ST7789_SendData8(0x3F);
    ST7789_SendData8(0x44);
    ST7789_SendData8(0x51);
    ST7789_SendData8(0x2F);
    ST7789_SendData8(0x1F);
    ST7789_SendData8(0x1F);
    ST7789_SendData8(0x20);
    ST7789_SendData8(0x23);

    ST7789_SendCmd(0x21);           // 显示反转（部分屏幕需要）

    ST7789_SendCmd(0x11);           // 退出睡眠
    HAL_Delay(120);

    ST7789_SendCmd(0x29);           // 开启显示
    HAL_Delay(20);
}

/* ================================================================
 * 屏幕旋转
 * ================================================================ */
void ST7789_SetRotation(uint8_t direction)
{
    ST7789_SendCmd(0x36);
    switch (direction)
    {
    case 0: ST7789_SendData8(0x00); break;
    case 1: ST7789_SendData8(0xA0); break;
    case 2: ST7789_SendData8(0xC0); break;
    case 3: ST7789_SendData8(0x60); break;
    default: break;
    }
}

/* ================================================================
 * 清屏（纯色填充整个屏幕）
 * ================================================================ */
void ST7789_Clear(uint16_t Color)
{
    uint16_t i, j;
    ST7789_Cursor(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    for (i = 0; i < ST7789_WIDTH; i++)
    {
        for (j = 0; j < ST7789_HEIGHT; j++)
        {
            ST7789_SendData16(Color);
        }
    }
}

/* ================================================================
 * 区域填充（纯色）
 * ================================================================ */
void ST7789_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t width  = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;
    uint32_t total  = (uint32_t)width * height;

    ST7789_Cursor(x1, y1, x2, y2);

    for (uint32_t i = 0; i < total; i++)
    {
        ST7789_SendData16(color);
    }
}

/* ================================================================
 * 画点
 * ================================================================ */
void ST7789_DrawPoint(uint16_t x, uint16_t y)
{
    ST7789_Cursor(x, y, x, y);
    ST7789_SendData16(POINT_COLOR);
}

void ST7789_DrawPoint_big(uint16_t x, uint16_t y)
{
    ST7789_Fill(x - 1, y - 1, x + 1, y + 1, POINT_COLOR);
}

/* ================================================================
 * Bresenham 画线
 * ================================================================ */
void ST7789_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    int16_t delta_x = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int16_t delta_y = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int16_t incx = (x2 > x1) ? 1 : ((x2 == x1) ? 0 : -1);
    int16_t incy = (y2 > y1) ? 1 : ((y2 == y1) ? 0 : -1);
    int16_t xerr = 0, yerr = 0;
    int16_t distance = (delta_x > delta_y) ? delta_x : delta_y;
    int16_t uRow = x1, uCol = y1;

    for (int16_t t = 0; t <= distance + 1; t++)
    {
        ST7789_DrawPoint(uRow, uCol);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) { xerr -= distance; uRow += incx; }
        if (yerr > distance) { yerr -= distance; uCol += incy; }
    }
}

/* ================================================================
 * 画矩形
 * ================================================================ */
void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    ST7789_DrawLine(x1, y1, x2, y1);
    ST7789_DrawLine(x1, y1, x1, y2);
    ST7789_DrawLine(x1, y2, x2, y2);
    ST7789_DrawLine(x2, y1, x2, y2);
}

/* ================================================================
 * Bresenham 画圆
 * ================================================================ */
void ST7789_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r)
{
    int16_t a = 0, b = r;
    int16_t di = 3 - (r << 1);

    while (a <= b)
    {
        ST7789_DrawPoint(x0 - b, y0 - a);
        ST7789_DrawPoint(x0 + b, y0 - a);
        ST7789_DrawPoint(x0 - a, y0 + b);
        ST7789_DrawPoint(x0 - b, y0 - a);
        ST7789_DrawPoint(x0 - a, y0 - b);
        ST7789_DrawPoint(x0 + b, y0 + a);
        ST7789_DrawPoint(x0 + a, y0 - b);
        ST7789_DrawPoint(x0 + a, y0 + b);
        ST7789_DrawPoint(x0 - b, y0 + a);

        a++;
        if (di < 0)
            di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
        ST7789_DrawPoint(x0 + a, y0 + b);
    }
}

// /* ================================================================
//  * 批量像素传输（给 TouchGFX 用）
//  * 设置窗口后，一次性把整个像素块通过 SPI 发出去
//  * ================================================================ */
// void ST7789_DrawBitmap(uint16_t x, uint16_t y, 
//                        uint16_t w, uint16_t h, 
//                        const uint8_t *pixels)
// {
//     uint32_t total = (uint32_t)w * h * 2;   // RGB565 = 每像素 2 字节
//     ST7789_Cursor(x, y, x + w - 1, y + h - 1);  // 设置窗口
//     HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);  // DC=1
//     // 一次性阻塞发送整个像素块
//     HAL_SPI_Transmit(&hspi1, (uint8_t *)pixels, total, HAL_MAX_DELAY);
// }

//以下版本为正确
void ST7789_DrawBitmap(uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h,
                       const uint8_t *pixels)
{
    uint32_t total = (uint32_t)w * h * 2;
    ST7789_Cursor(x, y, x + w - 1, y + h - 1);
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    // 临时修复：交换每两个字节（因为 TouchGFX 小端序 vs ST7789 大端序）
    for (uint32_t i = 0; i < total; i += 2) {
        uint8_t swapped[2];
        swapped[0] = pixels[i + 1];  // TouchGFX 的低字节 → 先发（ST7789 的高字节）
        swapped[1] = pixels[i];      // TouchGFX 的高字节 → 后发（ST7789 的低字节）
        HAL_SPI_Transmit(&hspi1, swapped, 2, HAL_MAX_DELAY);
    }
}

/* ================================================================
 * 显示 ASCII 字符（8×16 字体，需要 asc2_1608 字模数组）
 * ================================================================
 * 注意：原代码依赖 oledfont.h 中的 asc2_1608[] 数组。
 * 如果你没有这个文件，可以先用下面的简化版本，或者从网上找一份
 * 8×16 ASCII 字模数据。
 */

// ⚠️ 如果你有 oledfont.h，取消下面这行的注释：
// #include "oledfont.h"
//
// 如果你没有，定义一个空的占位符（字符将不显示）：
// #ifndef asc2_1608
// // 这里放你的字模数据，格式：每个字符 16 字节（8×16 像素）
// // 暂时用空数组占位，实际使用时请替换
// static const uint8_t asc2_1608[95][16] = {0};  // 95 个可打印 ASCII 字符
// #endif

// void ST7789_ShowChar(uint16_t x, uint16_t y, uint8_t ch, uint8_t mode)
// {
//     uint8_t temp, t;
//     uint16_t x0 = x;
//     uint16_t colorTemp = POINT_COLOR;

//     if (x > ST7789_WIDTH - 16 || y > ST7789_HEIGHT - 16) return;

//     ch = ch - ' ';  // 偏移到字模索引

//     ST7789_Cursor(x, y, x + 7, y + 15);

//     for (uint8_t pos = 0; pos < 16; pos++)
//     {
//         temp = asc2_1608[(uint16_t)ch * 16 + pos];
//         for (t = 0; t < 8; t++)
//         {
//             if (temp & 0x01)
//                 POINT_COLOR = colorTemp;
//             else
//                 POINT_COLOR = BACK_COLOR;

//             ST7789_SendData16(POINT_COLOR);
//             temp >>= 1;
//             x++;
//         }
//         x = x0;
//         y++;
//     }

//     POINT_COLOR = colorTemp;
// }

/* ================================================================
 * 显示字符串
 * ================================================================ */
// void ST7789_ShowString(uint16_t x, uint16_t y, char *str)
// {
//     while (*str != '\0')
//     {
//         if (x > ST7789_WIDTH - 16) { x = 0; y += 16; }
//         if (y > ST7789_HEIGHT - 16) { y = x = 0; ST7789_Clear(RED); }

//         ST7789_ShowChar(x, y, *str, 0);
//         x += 8;
//         str++;
//     }
// }

/* ================================================================
 * 格式化打印（类似 printf）
 * ================================================================ */
// void ST7789_Printf(uint16_t X, uint16_t Y, const char* format, ...)
// {
//     char String[256];
//     va_list arg;
//     va_start(arg, format);
//     vsprintf(String, format, arg);
//     va_end(arg);
//     ST7789_ShowString(X, Y, String);
// }

/* ================================================================
 * m 的 n 次方（用于 ShowNum）
 * ================================================================ */
// static uint32_t mypow(uint8_t m, uint8_t n)
// {
//     uint32_t result = 1;
//     while (n--) result *= m;
//     return result;
// }

// /* ================================================================
//  * 显示数字
//  * ================================================================ */
// void ST7789_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len)
// {
//     uint8_t t, temp;
//     uint8_t enshow = 0;

//     for (t = 0; t < len; t++)
//     {
//         temp = (num / mypow(10, len - t - 1)) % 10;
//         if (enshow == 0 && t < (len - 1))
//         {
//             if (temp == 0)
//             {
//                 ST7789_ShowChar(x + 8 * t, y, ' ', 0);
//                 continue;
//             }
//             else enshow = 1;
//         }
//         ST7789_ShowChar(x + 8 * t, y, temp + 48, 0);
//     }
// }

/* ================================================================
 * 显示图片（数据存储在 Flash 中）
 * ================================================================ */
void ST7789_ShowImage(uint16_t x, uint16_t y,
                      uint16_t width, uint16_t height,
                      const uint8_t *imageData)
{
    ST7789_Cursor(x, y, x + width - 1, y + height - 1);

    // DC = 1（数据模式）
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

    // 一次性发送所有像素数据（DMA 或阻塞模式）
    HAL_SPI_Transmit(&hspi1,
                     (uint8_t*)imageData,
                     (uint32_t)width * height * 2,
                     HAL_MAX_DELAY);
}