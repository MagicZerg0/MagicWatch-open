#ifndef SHAQIUSUMMARYSCREENPRESENTER_HPP
#define SHAQIUSUMMARYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class shaqiuSummaryScreenView;

class shaqiuSummaryScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    shaqiuSummaryScreenPresenter(shaqiuSummaryScreenView& v);

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

    virtual ~shaqiuSummaryScreenPresenter() {}

    const TrainingDone_t& getTrainingResult() { return model->getTrainingResult(); }
    const SwingResult_t&  getBestSwing()      { return model->getBestSwing(); }
    void onRetryTraining() { model->startSwingTraining(1, 20, 1); }  // 1=杀球

private:
    shaqiuSummaryScreenPresenter();

    shaqiuSummaryScreenView& view;
};

#endif // SHAQIUSUMMARYSCREENPRESENTER_HPP