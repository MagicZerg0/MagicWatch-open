#ifndef FANSHOUTIAOSUMMARYSCREENVIEW_HPP
#define FANSHOUTIAOSUMMARYSCREENVIEW_HPP

#include <gui_generated/fanshoutiaosummaryscreen_screen/fanshoutiaoSummaryScreenViewBase.hpp>
#include <gui/fanshoutiaosummaryscreen_screen/fanshoutiaoSummaryScreenPresenter.hpp>

class fanshoutiaoSummaryScreenView : public fanshoutiaoSummaryScreenViewBase
{
public:
    fanshoutiaoSummaryScreenView();
    virtual ~fanshoutiaoSummaryScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    virtual void onRetryClicked() override;
    virtual void onDetailClicked() override;
};

#endif // FANSHOUTIAOSUMMARYSCREENVIEW_HPP