/**
 ******************************************************************************
 * @file    RingingScreen.hpp
 * @brief   Active medication reminder screen with Snooze (up to 3x) & Dismiss.
 ******************************************************************************
 */

#ifndef RINGING_SCREEN_HPP
#define RINGING_SCREEN_HPP

#include <memory>

#include "gui/screens/Screen.hpp"
#include "gui/widgets/TwoTonePicker.hpp"

class RingingScreen : public Screen
{
public:
    explicit RingingScreen(Model& model);
    ~RingingScreen() override;

    void onShow() override;
    void onHide() override;
    void onKey(uint8_t code) override;

    void onIdleTimeout() override;
    void onAlarmActivated(const PillSync::ActiveAlert& alert) override;

protected:
    void build() override;

private:
    static constexpr uint32_t kPlayIntervalMs      = 5000;
    static constexpr uint32_t kRingingTimeoutMs    = 120000;

    static void playTimerCb(lv_timer_t* t);
    static void timeoutTimerCb(lv_timer_t* t);

    void updateDisplay();
    void stopTimers();
    void handleSnooze();
    void handleDismiss();

    std::unique_ptr<Widgets::Title>   mTitle;
    std::unique_ptr<Widgets::Buttons> mButtons;

    lv_obj_t* mMedNameLabel    = nullptr;
    lv_obj_t* mTimeLabel       = nullptr;
    lv_obj_t* mSnoozeInfoLabel = nullptr;
    lv_obj_t* mSnoozeHint      = nullptr;
    lv_obj_t* mDismissHint     = nullptr;

    lv_timer_t* mPlayTimer    = nullptr;
    lv_timer_t* mTimeoutTimer = nullptr;
};

#endif // RINGING_SCREEN_HPP

