#ifndef EQUIPMENT_INFO_SCREENPRESENTER_HPP
#define EQUIPMENT_INFO_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Equipment_Info_ScreenView;

class Equipment_Info_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Equipment_Info_ScreenPresenter(Equipment_Info_ScreenView& v);

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

    virtual ~Equipment_Info_ScreenPresenter() {}

private:
    Equipment_Info_ScreenPresenter();

    Equipment_Info_ScreenView& view;
};

#endif // EQUIPMENT_INFO_SCREENPRESENTER_HPP
