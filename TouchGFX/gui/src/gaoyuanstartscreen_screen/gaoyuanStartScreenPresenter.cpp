#include <gui/gaoyuanstartscreen_screen/gaoyuanStartScreenView.hpp>
#include <gui/gaoyuanstartscreen_screen/gaoyuanStartScreenPresenter.hpp>

gaoyuanStartScreenPresenter::gaoyuanStartScreenPresenter(gaoyuanStartScreenView& v)
    : view(v)
{

}

void gaoyuanStartScreenPresenter::activate()
{
    model->setListener(this);
}

void gaoyuanStartScreenPresenter::deactivate()
{

}

void gaoyuanStartScreenPresenter::onStartTraining() {
    model->startSwingTraining(0, 20, 1);   // 0=高远球, 1=业余
}