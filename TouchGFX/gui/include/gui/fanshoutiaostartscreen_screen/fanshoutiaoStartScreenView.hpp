#ifndef FANSHOUTIAOSTARTSCREENVIEW_HPP
#define FANSHOUTIAOSTARTSCREENVIEW_HPP

#include <gui_generated/fanshoutiaostartscreen_screen/fanshoutiaoStartScreenViewBase.hpp>
#include <gui/fanshoutiaostartscreen_screen/fanshoutiaoStartScreenPresenter.hpp>

class fanshoutiaoStartScreenView : public fanshoutiaoStartScreenViewBase
{
public:
    fanshoutiaoStartScreenView();
    virtual ~fanshoutiaoStartScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void onStartClicked() override;
protected:
};

#endif // FANSHOUTIAOSTARTSCREENVIEW_HPP