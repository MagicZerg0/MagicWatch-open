#include <gui/zhengshoutiaostartscreen_screen/zhengshoutiaoStartScreenView.hpp>

zhengshoutiaoStartScreenView::zhengshoutiaoStartScreenView()
{

}

void zhengshoutiaoStartScreenView::setupScreen()
{
    zhengshoutiaoStartScreenViewBase::setupScreen();
}

void zhengshoutiaoStartScreenView::tearDownScreen()
{
    zhengshoutiaoStartScreenViewBase::tearDownScreen();
}

void zhengshoutiaoStartScreenView::onStartClicked() {
    presenter->onStartTraining();
}