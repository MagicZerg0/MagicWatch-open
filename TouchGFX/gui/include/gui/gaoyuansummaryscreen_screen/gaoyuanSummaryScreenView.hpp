#ifndef GAOYUANSUMMARYSCREENVIEW_HPP
#define GAOYUANSUMMARYSCREENVIEW_HPP

#include <gui_generated/gaoyuansummaryscreen_screen/gaoyuanSummaryScreenViewBase.hpp>
#include <gui/gaoyuansummaryscreen_screen/gaoyuanSummaryScreenPresenter.hpp>

class gaoyuanSummaryScreenView : public gaoyuanSummaryScreenViewBase
{
public:
    gaoyuanSummaryScreenView();
    virtual ~gaoyuanSummaryScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    virtual void onRetryClicked() override;
    virtual void onDetailClicked() override;
};

#endif // GAOYUANSUMMARYSCREENVIEW_HPP