#include <gui/shaqiustartscreen_screen/shaqiuStartScreenView.hpp>
#include <gui/shaqiustartscreen_screen/shaqiuStartScreenPresenter.hpp>

shaqiuStartScreenPresenter::shaqiuStartScreenPresenter(shaqiuStartScreenView& v)
    : view(v)
{

}

void shaqiuStartScreenPresenter::activate()
{
    model->setListener(this);
}

void shaqiuStartScreenPresenter::deactivate()
{

}

void shaqiuStartScreenPresenter::onStartTraining() {
    model->startSwingTraining(1, 20, 1);   // 1=杀球, 1=业余
}