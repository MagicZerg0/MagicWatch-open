#ifndef FANSHOUTIAODETAILSCREENVIEW_HPP
#define FANSHOUTIAODETAILSCREENVIEW_HPP

#include <gui_generated/fanshoutiaodetailscreen_screen/fanshoutiaoDetailScreenViewBase.hpp>
#include <gui/fanshoutiaodetailscreen_screen/fanshoutiaoDetailScreenPresenter.hpp>

class fanshoutiaoDetailScreenView : public fanshoutiaoDetailScreenViewBase
{
public:
    fanshoutiaoDetailScreenView();
    virtual ~fanshoutiaoDetailScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // FANSHOUTIAODETAILSCREENVIEW_HPP