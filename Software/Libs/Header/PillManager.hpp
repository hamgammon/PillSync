/**
 ******************************************************************************
 * @file    PillManager.hpp
 * @brief   Medication schedule persistence, AppConfig sync, and 3x snooze engine.
 ******************************************************************************
 */

#ifndef PILL_MANAGER_HPP
#define PILL_MANAGER_HPP

#include "Medication.hpp"
#include "SDK/AppConfig/AppConfig.hpp"
#include "SDK/Kernel/Kernel.hpp"

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <functional>
#include <vector>

namespace PillSync
{

class PillManager
{
public:
    using AlarmCallback = std::function<void(const ActiveAlert& alert)>;

    struct SnoozedEntry {
        uint8_t     medIndex      = 0;
        uint8_t     alarmIndex    = 0;
        uint8_t     hour          = 0;
        uint8_t     minute        = 0;
        std::time_t nextTriggerAt = 0;
        uint8_t     snoozesUsed   = 0;
        uint8_t     ringsLeft     = kMaxSnoozeCount;
    };

    PillManager(const SDK::Kernel& kernel,
                const char*        storagePath = "pillsync.json",
                const char*        snoozesPath = "snoozes.json");

    /// Attach the SDK::AppConfig instance (created in Service::run()) and sync values.
    void attachAppConfig(SDK::AppConfig* config);

    /// Read all 25 fields from SDK::AppConfig into the medication schedule.
    void syncFromAppConfig();

    /// Write current medication schedule back to SDK::AppConfig and save app_config.json.
    bool writeToAppConfig();

    /// Load medications and pending snoozes from local JSON files.
    bool load();

    /// Save updated medications from watch UI, persist to JSON and write back to AppConfig.
    bool saveMedications(const Medication meds[kMaxMedications]);

    /// Access current 5 medications.
    const Medication* getMedications() const { return mMedications; }
    const Medication& getMedication(std::size_t index) const { return mMedications[index]; }

    /// Update a single medication and persist.
    bool updateMedication(std::size_t index, const Medication& med);

    /// True if any medication has at least one enabled alarm or a pending snooze.
    bool hasActiveAlarms() const;

    /// Register callback invoked whenever an alarm or snooze triggers.
    void setCallback(AlarmCallback cb);

    /// Evaluate alarms and snoozes at the given local HH:MM and UTC timestamp.
    void checkAlarms(uint8_t currentHour, uint8_t currentMinute, std::time_t nowUtc);

    /// Snooze the currently ringing alarm (up to kMaxSnoozeCount = 3 times).
    /// Returns true if snoozed, false if max snoozes reached (in which case it dismisses).
    bool snoozeActiveAlert(std::time_t nowUtc);

    /// Dismiss the currently ringing alarm and cancel any remaining snoozes for it.
    void dismissActiveAlert();

    /// Cancel all pending snoozes.
    void stopAll();

    /// Access currently ringing alert snapshot.
    const ActiveAlert& getActiveAlert() const { return mActiveAlert; }

    /// Access pending snoozed alarms (for testing and diagnostics).
    const std::vector<SnoozedEntry>& getSnoozedAlarms() const { return mSnoozedAlarms; }

    /// Parse "OFF" or "HH:MM" string into enabled/hour/minute (preserves hour/min when "OFF").
    static bool parseAlarmConfigString(const char* str, AlarmSlot& inoutSlot);

    /// Format an AlarmSlot as "HH:MM" (if enabled) or "OFF" (if disabled).
    static void formatAlarmConfigString(const AlarmSlot& slot, char* outBuf, std::size_t outSize);

private:
    static constexpr uint32_t kSnoozedTimeMinutes = kSnoozeMinutes;
    static constexpr uint8_t  kMaxSnoozeRings     = kMaxSnoozeCount;

    const SDK::Kernel&        mKernel;
    const char*               mStoragePath;
    const char*               mSnoozesPath;
    SDK::AppConfig*           mConfig = nullptr;

    Medication                mMedications[kMaxMedications] = {};
    std::vector<SnoozedEntry> mSnoozedAlarms;
    ActiveAlert               mActiveAlert {};

    AlarmCallback             mOnAlarm;
    uint8_t                   mLastCheckedHour   = 0xFF;
    uint8_t                   mLastCheckedMinute = 0xFF;

    void initDefaults();
    bool saveLocalState() const;
    void saveSnoozes() const;
    void loadSnoozes();
    void armSnooze(uint8_t medIdx, uint8_t alarmIdx, uint8_t hour, uint8_t minute, std::time_t nowUtc);
    bool isSnoozed(uint8_t medIdx, uint8_t alarmIdx) const;
    static std::time_t snoozeDeadline(std::time_t nowUtc);
};

} // namespace PillSync

#endif // PILL_MANAGER_HPP
