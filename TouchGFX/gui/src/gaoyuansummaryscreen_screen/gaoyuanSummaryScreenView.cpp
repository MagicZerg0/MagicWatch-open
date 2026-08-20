#include <gui/gaoyuansummaryscreen_screen/gaoyuanSummaryScreenView.hpp>
#include <string.h>

gaoyuanSummaryScreenView::gaoyuanSummaryScreenView()
{

}

void gaoyuanSummaryScreenView::setupScreen() {
    gaoyuanSummaryScreenViewBase::setupScreen();

    const TrainingDone_t& done = presenter->getTrainingResult();
    const SwingResult_t&  best = presenter->getBestSwing();

    // 统计文字
    Unicode::snprintf(statTextBuffer, STATTEXT_SIZE, "Avg:%d Stb:%d%%",
                      (int)done.avg_score, done.stability);
    statText.invalidate();

    // 最佳波形
    bestWaveGraph.clear();
    for (int i = 0; i < best.waveform_len; i++) {
        int stretchedX = (best.waveform_len > 0)
            ? (i * 240 / best.waveform_len)
            : i;
        bestWaveGraph.addDataPoint(stretchedX, (int)best.waveform[i]);
    }
}

void gaoyuanSummaryScreenView::tearDownScreen()
{
    gaoyuanSummaryScreenViewBase::tearDownScreen();
}

void gaoyuanSummaryScreenView::onRetryClicked() {
    presenter->onRetryTraining();
}

void gaoyuanSummaryScreenView::onDetailClicked() {
    // Designer 已配 Change Screen → 无需代码
}