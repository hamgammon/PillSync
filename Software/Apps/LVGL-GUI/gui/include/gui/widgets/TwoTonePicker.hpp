/**
 ******************************************************************************
 * @file    TwoTonePicker.hpp
 * @brief   Shared LVGL widgets for PillSync (Title, Buttons, WheelMenu, TwoTonePicker).
 ******************************************************************************
 */

#ifndef TWO_TONE_PICKER_HPP
#define TWO_TONE_PICKER_HPP

#include "lvgl.h"
#include "SDK/GUI/LVGL/Buttons.hpp"
#include "SDK/GUI/LVGL/Title.hpp"
#include "SDK/GUI/LVGL/WheelMenu.hpp"
#include "gui/theme/Theme.hpp"

#include <cstdint>
#include <memory>

namespace Widgets
{

using Buttons = SDK::LVGL::Buttons;

class Title : public SDK::LVGL::Title
{
public:
    Title(lv_obj_t* parent, const char* text)
        : SDK::LVGL::Title(parent, Theme::font(Theme::Font::Italic18), text)
    {}
};

class WheelMenu : public SDK::LVGL::WheelMenu
{
public:
    WheelMenu(lv_obj_t* parent, const Item* items, uint16_t count, int16_t itemOffsetY = 0);
};

class TwoTonePicker
{
public:
    explicit TwoTonePicker(lv_obj_t* parent);

    void setTitle(const char* title);
    void renderSubtitleDual(const char* left, const char* right, bool leftActive);
    void renderValue(bool leftActive, const char* left, const char* right,
                     const char* sep, const char* up1 = nullptr, const char* up2 = nullptr);

private:
    std::unique_ptr<Buttons> mButtons;
    std::unique_ptr<Title>   mTitle;
    lv_obj_t* mSubLeft  = nullptr;
    lv_obj_t* mSubRight = nullptr;
    lv_obj_t* mValLeft  = nullptr;
    lv_obj_t* mValSep   = nullptr;
    lv_obj_t* mValRight = nullptr;
    lv_obj_t* mNext1    = nullptr;
    lv_obj_t* mNext2    = nullptr;
};

} // namespace Widgets

#endif // TWO_TONE_PICKER_HPP

