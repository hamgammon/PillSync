/**
 ******************************************************************************
 * @file    main.cpp
 * @brief   PillSync LVGL PC simulator.
 ******************************************************************************
 */

#include <thread>

#include "SDK/Kernel/KernelProviderGUI.hpp"
#include "SDK/Kernel/KernelProviderService.hpp"
#include "SDK/Port/LVGL/LvglPort.hpp"
#include "SDK/Simulator/App/AppMessageCore.hpp"
#include "SDK/Simulator/App/KernelMessageDispatcher.hpp"
#include "SDK/Simulator/Kernel/Kernel.hpp"
#include "SDK/Simulator/Kernel/Mock/Backlight.hpp"
#include "SDK/Simulator/Kernel/Mock/Buzzer.hpp"
#include "SDK/Simulator/Kernel/Mock/System.hpp"
#include "SDK/Simulator/Kernel/Mock/Vibro.hpp"
#include "SDK/Simulator/LVGL/LvglHost.hpp"

#include "Service.hpp"

#define LOG_MODULE_PRX      "main"
#define LOG_MODULE_LEVEL    LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

extern "C" void una_lvgl_app_init(void);

int main(int, char**)
{
    SDK::Simulator::Mock::SystemService serviceSystem;
    SDK::Simulator::Kernel              serviceKernel("service");
    SDK::Simulator::Mock::SystemGUI     guiSystem;
    SDK::Simulator::Kernel              guiKernel("gui");

    SDK::App::MessageCore  appMessageCore;
    SDK::App::DualAppComm& appComm = appMessageCore.getAppComm();

    serviceKernel.setIAppComm(appComm.getServiceComm());
    serviceKernel.setISystem(&serviceSystem);
    guiKernel.setIAppComm(appComm.getGuiComm());
    guiKernel.setISystem(&guiSystem);
    SDK::Simulator::KernelHolder::Create(guiKernel);

    Logger_init(serviceKernel.getKernel().log);
    SDK::KernelProviderService::CreateInstance(&serviceKernel.getKernel());
    SDK::KernelProviderGUI::CreateInstance(&guiKernel.getKernel());

    Service service(SDK::KernelProviderService::GetInstance().getKernel());

    SDK::Simulator::Mock::Backlight   backlight;
    SDK::Simulator::Mock::Buzzer      buzzer;
    SDK::Simulator::Mock::Vibro       vibro;
    SDK::App::KernelMessageDispatcher dispatcher(appComm, appComm.getMsgManager(), vibro, backlight, buzzer);

    SDK::Simulator::LvglHost::Options options;
    options.title = "PillSync (LVGL)";
    options.scale = 2;
    SDK::Simulator::LvglHost host(appComm, serviceKernel.getKernel(), options);
    if (!host.init()) {
        return 1;
    }

    std::thread serviceThread(&Service::run, &service);
    std::thread dispatcherThread(&SDK::App::KernelMessageDispatcher::run, &dispatcher);

    SDK::LVGL::Port& port = SDK::LVGL::Port::GetInstance();
    port.init();
    una_lvgl_app_init();

    host.start();
    host.run();
    host.shutdown();

    serviceThread.join();
    dispatcherThread.join();

    LOG_INFO("Simulator finished\n");
    return 0;
}

