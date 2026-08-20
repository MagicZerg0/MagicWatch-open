#ifndef SHAQIUDETAILSCREENVIEW_HPP
#define SHAQIUDETAILSCREENVIEW_HPP

#include <gui_generated/shaqiudetailscreen_screen/shaqiuDetailScreenViewBase.hpp>
#include <gui/shaqiudetailscreen_screen/shaqiuDetailScreenPresenter.hpp>

class shaqiuDetailScreenView : public shaqiuDetailScreenViewBase
{
public:
    shaqiuDetailScreenView();
    virtual ~shaqiuDetailScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // SHAQIUDETAILSCREENVIEW_HPP