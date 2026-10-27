/**
 ******************************************************************************
 * @file    Service.hpp
 * @brief   PillSync background service (autostart, alarm triggers, IPC, AppConfig).
 ******************************************************************************
 */

#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "SDK/Kernel/KernelProviderService.hpp"
#include "SDK/AppConfig/AppConfig.hpp"
#include "SDK/Messages/CommandMessages.hpp"

#include "AppConfigFields.hpp"
#include "Commands.hpp"
#include "Medication.hpp"
#include "PillManager.hpp"

#include <memory>

class Service
{
public:
    explicit Service(SDK::Kernel& kernel);
    virtual ~Service() = default;

    void run();

private:
    const SDK::Kernel&              mKernel;
    std::unique_ptr<SDK::AppConfig> mConfig;
    PillSync::PillManager           mPillManager;

    bool                  mGUIStarted     = false;
    bool                  mGUISuspended   = false;
    bool                  mNeedTrigger    = false;
    PillSync::ActiveAlert mTriggeredAlert {};
    bool                  mTimeFormat12h  = false;

    void reloadAppConfig();
    void refreshTimeFormat();
    void onStartGUI();
    void onResumeGUI();
    void onSuspendGUI();
    void onStopGUI();

    void sendScheduleList();
    void sendActivatedAlarm();

    void runGUI();
    void playEffect(const PillSync::ActiveAlert& alert);
    void stopRinging();
};

#endif // SERVICE_HPP

