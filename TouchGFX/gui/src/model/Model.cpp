#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "stm32u5xx_hal.h"      // 鈫?鏂板锛欻AL RTC 鍑芥暟
#include "heart_rate_protocol.h"
// ===== 鏂板锛氬紩鐢?main.c 鐨勫叏灞€ RTC 鍙ユ焺 =====
extern RTC_HandleTypeDef hrtc;

Model::Model() : modelListener(0)
{

}
void Model::tick()
{
    HeartRateResult_t res;
    if (osMessageQueueGet(heartRateResultQueueHandle, &res, NULL, 0) == osOK)
    {
        modelListener->onHeartRateResult(
            res.heart_rate,
            res.spo2,
            res.hr_valid ? true : false
        );
    }

    // --- 鎸ユ媿缁撴灉 ---
    SwingResult_t swingRes;
    if (osMessageQueueGet(swingResultQueueHandle, &swingRes, NULL, 0) == osOK) {
        modelListener->onSwingResult(swingRes);
    }
    // --- 璁粌瀹屾垚 ---
    TrainingDone_t done;
    if (osMessageQueueGet(trainingDoneQueueHandle, &done, NULL, 0) == osOK) {
        modelListener->onTrainingDone(done);
    }
}

// ===== 鑾峰彇褰撳墠鏃堕棿 =====
void Model::getCurrentTime(uint8_t& hour, uint8_t& minute, uint8_t& second)
{
    // 鐩存帴璇?RTC 褰卞瓙瀵勫瓨鍣紙BCD 鏍煎紡锛?
    volatile uint32_t tr = RTC->TR;
    volatile uint32_t dr = RTC->DR;
    
    // 杞欢瑙ｇ爜 BCD锛堜笉渚濊禆 HAL锛?
    second = (tr & 0x0F) + ((tr >> 4) & 0x07) * 10;
    minute = ((tr >> 8) & 0x0F) + ((tr >> 12) & 0x07) * 10;
    hour   = ((tr >> 16) & 0x0F) + ((tr >> 20) & 0x03) * 10;
    
    volatile uint32_t debug_tr = tr;   // 鈫?鏂偣杩欓噷锛屽娆℃殏鍋滅湅 tr 鏄惁鍙樺寲
    (void)debug_tr;
}

void Model::getCurrentDate(uint8_t& year, uint8_t& month, uint8_t& day, uint8_t& weekday)
{
    uint32_t dr = RTC->DR;
    
    year    = ((dr >> 16) & 0x0F) + ((dr >> 20) & 0x0F) * 10;
    month   = ((dr >> 8)  & 0x0F) + ((dr >> 12) & 0x01) * 10;
    day     = (dr & 0x0F) + ((dr >> 4) & 0x03) * 10;
    weekday = ((dr >> 13) & 0x07);
}

void Model::startMeasurement()
{
    HeartRateCmd_t cmd = CMD_START_MEASUREMENT;
    osMessageQueuePut(heartRateCmdQueueHandle, &cmd, 0, 0);
}

void Model::startSwingTraining(int stroke, uint8_t target, int skill) {
    SwingCmdArgs_t cmd;
    cmd.cmd    = SWING_CMD_START;
    cmd.stroke = stroke;
    cmd.target = target;
    cmd.skill  = skill;
    osMessageQueuePut(swingCmdQueueHandle, &cmd, 0, 0);
}

void Model::stopSwingTraining() {
    SwingCmdArgs_t cmd;
    cmd.cmd    = SWING_CMD_STOP;
    cmd.stroke = 0;
    cmd.target = 0;
    cmd.skill  = 0;
    osMessageQueuePut(swingCmdQueueHandle, &cmd, 0, 0);
}

void Model::saveTrainingResult(const TrainingDone_t& done, const SwingResult_t& best) {
    lastTrainingResult = done;
    bestSwingResult    = best;
}

void Model::addOneMinute()
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    sTime.Minutes++;
    if (sTime.Minutes >= 60) {
        sTime.Minutes = 0;
        sTime.Hours++;
        if (sTime.Hours >= 24) {
            sTime.Hours = 0;
            sDate.Date++;   // 绠€鍖栵細涓嶇鏈堜唤澶╂暟
        }
    }
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void Model::subOneMinute()
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    if (sTime.Minutes == 0) {
        sTime.Minutes = 59;
        if (sTime.Hours == 0) {
            sTime.Hours = 23;
            if (sDate.Date > 1) sDate.Date--;
        } else {
            sTime.Hours--;
        }
    } else {
        sTime.Minutes--;
    }
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void Model::addOneHour()
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    sTime.Hours++;
    if (sTime.Hours >= 24) {
        sTime.Hours = 0;
        sDate.Date++;
    }
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void Model::subOneHour()
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    if (sTime.Hours == 0) {
        sTime.Hours = 23;
        if (sDate.Date > 1) sDate.Date--;
    } else {
        sTime.Hours--;
    }
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void Model::addOneDay()
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    sDate.Date++;
    // 绠€鍖栵細涓嶆鏌ユ渶澶ф湀浠藉ぉ鏁帮紝瑕佺簿纭彲鐢≧TC_DATE_MONTH_DAY_VALID
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void Model::subOneDay()
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    if (sDate.Date > 1) {
        sDate.Date--;
    }
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}