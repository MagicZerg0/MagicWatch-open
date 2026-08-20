#ifndef ZHENGSHOUTIAOSTARTSCREENVIEW_HPP
#define ZHENGSHOUTIAOSTARTSCREENVIEW_HPP

#include <gui_generated/zhengshoutiaostartscreen_screen/zhengshoutiaoStartScreenViewBase.hpp>
#include <gui/zhengshoutiaostartscreen_screen/zhengshoutiaoStartScreenPresenter.hpp>

class zhengshoutiaoStartScreenView : public zhengshoutiaoStartScreenViewBase
{
public:
    zhengshoutiaoStartScreenView();
    virtual ~zhengshoutiaoStartScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void onStartClicked() override;
protected:
};

#endif // ZHENGSHOUTIAOSTARTSCREENVIEW_HPP