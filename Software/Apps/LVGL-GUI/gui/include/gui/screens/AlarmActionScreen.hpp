/**
 ******************************************************************************
 * @file    AlarmActionScreen.hpp
 * @brief   Per-alarm slot menu: Toggle ON/OFF or Edit Time (HH:MM).
 ******************************************************************************
 */

#ifndef ALARM_ACTION_SCREEN_HPP
#define ALARM_ACTION_SCREEN_HPP

#include <memory>

#include "gui/screens/Screen.hpp"
#include "gui/widgets/TwoTonePicker.hpp"

class AlarmActionScreen : public Screen
{
public:
    explicit AlarmActionScreen(Model& model);

    void onShow() override;
    void onHide() override;
    void onKey(uint8_t code) override;

    void onScheduleUpdated() override;

protected:
    void build() override;

private:
    static constexpr uint16_t kItemToggle    = 0;
    static constexpr uint16_t kItemSetTime   = 1;
    static constexpr uint16_t kItemAlertMode = 2;
    static constexpr uint16_t kItemCount     = 3;

    void populateItems();

    Widgets::WheelMenu::Item mItems[kItemCount] {};
    char                     mTimeTipBuf[24] {};
    char                     mAlertTipBuf[24] {};
    char                     mTitleBuf[24] {};

    std::unique_ptr<Widgets::Title>     mTitle;
    std::unique_ptr<Widgets::Buttons>   mButtons;
    std::unique_ptr<Widgets::WheelMenu> mMenu;
};

#endif // ALARM_ACTION_SCREEN_HPP

