#ifndef MAIN_SCREENPRESENTER_HPP
#define MAIN_SCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Main_ScreenView;

class Main_ScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    Main_ScreenPresenter(Main_ScreenView& v);

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

    // ===== 新增 =====
    void handleTickEvent();

    // ===== 新增：让 View 获取 Model 指针 =====
    Model* getModel() { return static_cast<Model*>(model); }
    virtual ~Main_ScreenPresenter() {}

private:
    Main_ScreenPresenter();

    Main_ScreenView& view;

    // ===== 新增 =====
    //int tickCounter = 0;
    //static constexpr int TICKS_PER_SECOND = 60;
};

#endif // MAIN_SCREENPRESENTER_HPP
