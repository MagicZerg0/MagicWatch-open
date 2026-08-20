#ifndef CHOOSEACTIONSCREENVIEW_HPP
#define CHOOSEACTIONSCREENVIEW_HPP

#include <gui_generated/chooseactionscreen_screen/ChooseActionScreenViewBase.hpp>
#include <gui/chooseactionscreen_screen/ChooseActionScreenPresenter.hpp>

class ChooseActionScreenView : public ChooseActionScreenViewBase
{
public:
    ChooseActionScreenView();
    virtual ~ChooseActionScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // CHOOSEACTIONSCREENVIEW_HPP
