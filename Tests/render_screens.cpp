/**
 ******************************************************************************
 * @file    render_screens.cpp
 * @brief   Headless LVGL renderer using the UNA Watch SDK LVGL port to export
 *          PillSync 240x240 ABGR2222 framebuffers for watch mockup previews.
 ******************************************************************************
 */

#include "KernelTestDoubles.hpp"

#include "SDK/Kernel/KernelProviderGUI.hpp"
#include "SDK/Port/LVGL/LvglPort.hpp"
#include "SDK/UnaLogger/Logger.h"

#include "gui/model/Model.hpp"
#include "gui/screens/ScreenManager.hpp"
#include "gui/theme/Theme.hpp"

#include "lvgl.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace
{

void saveFrame(const std::string& path)
{
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(nullptr);

    const uint8_t* frame = SDK::LVGL::Port::GetInstance().frame();
    FILE*          fp    = std::fopen(path.c_str(), "wb");
    if (fp) {
        std::fwrite(frame, 1, SDK::LVGL::kDisplayWidth * SDK::LVGL::kDisplayHeight, fp);
        std::fclose(fp);
        std::printf("Saved %s\n", path.c_str());
    }
}

void pumpAsyncAndRender(SDK::TestSupport::KernelFixture& fx, const std::string& path)
{
    for (int i = 0; i < 5; ++i) {
        fx.system.nowMs += 35;
        lv_timer_handler();
    }
    saveFrame(path);
}

} // namespace

int main(int argc, char** argv)
{
    std::string outDir = ".";
    if (argc > 1) {
        outDir = argv[1];
    }

    SDK::TestSupport::KernelFixture fx;
    SDK::KernelProviderGUI::CreateInstance(&fx.kernel);
    Logger_init(SDK::KernelProviderGUI::GetInstance().getKernel().log);

    SDK::LVGL::Port::GetInstance().init();
    Theme::init();

    Model model;

    // Populate realistic medication schedule
    {
        auto& m0 = model.mutableMedication(0);
        m0.setName("Amoxicillin");
        m0.enabled           = true;
        m0.alarms[0]         = { true, 9, 0 };
        m0.alarms[1]         = { true, 10, 0 };
        m0.alarms[2]         = { true, 18, 0 };

        auto& m1 = model.mutableMedication(1);
        m1.setName("Vitamin D3");
        m1.enabled           = true;
        m1.alarms[0]         = { true, 9, 0 };
        m1.alarms[1]         = { false, 13, 0 };
        m1.alarms[2]         = { false, 18, 0 };

        auto& m2 = model.mutableMedication(2);
        m2.setName("Ibuprofen");
        m2.enabled           = false;
        m2.alarms[0]         = { false, 9, 0 };
        m2.alarms[1]         = { false, 14, 0 };
        m2.alarms[2]         = { false, 20, 0 };

        auto& m3 = model.mutableMedication(3);
        m3.setName("Metformin");
        m3.enabled           = true;
        m3.alarms[0]         = { true, 8, 30 };
        m3.alarms[1]         = { true, 19, 30 };
        m3.alarms[2]         = { false, 22, 0 };

        auto& m4 = model.mutableMedication(4);
        m4.setName("Melatonin");
        m4.enabled           = true;
        m4.alarms[0]         = { true, 22, 0 };
        m4.alarms[1]         = { false, 10, 0 };
        m4.alarms[2]         = { false, 18, 0 };
    }

    // 1. MainScreen: Medication Wheel Menu
    model.setSelectedMedIndex(0);
    ScreenManager::instance().start(model, ScreenId::Main);
    pumpAsyncAndRender(fx, outDir + "/screen_main.raw");

    // 2. MedDetailScreen: Amoxicillin Schedule (highlighting Alarm 1: 09:00 ON)
    model.setSelectedMedIndex(0);
    model.setMedDetailCursor(1);
    ScreenManager::instance().switchTo(ScreenId::MedDetail);
    pumpAsyncAndRender(fx, outDir + "/screen_detail.raw");

    // 3. TimePickerScreen: Setting Alarm 1 Time (09:00)
    model.setSelectedMedIndex(0);
    model.setSelectedAlarmIndex(0);
    ScreenManager::instance().switchTo(ScreenId::TimePicker);
    pumpAsyncAndRender(fx, outDir + "/screen_timepicker.raw");

    // 4. RingingScreen: Active Medication Alert (Amoxicillin 09:00, Snooze: 3 left)
    PillSync::ActiveAlert alert {};
    alert.active        = true;
    alert.medIndex      = 0;
    alert.alarmIndex    = 0;
    alert.hour          = 9;
    alert.minute        = 0;
    alert.snoozesUsed   = 0;
    alert.snoozesLeft   = 3;
    std::snprintf(alert.medName, sizeof(alert.medName), "Amoxicillin");

    CustomMessage::ActivatedAlarm msg(alert);
    static_cast<SDK::Interface::ICustomMessageHandler*>(&model)->customMessageHandler(&msg);
    ScreenManager::instance().switchTo(ScreenId::Ringing);
    pumpAsyncAndRender(fx, outDir + "/screen_ringing.raw");

    return 0;
}
