#include <gui/datechangescreen_screen/DateChangeScreenView.hpp>

DateChangeScreenView::DateChangeScreenView()
{

}

void DateChangeScreenView::setupScreen()
{
    DateChangeScreenViewBase::setupScreen();

    textArea1.setWildcard(clockBuffer);
    textAreaDate.setWildcard(dateBuffer);
    uint8_t h, m, s;
    presenter->getModel()->getCurrentTime(h, m, s);
    touchgfx::Unicode::snprintf(clockBuffer, CLOCK_BUFFER_SIZE, "%02d:%02d:%02d", h, m, s);
    textArea1.invalidate();

    uint8_t year, month, day, wd;
    presenter->getModel()->getCurrentDate(year, month, day, wd);
    touchgfx::Unicode::snprintf(dateBuffer, DATE_BUFFER_SIZE, "%04d-%02d-%02d", 2000 + year, month, day);
    textAreaDate.invalidate();
}

void DateChangeScreenView::tearDownScreen()
{
    DateChangeScreenViewBase::tearDownScreen();
}

void DateChangeScreenView::addMinfunction()
{
    presenter->addMinute();
}
void DateChangeScreenView::subMinfunction()
{
    presenter->subMinute();
}
void DateChangeScreenView::addHourfunction()
{
    presenter->addHour();
}
void DateChangeScreenView::subHourfunction()
{
    presenter->subHour();
}
void DateChangeScreenView::addDatefunction()
{
    presenter->addDate();
}
void DateChangeScreenView::subDatefunction()
{
    presenter->subDate();
}
void DateChangeScreenView::updateClock(uint8_t hour, uint8_t minute, uint8_t second)
{
    touchgfx::Unicode::snprintf(clockBuffer, CLOCK_BUFFER_SIZE,
                                "%02d:%02d:%02d", hour, minute, second);
    textArea1.invalidate();
}
void DateChangeScreenView::updateDate(uint8_t year, uint8_t month, uint8_t day)
{
    touchgfx::Unicode::snprintf(dateBuffer, DATE_BUFFER_SIZE,
                                "%04d-%02d-%02d", 2000 + year, month, day);
    textAreaDate.invalidate();
}