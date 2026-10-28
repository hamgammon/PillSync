/**
 ******************************************************************************
 * @file    MedDetailScreen.cpp
 * @brief   Per-medication menu: Enable toggle + Alarm 1, Alarm 2, Alarm 3.
 ******************************************************************************
 */

#include "gui/screens/MedDetailScreen.hpp"
#include "gui/screens/ScreenManager.hpp"
#include "gui/Assets.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>

using namespace SDK::GUI;
using Style = Widgets::WheelMenu::Item::Style;

MedDetailScreen::MedDetailScreen(Model& model)
    : Screen(model)
{
}

void MedDetailScreen::populateItems()
{
    const std::size_t           medIdx = mModel.getSelectedMedIndex();
    const PillSync::Medication& med    = mModel.getMedication(medIdx);

    // Format uppercase title
    for (std::size_t i = 0; i < PillSync::kMaxMedNameLen && med.name[i] != '\0'; ++i) {
        mTitleBuf[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(med.name[i])));
        mTitleBuf[i + 1] = '\0';
    }
    if (med.name[0] == '\0') {
        std::snprintf(mTitleBuf, sizeof(mTitleBuf), "MEDICATION %zu", medIdx + 1);
    }

    // Item 0: Medication Enabled toggle
    mItems[kItemEnabled].style       = Style::Toggle;
    mItems[kItemEnabled].text        = "Enabled";
    mItems[kItemEnabled].itemText    = "Enabled";
    mItems[kItemEnabled].centerFont  = &poppins_semibold_25;
    mItems[kItemEnabled].toggleState = med.enabled;

    // Items 1..3: Alarm 1..3
    for (std::size_t j = 0; j < PillSync::kAlarmsPerMed; ++j) {
        const PillSync::AlarmSlot& slot = med.alarms[j];
        std::snprintf(mAlarmLabelBufs[j], sizeof(mAlarmLabelBufs[j]), "Alarm %zu", j + 1);

        char timeStr[16] {};
        mModel.formatTime(slot.hour, slot.minute, timeStr, sizeof(timeStr));
        std::snprintf(mAlarmTipBufs[j], sizeof(mAlarmTipBufs[j]), "%s (%s)",
                      timeStr, slot.enabled ? "ON" : "OFF");

        const uint16_t itemIdx       = static_cast<uint16_t>(kItemAlarm1 + j);
        mItems[itemIdx].style        = Style::Tip;
        mItems[itemIdx].text         = mAlarmLabelBufs[j];
        mItems[itemIdx].itemText     = mAlarmLabelBufs[j];
        mItems[itemIdx].centerFont   = &poppins_semibold_25;
        mItems[itemIdx].tip          = mAlarmTipBufs[j];
        mItems[itemIdx].tipColor     = slot.enabled ? Color::TEAL : Color::GRAY;
    }
}

void MedDetailScreen::build()
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

void MedDetailScreen::onShow()
{
    populateItems();
    mTitle->setText(mTitleBuf);
    mMenu->refresh();
    mMenu->select(static_cast<uint16_t>(mModel.getMedDetailCursor()));
    mModel.resetIdleTimer();
}

void MedDetailScreen::onHide()
{
    mModel.setMedDetailCursor(mMenu->selected());
}

void MedDetailScreen::onScheduleUpdated()
{
    populateItems();
    if (mTitle) {
        mTitle->setText(mTitleBuf);
    }
    if (mMenu) {
        mMenu->refresh();
    }
}

void MedDetailScreen::onKey(uint8_t code)
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
            if (sel == kItemEnabled) {
                PillSync::Medication& med = mModel.mutableMedication(mModel.getSelectedMedIndex());
                med.enabled = !med.enabled;
                mModel.saveMedications();
            } else if (sel >= kItemAlarm1 && sel <= kItemAlarm3) {
                mModel.setMedDetailCursor(sel);
                mModel.setSelectedAlarmIndex(sel - kItemAlarm1);
                mModel.setAlarmActionCursor(0);
                ScreenManager::instance().switchTo(ScreenId::AlarmAction);
            }
        } break;
        case Btn::R2:
            mModel.setMedDetailCursor(0);
            ScreenManager::instance().switchTo(ScreenId::Main);
            break;
        default:
            break;
    }
}

