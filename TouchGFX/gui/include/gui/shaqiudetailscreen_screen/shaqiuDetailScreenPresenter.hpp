#ifndef SHAQIUDETAILSCREENPRESENTER_HPP
#define SHAQIUDETAILSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include "swing_protocol.h"

using namespace touchgfx;

class shaqiuDetailScreenView;

class shaqiuDetailScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    shaqiuDetailScreenPresenter(shaqiuDetailScreenView& v);

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

    virtual ~shaqiuDetailScreenPresenter() {}

    const TrainingDone_t& getTrainingResult() { return model->getTrainingResult(); }
private:
    shaqiuDetailScreenPresenter();

    shaqiuDetailScreenView& view;
};

#endif // SHAQIUDETAILSCREENPRESENTER_HPP