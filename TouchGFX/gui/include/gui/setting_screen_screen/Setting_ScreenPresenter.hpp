#ifndef SETTING_SCREENPRESENTER_HPP
#define SETTING_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Setting_ScreenView;

class Setting_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Setting_ScreenPresenter(Setting_ScreenView& v);

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

    virtual ~Setting_ScreenPresenter() {}

private:
    Setting_ScreenPresenter();

    Setting_ScreenView& view;
};

#endif // SETTING_SCREENPRESENTER_HPP
