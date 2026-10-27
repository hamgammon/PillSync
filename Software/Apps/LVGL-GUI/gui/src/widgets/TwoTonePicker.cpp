/**
 ******************************************************************************
 * @file    TwoTonePicker.cpp
 * @brief   Shared LVGL widgets for PillSync (WheelMenu, TwoTonePicker).
 ******************************************************************************
 */

#include "gui/widgets/TwoTonePicker.hpp"
#include "gui/model/Model.hpp"

using namespace SDK::GUI;

namespace Widgets
{

namespace
{
constexpr uint32_t kPickerActive   = Color::TEAL;
constexpr uint32_t kPickerInactive = Color::WHITE;

SDK::LVGL::WheelMenu::Fonts pillSyncMenuFonts()
{
    return {
        Theme::font(Theme::Font::SemiBold30),
        Theme::font(Theme::Font::Medium18),
        Theme::font(Theme::Font::Italic18)
    };
}
} // namespace

WheelMenu::WheelMenu(lv_obj_t* parent, const Item* items, uint16_t count, int16_t itemOffsetY)
    : SDK::LVGL::WheelMenu(parent, items, count, pillSyncMenuFonts(), itemOffsetY)
{
    setAnimationMs(App::Config::kMenuAnimationMs);
}

TwoTonePicker::TwoTonePicker(lv_obj_t* parent)
{
    using F  = Theme::Font;
    mButtons = std::make_unique<Buttons>(parent);
    mButtons->set(Buttons::WHITE, Buttons::WHITE, Buttons::AMBER, Buttons::WHITE);

    mSubLeft  = Theme::label(parent, F::Italic20, "", 5, 58, 110, LV_TEXT_ALIGN_CENTER, kPickerActive);
    mSubRight = Theme::label(parent, F::Italic20, "", 127, 58, 108, LV_TEXT_ALIGN_CENTER, kPickerInactive);
    mValLeft  = Theme::label(parent, F::SemiBold60, "", 5, 92, 110, LV_TEXT_ALIGN_RIGHT, kPickerActive);
    mValSep   = Theme::label(parent, F::Light60, "", 110, 92, 22, LV_TEXT_ALIGN_CENTER, kPickerInactive);
    mValRight = Theme::label(parent, F::Light60, "", 127, 92, 108, LV_TEXT_ALIGN_LEFT, kPickerInactive);
    mNext1    = Theme::label(parent, F::Medium40, "", 5, 151, 110, LV_TEXT_ALIGN_RIGHT, kPickerInactive);
    mNext2    = Theme::label(parent, F::Medium25, "", 5, 193, 110, LV_TEXT_ALIGN_RIGHT, kPickerInactive);
    mTitle    = std::make_unique<Title>(parent, "");
}

void TwoTonePicker::setTitle(const char* title)
{
    mTitle->setText(title);
}

void TwoTonePicker::renderSubtitleDual(const char* left, const char* right, bool leftActive)
{
    lv_obj_set_pos(mSubLeft, 5, 58);
    lv_obj_set_width(mSubLeft, 110);
    lv_label_set_text(mSubLeft, left);
    lv_obj_set_style_text_color(mSubLeft, Theme::rgb(leftActive ? kPickerActive : kPickerInactive), LV_PART_MAIN);
    lv_label_set_text(mSubRight, right);
    lv_obj_set_style_text_color(mSubRight, Theme::rgb(leftActive ? kPickerInactive : kPickerActive), LV_PART_MAIN);
}

void TwoTonePicker::renderValue(bool leftActive, const char* left, const char* right,
                                const char* sep, const char* up1, const char* up2)
{
    lv_label_set_text(mValSep, sep);

    const auto leftFont  = Theme::font(leftActive ? Theme::Font::SemiBold60 : Theme::Font::Light60);
    const auto rightFont = Theme::font(leftActive ? Theme::Font::Light60 : Theme::Font::SemiBold60);

    lv_label_set_text(mValLeft, left);
    lv_obj_set_style_text_font(mValLeft, leftFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(mValLeft, Theme::rgb(leftActive ? kPickerActive : kPickerInactive), LV_PART_MAIN);

    lv_label_set_text(mValRight, right);
    lv_obj_set_style_text_font(mValRight, rightFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(mValRight, Theme::rgb(leftActive ? kPickerInactive : kPickerActive), LV_PART_MAIN);

    if (leftActive) {
        lv_obj_set_pos(mNext1, 5, 151);
        lv_obj_set_width(mNext1, 110);
        lv_obj_set_pos(mNext2, 5, 193);
        lv_obj_set_width(mNext2, 110);
        lv_obj_set_style_text_align(mNext1, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
        lv_obj_set_style_text_align(mNext2, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    } else {
        lv_obj_set_pos(mNext1, 127, 151);
        lv_obj_set_width(mNext1, 108);
        lv_obj_set_pos(mNext2, 127, 193);
        lv_obj_set_width(mNext2, 108);
        lv_obj_set_style_text_align(mNext1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
        lv_obj_set_style_text_align(mNext2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    }
    lv_label_set_text(mNext1, up1 ? up1 : "");
    lv_label_set_text(mNext2, up2 ? up2 : "");
}

} // namespace Widgets

