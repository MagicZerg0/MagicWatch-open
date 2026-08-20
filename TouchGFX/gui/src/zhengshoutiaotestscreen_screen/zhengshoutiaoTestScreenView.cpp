#include <gui/zhengshoutiaotestscreen_screen/zhengshoutiaoTestScreenView.hpp>
#include <string.h>
#include <stdio.h>

zhengshoutiaoTestScreenView::zhengshoutiaoTestScreenView()
    : waveformLen(0), animIndex(0), animating(false),
    swingCount(0), lastPeak(0), lastScore(0)
{
}

void zhengshoutiaoTestScreenView::setupScreen()
{
    zhengshoutiaoTestScreenViewBase::setupScreen();

    // 初始显示
    Unicode::snprintf(countTextBuffer, COUNTTEXT_SIZE, "0/20");
    countText.invalidate();
    Unicode::snprintf(peakValueBuffer, PEAKVALUE_SIZE, "--");
    peakValue.invalidate();
    Unicode::snprintf(evalTextBuffer, EVALTEXT_SIZE, "");
    evalText.invalidate();
    waveGraph.clear();
}

void zhengshoutiaoTestScreenView::tearDownScreen()
{
    zhengshoutiaoTestScreenViewBase::tearDownScreen();
}

// ── 辅助：浮点 → 整数.1位小数 ──
static void formatFloat1(char* buf, int bufSize, float val)
{
    int i = (int)val;
    int d = (int)((val - (float)i) * 10.0f);
    if (d < 0) d = -d;
    snprintf(buf, bufSize, "%d.%d", i, d);
}

void zhengshoutiaoTestScreenView::onSwingResult(const SwingResult_t& res)
{
    swingCount = res.swing_index;
    Unicode::snprintf(countTextBuffer, COUNTTEXT_SIZE, "%d/20", swingCount);
    countText.invalidate();

    lastPeak  = res.peak_accel;
    lastScore = res.score;

    memcpy(waveformData, res.waveform, res.waveform_len * sizeof(float));
    waveformLen = res.waveform_len;
    animIndex   = 0;
    animating   = true;
    waveGraph.clear();

    // 追踪最佳挥拍
    if (res.score > bestScore) {
        bestScore = res.score;
        bestSwing = res;
    }
}

void zhengshoutiaoTestScreenView::handleTickEvent()
{
    if (!animating) return;

    int perTick = 5;
    for (int n = 0; n < perTick && animIndex < waveformLen; n++) {
        int stretchedX = (waveformLen > 0)
            ? (animIndex * 240 / waveformLen)
            : animIndex;
        waveGraph.addDataPoint(stretchedX, (int)waveformData[animIndex]);
        animIndex++;
    }

    if (animIndex >= waveformLen) {
        animating = false;

        // 峰值：浮点 → "56.2"
        char tmp[16];
        formatFloat1(tmp, sizeof(tmp), lastPeak);
        Unicode::fromUTF8((const uint8_t*)tmp, peakValueBuffer, PEAKVALUE_SIZE);
        peakValue.invalidate();

        // 评价：中文用 fromUTF8
        if (lastScore >= 85) {
            Unicode::fromUTF8((const uint8_t*)"标准", evalTextBuffer, EVALTEXT_SIZE);
        } else if (lastScore >= 70) {
            Unicode::fromUTF8((const uint8_t*)"良好", evalTextBuffer, EVALTEXT_SIZE);
        } else if (lastScore >= 30) {
            Unicode::fromUTF8((const uint8_t*)"力度偏弱", evalTextBuffer, EVALTEXT_SIZE);
        } else {
            Unicode::fromUTF8((const uint8_t*)"动作错误", evalTextBuffer, EVALTEXT_SIZE);
        }
        evalText.invalidate();
    }
}

void zhengshoutiaoTestScreenView::onStopClicked()
{
    presenter->onStopTraining();
}

void zhengshoutiaoTestScreenView::onTrainingDone(const TrainingDone_t& done) {
    animating = false;

    presenter->saveTrainingResult(done, bestSwing);
    application().gotozhengshoutiaoSummaryScreenScreenWipeTransitionWest();
}