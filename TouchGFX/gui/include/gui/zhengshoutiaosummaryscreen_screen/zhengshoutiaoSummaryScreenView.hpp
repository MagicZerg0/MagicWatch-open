#ifndef ZHENGSHOUTIAOSUMMARYSCREENVIEW_HPP
#define ZHENGSHOUTIAOSUMMARYSCREENVIEW_HPP

#include <gui_generated/zhengshoutiaosummaryscreen_screen/zhengshoutiaoSummaryScreenViewBase.hpp>
#include <gui/zhengshoutiaosummaryscreen_screen/zhengshoutiaoSummaryScreenPresenter.hpp>

class zhengshoutiaoSummaryScreenView : public zhengshoutiaoSummaryScreenViewBase
{
public:
    zhengshoutiaoSummaryScreenView();
    virtual ~zhengshoutiaoSummaryScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    virtual void onRetryClicked() override;
    virtual void onDetailClicked() override;
};

#endif // ZHENGSHOUTIAOSUMMARYSCREENVIEW_HPP