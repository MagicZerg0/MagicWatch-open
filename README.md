# MagicWatch 智能手表

基于 **STM32U585 + TouchGFX** 的智能羽毛球运动检测手表，全国大学生嵌入式芯片与系统竞赛获奖项目。

## 功能

- 心率 / 血氧检测（MAX30102 传感器）
- 多种运动模式（反手挑球、正手跳、杀球、正手高远等）
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
```

## 烧录

建议连接 ST-Link 后，使用 STM32 官方 VSCode 插件在 VSCode 中烧录；或者使用命令行：

```powershell
STM32_Programmer_CLI -c port=SWD mode=UR -w build/Debug/u585_newscreen_test.elf -v -rst
```

## 目录结构

- `Core/` — 主逻辑、FreeRTOS 任务
- `Lib/` — 外设驱动（ST7789、CST816、MAX30102）
- `TouchGFX/` — GUI 界面（assets 图片字体、gui 源码）
- `Middlewares/` — TouchGFX 框架、FreeRTOS

## 许可证

MIT License（第三方组件遵循各自许可：TouchGFX 为 ST 许可、FreeRTOS 为 MIT、HAL 库为 BSD-3-Clause）

## 致谢

请给薯条面子

---

## English Version

# MagicWatch Smart Watch

A smart badminton motion-detection watch based on **STM32U585 + TouchGFX**. Award-winning project of the National College Embedded Chip and System Design Competition.

## Features

- Heart rate / SpO2 detection (MAX30102 sensor)
- Multiple badminton motion modes (backhand lift, forehand jump smash, shuttle, forehand clear, etc.)
- 240x240 color display (ST7789) + touch interaction (CST816)
- Clock, brightness, screen-off and other basic functions

## Hardware

| Module | Model |
|--------|-------|
| MCU | STM32U585 |
| Display | ST7789 240x240 |
| Touch | CST816 |
| Heart rate sensor | MAX30102 |

## Build

Requires [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html) (cmake / ninja / arm-none-eabi-gcc). Run in the project root:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

## Flash

Connect an ST-Link, then either use the official STM32 VS Code extension to flash, or run:

```powershell
STM32_Programmer_CLI -c port=SWD mode=UR -w build/Debug/u585_newscreen_test.elf -v -rst
```

## Directory Structure

- `Core/` — main logic, FreeRTOS tasks
- `Lib/` — peripheral drivers (ST7789, CST816, MAX30102)
- `TouchGFX/` — GUI (assets, gui source)
- `Middlewares/` — TouchGFX framework, FreeRTOS

## License

MIT License (third-party components under their own licenses)

## Acknowledgments

Special thanks to our teammate Fries (薯条) — respect to Fries!
