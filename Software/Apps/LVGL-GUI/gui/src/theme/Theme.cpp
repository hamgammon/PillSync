/**
 ******************************************************************************
 * @file    Theme.cpp
 * @brief   Font lookup table for PillSync.
 ******************************************************************************
 */

#include "gui/theme/Theme.hpp"
#include "gui/Assets.hpp"

namespace Theme
{

const lv_font_t* font(Font id)
{
    switch (id) {
        case Font::Regular18:  return &poppins_regular_18;
        case Font::Italic18:   return &poppins_italic_18;
        case Font::Italic20:   return &poppins_italic_20;
        case Font::Medium18:   return &poppins_medium_18;
        case Font::Medium25:   return &poppins_medium_25;
        case Font::Medium40:   return &poppins_medium_40;
        case Font::SemiBold20: return &poppins_semibold_20;
        case Font::SemiBold25: return &poppins_semibold_25;
        case Font::SemiBold30: return &poppins_semibold_30;
        case Font::SemiBold35: return &poppins_semibold_35;
        case Font::SemiBold40: return &poppins_semibold_40;
        case Font::Light60:    return &poppins_light_60;
        case Font::SemiBold60: return &poppins_semibold_60;
    }
    return &poppins_regular_18;
}

} // namespace Theme

