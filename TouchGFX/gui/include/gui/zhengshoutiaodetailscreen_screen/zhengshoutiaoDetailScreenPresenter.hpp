#ifndef ZHENGSHOUTIAODETAILSCREENPRESENTER_HPP
#define ZHENGSHOUTIAODETAILSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include "swing_protocol.h"

using namespace touchgfx;

class zhengshoutiaoDetailScreenView;

class zhengshoutiaoDetailScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    zhengshoutiaoDetailScreenPresenter(zhengshoutiaoDetailScreenView& v);

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

    virtual ~zhengshoutiaoDetailScreenPresenter() {}

    const TrainingDone_t& getTrainingResult() { return model->getTrainingResult(); }
private:
    zhengshoutiaoDetailScreenPresenter();

    zhengshoutiaoDetailScreenView& view;
};

#endif // ZHENGSHOUTIAODETAILSCREENPRESENTER_HPP