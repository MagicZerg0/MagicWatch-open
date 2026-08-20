#ifndef SHAQIUSUMMARYSCREENVIEW_HPP
#define SHAQIUSUMMARYSCREENVIEW_HPP

#include <gui_generated/shaqiusummaryscreen_screen/shaqiuSummaryScreenViewBase.hpp>
#include <gui/shaqiusummaryscreen_screen/shaqiuSummaryScreenPresenter.hpp>

class shaqiuSummaryScreenView : public shaqiuSummaryScreenViewBase
{
public:
    shaqiuSummaryScreenView();
    virtual ~shaqiuSummaryScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    virtual void onRetryClicked() override;
    virtual void onDetailClicked() override;
};

#endif // SHAQIUSUMMARYSCREENVIEW_HPP