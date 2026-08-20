#include <gui/light_screen_screen/Light_ScreenView.hpp>

#ifdef __cplusplus
extern "C" {
#endif
void Backlight_SetLevel(uint8_t level);
uint8_t Backlight_GetLevel(void);
#ifdef __cplusplus
}
#endif


Light_ScreenView::Light_ScreenView()
    : currentBrightness(3)
{
    
}

void Light_ScreenView::setupScreen()
{
    Light_ScreenViewBase::setupScreen();
    // 从硬件读取当前亮度档位
    currentBrightness = Backlight_GetLevel();
}

void Light_ScreenView::subtractLightFunction()
{
    if (currentBrightness > 0)     // 最低到0档（微亮，不是全黑）
    {
        currentBrightness--;
        Backlight_SetLevel(currentBrightness);
    }
}
void Light_ScreenView::addLightFunction()
{
    if (currentBrightness < 9)     // ← 从 3 改为 9
    {
        currentBrightness++;
        Backlight_SetLevel(currentBrightness);
    }
}

void Light_ScreenView::tearDownScreen()
{
    Light_ScreenViewBase::tearDownScreen();
}
