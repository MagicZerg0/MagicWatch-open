#include <gui/main_screen_screen/Main_ScreenView.hpp>
#include <gui/main_screen_screen/Main_ScreenPresenter.hpp>
#include "cmsis_os2.h"                 // ← 改为 CMSIS RTOS

Main_ScreenPresenter::Main_ScreenPresenter(Main_ScreenView& v)
    : view(v)
{

}

void Main_ScreenPresenter::activate()
{

}

void Main_ScreenPresenter::deactivate()
{

}

// ===== 新增 =====
void Main_ScreenPresenter::handleTickEvent()
{
    uint8_t hour, minute, second;
    static_cast<Model*>(model)->getCurrentTime(hour, minute, second);
    static uint8_t lastSecond = 0xFF;
    if (second != lastSecond) {
        lastSecond = second;
        view.updateClock(hour, minute, second);

        if (hour == 0 && minute == 0 && second == 0) {
            uint8_t year, month, day, wd;
            static_cast<Model*>(model)->getCurrentDate(year, month, day, wd);
            view.updateDate(year, month, day);
        }
    }
}