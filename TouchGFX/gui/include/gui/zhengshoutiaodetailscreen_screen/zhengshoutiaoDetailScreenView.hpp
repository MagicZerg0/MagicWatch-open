#ifndef ZHENGSHOUTIAODETAILSCREENVIEW_HPP
#define ZHENGSHOUTIAODETAILSCREENVIEW_HPP

#include <gui_generated/zhengshoutiaodetailscreen_screen/zhengshoutiaoDetailScreenViewBase.hpp>
#include <gui/zhengshoutiaodetailscreen_screen/zhengshoutiaoDetailScreenPresenter.hpp>

class zhengshoutiaoDetailScreenView : public zhengshoutiaoDetailScreenViewBase
{
public:
    zhengshoutiaoDetailScreenView();
    virtual ~zhengshoutiaoDetailScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // ZHENGSHOUTIAODETAILSCREENVIEW_HPP