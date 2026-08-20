#ifndef GAOYUANTESTSCREENVIEW_HPP
#define GAOYUANTESTSCREENVIEW_HPP

#include <gui_generated/gaoyuantestscreen_screen/gaoyuanTestScreenViewBase.hpp>
#include <gui/gaoyuantestscreen_screen/gaoyuanTestScreenPresenter.hpp>

#include "swing_protocol.h"

class gaoyuanTestScreenView : public gaoyuanTestScreenViewBase
{
public:
    gaoyuanTestScreenView();
    virtual ~gaoyuanTestScreenView() {}
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

#endif // GAOYUANTESTSCREENVIEW_HPP
