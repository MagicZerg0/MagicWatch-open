#ifndef ZHENGSHOUTIAOSTARTSCREENPRESENTER_HPP
#define ZHENGSHOUTIAOSTARTSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class zhengshoutiaoStartScreenView;

class zhengshoutiaoStartScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    zhengshoutiaoStartScreenPresenter(zhengshoutiaoStartScreenView& v);

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

    void onStartTraining();
    virtual ~zhengshoutiaoStartScreenPresenter() {}

private:
    zhengshoutiaoStartScreenPresenter();

    zhengshoutiaoStartScreenView& view;
};

#endif // ZHENGSHOUTIAOSTARTSCREENPRESENTER_HPP