#ifndef MAIN_SCREENVIEW_HPP
#define MAIN_SCREENVIEW_HPP

#include <gui_generated/main_screen_screen/Main_ScreenViewBase.hpp>
#include <gui/main_screen_screen/Main_ScreenPresenter.hpp>
#include <touchgfx/Unicode.hpp>   // ← 新增，使用 snprintf 需要

class Main_ScreenView : public Main_ScreenViewBase
{
public:
    Main_ScreenView();
    virtual ~Main_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    
    virtual void handleTickEvent() override; 
    void updateClock(uint8_t hour, uint8_t minute, uint8_t second);
    void updateDate(uint8_t year, uint8_t month, uint8_t day);
protected:
    virtual void sleepHandler();

    static constexpr int CLOCK_BUFFER_SIZE = 9;
    touchgfx::Unicode::UnicodeChar clockBuffer[CLOCK_BUFFER_SIZE];

    static constexpr int DATE_BUFFER_SIZE = 11;  // "2025-01-15\0"
    touchgfx::Unicode::UnicodeChar dateBuffer[DATE_BUFFER_SIZE];
};

#endif // MAIN_SCREENVIEW_HPP
