/**
 ******************************************************************************
 * @file    AppConfigFields.hpp
 * @brief   The app's copy of the configuration contract in its app-manifest.json.
 * @details app-manifest.json never reaches the watch, so the app repeats the id,
 *          type, default and bounds of every field here. CI compares the two
 *          (validate_app_config.py --check-bounds) and fails the build if they
 *          disagree.
 ******************************************************************************
 */

#ifndef APP_CONFIG_FIELDS_HPP
#define APP_CONFIG_FIELDS_HPP

#include "SDK/AppConfig/AppConfig.hpp"

#include <cstddef>

namespace AppConfigSchema
{

/// The values file the companion app writes, as declared by "configFile".
constexpr const char* kConfigFile = "app_config.json";

/// Longest medication name in bytes (maxLength = 16), plus null terminator.
constexpr size_t kNameBytes = 16 + 1;

/// Number of fields in kFields (5 medications x 5 fields = 25 <= skMaxFields).
constexpr size_t kFieldCount = 25;

/// The declaration, mirroring app-manifest.json's "configFields".
extern const SDK::AppConfig::Field kFields[kFieldCount];

/// Helper lookup arrays indexed by medication (0..4) and alarm slot (0..2).
extern const char* const kMedNameKeys[5];
extern const char* const kMedEnabledKeys[5];
extern const char* const kMedAlarmKeys[5][3];

} // namespace AppConfigSchema

#endif // APP_CONFIG_FIELDS_HPP

