#include <gui/fanshoutiaostartscreen_screen/fanshoutiaoStartScreenView.hpp>
#include <gui/fanshoutiaostartscreen_screen/fanshoutiaoStartScreenPresenter.hpp>

fanshoutiaoStartScreenPresenter::fanshoutiaoStartScreenPresenter(fanshoutiaoStartScreenView& v)
    : view(v)
{

}

void fanshoutiaoStartScreenPresenter::activate()
{
    model->setListener(this);
}

void fanshoutiaoStartScreenPresenter::deactivate()
{

}

void fanshoutiaoStartScreenPresenter::onStartTraining() {
    model->startSwingTraining(2, 20, 1);   // 2=挑球, 1=业余
}