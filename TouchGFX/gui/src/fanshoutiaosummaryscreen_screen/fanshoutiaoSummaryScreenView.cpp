#include <gui/fanshoutiaosummaryscreen_screen/fanshoutiaoSummaryScreenView.hpp>
#include <string.h>

fanshoutiaoSummaryScreenView::fanshoutiaoSummaryScreenView()
{

}

void fanshoutiaoSummaryScreenView::setupScreen() {
    fanshoutiaoSummaryScreenViewBase::setupScreen();

    const TrainingDone_t& done = presenter->getTrainingResult();
    const SwingResult_t&  best = presenter->getBestSwing();

    //float debugAvg = done.avg_score;
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

void fanshoutiaoSummaryScreenView::tearDownScreen()
{
    fanshoutiaoSummaryScreenViewBase::tearDownScreen();
}

void fanshoutiaoSummaryScreenView::onRetryClicked() {
    presenter->onRetryTraining();
}

void fanshoutiaoSummaryScreenView::onDetailClicked() {
    // Designer 已配 Change Screen → 无需代码
}