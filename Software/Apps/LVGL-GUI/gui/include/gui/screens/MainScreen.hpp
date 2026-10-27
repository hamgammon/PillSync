/**
 ******************************************************************************
 * @file    MainScreen.hpp
 * @brief   Main wheel menu listing the 5 medications and their active status.
 ******************************************************************************
 */

#ifndef MAIN_SCREEN_HPP
#define MAIN_SCREEN_HPP

#include <memory>

#include "gui/screens/Screen.hpp"
#include "gui/widgets/TwoTonePicker.hpp"

class MainScreen : public Screen
{
public:
    explicit MainScreen(Model& model);

    void onShow() override;
    void onHide() override;
    void onKey(uint8_t code) override;

    void onScheduleUpdated() override;

protected:
    void build() override;

private:
    void populateItems();

    Widgets::WheelMenu::Item mItems[PillSync::kMaxMedications] {};
    char                     mTipBufs[PillSync::kMaxMedications][24] {};

    std::unique_ptr<Widgets::Title>     mTitle;
    std::unique_ptr<Widgets::Buttons>   mButtons;
    std::unique_ptr<Widgets::WheelMenu> mMenu;
};

#endif // MAIN_SCREEN_HPP

