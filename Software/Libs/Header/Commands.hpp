/**
 ******************************************************************************
 * @file    Commands.hpp
 * @brief   Custom IPC messages exchanged between the PillSync Service and GUI.
 ******************************************************************************
 */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "SDK/Messages/MessageBase.hpp"
#include "SDK/Messages/MessageTypes.hpp"
#include "SDK/Messages/CommandMessages.hpp"
#include "Medication.hpp"

#include <cstdint>
#include <cstring>

#pragma pack(push, 4)

namespace CustomMessage
{

// Service <-> GUI
constexpr SDK::MessageType::Type SCHEDULE_LIST         = 0x00000001;

// Service --> GUI
constexpr SDK::MessageType::Type ACTIVATED_ALARM       = 0x00000002;

// GUI --> Service
constexpr SDK::MessageType::Type ALARM_ACTIVATE_EFFECT = 0x00000003;
constexpr SDK::MessageType::Type ALARM_SNOOZE          = 0x00000004;
constexpr SDK::MessageType::Type ALARM_DISMISS         = 0x00000005;

struct ScheduleList : public SDK::MessageBase {
    PillSync::Medication meds[PillSync::kMaxMedications] = {};
    bool                 timeFormat12h                   = false;
    bool                 vibrateOnly                     = false;

    explicit ScheduleList(const PillSync::Medication list[PillSync::kMaxMedications],
                          bool                       format12h = false,
                          bool                       vibOnly   = false)
        : SDK::MessageBase(SCHEDULE_LIST)
        , timeFormat12h(format12h)
        , vibrateOnly(vibOnly)
    {
        std::memcpy(meds, list, sizeof(meds));
    }
};
static_assert(sizeof(ScheduleList) <= 256, "ScheduleList exceeds 256-byte IPC message limit");

struct ActivatedAlarm : public SDK::MessageBase {
    PillSync::ActiveAlert alert;

    explicit ActivatedAlarm(const PillSync::ActiveAlert& a)
        : SDK::MessageBase(ACTIVATED_ALARM)
        , alert(a)
    {}
};
static_assert(sizeof(ActivatedAlarm) <= 256, "ActivatedAlarm exceeds 256-byte IPC message limit");

struct AlarmActivateEffect : public SDK::MessageBase {
    PillSync::ActiveAlert alert;

    explicit AlarmActivateEffect(const PillSync::ActiveAlert& a)
        : SDK::MessageBase(ALARM_ACTIVATE_EFFECT)
        , alert(a)
    {}
};
static_assert(sizeof(AlarmActivateEffect) <= 256, "AlarmActivateEffect exceeds 256-byte IPC message limit");

struct AlarmSnooze : public SDK::MessageBase {
    AlarmSnooze()
        : SDK::MessageBase(ALARM_SNOOZE)
    {}
};
static_assert(sizeof(AlarmSnooze) <= 256, "AlarmSnooze exceeds 256-byte IPC message limit");

struct AlarmDismiss : public SDK::MessageBase {
    AlarmDismiss()
        : SDK::MessageBase(ALARM_DISMISS)
    {}
};
static_assert(sizeof(AlarmDismiss) <= 256, "AlarmDismiss exceeds 256-byte IPC message limit");

} // namespace CustomMessage

#pragma pack(pop)

#endif // COMMANDS_HPP

