// PowerManager.c
#include "PowerManager.h"
#include "Backlight.h"
#include "ST7789.h"
#include "main.h"
#include "bsp_touch_cst816.h"
#include "BackLight.h"

#include "FreeRTOS.h"       
#include "semphr.h"         
extern SemaphoreHandle_t g_wakeupSemaphore;   

/**
 * @brief  进入 SLEEP 模式（息屏，等待 PA0 按键唤醒）
 *         - 关闭屏幕背光
 *         - CPU 暂停（WFI），外设继续运行
 *         - RTC 保持走时
 *         - 任意 EXTI 中断（如 PA0）唤醒
 *         - 唤醒后自动恢复背光，屏幕重新显示
 */
void EnterSleepMode(void)
{
    // ① 关背光
    Backlight_Off();
    //HAL_GPIO_WritePin(ST7789_PWR_GPIO_Port, ST7789_PWR_Pin, GPIO_PIN_SET);
    // ② 读取触摸数据，释放 CST816 的 INT 引脚
    CST816_Scan();
    // ③ 每 50ms 醒来检查一次，直到检测到触摸
    while (1)
    {
        // 阻塞 50ms（让 CPU 休眠，低功耗）
        vTaskDelay(pdMS_TO_TICKS(50));
        // 读 CST816 状态寄存器（只读 1 字节，不读完整数据）
        uint8_t sta = 0;
        CST816_RD_Reg(0x02, &sta, 1);
        if (sta & 0x0F)   // 有手指按下
        {
            break;        // 退出循环，唤醒
        }
    }
    // ④ 完整读取一次触摸数据，防止唤醒后误触
    CST816_Scan();
    // ⑨ 开背光
    //HAL_GPIO_WritePin(ST7789_PWR_GPIO_Port, ST7789_PWR_Pin, GPIO_PIN_RESET);
    Backlight_On();
}