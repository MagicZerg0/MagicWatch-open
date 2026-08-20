#ifndef BLUETOOTH_SCREENPRESENTER_HPP
#define BLUETOOTH_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class BlueTooth_ScreenView;

class BlueTooth_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    BlueTooth_ScreenPresenter(BlueTooth_ScreenView& v);

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

    virtual ~BlueTooth_ScreenPresenter() {}

private:
    BlueTooth_ScreenPresenter();

    BlueTooth_ScreenView& view;
};

#endif // BLUETOOTH_SCREENPRESENTER_HPP
