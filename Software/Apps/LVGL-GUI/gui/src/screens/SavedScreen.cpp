/**
 ******************************************************************************
 * @file    SavedScreen.cpp
 * @brief   Brief confirmation screen shown after saving an alarm time.
 ******************************************************************************
 */

#include "gui/screens/SavedScreen.hpp"
#include "gui/screens/ScreenManager.hpp"
#include "gui/theme/Theme.hpp"

using namespace SDK::GUI;

SavedScreen::SavedScreen(Model& model)
    : Screen(model)
{
}

SavedScreen::~SavedScreen()
{
    stopTimer();
}

void SavedScreen::build()
{
    Theme::label(mRoot, Theme::Font::SemiBold35, "Saved", 0, 95, 240,
                 LV_TEXT_ALIGN_CENTER, Color::TEAL);
}

void SavedScreen::onShow()
{
    mModel.resetIdleTimer();
    stopTimer();
    mTimer = lv_timer_create(&SavedScreen::timerCb, 1000, this);
    if (mTimer) {
        lv_timer_set_repeat_count(mTimer, 1);
    }
}

void SavedScreen::onHide()
{
    stopTimer();
}

void SavedScreen::onKey(uint8_t code)
{
    namespace Btn = SDK::GUI::Button;
    if (code == Btn::R1 || code == Btn::R2) {
        stopTimer();
        ScreenManager::instance().switchTo(ScreenId::MedDetail);
    }
}

void SavedScreen::timerCb(lv_timer_t* t)
{
    auto* self = static_cast<SavedScreen*>(lv_timer_get_user_data(t));
    self->mTimer = nullptr;
    ScreenManager::instance().switchTo(ScreenId::MedDetail);
}

void SavedScreen::stopTimer()
{
    if (mTimer) {
        lv_timer_delete(mTimer);
        mTimer = nullptr;
    }
}

