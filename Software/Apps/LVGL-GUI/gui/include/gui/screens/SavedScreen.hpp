/**
 ******************************************************************************
 * @file    SavedScreen.hpp
 * @brief   Brief confirmation screen shown after saving an alarm time.
 ******************************************************************************
 */

#ifndef SAVED_SCREEN_HPP
#define SAVED_SCREEN_HPP

#include "gui/screens/Screen.hpp"

class SavedScreen : public Screen
{
public:
    explicit SavedScreen(Model& model);
    ~SavedScreen() override;

    void onShow() override;
    void onHide() override;
    void onKey(uint8_t code) override;

protected:
    void build() override;

private:
    static void timerCb(lv_timer_t* t);
    void        stopTimer();

    lv_timer_t* mTimer = nullptr;
};

#endif // SAVED_SCREEN_HPP

