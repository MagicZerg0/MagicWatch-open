#ifndef HRSPO2TESTSCREENPRESENTER_HPP
#define HRSPO2TESTSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class HrSpo2TestScreenView;

class HrSpo2TestScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    HrSpo2TestScreenPresenter(HrSpo2TestScreenView& v);

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

    virtual void onStartMeasurement();   // ← 覆盖 Designer 生成的虚函数
    virtual void onHeartRateResult(int32_t hr, int32_t spo2, bool valid);

    virtual ~HrSpo2TestScreenPresenter() {}

private:
    HrSpo2TestScreenPresenter();

    HrSpo2TestScreenView& view;
};

#endif // HRSPO2TESTSCREENPRESENTER_HPP
