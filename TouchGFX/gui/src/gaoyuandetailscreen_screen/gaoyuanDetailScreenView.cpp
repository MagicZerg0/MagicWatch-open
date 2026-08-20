#include <gui/gaoyuandetailscreen_screen/gaoyuanDetailScreenView.hpp>
#include <string.h>
#include <stdio.h>

gaoyuanDetailScreenView::gaoyuanDetailScreenView()
{

}

void gaoyuanDetailScreenView::setupScreen() {
    gaoyuanDetailScreenViewBase::setupScreen();

    const TrainingDone_t& done = presenter->getTrainingResult();

    int peakInt = (int)done.max_peak;
    int peakDec = (int)((done.max_peak - (float)peakInt) * 10.0f);
    if (peakDec < 0) peakDec = -peakDec;

    // 建议文字：把 ; | ； 都替换成换行
    char adviceLines[300];
    strncpy(adviceLines, done.advice, sizeof(adviceLines) - 1);
    adviceLines[sizeof(adviceLines) - 1] = '\0';
    for (int i = 0; adviceLines[i] != '\0'; i++) {
        if (adviceLines[i] == ';' || adviceLines[i] == '|') {
            adviceLines[i] = '\n';
        }
    }

    char tmp[600];
    snprintf(tmp, sizeof(tmp),
        "训练结果详情\n\n"
        "完成次数: %d\n"
        "最大峰值: %d.%d m/s2\n"
        "平均评分: %d\n"
        "稳定性: %d%%\n\n"
        "建议:\n%s",
        done.total,
        peakInt, peakDec,
        (int)done.avg_score,
        done.stability,
        adviceLines);

    Unicode::fromUTF8((const uint8_t*)tmp, detailTextBuffer, 600);
    detailText.invalidate();

    scrollableContainer.invalidate();
}

void gaoyuanDetailScreenView::tearDownScreen()
{
    gaoyuanDetailScreenViewBase::tearDownScreen();
}
