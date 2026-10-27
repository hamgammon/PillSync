/**
 ******************************************************************************
 * @file    Medication.hpp
 * @brief   Data types for PillSync medications, alarm slots, and active alerts.
 ******************************************************************************
 */

#ifndef MEDICATION_HPP
#define MEDICATION_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace PillSync
{

constexpr std::size_t kMaxMedications = 5;
constexpr std::size_t kAlarmsPerMed   = 3;
constexpr std::size_t kMaxMedNameLen  = 16;
constexpr uint8_t     kMaxSnoozeCount = 3;
constexpr uint8_t     kSnoozeMinutes  = 5;

struct AlarmSlot {
    bool    enabled = false;
    uint8_t hour    = 9;
    uint8_t minute  = 0;

    bool operator==(const AlarmSlot& other) const
    {
        return enabled == other.enabled &&
               hour    == other.hour    &&
               minute  == other.minute;
    }

    bool operator!=(const AlarmSlot& other) const
    {
        return !(*this == other);
    }
};

struct Medication {
    char      name[kMaxMedNameLen + 1] = {};
    bool      enabled                  = false;
    AlarmSlot alarms[kAlarmsPerMed]    = {};

    void setName(const char* src)
    {
        if (!src) {
            name[0] = '\0';
            return;
        }
        std::strncpy(name, src, kMaxMedNameLen);
        name[kMaxMedNameLen] = '\0';
    }

    uint8_t activeAlarmCount() const
    {
        uint8_t count = 0;
        for (std::size_t i = 0; i < kAlarmsPerMed; ++i) {
            if (alarms[i].enabled) {
                ++count;
            }
        }
        return count;
    }
};

struct ActiveAlert {
    bool    active                      = false;
    uint8_t medIndex                    = 0;
    uint8_t alarmIndex                  = 0;
    uint8_t hour                        = 0;
    uint8_t minute                      = 0;
    uint8_t snoozesUsed                 = 0;
    uint8_t snoozesLeft                 = kMaxSnoozeCount;
    char    medName[kMaxMedNameLen + 1] = {};

    void setMedName(const char* src)
    {
        if (!src) {
            medName[0] = '\0';
            return;
        }
        std::strncpy(medName, src, kMaxMedNameLen);
        medName[kMaxMedNameLen] = '\0';
    }
};

} // namespace PillSync

#endif // MEDICATION_HPP

