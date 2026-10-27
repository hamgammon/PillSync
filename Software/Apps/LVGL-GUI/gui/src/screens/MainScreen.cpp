/**
 ******************************************************************************
 * @file    MainScreen.cpp
 * @brief   Main wheel menu listing the 5 medications and their active status.
 ******************************************************************************
 */

#include "gui/screens/MainScreen.hpp"
#include "gui/screens/ScreenManager.hpp"
#include "gui/Assets.hpp"

#include <cstdio>

using namespace SDK::GUI;
using Style = Widgets::WheelMenu::Item::Style;

MainScreen::MainScreen(Model& model)
    : Screen(model)
{
}

void MainScreen::populateItems()
{
    for (std::size_t i = 0; i < PillSync::kMaxMedications; ++i) {
        const PillSync::Medication& med   = mModel.getMedication(i);
        const uint8_t               count = med.activeAlarmCount();

        if (!med.enabled || count == 0) {
            std::snprintf(mTipBufs[i], sizeof(mTipBufs[i]), "OFF");
        } else if (count == 1) {
            std::snprintf(mTipBufs[i], sizeof(mTipBufs[i]), "1 alarm ON");
        } else {
            std::snprintf(mTipBufs[i], sizeof(mTipBufs[i]), "%u alarms ON",
                          static_cast<unsigned>(count));
        }

        mItems[i].style      = Style::Tip;
        mItems[i].text       = med.name;
        mItems[i].itemText   = med.name;
        mItems[i].centerFont = &poppins_semibold_25;
        mItems[i].tip        = mTipBufs[i];
        mItems[i].tipColor   = (med.enabled && count > 0) ? Color::TEAL : Color::GRAY;
    }
}

void MainScreen::build()
{
    populateItems();

    mMenu    = std::make_unique<Widgets::WheelMenu>(mRoot, mItems, PillSync::kMaxMedications);
    mButtons = std::make_unique<Widgets::Buttons>(mRoot);
    mTitle   = std::make_unique<Widgets::Title>(mRoot, "PILLSYNC");

    mButtons->set(Widgets::Buttons::NONE,
                  Widgets::Buttons::NONE,
                  Widgets::Buttons::AMBER,
                  Widgets::Buttons::WHITE);
}

void MainScreen::onShow()
{
    populateItems();
    mMenu->refresh();
    mMenu->select(static_cast<uint16_t>(mModel.getSelectedMedIndex()));
    mModel.resetIdleTimer();

    if (mModel.getActiveAlert().active) {
        ScreenManager::instance().switchTo(ScreenId::Ringing);
    }
}

void MainScreen::onHide()
{
    mModel.setSelectedMedIndex(mMenu->selected());
}

void MainScreen::onScheduleUpdated()
{
    populateItems();
    if (mMenu) {
        mMenu->refresh();
    }
}

void MainScreen::onKey(uint8_t code)
{
    namespace Btn = SDK::GUI::Button;
    switch (code) {
        case Btn::L1:
            mMenu->prev();
            break;
        case Btn::L2:
            mMenu->next();
            break;
        case Btn::R1:
            mModel.setSelectedMedIndex(mMenu->selected());
            mModel.setMedDetailCursor(0);
            ScreenManager::instance().switchTo(ScreenId::MedDetail);
            break;
        case Btn::R2:
            mModel.exitApp();
            break;
        default:
            break;
    }
}

