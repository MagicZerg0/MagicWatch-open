#include <gui/hrspo2testscreen_screen/HrSpo2TestScreenView.hpp>

HrSpo2TestScreenView::HrSpo2TestScreenView()
{

}

void HrSpo2TestScreenView::setupScreen()
{
    HrSpo2TestScreenViewBase::setupScreen();
    // ✅ 绑定通配符缓冲区（只需一次）
    textAreaHrValue.setWildcard(hrBuf);
    textAreaSpo2Value.setWildcard(spo2Buf);

    Unicode::snprintf(hrBuf, 10, "--");
    Unicode::snprintf(spo2Buf, 10, "--");
    textAreaHrValue.invalidate();
    textAreaSpo2Value.invalidate();
}

void HrSpo2TestScreenView::tearDownScreen()
{
    HrSpo2TestScreenViewBase::tearDownScreen();
}

void HrSpo2TestScreenView::showMeasuring()
{
    // 隐藏按钮
    buttonStart.setVisible(false);
    // 显示测量提示
    textAreaTip1.setVisible(true);
    textAreaTip2.setVisible(true);
    // 隐藏结果和失败
    textAreaHrValue.setVisible(false);
    textAreaSpo2Value.setVisible(false);
    textAreaTestFail.setVisible(false);
    // 刷新（不再操作 image1 和 image2）
    buttonStart.invalidate();
    textAreaTip1.invalidate();
    textAreaTip2.invalidate();
    textAreaHrValue.invalidate();
    textAreaSpo2Value.invalidate();
    textAreaTestFail.invalidate();
}

void HrSpo2TestScreenView::showResult(int32_t hr, int32_t spo2)
{
    // 隐藏测量提示、失败提示
    textAreaTip1.setVisible(false);
    textAreaTip2.setVisible(false);
    textAreaTestFail.setVisible(false);

    // 显示结果
    textAreaHrValue.setVisible(true);
    textAreaSpo2Value.setVisible(true);

    // 设置数值
    Unicode::snprintf(hrBuf, 10, "%d", hr);
    textAreaHrValue.setWildcard(hrBuf);
    Unicode::snprintf(spo2Buf, 10, "%d", spo2);
    textAreaSpo2Value.setWildcard(spo2Buf);

    // 显示开始按钮
    buttonStart.setVisible(true);

    // 刷新
    textAreaTip1.invalidate();
    textAreaTip2.invalidate();
    textAreaTestFail.invalidate();
    textAreaHrValue.invalidate();
    textAreaSpo2Value.invalidate();
    buttonStart.invalidate();
}

void HrSpo2TestScreenView::showFailure()
{
    // 隐藏提示和结果
    textAreaTip1.setVisible(false);
    textAreaTip2.setVisible(false);
    textAreaHrValue.setVisible(false);
    textAreaSpo2Value.setVisible(false);

    // 显示失败提示 + 按钮
    textAreaTestFail.setVisible(true);
    buttonStart.setVisible(true);

    textAreaTip1.invalidate();
    textAreaTip2.invalidate();
    textAreaHrValue.invalidate();
    textAreaSpo2Value.invalidate();
    textAreaTestFail.invalidate();
    buttonStart.invalidate();
}

void HrSpo2TestScreenView::onStartMeasurement()
{
    // 把按钮事件转给 Presenter 处理
    presenter->onStartMeasurement();
}