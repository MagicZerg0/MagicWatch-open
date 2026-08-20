#ifndef GAOYUANDETAILSCREENPRESENTER_HPP
#define GAOYUANDETAILSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include "swing_protocol.h"

using namespace touchgfx;

class gaoyuanDetailScreenView;

class gaoyuanDetailScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    gaoyuanDetailScreenPresenter(gaoyuanDetailScreenView& v);

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

    virtual ~gaoyuanDetailScreenPresenter() {}

    const TrainingDone_t& getTrainingResult() { return model->getTrainingResult(); }
private:
    gaoyuanDetailScreenPresenter();

    gaoyuanDetailScreenView& view;
};

#endif // GAOYUANDETAILSCREENPRESENTER_HPP