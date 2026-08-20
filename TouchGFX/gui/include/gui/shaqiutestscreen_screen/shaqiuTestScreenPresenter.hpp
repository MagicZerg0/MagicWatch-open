#ifndef SHAQIUTESTSCREENPRESENTER_HPP
#define SHAQIUTESTSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class shaqiuTestScreenView;

class shaqiuTestScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    shaqiuTestScreenPresenter(shaqiuTestScreenView& v);

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

    virtual void onSwingResult(const SwingResult_t& res) override;
    virtual void onTrainingDone(const TrainingDone_t& done) override;
    void onStopTraining();
    void saveTrainingResult(const TrainingDone_t& done, const SwingResult_t& best);
    virtual ~shaqiuTestScreenPresenter() {}

private:
    shaqiuTestScreenPresenter();

    shaqiuTestScreenView& view;
};

#endif // SHAQIUTESTSCREENPRESENTER_HPP