# MagicWatch 智能手表

基于 **STM32U585 + TouchGFX** 的智能羽毛球运动检测手表，全国大学生嵌入式芯片与系统竞赛获奖项目。

## 功能

- 心率 / 血氧检测（MAX30102 传感器）
- 多种运动模式（【反手挑球、正手跳、沙球、正手高远】等）
- 240x240 彩色屏幕（ST7789）+ 触摸交互（CST816）
- 时钟、亮度、息屏等基本功能

## 硬件

| 模块 | 型号 |
|------|------|
| MCU | STM32U585 |
| 屏幕 | ST7789 240x240 |
| 触摸 | CST816 |
| 心率传感器 | MAX30102 |
| 【其他】 | 【型号】 |

## 构建

需要 [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html)（含 cmake / ninja / arm-none-eabi-gcc）：

```powershell
cmake --preset Debug
cmake --build --preset Debug
