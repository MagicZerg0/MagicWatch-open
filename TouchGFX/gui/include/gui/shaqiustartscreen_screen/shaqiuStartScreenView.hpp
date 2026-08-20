#ifndef SHAQIUSTARTSCREENVIEW_HPP
#define SHAQIUSTARTSCREENVIEW_HPP

#include <gui_generated/shaqiustartscreen_screen/shaqiuStartScreenViewBase.hpp>
#include <gui/shaqiustartscreen_screen/shaqiuStartScreenPresenter.hpp>

class shaqiuStartScreenView : public shaqiuStartScreenViewBase
{
public:
    shaqiuStartScreenView();
    virtual ~shaqiuStartScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void onStartClicked() override;
protected:
};

#endif // SHAQIUSTARTSCREENVIEW_HPP