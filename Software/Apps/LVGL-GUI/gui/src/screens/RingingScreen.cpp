/**
 ******************************************************************************
 * @file    RingingScreen.cpp
 * @brief   Active medication reminder screen with Snooze (up to 3x) & Dismiss.
 ******************************************************************************
 */

#include "gui/screens/RingingScreen.hpp"
#include "gui/screens/ScreenManager.hpp"
#include "gui/theme/Theme.hpp"

#include <cstdio>

using namespace SDK::GUI;

RingingScreen::RingingScreen(Model& model)
    : Screen(model)
{
}

RingingScreen::~RingingScreen()
{
    stopTimers();
}

void RingingScreen::build()
{
    mButtons = std::make_unique<Widgets::Buttons>(mRoot);
    mTitle   = std::make_unique<Widgets::Title>(mRoot, "PILL ALERT");

    mSnoozeHint = Theme::label(mRoot, Theme::Font::Italic18, "Snooze",
                               106, 52, 100, LV_TEXT_ALIGN_RIGHT, Color::TEAL);

    mMedNameLabel = Theme::label(mRoot, Theme::Font::SemiBold25, "",
                                 12, 82, 216, LV_TEXT_ALIGN_CENTER, Color::YELLOW_DARK);

    mTimeLabel = Theme::label(mRoot, Theme::Font::SemiBold25, "",
                              12, 114, 216, LV_TEXT_ALIGN_CENTER, Color::WHITE);

    mSnoozeInfoLabel = Theme::label(mRoot, Theme::Font::Regular18, "",
                                    12, 146, 216, LV_TEXT_ALIGN_CENTER, Color::GRAY);

    mDismissHint = Theme::label(mRoot, Theme::Font::Italic18, "Dismiss",
                                108, 168, 100, LV_TEXT_ALIGN_RIGHT, Color::WHITE);

    updateDisplay();
}

void RingingScreen::updateDisplay()
{
    const PillSync::ActiveAlert& alert = mModel.getActiveAlert();

    lv_label_set_text(mMedNameLabel, alert.medName[0] ? alert.medName : "Medication");

    char timeBuf[16] {};
    mModel.formatTime(alert.hour, alert.minute, timeBuf, sizeof(timeBuf));
    lv_label_set_text(mTimeLabel, timeBuf);

    char infoBuf[32] {};
    if (alert.snoozesLeft > 0) {
        std::snprintf(infoBuf, sizeof(infoBuf), "Snooze: %u left",
                      static_cast<unsigned>(alert.snoozesLeft));
        lv_label_set_text(mSnoozeInfoLabel, infoBuf);
        lv_obj_set_style_text_color(mSnoozeInfoLabel, Theme::rgb(Color::TEAL), LV_PART_MAIN);

        mButtons->set(Widgets::Buttons::NONE,
                      Widgets::Buttons::NONE,
                      Widgets::Buttons::AMBER,
                      Widgets::Buttons::GREEN);
        Theme::setHidden(mSnoozeHint, false);
    } else {
        std::snprintf(infoBuf, sizeof(infoBuf), "Max snoozes (3/3)");
        lv_label_set_text(mSnoozeInfoLabel, infoBuf);
        lv_obj_set_style_text_color(mSnoozeInfoLabel, Theme::rgb(Color::YELLOW_DARK), LV_PART_MAIN);

        mButtons->set(Widgets::Buttons::NONE,
                      Widgets::Buttons::NONE,
                      Widgets::Buttons::NONE,
                      Widgets::Buttons::GREEN);
        Theme::setHidden(mSnoozeHint, true);
    }
}

void RingingScreen::onShow()
{
    updateDisplay();
    mModel.playAlarmEffect();

    stopTimers();
    mPlayTimer    = lv_timer_create(&RingingScreen::playTimerCb, kPlayIntervalMs, this);
    mTimeoutTimer = lv_timer_create(&RingingScreen::timeoutTimerCb, kRingingTimeoutMs, this);
    if (mTimeoutTimer) {
        lv_timer_set_repeat_count(mTimeoutTimer, 1);
    }
}

void RingingScreen::onHide()
{
    stopTimers();
    if (mModel.getActiveAlert().active) {
        mModel.dismissAlarm();
    }
}

void RingingScreen::onIdleTimeout()
{
    // Do not exit on normal 30s idle timeout while alarm is actively ringing;
    // mTimeoutTimer (120s) handles auto-snooze/dismiss.
}

void RingingScreen::onAlarmActivated(const PillSync::ActiveAlert& /*alert*/)
{
    updateDisplay();
    mModel.playAlarmEffect();
    if (mTimeoutTimer) {
        lv_timer_reset(mTimeoutTimer);
    }
}

void RingingScreen::onKey(uint8_t code)
{
    namespace Btn = SDK::GUI::Button;
    if (code == Btn::R1) {
        if (mModel.getActiveAlert().snoozesLeft > 0) {
            handleSnooze();
        }
    } else if (code == Btn::R2) {
        handleDismiss();
    }
}

void RingingScreen::handleSnooze()
{
    stopTimers();
    mModel.snoozeAlarm();
    mModel.switchToNextPriorityScreen();
}

void RingingScreen::handleDismiss()
{
    stopTimers();
    mModel.dismissAlarm();
    mModel.switchToNextPriorityScreen();
}

void RingingScreen::playTimerCb(lv_timer_t* t)
{
    auto* self = static_cast<RingingScreen*>(lv_timer_get_user_data(t));
    self->mModel.playAlarmEffect();
}

void RingingScreen::timeoutTimerCb(lv_timer_t* t)
{
    auto* self = static_cast<RingingScreen*>(lv_timer_get_user_data(t));
    self->mTimeoutTimer = nullptr;
    if (self->mModel.getActiveAlert().snoozesLeft > 0) {
        self->handleSnooze();
    } else {
        self->handleDismiss();
    }
}

void RingingScreen::stopTimers()
{
    if (mPlayTimer) {
        lv_timer_delete(mPlayTimer);
        mPlayTimer = nullptr;
    }
    if (mTimeoutTimer) {
        lv_timer_delete(mTimeoutTimer);
        mTimeoutTimer = nullptr;
    }
}
