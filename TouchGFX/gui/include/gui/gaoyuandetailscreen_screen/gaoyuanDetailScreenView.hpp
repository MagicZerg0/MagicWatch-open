#ifndef GAOYUANDETAILSCREENVIEW_HPP
#define GAOYUANDETAILSCREENVIEW_HPP

#include <gui_generated/gaoyuandetailscreen_screen/gaoyuanDetailScreenViewBase.hpp>
#include <gui/gaoyuandetailscreen_screen/gaoyuanDetailScreenPresenter.hpp>

class gaoyuanDetailScreenView : public gaoyuanDetailScreenViewBase
{
public:
    gaoyuanDetailScreenView();
    virtual ~gaoyuanDetailScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // GAOYUANDETAILSCREENVIEW_HPP