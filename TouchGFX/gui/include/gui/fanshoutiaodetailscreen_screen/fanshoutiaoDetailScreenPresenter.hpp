#ifndef FANSHOUTIAODETAILSCREENPRESENTER_HPP
#define FANSHOUTIAODETAILSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include "swing_protocol.h"

using namespace touchgfx;

class fanshoutiaoDetailScreenView;

class fanshoutiaoDetailScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    fanshoutiaoDetailScreenPresenter(fanshoutiaoDetailScreenView& v);

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

    virtual ~fanshoutiaoDetailScreenPresenter() {}

    const TrainingDone_t& getTrainingResult() { return model->getTrainingResult(); }
private:
    fanshoutiaoDetailScreenPresenter();

    fanshoutiaoDetailScreenView& view;
};

#endif // FANSHOUTIAODETAILSCREENPRESENTER_HPP