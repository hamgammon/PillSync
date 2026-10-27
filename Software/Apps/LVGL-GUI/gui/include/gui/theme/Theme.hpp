/**
 ******************************************************************************
 * @file    Theme.hpp
 * @brief   LVGL theme, font table, and SDK::LVGL::Draw helpers for PillSync.
 ******************************************************************************
 */

#ifndef THEME_HPP
#define THEME_HPP

#include <cstdint>

#include "lvgl.h"

#include "SDK/GUI/Color.hpp"
#include "SDK/GUI/Config.hpp"
#include "SDK/GUI/LVGL/Draw.hpp"

namespace Theme
{

enum class Font : uint8_t {
    Regular18,
    Italic18,
    Italic20,
    Medium18,
    Medium25,
    Medium40,
    SemiBold20,
    SemiBold25,
    SemiBold30,
    SemiBold35,
    SemiBold40,
    Light60,
    SemiBold60,
};

const lv_font_t* font(Font id);

using SDK::LVGL::Draw::rgb;
using SDK::LVGL::Draw::arcAngle;
using SDK::LVGL::Draw::init;
using SDK::LVGL::Draw::applyScreen;
using SDK::LVGL::Draw::setHidden;
using SDK::LVGL::Draw::container;
using SDK::LVGL::Draw::label;
using SDK::LVGL::Draw::hline;
using SDK::LVGL::Draw::vline;
using SDK::LVGL::Draw::box;
using SDK::LVGL::Draw::image;
using SDK::LVGL::Draw::imageTinted;
using SDK::LVGL::Draw::tint;
using SDK::LVGL::Draw::dot;
using SDK::LVGL::Draw::arc;
using SDK::LVGL::Draw::setArc;
using SDK::LVGL::Draw::setArcColor;

inline lv_obj_t* label(lv_obj_t* parent, Font f, const char* text,
                       int32_t x, int32_t y, int32_t w,
                       lv_text_align_t align = LV_TEXT_ALIGN_CENTER,
                       uint32_t color = SDK::GUI::Color::WHITE)
{
    return SDK::LVGL::Draw::label(parent, font(f), text, x, y, w, align, color);
}

} // namespace Theme

#endif // THEME_HPP

