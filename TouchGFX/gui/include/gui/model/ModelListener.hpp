#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include "swing_protocol.h"

class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }

    /* ========== 心率/血氧回调 ========== */
    virtual void onHeartRateResult(int32_t hr, int32_t spo2, bool valid) {}

    // 在现有虚函数后面加
    virtual void onSwingResult(const SwingResult_t& res) {}
    virtual void onTrainingDone(const TrainingDone_t& done) {}
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
