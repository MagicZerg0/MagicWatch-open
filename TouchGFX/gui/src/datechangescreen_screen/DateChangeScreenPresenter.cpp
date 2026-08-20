#include <gui/datechangescreen_screen/DateChangeScreenView.hpp>
#include <gui/datechangescreen_screen/DateChangeScreenPresenter.hpp>
#include <gui/model/Model.hpp>

DateChangeScreenPresenter::DateChangeScreenPresenter(DateChangeScreenView& v)
    : view(v)
{

}

void DateChangeScreenPresenter::activate()
{

}

void DateChangeScreenPresenter::deactivate()
{

}

void DateChangeScreenPresenter::addMinute()
{
    static_cast<Model*>(model)->addOneMinute();
    uint8_t h, m, s;
    static_cast<Model*>(model)->getCurrentTime(h, m, s);
    view.updateClock(h, m, s);
}
void DateChangeScreenPresenter::subMinute()
{
    static_cast<Model*>(model)->subOneMinute();
    uint8_t h, m, s;
    static_cast<Model*>(model)->getCurrentTime(h, m, s);
    view.updateClock(h, m, s);
}
void DateChangeScreenPresenter::addHour()
{
    static_cast<Model*>(model)->addOneHour();
    uint8_t h, m, s;
    static_cast<Model*>(model)->getCurrentTime(h, m, s);
    view.updateClock(h, m, s);
}
void DateChangeScreenPresenter::subHour()
{
    static_cast<Model*>(model)->subOneHour();
    uint8_t h, m, s;
    static_cast<Model*>(model)->getCurrentTime(h, m, s);
    view.updateClock(h, m, s);
}
void DateChangeScreenPresenter::addDate()
{
    static_cast<Model*>(model)->addOneDay();
    uint8_t y, mo, d, wd;
    static_cast<Model*>(model)->getCurrentDate(y, mo, d, wd);
    view.updateDate(y, mo, d);
}
void DateChangeScreenPresenter::subDate()
{
    static_cast<Model*>(model)->subOneDay();
    uint8_t y, mo, d, wd;
    static_cast<Model*>(model)->getCurrentDate(y, mo, d, wd);
    view.updateDate(y, mo, d);
}