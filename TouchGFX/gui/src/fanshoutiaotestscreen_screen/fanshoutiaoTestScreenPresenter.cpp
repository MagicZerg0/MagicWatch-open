#include <gui/fanshoutiaotestscreen_screen/fanshoutiaoTestScreenView.hpp>
#include <gui/fanshoutiaotestscreen_screen/fanshoutiaoTestScreenPresenter.hpp>

fanshoutiaoTestScreenPresenter::fanshoutiaoTestScreenPresenter(fanshoutiaoTestScreenView& v)
    : view(v)
{

}

void fanshoutiaoTestScreenPresenter::activate()
{
    model->setListener(this);
}

void fanshoutiaoTestScreenPresenter::deactivate()
{

}

void fanshoutiaoTestScreenPresenter::onSwingResult(const SwingResult_t& res) {
    view.onSwingResult(res);
}

void fanshoutiaoTestScreenPresenter::onTrainingDone(const TrainingDone_t& done) {
    view.onTrainingDone(done);
}

void fanshoutiaoTestScreenPresenter::onStopTraining() {
    model->stopSwingTraining();
}

void fanshoutiaoTestScreenPresenter::saveTrainingResult(const TrainingDone_t& done, const SwingResult_t& best) {
    model->saveTrainingResult(done, best);
}