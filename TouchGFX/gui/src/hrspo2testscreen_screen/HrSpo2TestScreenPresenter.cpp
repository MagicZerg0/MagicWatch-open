#include <gui/hrspo2testscreen_screen/HrSpo2TestScreenView.hpp>
#include <gui/hrspo2testscreen_screen/HrSpo2TestScreenPresenter.hpp>

HrSpo2TestScreenPresenter::HrSpo2TestScreenPresenter(HrSpo2TestScreenView& v)
    : view(v)
{

}

void HrSpo2TestScreenPresenter::activate()
{
    // 注册自己为 Model 的监听者
    model->setListener(this);
}

void HrSpo2TestScreenPresenter::deactivate()
{
    // 离开画面时清除监听
    model->setListener(nullptr);
}
void HrSpo2TestScreenPresenter::onStartMeasurement()
{
    model->startMeasurement();   // 通知 FreeRTOS 开始测量
    view.showMeasuring();        // 更新 UI
}
void HrSpo2TestScreenPresenter::onHeartRateResult(int32_t hr, int32_t spo2, bool valid)
{
    if (valid)
        view.showResult(hr, spo2);
    else
        view.showFailure();
}