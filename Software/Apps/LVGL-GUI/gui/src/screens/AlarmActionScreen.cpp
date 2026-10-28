/**
 ******************************************************************************
 * @file    AlarmActionScreen.cpp
 * @brief   Per-alarm slot menu: Toggle ON/OFF or Edit Time (HH:MM).
 ******************************************************************************
 */

#include "gui/screens/AlarmActionScreen.hpp"
#include "gui/screens/ScreenManager.hpp"
#include "gui/Assets.hpp"

#include <cstdio>

using namespace SDK::GUI;
using Style = Widgets::WheelMenu::Item::Style;

AlarmActionScreen::AlarmActionScreen(Model& model)
    : Screen(model)
{
}

void AlarmActionScreen::populateItems()
{
    const std::size_t          medIdx   = mModel.getSelectedMedIndex();
    const std::size_t          alarmIdx = mModel.getSelectedAlarmIndex();
    const PillSync::AlarmSlot& slot     = mModel.getMedication(medIdx).alarms[alarmIdx];

    std::snprintf(mTitleBuf, sizeof(mTitleBuf), "ALARM %zu", alarmIdx + 1);
    mModel.formatTime(slot.hour, slot.minute, mTimeTipBuf, sizeof(mTimeTipBuf));

    mItems[kItemToggle].style       = Style::Toggle;
    mItems[kItemToggle].text        = "Enabled";
    mItems[kItemToggle].itemText    = "Enabled";
    mItems[kItemToggle].centerFont  = &poppins_semibold_25;
    mItems[kItemToggle].toggleState = slot.enabled;

    mItems[kItemSetTime].style      = Style::Tip;
    mItems[kItemSetTime].text       = "Set Time";
    mItems[kItemSetTime].itemText   = "Set Time";
    mItems[kItemSetTime].centerFont = &poppins_semibold_25;
    mItems[kItemSetTime].tip        = mTimeTipBuf;
    mItems[kItemSetTime].tipColor   = Color::TEAL;

    std::snprintf(mAlertTipBuf, sizeof(mAlertTipBuf), "%s",
                  mModel.isVibrateOnly() ? "Vibrate Only" : "Sound & Vibrate");
    mItems[kItemAlertMode].style      = Style::Tip;
    mItems[kItemAlertMode].text       = "Alert Mode";
    mItems[kItemAlertMode].itemText   = "Alert Mode";
    mItems[kItemAlertMode].centerFont = &poppins_semibold_25;
    mItems[kItemAlertMode].tip        = mAlertTipBuf;
    mItems[kItemAlertMode].tipColor   = Color::TEAL;
}

void AlarmActionScreen::build()
{
    populateItems();

    mMenu    = std::make_unique<Widgets::WheelMenu>(mRoot, mItems, kItemCount);
    mMenu->setCircular(false);
    mButtons = std::make_unique<Widgets::Buttons>(mRoot);
    mTitle   = std::make_unique<Widgets::Title>(mRoot, mTitleBuf);

    mButtons->set(Widgets::Buttons::NONE,
                  Widgets::Buttons::NONE,
                  Widgets::Buttons::AMBER,
                  Widgets::Buttons::WHITE);
}

void AlarmActionScreen::onShow()
{
    populateItems();
    mTitle->setText(mTitleBuf);
    mMenu->refresh();
    mMenu->select(static_cast<uint16_t>(mModel.getAlarmActionCursor()));
    mModel.resetIdleTimer();
}

void AlarmActionScreen::onHide()
{
    mModel.setAlarmActionCursor(mMenu->selected());
}

void AlarmActionScreen::onScheduleUpdated()
{
    populateItems();
    if (mTitle) {
        mTitle->setText(mTitleBuf);
    }
    if (mMenu) {
        mMenu->refresh();
    }
}

void AlarmActionScreen::onKey(uint8_t code)
{
    namespace Btn = SDK::GUI::Button;
    switch (code) {
        case Btn::L1:
            mMenu->prev();
            break;
        case Btn::L2:
            mMenu->next();
            break;
        case Btn::R1: {
            const uint16_t sel = mMenu->selected();
            if (sel == kItemToggle) {
                PillSync::Medication& med  = mModel.mutableMedication(mModel.getSelectedMedIndex());
                PillSync::AlarmSlot&  slot = med.alarms[mModel.getSelectedAlarmIndex()];
                slot.enabled               = !slot.enabled;
                if (slot.enabled) {
                    med.enabled = true;
                }
                mModel.saveMedications();
            } else if (sel == kItemSetTime) {
                mModel.setAlarmActionCursor(sel);
                ScreenManager::instance().switchTo(ScreenId::TimePicker);
            } else if (sel == kItemAlertMode) {
                mModel.toggleVibrateOnly();
            }
        } break;
        case Btn::R2:
            mModel.setAlarmActionCursor(0);
            ScreenManager::instance().switchTo(ScreenId::MedDetail);
            break;
        default:
            break;
    }
}

