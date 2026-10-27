/**
 ******************************************************************************
 * @file    TimePickerScreen.cpp
 * @brief   Two-stage HH:MM time picker screen using TwoTonePicker.
 ******************************************************************************
 */

#include "gui/screens/TimePickerScreen.hpp"
#include "gui/screens/ScreenManager.hpp"

#include <cstdio>

TimePickerScreen::TimePickerScreen(Model& model)
    : Screen(model)
{
    const std::size_t          medIdx   = mModel.getSelectedMedIndex();
    const std::size_t          alarmIdx = mModel.getSelectedAlarmIndex();
    const PillSync::AlarmSlot& slot     = mModel.getMedication(medIdx).alarms[alarmIdx];

    mHour      = slot.hour % 24;
    mMinute    = slot.minute % 60;
    mEditHours = true;
    std::snprintf(mTitleBuf, sizeof(mTitleBuf), "ALARM %zu TIME", alarmIdx + 1);
}

void TimePickerScreen::build()
{
    mPicker = std::make_unique<Widgets::TwoTonePicker>(mRoot);
    mPicker->setTitle(mTitleBuf);
    render();
}

void TimePickerScreen::onShow()
{
    mModel.resetIdleTimer();
    render();
}

void TimePickerScreen::render()
{
    char hStr[8] {}, mStr[8] {}, up1[8] {}, up2[8] {};
    std::snprintf(hStr, sizeof(hStr), "%02u", static_cast<unsigned>(mHour));
    std::snprintf(mStr, sizeof(mStr), "%02u", static_cast<unsigned>(mMinute));

    if (mEditHours) {
        std::snprintf(up1, sizeof(up1), "%02u", static_cast<unsigned>((mHour + 1) % 24));
        std::snprintf(up2, sizeof(up2), "%02u", static_cast<unsigned>((mHour + 2) % 24));
    } else {
        std::snprintf(up1, sizeof(up1), "%02u", static_cast<unsigned>((mMinute + 1) % 60));
        std::snprintf(up2, sizeof(up2), "%02u", static_cast<unsigned>((mMinute + 2) % 60));
    }

    mPicker->renderSubtitleDual("hr", "min", mEditHours);
    mPicker->renderValue(mEditHours, hStr, mStr, ":", up1, up2);
}

void TimePickerScreen::onKey(uint8_t code)
{
    namespace Btn = SDK::GUI::Button;
    switch (code) {
        case Btn::L1:
            if (mEditHours) {
                mHour = static_cast<uint8_t>((mHour + 23) % 24);
            } else {
                mMinute = static_cast<uint8_t>((mMinute + 59) % 60);
            }
            render();
            break;

        case Btn::L2:
            if (mEditHours) {
                mHour = static_cast<uint8_t>((mHour + 1) % 24);
            } else {
                mMinute = static_cast<uint8_t>((mMinute + 1) % 60);
            }
            render();
            break;

        case Btn::R1:
            if (mEditHours) {
                mEditHours = false;
                render();
            } else {
                PillSync::Medication& med  = mModel.mutableMedication(mModel.getSelectedMedIndex());
                PillSync::AlarmSlot&  slot = med.alarms[mModel.getSelectedAlarmIndex()];
                slot.hour                  = mHour;
                slot.minute                = mMinute;
                slot.enabled               = true;
                med.enabled                = true;
                mModel.saveMedications();
                ScreenManager::instance().switchTo(ScreenId::Saved);
            }
            break;

        case Btn::R2:
            if (!mEditHours) {
                mEditHours = true;
                render();
            } else {
                ScreenManager::instance().switchTo(ScreenId::AlarmAction);
            }
            break;

        default:
            break;
    }
}

