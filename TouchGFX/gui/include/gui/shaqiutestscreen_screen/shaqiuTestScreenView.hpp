#ifndef SHAQIUTESTSCREENVIEW_HPP
#define SHAQIUTESTSCREENVIEW_HPP

#include <gui_generated/shaqiutestscreen_screen/shaqiuTestScreenViewBase.hpp>
#include <gui/shaqiutestscreen_screen/shaqiuTestScreenPresenter.hpp>

#include "swing_protocol.h"

class shaqiuTestScreenView : public shaqiuTestScreenViewBase
{
public:
    shaqiuTestScreenView();
    virtual ~shaqiuTestScreenView() {}
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

#endif // SHAQIUTESTSCREENVIEW_HPP