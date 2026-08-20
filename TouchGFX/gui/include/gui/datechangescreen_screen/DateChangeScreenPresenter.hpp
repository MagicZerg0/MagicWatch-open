#ifndef DATECHANGESCREENPRESENTER_HPP
#define DATECHANGESCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class DateChangeScreenView;

class DateChangeScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    DateChangeScreenPresenter(DateChangeScreenView& v);

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

    void addMinute();
    void subMinute();
    void addHour();
    void subHour();
    void addDate();
    void subDate();
    Model* getModel() { return static_cast<Model*>(model); }

    virtual ~DateChangeScreenPresenter() {}

private:
    DateChangeScreenPresenter();

    DateChangeScreenView& view;
};

#endif // DATECHANGESCREENPRESENTER_HPP