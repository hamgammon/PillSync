/**
 ******************************************************************************
 * @file    AppConfigFields.cpp
 * @brief   The app's copy of the configuration contract in its app-manifest.json.
 ******************************************************************************
 */

#include "AppConfigFields.hpp"

namespace AppConfigSchema
{

using SDK::AppConfig;

const char* const kMedNameKeys[5] = {
    "med1Name", "med2Name", "med3Name", "med4Name", "med5Name"
};

const char* const kMedEnabledKeys[5] = {
    "med1Enabled", "med2Enabled", "med3Enabled", "med4Enabled", "med5Enabled"
};

const char* const kMedAlarmKeys[5][3] = {
    { "med1Alarm1", "med1Alarm2", "med1Alarm3" },
    { "med2Alarm1", "med2Alarm2", "med2Alarm3" },
    { "med3Alarm1", "med3Alarm2", "med3Alarm3" },
    { "med4Alarm1", "med4Alarm2", "med4Alarm3" },
    { "med5Alarm1", "med5Alarm2", "med5Alarm3" }
};

// Every value here must match Output/app-manifest.json exactly. Checked with
// validate_app_config.py --check Output/app-manifest.json --check-bounds ...
const AppConfig::Field kFields[kFieldCount] = {
    AppConfig::stringField("med1Name", "Medication 1", 1, 16),
    AppConfig::boolField("med1Enabled", true),
    AppConfig::stringField("med1Alarm1", "09:00", 3, 5),
    AppConfig::stringField("med1Alarm2", "10:00", 3, 5),
    AppConfig::stringField("med1Alarm3", "18:00", 3, 5),

    AppConfig::stringField("med2Name", "Medication 2", 1, 16),
    AppConfig::boolField("med2Enabled", false),
    AppConfig::stringField("med2Alarm1", "09:00", 3, 5),
    AppConfig::stringField("med2Alarm2", "OFF", 3, 5),
    AppConfig::stringField("med2Alarm3", "OFF", 3, 5),

    AppConfig::stringField("med3Name", "Medication 3", 1, 16),
    AppConfig::boolField("med3Enabled", false),
    AppConfig::stringField("med3Alarm1", "09:00", 3, 5),
    AppConfig::stringField("med3Alarm2", "OFF", 3, 5),
    AppConfig::stringField("med3Alarm3", "OFF", 3, 5),

    AppConfig::stringField("med4Name", "Medication 4", 1, 16),
    AppConfig::boolField("med4Enabled", false),
    AppConfig::stringField("med4Alarm1", "09:00", 3, 5),
    AppConfig::stringField("med4Alarm2", "OFF", 3, 5),
    AppConfig::stringField("med4Alarm3", "OFF", 3, 5),

    AppConfig::stringField("med5Name", "Medication 5", 1, 16),
    AppConfig::boolField("med5Enabled", false),
    AppConfig::stringField("med5Alarm1", "09:00", 3, 5),
    AppConfig::stringField("med5Alarm2", "OFF", 3, 5),
    AppConfig::stringField("med5Alarm3", "OFF", 3, 5),

    AppConfig::boolField("vibrateOnly", false),
};

} // namespace AppConfigSchema

