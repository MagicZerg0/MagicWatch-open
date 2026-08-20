#ifndef DATECHANGESCREENVIEW_HPP
#define DATECHANGESCREENVIEW_HPP

#include <gui_generated/datechangescreen_screen/DateChangeScreenViewBase.hpp>
#include <gui/datechangescreen_screen/DateChangeScreenPresenter.hpp>
#include <touchgfx/Unicode.hpp>

class DateChangeScreenView : public DateChangeScreenViewBase
{
public:
    DateChangeScreenView();
    virtual ~DateChangeScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void addMinfunction() override;
    virtual void subMinfunction() override;
    virtual void addHourfunction() override;
    virtual void subHourfunction() override;
    virtual void addDatefunction() override;
    virtual void subDatefunction() override;
    void updateClock(uint8_t hour, uint8_t minute, uint8_t second);
    void updateDate(uint8_t year, uint8_t month, uint8_t day);
protected:
    static constexpr int CLOCK_BUFFER_SIZE = 9;
    touchgfx::Unicode::UnicodeChar clockBuffer[CLOCK_BUFFER_SIZE];
    static constexpr int DATE_BUFFER_SIZE = 11;
    touchgfx::Unicode::UnicodeChar dateBuffer[DATE_BUFFER_SIZE];
};

#endif // DATECHANGESCREENVIEW_HPP