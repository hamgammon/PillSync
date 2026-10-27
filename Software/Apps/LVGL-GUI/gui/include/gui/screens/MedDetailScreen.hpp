/**
 ******************************************************************************
 * @file    MedDetailScreen.hpp
 * @brief   Per-medication menu: Enable toggle + Alarm 1, Alarm 2, Alarm 3.
 ******************************************************************************
 */

#ifndef MED_DETAIL_SCREEN_HPP
#define MED_DETAIL_SCREEN_HPP

#include <memory>

#include "gui/screens/Screen.hpp"
#include "gui/widgets/TwoTonePicker.hpp"

class MedDetailScreen : public Screen
{
public:
    explicit MedDetailScreen(Model& model);

    void onShow() override;
    void onHide() override;
    void onKey(uint8_t code) override;

    void onScheduleUpdated() override;

protected:
    void build() override;

private:
    static constexpr uint16_t kItemEnabled = 0;
    static constexpr uint16_t kItemAlarm1  = 1;
    static constexpr uint16_t kItemAlarm2  = 2;
    static constexpr uint16_t kItemAlarm3  = 3;
    static constexpr uint16_t kItemCount   = 4;

    void populateItems();

    Widgets::WheelMenu::Item mItems[kItemCount] {};
    char                     mAlarmLabelBufs[PillSync::kAlarmsPerMed][16] {};
    char                     mAlarmTipBufs[PillSync::kAlarmsPerMed][24] {};
    char                     mTitleBuf[PillSync::kMaxMedNameLen + 1] {};

    std::unique_ptr<Widgets::Title>     mTitle;
    std::unique_ptr<Widgets::Buttons>   mButtons;
    std::unique_ptr<Widgets::WheelMenu> mMenu;
};

#endif // MED_DETAIL_SCREEN_HPP

