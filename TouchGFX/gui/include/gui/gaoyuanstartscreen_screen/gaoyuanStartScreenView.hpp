#ifndef GAOYUANSTARTSCREENVIEW_HPP
#define GAOYUANSTARTSCREENVIEW_HPP

#include <gui_generated/gaoyuanstartscreen_screen/gaoyuanStartScreenViewBase.hpp>
#include <gui/gaoyuanstartscreen_screen/gaoyuanStartScreenPresenter.hpp>

class gaoyuanStartScreenView : public gaoyuanStartScreenViewBase
{
public:
    gaoyuanStartScreenView();
    virtual ~gaoyuanStartScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void onStartClicked() override;
protected:
};

#endif // GAOYUANSTARTSCREENVIEW_HPP
