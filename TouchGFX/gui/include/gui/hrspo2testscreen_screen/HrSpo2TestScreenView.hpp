#ifndef HRSPO2TESTSCREENVIEW_HPP
#define HRSPO2TESTSCREENVIEW_HPP

#include <gui_generated/hrspo2testscreen_screen/HrSpo2TestScreenViewBase.hpp>
#include <gui/hrspo2testscreen_screen/HrSpo2TestScreenPresenter.hpp>

class HrSpo2TestScreenView : public HrSpo2TestScreenViewBase
{
public:
    HrSpo2TestScreenView();
    virtual ~HrSpo2TestScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void onStartMeasurement();    // ← 覆盖 ViewBase 中的虚函数
    void showMeasuring();
    void showResult(int32_t hr, int32_t spo2);
    void showFailure();
protected:
    Unicode::UnicodeChar hrBuf[10];
    Unicode::UnicodeChar spo2Buf[10];
};

#endif // HRSPO2TESTSCREENVIEW_HPP
