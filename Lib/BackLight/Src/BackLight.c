// Backlight.c
#include "Backlight.h"

extern TIM_HandleTypeDef htim2;

static uint8_t currentLevel = 9;
static uint8_t savedLevel  = 9;

static const uint16_t ccrTable[10] = {
     50,    // 0 档：微亮（不会唤不醒）
    155,    // 1 档
    260,    // 2 档
    365,    // 3 档
    470,    // 4 档
    575,    // 5 档
    680,    // 6 档
    785,    // 7 档
    890,    // 8 档
    999     // 9 档：最亮
};

void Backlight_Init(void)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ccrTable[currentLevel]);
}

void Backlight_SetLevel(uint8_t level)
{
    if (level > BACKLIGHT_LEVEL_MAX) level = BACKLIGHT_LEVEL_MAX;
    currentLevel = level;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ccrTable[level]);
}

uint8_t Backlight_GetLevel(void)
{
    return currentLevel;
}

void Backlight_Off(void)
{
    savedLevel = currentLevel;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);  // CCR=0 → 灭
}

void Backlight_On(void)
{
    currentLevel = savedLevel;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ccrTable[savedLevel]);
}