#include <gui/gaoyuantestscreen_screen/gaoyuanTestScreenView.hpp>
#include <gui/gaoyuantestscreen_screen/gaoyuanTestScreenPresenter.hpp>

gaoyuanTestScreenPresenter::gaoyuanTestScreenPresenter(gaoyuanTestScreenView& v)
    : view(v)
{

}

void gaoyuanTestScreenPresenter::activate()
{
    model->setListener(this);
}

void gaoyuanTestScreenPresenter::deactivate()
{

}

void gaoyuanTestScreenPresenter::onSwingResult(const SwingResult_t& res) {
    view.onSwingResult(res);
}

void gaoyuanTestScreenPresenter::onTrainingDone(const TrainingDone_t& done) {
    view.onTrainingDone(done);
}

void gaoyuanTestScreenPresenter::onStopTraining() {
    model->stopSwingTraining();
}

void gaoyuanTestScreenPresenter::saveTrainingResult(const TrainingDone_t& done, const SwingResult_t& best) {
    model->saveTrainingResult(done, best);
}