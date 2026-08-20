#ifndef EQUIPMENT_INFO_SCREENVIEW_HPP
#define EQUIPMENT_INFO_SCREENVIEW_HPP

#include <gui_generated/equipment_info_screen_screen/Equipment_Info_ScreenViewBase.hpp>
#include <gui/equipment_info_screen_screen/Equipment_Info_ScreenPresenter.hpp>

class Equipment_Info_ScreenView : public Equipment_Info_ScreenViewBase
{
public:
    Equipment_Info_ScreenView();
    virtual ~Equipment_Info_ScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // EQUIPMENT_INFO_SCREENVIEW_HPP
