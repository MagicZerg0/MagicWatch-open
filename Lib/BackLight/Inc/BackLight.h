#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BACKLIGHT_LEVEL_MAX   9     // ← 从 3 改为 9
#define BACKLIGHT_LEVEL_MIN   0

void Backlight_Init(void);
void Backlight_SetLevel(uint8_t level);
uint8_t Backlight_GetLevel(void);
void Backlight_Off(void);
void Backlight_On(void);

#ifdef __cplusplus
}
#endif

#endif