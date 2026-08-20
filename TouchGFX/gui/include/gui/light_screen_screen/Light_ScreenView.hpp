#ifndef LIGHT_SCREENVIEW_HPP
#define LIGHT_SCREENVIEW_HPP

#include <gui_generated/light_screen_screen/Light_ScreenViewBase.hpp>
#include <gui/light_screen_screen/Light_ScreenPresenter.hpp>

class Light_ScreenView : public Light_ScreenViewBase
{
public:
    Light_ScreenView();
    virtual ~Light_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    virtual void subtractLightFunction();
    virtual void addLightFunction();
private:
    uint8_t currentBrightness;
};

#endif // LIGHT_SCREENVIEW_HPP
