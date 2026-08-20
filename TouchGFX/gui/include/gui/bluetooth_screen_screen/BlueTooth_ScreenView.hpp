#ifndef BLUETOOTH_SCREENVIEW_HPP
#define BLUETOOTH_SCREENVIEW_HPP

#include <gui_generated/bluetooth_screen_screen/BlueTooth_ScreenViewBase.hpp>
#include <gui/bluetooth_screen_screen/BlueTooth_ScreenPresenter.hpp>

class BlueTooth_ScreenView : public BlueTooth_ScreenViewBase
{
public:
    BlueTooth_ScreenView();
    virtual ~BlueTooth_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void togglefunction() override;
protected:
};

#endif // BLUETOOTH_SCREENVIEW_HPP
