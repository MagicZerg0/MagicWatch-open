#include <gui/fanshoutiaostartscreen_screen/fanshoutiaoStartScreenView.hpp>

fanshoutiaoStartScreenView::fanshoutiaoStartScreenView()
{

}

void fanshoutiaoStartScreenView::setupScreen()
{
    fanshoutiaoStartScreenViewBase::setupScreen();
}

void fanshoutiaoStartScreenView::tearDownScreen()
{
    fanshoutiaoStartScreenViewBase::tearDownScreen();
}

void fanshoutiaoStartScreenView::onStartClicked() {
    presenter->onStartTraining();
}