#ifndef ZHENGSHOUTIAOSUMMARYSCREENPRESENTER_HPP
#define ZHENGSHOUTIAOSUMMARYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class zhengshoutiaoSummaryScreenView;

class zhengshoutiaoSummaryScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    zhengshoutiaoSummaryScreenPresenter(zhengshoutiaoSummaryScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~zhengshoutiaoSummaryScreenPresenter() {}

    const TrainingDone_t& getTrainingResult() { return model->getTrainingResult(); }
    const SwingResult_t&  getBestSwing()      { return model->getBestSwing(); }
    void onRetryTraining() { model->startSwingTraining(2, 20, 1); }  // 2=挑球

private:
    zhengshoutiaoSummaryScreenPresenter();

    zhengshoutiaoSummaryScreenView& view;
};

#endif // ZHENGSHOUTIAOSUMMARYSCREENPRESENTER_HPP