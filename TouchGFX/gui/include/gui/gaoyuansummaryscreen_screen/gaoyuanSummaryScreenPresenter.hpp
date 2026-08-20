#ifndef GAOYUANSUMMARYSCREENPRESENTER_HPP
#define GAOYUANSUMMARYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class gaoyuanSummaryScreenView;

class gaoyuanSummaryScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    gaoyuanSummaryScreenPresenter(gaoyuanSummaryScreenView& v);

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

    virtual ~gaoyuanSummaryScreenPresenter() {}

    const TrainingDone_t& getTrainingResult() { return model->getTrainingResult(); }
    const SwingResult_t&  getBestSwing()      { return model->getBestSwing(); }
    void onRetryTraining() { model->startSwingTraining(0, 20, 1); }

private:
    gaoyuanSummaryScreenPresenter();

    gaoyuanSummaryScreenView& view;
};

#endif // GAOYUANSUMMARYSCREENPRESENTER_HPP