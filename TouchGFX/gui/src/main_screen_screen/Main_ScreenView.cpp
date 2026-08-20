#include <gui/main_screen_screen/Main_ScreenView.hpp>

#ifdef __cplusplus
extern "C" {
#endif
void EnterSleepMode(void);
#ifdef __cplusplus
}
#endif

Main_ScreenView::Main_ScreenView()
{

}

void Main_ScreenView::setupScreen()
{
    Main_ScreenViewBase::setupScreen();
    textArea1.setWildcard(clockBuffer);
    uint8_t h, m, s;
    static_cast<Model*>(presenter->getModel())->getCurrentTime(h, m, s);
    touchgfx::Unicode::snprintf(clockBuffer, CLOCK_BUFFER_SIZE,
                                "%02d:%02d:%02d", h, m, s);
    textArea1.invalidate();
    

    textAreaDate.setWildcard(dateBuffer);
    uint8_t y, mo, d, wd;
    presenter->getModel()->getCurrentDate(y, mo, d, wd);
    touchgfx::Unicode::snprintf(dateBuffer, DATE_BUFFER_SIZE, "20%02d-%02d-%02d", y, mo, d);
    textAreaDate.invalidate();
}

void Main_ScreenView::tearDownScreen()
{
    Main_ScreenViewBase::tearDownScreen();
}

void Main_ScreenView::sleepHandler()
{
    // 在真机上会关闭屏幕进入待机，等待 PA0 按键唤醒
    EnterSleepMode();
    // 模拟器中可以测试时改为：
    // touchgfx_printf("Sleep button pressed!\n");
}

// ---- 新增：更新时钟实现 ----
void Main_ScreenView::updateClock(uint8_t hour, uint8_t minute, uint8_t second)
{
    touchgfx::Unicode::snprintf(clockBuffer, CLOCK_BUFFER_SIZE,
                                "%02d:%02d:%02d", hour, minute, second);
}

void Main_ScreenView::updateDate(uint8_t year, uint8_t month, uint8_t day)
{
    touchgfx::Unicode::snprintf(dateBuffer, DATE_BUFFER_SIZE, "20%02d-%02d-%02d", year, month, day);
    textAreaDate.invalidate();
}

void Main_ScreenView::handleTickEvent()
{
    presenter->handleTickEvent();
    // ===== 关键：主动 invalidate，保活渲染管线 =====
    textArea1.invalidate();
}