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

    /// Check if companion app updated app_config.json on disk; if so, reload and sync.
    /// Returns true if configuration was reloaded.
    bool checkAppConfigUpdates();

    /// Compute FNV-1a hash of app_config.json to detect external file updates.
    uint32_t computeConfigFileHash() const;

    /// Access internal PillManager (for testing).
    const PillSync::PillManager& getPillManager() const { return mPillManager; }

private:
    const SDK::Kernel&              mKernel;
    std::unique_ptr<SDK::AppConfig> mConfig;
    PillSync::PillManager           mPillManager;

    bool                  mGUIStarted     = false;
    bool                  mGUISuspended   = false;
    bool                  mNeedTrigger    = false;
    PillSync::ActiveAlert mTriggeredAlert {};
    bool                  mTimeFormat12h  = false;
    uint32_t              mLastConfigHash = 0;

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

