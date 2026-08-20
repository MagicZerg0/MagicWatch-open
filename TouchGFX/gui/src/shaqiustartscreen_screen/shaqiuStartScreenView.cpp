#include <gui/shaqiustartscreen_screen/shaqiuStartScreenView.hpp>

shaqiuStartScreenView::shaqiuStartScreenView()
{

}

void shaqiuStartScreenView::setupScreen()
{
    shaqiuStartScreenViewBase::setupScreen();
}

void shaqiuStartScreenView::tearDownScreen()
{
    shaqiuStartScreenViewBase::tearDownScreen();
}

void shaqiuStartScreenView::onStartClicked() {
    presenter->onStartTraining();
}