#ifndef FANSHOUTIAOSUMMARYSCREENPRESENTER_HPP
#define FANSHOUTIAOSUMMARYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class fanshoutiaoSummaryScreenView;

class fanshoutiaoSummaryScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    fanshoutiaoSummaryScreenPresenter(fanshoutiaoSummaryScreenView& v);

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

    virtual ~fanshoutiaoSummaryScreenPresenter() {}

    const TrainingDone_t& getTrainingResult() { return model->getTrainingResult(); }
    const SwingResult_t&  getBestSwing()      { return model->getBestSwing(); }
    void onRetryTraining() { model->startSwingTraining(2, 20, 1); }  // 2=挑球

private:
    fanshoutiaoSummaryScreenPresenter();

    fanshoutiaoSummaryScreenView& view;
};

#endif // FANSHOUTIAOSUMMARYSCREENPRESENTER_HPP