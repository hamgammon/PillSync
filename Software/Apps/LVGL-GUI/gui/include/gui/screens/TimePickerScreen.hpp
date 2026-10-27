/**
 ******************************************************************************
 * @file    TimePickerScreen.hpp
 * @brief   Two-stage HH:MM time picker screen using TwoTonePicker.
 ******************************************************************************
 */

#ifndef TIME_PICKER_SCREEN_HPP
#define TIME_PICKER_SCREEN_HPP

#include <memory>

#include "gui/screens/Screen.hpp"
#include "gui/widgets/TwoTonePicker.hpp"

class TimePickerScreen : public Screen
{
public:
    explicit TimePickerScreen(Model& model);

    void onShow() override;
    void onKey(uint8_t code) override;

protected:
    void build() override;

private:
    void render();

    uint8_t mHour      = 9;
    uint8_t mMinute    = 0;
    bool    mEditHours = true;
    char    mTitleBuf[24] {};

    std::unique_ptr<Widgets::TwoTonePicker> mPicker;
};

#endif // TIME_PICKER_SCREEN_HPP

