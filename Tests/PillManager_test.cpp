/**
 ******************************************************************************
 * @file    PillManager_test.cpp
 * @brief   Host unit tests for PillSync's PillManager and AppConfig schema.
 ******************************************************************************
 */

#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

#include "KernelTestDoubles.hpp"
#include "AppConfigFields.hpp"
#include "Commands.hpp"
#include "Medication.hpp"
#include "PillManager.hpp"
#include "Service.hpp"

namespace
{

using PillSync::ActiveAlert;
using PillSync::AlarmSlot;
using PillSync::Medication;
using PillSync::PillManager;

class PillManagerTest : public ::testing::Test
{
protected:
    SDK::TestSupport::KernelFixture fx;
};

TEST_F(PillManagerTest, DefaultScheduleHas5MedicationsAnd3AlarmsEach)
{
    PillManager mgr(fx.kernel);

    const Medication* meds = mgr.getMedications();
    ASSERT_NE(meds, nullptr);

    // Medication 1 is enabled by default with 09:00, 10:00, and 18:00 alarms
    EXPECT_STREQ(meds[0].name, "Medication 1");
    EXPECT_TRUE(meds[0].enabled);
    EXPECT_EQ(meds[0].activeAlarmCount(), 3u);

    EXPECT_TRUE(meds[0].alarms[0].enabled);
    EXPECT_EQ(meds[0].alarms[0].hour, 9u);
    EXPECT_EQ(meds[0].alarms[0].minute, 0u);

    EXPECT_TRUE(meds[0].alarms[1].enabled);
    EXPECT_EQ(meds[0].alarms[1].hour, 10u);
    EXPECT_EQ(meds[0].alarms[1].minute, 0u);

    EXPECT_TRUE(meds[0].alarms[2].enabled);
    EXPECT_EQ(meds[0].alarms[2].hour, 18u);
    EXPECT_EQ(meds[0].alarms[2].minute, 0u);

    // Medications 2..5 exist and default to disabled
    for (std::size_t i = 1; i < PillSync::kMaxMedications; ++i) {
        EXPECT_FALSE(meds[i].enabled);
    }
}

TEST_F(PillManagerTest, AppConfigSyncLoadsCompanionValuesAndWritesBackWatchEdits)
{
    // Seed companion app configuration in app_config.json
    fx.fileSystem.seedFile(
        "/app_config.json",
        R"({
            "schema": 1,
            "values": {
                "med1Name": "Amoxicillin",
                "med1Enabled": true,
                "med1Alarm1": "08:30",
                "med1Alarm2": "14:15",
                "med1Alarm3": "20:45",
                "med2Name": "Vitamin D",
                "med2Enabled": true,
                "med2Alarm1": "09:00",
                "med2Alarm2": "OFF",
                "med2Alarm3": "OFF"
            }
        })");

    SDK::AppConfig cfg(fx.kernel,
                       AppConfigSchema::kConfigFile,
                       AppConfigSchema::kFields,
                       AppConfigSchema::kFieldCount);

    PillManager mgr(fx.kernel);
    mgr.attachAppConfig(&cfg);

    const Medication& med1 = mgr.getMedication(0);
    EXPECT_STREQ(med1.name, "Amoxicillin");
    EXPECT_TRUE(med1.enabled);
    EXPECT_TRUE(med1.alarms[0].enabled);
    EXPECT_EQ(med1.alarms[0].hour, 8u);
    EXPECT_EQ(med1.alarms[0].minute, 30u);
    EXPECT_TRUE(med1.alarms[1].enabled);
    EXPECT_EQ(med1.alarms[1].hour, 14u);
    EXPECT_EQ(med1.alarms[1].minute, 15u);
    EXPECT_TRUE(med1.alarms[2].enabled);
    EXPECT_EQ(med1.alarms[2].hour, 20u);
    EXPECT_EQ(med1.alarms[2].minute, 45u);

    const Medication& med2 = mgr.getMedication(1);
    EXPECT_STREQ(med2.name, "Vitamin D");
    EXPECT_TRUE(med2.enabled);
    EXPECT_TRUE(med2.alarms[0].enabled);
    EXPECT_EQ(med2.alarms[0].hour, 9u);
    EXPECT_EQ(med2.alarms[0].minute, 0u);
    EXPECT_FALSE(med2.alarms[1].enabled);
    EXPECT_FALSE(med2.alarms[2].enabled);

    // Edit Medication 2 Alarm 2 on the watch and verify write-back to AppConfig
    Medication updatedMed2 = med2;
    updatedMed2.alarms[1].enabled = true;
    updatedMed2.alarms[1].hour    = 12;
    updatedMed2.alarms[1].minute  = 30;
    EXPECT_TRUE(mgr.updateMedication(1, updatedMed2));

    char alarm2Str[8] {};
    cfg.getString("med2Alarm2", alarm2Str, sizeof(alarm2Str));
    EXPECT_STREQ(alarm2Str, "12:30");

    // Verify re-opening AppConfig from disk reads the saved watch edit
    SDK::AppConfig reloadedCfg(fx.kernel,
                               AppConfigSchema::kConfigFile,
                               AppConfigSchema::kFields,
                               AppConfigSchema::kFieldCount);
    reloadedCfg.getString("med2Alarm2", alarm2Str, sizeof(alarm2Str));
    EXPECT_STREQ(alarm2Str, "12:30");
}

TEST_F(PillManagerTest, Triggers3AlarmsDuringTheDayForMedication)
{
    PillManager mgr(fx.kernel);
    std::vector<ActiveAlert> fired;
    mgr.setCallback([&fired](const ActiveAlert& alert) {
        fired.push_back(alert);
    });

    const std::time_t baseUtc = 1700000000;

    // 09:00 alarm triggers
    mgr.checkAlarms(9, 0, baseUtc);
    ASSERT_EQ(fired.size(), 1u);
    EXPECT_STREQ(fired[0].medName, "Medication 1");
    EXPECT_EQ(fired[0].hour, 9u);
    EXPECT_EQ(fired[0].minute, 0u);
    EXPECT_EQ(fired[0].snoozesUsed, 0u);
    EXPECT_EQ(fired[0].snoozesLeft, 3u);
    mgr.dismissActiveAlert();

    // 10:00 alarm triggers
    mgr.checkAlarms(10, 0, baseUtc + 3600);
    ASSERT_EQ(fired.size(), 2u);
    EXPECT_EQ(fired[1].hour, 10u);
    EXPECT_EQ(fired[1].minute, 0u);
    mgr.dismissActiveAlert();

    // 18:00 alarm triggers
    mgr.checkAlarms(18, 0, baseUtc + 9 * 3600);
    ASSERT_EQ(fired.size(), 3u);
    EXPECT_EQ(fired[2].hour, 18u);
    EXPECT_EQ(fired[2].minute, 0u);
    mgr.dismissActiveAlert();
}

TEST_F(PillManagerTest, SnoozesUpTo3TimesThenDismisses)
{
    PillManager mgr(fx.kernel);
    std::vector<ActiveAlert> fired;
    mgr.setCallback([&fired](const ActiveAlert& alert) {
        fired.push_back(alert);
    });

    // Align base timestamp to minute boundary
    const std::time_t t0 = 1700000400; // 09:00

    // Initial trigger at 09:00 (snoozesUsed = 0, snoozesLeft = 3)
    mgr.checkAlarms(9, 0, t0);
    ASSERT_EQ(fired.size(), 1u);
    EXPECT_EQ(fired[0].snoozesUsed, 0u);
    EXPECT_EQ(fired[0].snoozesLeft, 3u);

    // Snooze #1 at 09:00 -> rings at 09:05 (snoozesUsed = 1, snoozesLeft = 2)
    EXPECT_TRUE(mgr.snoozeActiveAlert(t0));
    mgr.checkAlarms(9, 4, t0 + 4 * 60);
    EXPECT_EQ(fired.size(), 1u); // Not yet 5 minutes

    mgr.checkAlarms(9, 5, t0 + 5 * 60);
    ASSERT_EQ(fired.size(), 2u);
    EXPECT_EQ(fired[1].snoozesUsed, 1u);
    EXPECT_EQ(fired[1].snoozesLeft, 2u);

    // Snooze #2 at 09:05 -> rings at 09:10 (snoozesUsed = 2, snoozesLeft = 1)
    EXPECT_TRUE(mgr.snoozeActiveAlert(t0 + 5 * 60));
    mgr.checkAlarms(9, 10, t0 + 10 * 60);
    ASSERT_EQ(fired.size(), 3u);
    EXPECT_EQ(fired[2].snoozesUsed, 2u);
    EXPECT_EQ(fired[2].snoozesLeft, 1u);

    // Snooze #3 at 09:10 -> rings at 09:15 (snoozesUsed = 3, snoozesLeft = 0)
    EXPECT_TRUE(mgr.snoozeActiveAlert(t0 + 10 * 60));
    mgr.checkAlarms(9, 15, t0 + 15 * 60);
    ASSERT_EQ(fired.size(), 4u);
    EXPECT_EQ(fired[3].snoozesUsed, 3u);
    EXPECT_EQ(fired[3].snoozesLeft, 0u);
    EXPECT_TRUE(mgr.getSnoozedAlarms().empty());

    // Attempting a 4th snooze when snoozesLeft == 0 returns false and dismisses
    EXPECT_FALSE(mgr.snoozeActiveAlert(t0 + 15 * 60));
    EXPECT_FALSE(mgr.getActiveAlert().active);

    // Advancing another 5, 10, 15 minutes does NOT trigger a 4th snooze ring
    mgr.checkAlarms(9, 20, t0 + 20 * 60);
    mgr.checkAlarms(9, 25, t0 + 25 * 60);
    EXPECT_EQ(fired.size(), 4u);
}

TEST_F(PillManagerTest, DismissCancelsRemainingSnoozes)
{
    PillManager mgr(fx.kernel);
    std::vector<ActiveAlert> fired;
    mgr.setCallback([&fired](const ActiveAlert& alert) {
        fired.push_back(alert);
    });

    const std::time_t t0 = 1700000400;

    // Initial trigger at 09:00
    mgr.checkAlarms(9, 0, t0);
    ASSERT_EQ(fired.size(), 1u);
    EXPECT_EQ(mgr.getSnoozedAlarms().size(), 1u);

    // Snooze once -> rings at 09:05
    EXPECT_TRUE(mgr.snoozeActiveAlert(t0));
    mgr.checkAlarms(9, 5, t0 + 5 * 60);
    ASSERT_EQ(fired.size(), 2u);

    // User presses Dismiss on first snooze ring -> cancels remaining 2 snoozes
    mgr.dismissActiveAlert();
    EXPECT_TRUE(mgr.getSnoozedAlarms().empty());

    // Does not ring at 09:10 or 09:15
    mgr.checkAlarms(9, 10, t0 + 10 * 60);
    mgr.checkAlarms(9, 15, t0 + 15 * 60);
    EXPECT_EQ(fired.size(), 2u);
}

TEST_F(PillManagerTest, PersistsAndRestoresScheduleAndSnoozesAcrossRestart)
{
    const std::time_t t0 = 1700000400;
    {
        PillManager mgr(fx.kernel);
        Medication  med3 = mgr.getMedication(2);
        med3.setName("Ibuprofen");
        med3.enabled           = true;
        med3.alarms[0].enabled = true;
        med3.alarms[0].hour    = 11;
        med3.alarms[0].minute  = 30;
        EXPECT_TRUE(mgr.updateMedication(2, med3));

        mgr.checkAlarms(11, 30, t0);
        EXPECT_TRUE(mgr.snoozeActiveAlert(t0));
    }

    // Reconstruct PillManager on the same filesystem
    PillManager restored(fx.kernel);
    EXPECT_TRUE(restored.load());

    const Medication& med3 = restored.getMedication(2);
    EXPECT_STREQ(med3.name, "Ibuprofen");
    EXPECT_TRUE(med3.enabled);
    EXPECT_TRUE(med3.alarms[0].enabled);
    EXPECT_EQ(med3.alarms[0].hour, 11u);
    EXPECT_EQ(med3.alarms[0].minute, 30u);

    ASSERT_EQ(restored.getSnoozedAlarms().size(), 1u);
    EXPECT_EQ(restored.getSnoozedAlarms()[0].medIndex, 2u);
    EXPECT_EQ(restored.getSnoozedAlarms()[0].ringsLeft, 3u);
}

TEST_F(PillManagerTest, SupportsSoundAndVibrateOrVibrateOnlyMode)
{
    fx.fileSystem.seedFile(
        "/app_config.json",
        R"({
            "schema": 1,
            "values": {
                "vibrateOnly": true
            }
        })");

    SDK::AppConfig cfg(fx.kernel,
                       AppConfigSchema::kConfigFile,
                       AppConfigSchema::kFields,
                       AppConfigSchema::kFieldCount);

    PillManager mgr(fx.kernel);
    EXPECT_FALSE(mgr.isVibrateOnly()); // default is Sound & Vibrate

    mgr.attachAppConfig(&cfg);
    EXPECT_TRUE(mgr.isVibrateOnly());  // synced from app_config.json

    std::vector<ActiveAlert> fired;
    mgr.setCallback([&fired](const ActiveAlert& alert) {
        fired.push_back(alert);
    });

    const std::time_t t0 = 1700000400;
    mgr.checkAlarms(9, 0, t0);
    ASSERT_EQ(fired.size(), 1u);
    EXPECT_TRUE(fired[0].vibrateOnly);

    // Toggle back to Sound & Vibrate on watch and verify write-back + persistence
    EXPECT_TRUE(mgr.setVibrateOnly(false));
    EXPECT_FALSE(mgr.isVibrateOnly());
    EXPECT_FALSE(cfg.getBool("vibrateOnly"));

    PillManager restored(fx.kernel);
    EXPECT_TRUE(restored.load());
    EXPECT_FALSE(restored.isVibrateOnly());
}

TEST_F(PillManagerTest, ExecuteCalculatesSleepAndHandlesNoWork)
{
    PillManager mgr(fx.kernel);

    // Default schedule has Medication 1 enabled with alarms -> active alarms exist
    std::tm tmNow {};
    tmNow.tm_hour = 8;
    tmNow.tm_min  = 30;
    tmNow.tm_sec  = 15;
    const std::time_t nowUtc = 1700000000;

    // 60 - 15 = 45 seconds left until next minute
    uint32_t sleepMs = mgr.execute(tmNow, nowUtc);
    EXPECT_EQ(sleepMs, 45000u);

    // Disable all medications -> execute returns kNoWork
    Medication meds[PillSync::kMaxMedications] {};
    for (std::size_t i = 0; i < PillSync::kMaxMedications; ++i) {
        meds[i] = mgr.getMedication(i);
        meds[i].enabled = false;
    }
    EXPECT_TRUE(mgr.saveMedications(meds));
    EXPECT_FALSE(mgr.hasActiveAlarms());

    sleepMs = mgr.execute(tmNow, nowUtc);
    EXPECT_EQ(sleepMs, PillManager::kNoWork);
}

TEST_F(PillManagerTest, ServiceDetectsCompanionAppConfigUpdatesInBackground)
{
    fx.fileSystem.seedFile(
        "/app_config.json",
        R"({
            "schema": 1,
            "values": {
                "med1Name": "Metformin",
                "med1Enabled": true,
                "med1Alarm1": "08:00",
                "med1Alarm2": "OFF",
                "med1Alarm3": "OFF"
            }
        })");

    Service srv(fx.kernel);
    EXPECT_FALSE(srv.checkAppConfigUpdates());

    // Initially, med 1 alarm 1 is at 08:00
    // Note: srv.run() calls reloadAppConfig, but in this unit test we trigger checkAppConfigUpdates directly
    // Force initial load:
    EXPECT_TRUE(srv.computeConfigFileHash() != 0);

    // Simulate companion app writing new schedule to /app_config.json via BLE FTS
    fx.fileSystem.seedFile(
        "/app_config.json",
        R"({
            "schema": 1,
            "values": {
                "med1Name": "Metformin",
                "med1Enabled": true,
                "med1Alarm1": "11:45",
                "med1Alarm2": "OFF",
                "med1Alarm3": "OFF"
            }
        })");

    // Service detects the hash change, reloads config, and syncs immediately
    EXPECT_TRUE(srv.checkAppConfigUpdates());
    EXPECT_FALSE(srv.checkAppConfigUpdates());

    // Verify the internal PillManager has received the new 11:45 alarm without opening GUI
    const Medication& med1 = srv.getPillManager().getMedication(0);
    EXPECT_STREQ(med1.name, "Metformin");
    EXPECT_TRUE(med1.enabled);
    EXPECT_TRUE(med1.alarms[0].enabled);
    EXPECT_EQ(med1.alarms[0].hour, 11u);
    EXPECT_EQ(med1.alarms[0].minute, 45u);
}

} // namespace
