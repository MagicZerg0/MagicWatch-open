#include <gui/gaoyuanstartscreen_screen/gaoyuanStartScreenView.hpp>

gaoyuanStartScreenView::gaoyuanStartScreenView()
{

}

void gaoyuanStartScreenView::setupScreen()
{
    gaoyuanStartScreenViewBase::setupScreen();
}

void gaoyuanStartScreenView::tearDownScreen()
{
    gaoyuanStartScreenViewBase::tearDownScreen();
}

void gaoyuanStartScreenView::onStartClicked() {
    presenter->onStartTraining();
}