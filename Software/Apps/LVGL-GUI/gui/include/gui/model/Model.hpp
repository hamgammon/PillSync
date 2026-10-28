/**
 ******************************************************************************
 * @file    Model.hpp
 * @brief   GUI-side state of PillSync and IPC bridge to Service.
 ******************************************************************************
 */

#ifndef MODEL_HPP
#define MODEL_HPP

#include "SDK/Kernel/Kernel.hpp"
#include "SDK/Interfaces/IGuiLifeCycleCallback.hpp"
#include "SDK/Interfaces/ICustomMessageHandler.hpp"
#include "SDK/Utils/Utils.hpp"
#include "SDK/GUI/Config.hpp"
#include "SDK/GUI/Button.hpp"

#include "Commands.hpp"
#include "Medication.hpp"

#include <cstddef>
#include <cstdint>

namespace App::Config
{
constexpr uint32_t kFrameRate          = SDK::GUI::Config::kFrameRate;
constexpr uint32_t kMenuAnimationMs    = 350;
constexpr uint32_t kScreenTimeoutSteps = SDK::Utils::secToTicks(30, kFrameRate);
} // namespace App::Config

class ModelListener;

class Model : public SDK::Interface::IGuiLifeCycleCallback,
              public SDK::Interface::ICustomMessageHandler
{
public:
    Model();

    void bind(ModelListener* listener) { modelListener = listener; }

    // Controls
    void handleKeyEvent(uint8_t key);
    void resetIdleTimer();
    void exitApp();
    void switchToNextPriorityScreen();

    // Schedule state
    const PillSync::Medication* getMedications() const { return mMedications; }
    const PillSync::Medication& getMedication(std::size_t idx) const { return mMedications[idx]; }
    PillSync::Medication&       mutableMedication(std::size_t idx) { return mMedications[idx]; }

    void saveMedications();
    bool is12HourFormat() const { return mTimeFormat12h; }
    bool isVibrateOnly() const { return mVibrateOnly; }
    void setVibrateOnly(bool vibrateOnly);
    void toggleVibrateOnly();

    // Navigation selection state
    void        setMainCursor(std::size_t idx) { mMainCursor = idx; }
    std::size_t getMainCursor() const { return mMainCursor; }

    void        setSelectedMedIndex(std::size_t idx);
    std::size_t getSelectedMedIndex() const { return mSelectedMedIndex; }

    void        setSelectedAlarmIndex(std::size_t idx);
    std::size_t getSelectedAlarmIndex() const { return mSelectedAlarmIndex; }

    void        setMedDetailCursor(std::size_t idx) { mMedDetailCursor = idx; }
    std::size_t getMedDetailCursor() const { return mMedDetailCursor; }

    void        setAlarmActionCursor(std::size_t idx) { mAlarmActionCursor = idx; }
    std::size_t getAlarmActionCursor() const { return mAlarmActionCursor; }

    // Active ringing alarm
    const PillSync::ActiveAlert& getActiveAlert() const { return mActiveAlert; }
    void playAlarmEffect();
    void snoozeAlarm();
    void dismissAlarm();

    // Helper for formatting HH:MM in 24h or 12h mode
    void formatTime(uint8_t hour, uint8_t minute, char* buf, std::size_t bufSize) const;

private:
    ModelListener*     modelListener;
    const SDK::Kernel& mKernel;

    // IGuiLifeCycleCallback
    void onStart()   override;
    void onFrame()   override;
    void onResume()  override;
    void onSuspend() override;
    void onStop()    override;

    // ICustomMessageHandler
    bool customMessageHandler(SDK::MessageBase* msg) override;

    void decIdleTimer();
    void setCapabilities();
    static bool isClick(uint8_t key);
    void initDefaultSchedule();

    bool     mIsRunning  = false;
    uint32_t mIdleTimer  = 0;
    bool     mStayInApp  = false;

    PillSync::Medication  mMedications[PillSync::kMaxMedications] = {};
    PillSync::ActiveAlert mActiveAlert {};
    bool                  mTimeFormat12h = false;
    bool                  mVibrateOnly   = false;

    std::size_t mMainCursor          = 0;
    std::size_t mSelectedMedIndex   = 0;
    std::size_t mSelectedAlarmIndex = 0;
    std::size_t mMedDetailCursor    = 0;
    std::size_t mAlarmActionCursor  = 0;
};

#endif // MODEL_HPP

