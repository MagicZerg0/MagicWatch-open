# 一键烧录脚本：u585_newscreen_test (STM32U585)
# 用法：
#   1. 用 USB 连接 ST-Link 到电脑，ST-Link 的 SWD 三根线(或排线)接到开发板
#   2. 在项目目录运行:  powershell -ExecutionPolicy Bypass -File flash.ps1
#   3. 如果报错 "No ST-Link detected"，检查 ST-Link 的 USB 连接和 SWD 接线

$ErrorActionPreference = "Stop"
$elf = "build\Debug\u585_newscreen_test.elf"
$cli = "STM32_Programmer_CLI"

if (-not (Test-Path $elf)) {
    Write-Host "[错误] 找不到 $elf ，请先编译 (cmake --build --preset Debug)" -ForegroundColor Red
    exit 1
}

Write-Host "===== 烧录 $elf =====" -ForegroundColor Cyan

# -c port=SWD mode=UR : SWD 接口，热复位模式(不先复位芯片，防止芯片被锁)
# -w <elf>            : 写入固件
# -v                  : 烧录后校验
# -rst                : 烧录完成后复位运行
& $cli -c port=SWD mode=UR -w $elf -v -rst

if ($LASTEXITCODE -eq 0) {
    Write-Host "===== 烧录成功！板子已复位运行 =====" -ForegroundColor Green
} else {
    Write-Host "===== 烧录失败，请检查 ST-Link 连接 =====" -ForegroundColor Red
}
