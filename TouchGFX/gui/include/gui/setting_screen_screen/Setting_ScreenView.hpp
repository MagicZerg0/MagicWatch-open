#ifndef SETTING_SCREENVIEW_HPP
#define SETTING_SCREENVIEW_HPP

#include <gui_generated/setting_screen_screen/Setting_ScreenViewBase.hpp>
#include <gui/setting_screen_screen/Setting_ScreenPresenter.hpp>

class Setting_ScreenView : public Setting_ScreenViewBase
{
public:
    Setting_ScreenView();
    virtual ~Setting_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // SETTING_SCREENVIEW_HPP
