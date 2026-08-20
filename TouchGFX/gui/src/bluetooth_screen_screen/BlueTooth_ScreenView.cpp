#include <gui/bluetooth_screen_screen/BlueTooth_ScreenView.hpp>
#include "main.h"

// UART 句柄（CubeMX 生成在 usart.c / main.c 中，需 extern 引入）
extern UART_HandleTypeDef huart1;

BlueTooth_ScreenView::BlueTooth_ScreenView()
{
}

void BlueTooth_ScreenView::setupScreen()
{
    BlueTooth_ScreenViewBase::setupScreen();
}

void BlueTooth_ScreenView::tearDownScreen()
{
    BlueTooth_ScreenViewBase::tearDownScreen();
}

void BlueTooth_ScreenView::togglefunction() {
    bool isOn = bluetoothToggleButton.getState();
    if (isOn) {
        __HAL_UART_ENABLE(&huart1);
    } else {
        __HAL_UART_DISABLE(&huart1);
    }
}