#ifndef ZHENGSHOUTIAOTESTSCREENVIEW_HPP
#define ZHENGSHOUTIAOTESTSCREENVIEW_HPP

#include <gui_generated/zhengshoutiaotestscreen_screen/zhengshoutiaoTestScreenViewBase.hpp>
#include <gui/zhengshoutiaotestscreen_screen/zhengshoutiaoTestScreenPresenter.hpp>

#include "swing_protocol.h"

class zhengshoutiaoTestScreenView : public zhengshoutiaoTestScreenViewBase
{
public:
    zhengshoutiaoTestScreenView();
    virtual ~zhengshoutiaoTestScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent() override;
    void onSwingResult(const SwingResult_t& res);
    void onTrainingDone(const TrainingDone_t& done);
protected:
    virtual void onStopClicked() override;
private:
    SwingResult_t bestSwing;
    int           bestScore = -1;
    static const int MAX_WF = 256;
    float    waveformData[MAX_WF];
    uint16_t waveformLen;
    int      animIndex;
    bool     animating;
    int      swingCount;
    float    lastPeak;
    int      lastScore;
};

#endif // ZHENGSHOUTIAOTESTSCREENVIEW_HPP