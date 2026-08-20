#include <gui/shaqiutestscreen_screen/shaqiuTestScreenView.hpp>
#include <gui/shaqiutestscreen_screen/shaqiuTestScreenPresenter.hpp>

shaqiuTestScreenPresenter::shaqiuTestScreenPresenter(shaqiuTestScreenView& v)
    : view(v)
{

}

void shaqiuTestScreenPresenter::activate()
{
    model->setListener(this);
}

void shaqiuTestScreenPresenter::deactivate()
{

}

void shaqiuTestScreenPresenter::onSwingResult(const SwingResult_t& res) {
    view.onSwingResult(res);
}

void shaqiuTestScreenPresenter::onTrainingDone(const TrainingDone_t& done) {
    view.onTrainingDone(done);
}

void shaqiuTestScreenPresenter::onStopTraining() {
    model->stopSwingTraining();
}

void shaqiuTestScreenPresenter::saveTrainingResult(const TrainingDone_t& done, const SwingResult_t& best) {
    model->saveTrainingResult(done, best);
}