#ifndef MODEL_HPP
#define MODEL_HPP
#include <cstdint>              // 鈫?鏂板锛歶int8_t 绫诲瀷
#include "swing_protocol.h"
class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    // ===== 鑾峰彇 RTC 鏃堕棿 =====
    void getCurrentTime(uint8_t& hour, uint8_t& minute, uint8_t& second);
    void getCurrentDate(uint8_t& year, uint8_t& month, uint8_t& day, uint8_t& weekday);

    void addOneMinute();
    void subOneMinute();
    void addOneHour();
    void subOneHour();
    void addOneDay();
    void subOneDay();

    void startMeasurement();
    void setListener(ModelListener* listener) { modelListener = listener; }

    void startSwingTraining(int stroke, uint8_t target, int skill);
    void stopSwingTraining();

    void saveTrainingResult(const TrainingDone_t& done, const SwingResult_t& best);
    const TrainingDone_t& getTrainingResult() const { return lastTrainingResult; }
    const SwingResult_t&  getBestSwing()      const { return bestSwingResult; }
protected:
    ModelListener* modelListener;
    TrainingDone_t lastTrainingResult;
    SwingResult_t  bestSwingResult;
};

#endif // MODEL_HPP