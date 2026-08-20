#include <gui/zhengshoutiaostartscreen_screen/zhengshoutiaoStartScreenView.hpp>
#include <gui/zhengshoutiaostartscreen_screen/zhengshoutiaoStartScreenPresenter.hpp>

zhengshoutiaoStartScreenPresenter::zhengshoutiaoStartScreenPresenter(zhengshoutiaoStartScreenView& v)
    : view(v)
{

}

void zhengshoutiaoStartScreenPresenter::activate()
{
    model->setListener(this);
}

void zhengshoutiaoStartScreenPresenter::deactivate()
{

}

void zhengshoutiaoStartScreenPresenter::onStartTraining() {
    model->startSwingTraining(2, 20, 1);   // 2=挑球, 1=业余
}