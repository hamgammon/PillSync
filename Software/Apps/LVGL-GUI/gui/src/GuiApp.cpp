/**
 ******************************************************************************
 * @file    GuiApp.cpp
 * @brief   PillSync LVGL GUI entry point.
 ******************************************************************************
 */

#include <new>

#include "gui/Assets.hpp"
#include "gui/model/Model.hpp"
#include "gui/theme/Theme.hpp"
#include "gui/screens/ScreenManager.hpp"

namespace
{

alignas(Model) uint8_t sModelStorage[sizeof(Model)];
Model* sModel = nullptr;

} // namespace

extern "C" const lv_font_t* una_lvgl_default_font(void)
{
    return &poppins_regular_18;
}

extern "C" void una_lvgl_app_init(void)
{
    Theme::init();
    sModel = new (sModelStorage) Model();
    ScreenManager::instance().start(*sModel, ScreenId::Main);
}

