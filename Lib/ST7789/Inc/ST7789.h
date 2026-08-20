#ifndef __ST7789_H
#define __ST7789_H

#include "main.h"          // ← HAL 库和 CubeMX 生成的头文件
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif



/* ========== 屏幕尺寸（根据你的屏幕修改！）========== */
#define ST7789_WIDTH   240
#define ST7789_HEIGHT  240// 如果是 240×320 屏幕，改为 320

/* ========== 控制引脚（和 CubeMX 的 User Label 一致） ========== */
#define ST7789_DC_PIN     ST7789_DC_Pin
#define ST7789_DC_PORT    ST7789_DC_GPIO_Port
#define ST7789_RST_PIN    ST7789_RST_Pin
#define ST7789_RST_PORT   ST7789_RST_GPIO_Port

/* ========== 颜色定义（RGB565） ========== */
#define WHITE         0xFFFF
#define BLACK         0x0000
#define BLUE          0x001F
#define BRED          0xF81F
#define GRED          0xFFE0
#define GBLUE         0x07FF
#define RED           0xF800
#define MAGENTA       0xF81F
#define GREEN         0x07E0
#define CYAN          0x7FFF
#define YELLOW        0xFFE0
#define BROWN         0xBC40
#define BRRED         0xFC07
#define GRAY          0x8430

/* ========== 全局变量 ========== */
extern uint16_t BACK_COLOR;
extern uint16_t POINT_COLOR;

/* ========== API 函数声明 ========== */
void ST7789_Init(uint16_t Back_color, uint16_t Pen_color);
void ST7789_SetRotation(uint8_t direction);
void ST7789_Clear(uint16_t Color);
void ST7789_Cursor(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ST7789_DrawPoint(uint16_t x, uint16_t y);
void ST7789_DrawPoint_big(uint16_t x, uint16_t y);
void ST7789_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r);
//void ST7789_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode);
//void ST7789_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len);
//void ST7789_ShowString(uint16_t x, uint16_t y, char *p);
//void ST7789_Printf(uint16_t X, uint16_t Y, const char* format, ...);
void ST7789_ShowImage(uint16_t x, uint16_t y, 
                      uint16_t width, uint16_t height, 
                      const uint8_t *imageData);
void ST7789_DrawBitmap(uint16_t x, uint16_t y, 
                       uint16_t w, uint16_t h, 
                       const uint8_t *pixels);

//DMA相关
uint8_t ST7789_DMA_IsBusy(void);
void ST7789_DMA_FillScreen(uint16_t color);
void ST7789_DMA_DrawBitmap(uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h,
                       const uint8_t *pixels);
void ST7789_DrawBitmap_NoWait(uint16_t x, uint16_t y,
                               uint16_t w, uint16_t h,
                               const uint8_t *pixels);
                               
#ifdef __cplusplus
}
#endif


#endif