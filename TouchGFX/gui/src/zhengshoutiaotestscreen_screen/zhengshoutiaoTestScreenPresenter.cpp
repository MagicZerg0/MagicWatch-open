#include <gui/zhengshoutiaotestscreen_screen/zhengshoutiaoTestScreenView.hpp>
#include <gui/zhengshoutiaotestscreen_screen/zhengshoutiaoTestScreenPresenter.hpp>

zhengshoutiaoTestScreenPresenter::zhengshoutiaoTestScreenPresenter(zhengshoutiaoTestScreenView& v)
    : view(v)
{

}

void zhengshoutiaoTestScreenPresenter::activate()
{
    model->setListener(this);
}

void zhengshoutiaoTestScreenPresenter::deactivate()
{

}

void zhengshoutiaoTestScreenPresenter::onSwingResult(const SwingResult_t& res) {
    view.onSwingResult(res);
}

void zhengshoutiaoTestScreenPresenter::onTrainingDone(const TrainingDone_t& done) {
    view.onTrainingDone(done);
}

void zhengshoutiaoTestScreenPresenter::onStopTraining() {
    model->stopSwingTraining();
}

void zhengshoutiaoTestScreenPresenter::saveTrainingResult(const TrainingDone_t& done, const SwingResult_t& best) {
    model->saveTrainingResult(done, best);
}