/**
 ******************************************************************************
 * @file    PillManager.cpp
 * @brief   Medication schedule persistence, AppConfig sync, and 3x snooze engine.
 ******************************************************************************
 */

#include "PillManager.hpp"
#include "AppConfigFields.hpp"

#include "SDK/JSON/JsonStreamReader.hpp"
#include "SDK/JSON/JsonStreamWriter.hpp"

#define LOG_MODULE_PRX      "PillManager"
#define LOG_MODULE_LEVEL    LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string_view>

namespace PillSync
{

namespace
{
constexpr uint32_t kJsonBufferSize = 2048;
} // namespace

PillManager::PillManager(const SDK::Kernel& kernel,
                         const char*        storagePath,
                         const char*        snoozesPath)
    : mKernel(kernel)
    , mStoragePath(storagePath)
    , mSnoozesPath(snoozesPath)
{
    initDefaults();
}

void PillManager::initDefaults()
{
    static const uint8_t kDefaultHours[kAlarmsPerMed]   = { 9, 10, 18 };
    static const uint8_t kDefaultMinutes[kAlarmsPerMed] = { 0,  0,  0 };

    for (std::size_t i = 0; i < kMaxMedications; ++i) {
        char defaultName[kMaxMedNameLen + 1];
        std::snprintf(defaultName, sizeof(defaultName), "Medication %zu", i + 1);
        mMedications[i].setName(defaultName);
        mMedications[i].enabled = (i == 0);

        for (std::size_t j = 0; j < kAlarmsPerMed; ++j) {
            mMedications[i].alarms[j].hour    = (i == 0) ? kDefaultHours[j] : ((j == 0) ? 9 : (j == 1 ? 13 : 18));
            mMedications[i].alarms[j].minute  = kDefaultMinutes[j];
            mMedications[i].alarms[j].enabled = (i == 0) ? true : (j == 0);
        }
    }
}

bool PillManager::parseAlarmConfigString(const char* str, AlarmSlot& inoutSlot)
{
    if (!str || str[0] == '\0') {
        return false;
    }

    if ((str[0] == 'O' || str[0] == 'o') &&
        (str[1] == 'F' || str[1] == 'f') &&
        (str[2] == 'F' || str[2] == 'f') &&
        str[3] == '\0') {
        inoutSlot.enabled = false;
        return true;
    }

    if (std::strlen(str) == 5 && str[2] == ':') {
        if (str[0] >= '0' && str[0] <= '9' &&
            str[1] >= '0' && str[1] <= '9' &&
            str[3] >= '0' && str[3] <= '9' &&
            str[4] >= '0' && str[4] <= '9') {
            int h = (str[0] - '0') * 10 + (str[1] - '0');
            int m = (str[3] - '0') * 10 + (str[4] - '0');
            if (h >= 0 && h <= 23 && m >= 0 && m <= 59) {
                inoutSlot.enabled = true;
                inoutSlot.hour    = static_cast<uint8_t>(h);
                inoutSlot.minute  = static_cast<uint8_t>(m);
                return true;
            }
        }
    }

    return false;
}

void PillManager::formatAlarmConfigString(const AlarmSlot& slot, char* outBuf, std::size_t outSize)
{
    if (!outBuf || outSize == 0) {
        return;
    }
    if (!slot.enabled) {
        std::snprintf(outBuf, outSize, "OFF");
    } else {
        std::snprintf(outBuf, outSize, "%02u:%02u",
                      static_cast<unsigned>(slot.hour % 24),
                      static_cast<unsigned>(slot.minute % 60));
    }
}

void PillManager::attachAppConfig(SDK::AppConfig* config)
{
    mConfig = config;
    if (mConfig) {
        syncFromAppConfig();
    }
}

void PillManager::syncFromAppConfig()
{
    if (!mConfig) {
        return;
    }

    for (std::size_t i = 0; i < kMaxMedications; ++i) {
        char cfgName[AppConfigSchema::kNameBytes] {};
        mConfig->getString(AppConfigSchema::kMedNameKeys[i], cfgName, sizeof(cfgName));
        if (cfgName[0] != '\0') {
            mMedications[i].setName(cfgName);
        }
        mMedications[i].enabled = mConfig->getBool(AppConfigSchema::kMedEnabledKeys[i]);

        for (std::size_t j = 0; j < kAlarmsPerMed; ++j) {
            char alarmStr[8] {};
            mConfig->getString(AppConfigSchema::kMedAlarmKeys[i][j], alarmStr, sizeof(alarmStr));
            parseAlarmConfigString(alarmStr, mMedications[i].alarms[j]);
        }
    }

    mSnoozedAlarms.erase(
        std::remove_if(mSnoozedAlarms.begin(), mSnoozedAlarms.end(),
                       [this](const SnoozedEntry& s) {
                           if (s.medIndex >= kMaxMedications || s.alarmIndex >= kAlarmsPerMed) {
                               return true;
                           }
                           const Medication& med  = mMedications[s.medIndex];
                           const AlarmSlot&  slot = med.alarms[s.alarmIndex];
                           return !med.enabled || !slot.enabled ||
                                  slot.hour != s.hour || slot.minute != s.minute;
                       }),
        mSnoozedAlarms.end());

    saveLocalState();
    saveSnoozes();
}

bool PillManager::writeToAppConfig()
{
    if (!mConfig) {
        return true;
    }

    for (std::size_t i = 0; i < kMaxMedications; ++i) {
        mConfig->setString(AppConfigSchema::kMedNameKeys[i], mMedications[i].name);
        mConfig->setBool(AppConfigSchema::kMedEnabledKeys[i], mMedications[i].enabled);

        for (std::size_t j = 0; j < kAlarmsPerMed; ++j) {
            char buf[8];
            formatAlarmConfigString(mMedications[i].alarms[j], buf, sizeof(buf));
            mConfig->setString(AppConfigSchema::kMedAlarmKeys[i][j], buf);
        }
    }

    return mConfig->save();
}

bool PillManager::load()
{
    loadSnoozes();

    auto file = mKernel.fs.file(mStoragePath);
    if (!file || !file->exist()) {
        LOG_INFO("No local %s found, using defaults\n", mStoragePath);
        if (mConfig) {
            syncFromAppConfig();
        }
        return false;
    }

    char        buff[kJsonBufferSize] {};
    std::size_t len = file->size();
    std::size_t br  = 0;

    if (len == 0 || len >= sizeof(buff) || !file->open()) {
        if (mConfig) {
            syncFromAppConfig();
        }
        return false;
    }

    const bool readOk = file->read(buff, len, br) && br > 0;
    file->close();
    if (!readOk) {
        if (mConfig) {
            syncFromAppConfig();
        }
        return false;
    }

    SDK::JsonStreamReader reader(buff, static_cast<uint32_t>(br));
    if (!reader.validate()) {
        LOG_ERROR("Invalid JSON in %s\n", mStoragePath);
        if (mConfig) {
            syncFromAppConfig();
        }
        return false;
    }

    std::size_t arrayLen = 0;
    if (!reader.getArrayLength("medications", arrayLen)) {
        if (mConfig) {
            syncFromAppConfig();
        }
        return false;
    }

    for (std::size_t i = 0; i < arrayLen && i < kMaxMedications; ++i) {
        Medication& med = mMedications[i];
        char        query[48];

        std::snprintf(query, sizeof(query), "medications[%u].name", static_cast<unsigned>(i));
        std::string_view nameView;
        if (reader.get(query, nameView) && !nameView.empty()) {
            char nameBuf[kMaxMedNameLen + 1] {};
            const std::size_t copyLen = std::min(nameView.size(), kMaxMedNameLen);
            std::memcpy(nameBuf, nameView.data(), copyLen);
            nameBuf[copyLen] = '\0';
            med.setName(nameBuf);
        }

        std::snprintf(query, sizeof(query), "medications[%u].enabled", static_cast<unsigned>(i));
        bool en = med.enabled;
        if (reader.get(query, en)) {
            med.enabled = en;
        }

        for (std::size_t j = 0; j < kAlarmsPerMed; ++j) {
            std::snprintf(query, sizeof(query), "medications[%u].a%u_en",
                          static_cast<unsigned>(i), static_cast<unsigned>(j));
            bool aEn = med.alarms[j].enabled;
            if (reader.get(query, aEn)) {
                med.alarms[j].enabled = aEn;
            }

            std::snprintf(query, sizeof(query), "medications[%u].a%u_h",
                          static_cast<unsigned>(i), static_cast<unsigned>(j));
            uint8_t aH = med.alarms[j].hour;
            if (reader.get(query, aH) && aH < 24) {
                med.alarms[j].hour = aH;
            }

            std::snprintf(query, sizeof(query), "medications[%u].a%u_m",
                          static_cast<unsigned>(i), static_cast<unsigned>(j));
            uint8_t aM = med.alarms[j].minute;
            if (reader.get(query, aM) && aM < 60) {
                med.alarms[j].minute = aM;
            }
        }
    }

    if (mConfig) {
        syncFromAppConfig();
    }

    return true;
}

bool PillManager::saveLocalState() const
{
    char                  buff[kJsonBufferSize] {};
    SDK::JsonStreamWriter writer(buff, sizeof(buff));

    writer.startMap();
    {
        SDK::JsonStreamWriter::KeyedArrayScope medsArray(writer, "medications", kMaxMedications);
        for (std::size_t i = 0; i < kMaxMedications; ++i) {
            const Medication&                  med = mMedications[i];
            SDK::JsonStreamWriter::MapScope medObj(writer);
            writer.add("name", med.name);
            writer.add("enabled", med.enabled);
            for (std::size_t j = 0; j < kAlarmsPerMed; ++j) {
                char keyEn[16], keyH[16], keyM[16];
                std::snprintf(keyEn, sizeof(keyEn), "a%u_en", static_cast<unsigned>(j));
                std::snprintf(keyH, sizeof(keyH), "a%u_h", static_cast<unsigned>(j));
                std::snprintf(keyM, sizeof(keyM), "a%u_m", static_cast<unsigned>(j));

                writer.add(keyEn, med.alarms[j].enabled);
                writer.add(keyH, med.alarms[j].hour);
                writer.add(keyM, med.alarms[j].minute);
            }
        }
    }
    writer.endMap();

    if (writer.isError()) {
        LOG_ERROR("Failed to serialize medications JSON\n");
        return false;
    }

    auto file = mKernel.fs.file(mStoragePath);
    if (!file || !file->open(true, true)) {
        LOG_ERROR("Cannot open %s for writing\n", mStoragePath);
        return false;
    }

    const std::size_t len = std::strlen(buff);
    std::size_t       bw  = 0;
    const bool        ok  = file->write(buff, len, bw) && (bw == len);
    file->close();
    return ok;
}

bool PillManager::saveMedications(const Medication meds[kMaxMedications])
{
    std::memcpy(mMedications, meds, sizeof(mMedications));

    mSnoozedAlarms.erase(
        std::remove_if(mSnoozedAlarms.begin(), mSnoozedAlarms.end(),
                       [this](const SnoozedEntry& s) {
                           if (s.medIndex >= kMaxMedications || s.alarmIndex >= kAlarmsPerMed) {
                               return true;
                           }
                           const Medication& med  = mMedications[s.medIndex];
                           const AlarmSlot&  slot = med.alarms[s.alarmIndex];
                           return !med.enabled || !slot.enabled ||
                                  slot.hour != s.hour || slot.minute != s.minute;
                       }),
        mSnoozedAlarms.end());

    saveSnoozes();
    bool localOk = saveLocalState();
    bool cfgOk   = writeToAppConfig();
    return localOk && cfgOk;
}

bool PillManager::updateMedication(std::size_t index, const Medication& med)
{
    if (index >= kMaxMedications) {
        return false;
    }
    Medication copy[kMaxMedications];
    std::memcpy(copy, mMedications, sizeof(copy));
    copy[index] = med;
    return saveMedications(copy);
}

bool PillManager::hasActiveAlarms() const
{
    if (!mSnoozedAlarms.empty()) {
        return true;
    }
    for (std::size_t i = 0; i < kMaxMedications; ++i) {
        if (mMedications[i].enabled && mMedications[i].activeAlarmCount() > 0) {
            return true;
        }
    }
    return false;
}

void PillManager::saveSnoozes() const
{
    if (!mSnoozesPath) {
        return;
    }

    char                  buff[kJsonBufferSize] {};
    SDK::JsonStreamWriter writer(buff, sizeof(buff));

    writer.startMap();
    {
        SDK::JsonStreamWriter::KeyedArrayScope snoozesArray(writer, "snoozes", mSnoozedAlarms.size());
        for (const auto& s : mSnoozedAlarms) {
            SDK::JsonStreamWriter::MapScope snoozeObj(writer);
            writer.add("med_idx", s.medIndex);
            writer.add("alarm_idx", s.alarmIndex);
            writer.add("hour", s.hour);
            writer.add("minute", s.minute);
            writer.add("next_at", static_cast<uint32_t>(s.nextTriggerAt));
            writer.add("used", s.snoozesUsed);
            writer.add("rings_left", s.ringsLeft);
        }
    }
    writer.endMap();

    if (writer.isError()) {
        LOG_ERROR("Failed to serialize snoozes JSON\n");
        return;
    }

    auto file = mKernel.fs.file(mSnoozesPath);
    if (!file || !file->open(true, true)) {
        LOG_ERROR("Cannot open %s for writing\n", mSnoozesPath);
        return;
    }

    const std::size_t len = std::strlen(buff);
    std::size_t       bw  = 0;
    file->write(buff, len, bw);
    file->close();
}

void PillManager::loadSnoozes()
{
    mSnoozedAlarms.clear();
    if (!mSnoozesPath) {
        return;
    }

    auto file = mKernel.fs.file(mSnoozesPath);
    if (!file || !file->exist()) {
        return;
    }

    char        buff[kJsonBufferSize] {};
    std::size_t len = file->size();
    std::size_t br  = 0;
    if (len == 0 || len >= sizeof(buff) || !file->open()) {
        return;
    }

    const bool readOk = file->read(buff, len, br) && br > 0;
    file->close();
    if (!readOk) {
        return;
    }

    SDK::JsonStreamReader reader(buff, static_cast<uint32_t>(br));
    if (!reader.validate()) {
        LOG_ERROR("Invalid JSON in %s\n", mSnoozesPath);
        return;
    }

    std::size_t arrayLen = 0;
    if (!reader.getArrayLength("snoozes", arrayLen)) {
        return;
    }

    for (std::size_t i = 0; i < arrayLen; ++i) {
        char     query[48];
        uint8_t  medIdx   = 0;
        uint8_t  alarmIdx = 0;
        uint8_t  h        = 0;
        uint8_t  m        = 0;
        uint32_t nextAt   = 0;
        uint8_t  used     = 0;
        uint8_t  left     = 0;

        std::snprintf(query, sizeof(query), "snoozes[%u].med_idx", static_cast<unsigned>(i));
        if (!reader.get(query, medIdx) || medIdx >= kMaxMedications) continue;

        std::snprintf(query, sizeof(query), "snoozes[%u].alarm_idx", static_cast<unsigned>(i));
        if (!reader.get(query, alarmIdx) || alarmIdx >= kAlarmsPerMed) continue;

        std::snprintf(query, sizeof(query), "snoozes[%u].hour", static_cast<unsigned>(i));
        if (!reader.get(query, h) || h >= 24) continue;

        std::snprintf(query, sizeof(query), "snoozes[%u].minute", static_cast<unsigned>(i));
        if (!reader.get(query, m) || m >= 60) continue;

        std::snprintf(query, sizeof(query), "snoozes[%u].next_at", static_cast<unsigned>(i));
        if (!reader.get(query, nextAt) || nextAt == 0) continue;

        std::snprintf(query, sizeof(query), "snoozes[%u].used", static_cast<unsigned>(i));
        if (!reader.get(query, used)) continue;

        std::snprintf(query, sizeof(query), "snoozes[%u].rings_left", static_cast<unsigned>(i));
        if (!reader.get(query, left) || left == 0 || left > kMaxSnoozeRings) continue;

        SnoozedEntry entry {};
        entry.medIndex      = medIdx;
        entry.alarmIndex    = alarmIdx;
        entry.hour          = h;
        entry.minute        = m;
        entry.nextTriggerAt = static_cast<std::time_t>(nextAt);
        entry.snoozesUsed   = used;
        entry.ringsLeft     = left;
        mSnoozedAlarms.push_back(entry);
    }
}

void PillManager::setCallback(AlarmCallback cb)
{
    mOnAlarm = std::move(cb);
}

std::time_t PillManager::snoozeDeadline(std::time_t nowUtc)
{
    const std::time_t minuteStart = (nowUtc / 60) * 60;
    return minuteStart + static_cast<std::time_t>(kSnoozedTimeMinutes) * 60;
}

bool PillManager::isSnoozed(uint8_t medIdx, uint8_t alarmIdx) const
{
    for (const auto& s : mSnoozedAlarms) {
        if (s.medIndex == medIdx && s.alarmIndex == alarmIdx) {
            return true;
        }
    }
    return false;
}

void PillManager::armSnooze(uint8_t medIdx, uint8_t alarmIdx, uint8_t hour, uint8_t minute, std::time_t nowUtc)
{
    const std::time_t nextAt = snoozeDeadline(nowUtc);

    for (auto& s : mSnoozedAlarms) {
        if (s.medIndex == medIdx && s.alarmIndex == alarmIdx) {
            s.hour          = hour;
            s.minute        = minute;
            s.nextTriggerAt = nextAt;
            s.snoozesUsed   = 0;
            s.ringsLeft     = kMaxSnoozeRings;
            saveSnoozes();
            return;
        }
    }

    SnoozedEntry entry {};
    entry.medIndex      = medIdx;
    entry.alarmIndex    = alarmIdx;
    entry.hour          = hour;
    entry.minute        = minute;
    entry.nextTriggerAt = nextAt;
    entry.snoozesUsed   = 0;
    entry.ringsLeft     = kMaxSnoozeRings;
    mSnoozedAlarms.push_back(entry);
    saveSnoozes();
}

void PillManager::checkAlarms(uint8_t currentHour, uint8_t currentMinute, std::time_t nowUtc)
{
    const bool newMinute = (currentHour != mLastCheckedHour) ||
                           (currentMinute != mLastCheckedMinute);
    mLastCheckedHour     = currentHour;
    mLastCheckedMinute   = currentMinute;

    // 1. Check scheduled daily alarms on minute transitions
    if (newMinute) {
        for (uint8_t i = 0; i < kMaxMedications; ++i) {
            const Medication& med = mMedications[i];
            if (!med.enabled) {
                continue;
            }
            for (uint8_t j = 0; j < kAlarmsPerMed; ++j) {
                const AlarmSlot& slot = med.alarms[j];
                if (!slot.enabled) {
                    continue;
                }
                if (slot.hour == currentHour && slot.minute == currentMinute && !isSnoozed(i, j)) {
                    LOG_INFO("Medication alarm triggered: %s (%02u:%02u)\n",
                             med.name, slot.hour, slot.minute);

                    armSnooze(i, j, slot.hour, slot.minute, nowUtc);

                    ActiveAlert alert {};
                    alert.active      = true;
                    alert.medIndex    = i;
                    alert.alarmIndex  = j;
                    alert.hour        = slot.hour;
                    alert.minute      = slot.minute;
                    alert.snoozesUsed = 0;
                    alert.snoozesLeft = kMaxSnoozeRings;
                    alert.setMedName(med.name);

                    mActiveAlert = alert;
                    if (mOnAlarm) {
                        mOnAlarm(mActiveAlert);
                    }
                }
            }
        }
    }

    // 2. Check pending snoozed alarms (up to 3 snooze rings, 5 minutes apart)
    bool snoozesDirty = false;
    for (auto it = mSnoozedAlarms.begin(); it != mSnoozedAlarms.end(); ) {
        if (nowUtc >= it->nextTriggerAt) {
            const uint8_t medIdx   = it->medIndex;
            const uint8_t alarmIdx = it->alarmIndex;
            const uint8_t origH    = it->hour;
            const uint8_t origM    = it->minute;

            it->snoozesUsed    = static_cast<uint8_t>(it->snoozesUsed + 1);
            const uint8_t used = it->snoozesUsed;
            const uint8_t left = (it->ringsLeft > 0) ? static_cast<uint8_t>(it->ringsLeft - 1) : 0;

            LOG_INFO("Snoozed medication alarm re-triggered (%u/%u): med=%u alarm=%u\n",
                     static_cast<unsigned>(used),
                     static_cast<unsigned>(kMaxSnoozeRings),
                     static_cast<unsigned>(medIdx),
                     static_cast<unsigned>(alarmIdx));

            if (--it->ringsLeft == 0) {
                it = mSnoozedAlarms.erase(it);
            } else {
                it->nextTriggerAt = snoozeDeadline(nowUtc);
                ++it;
            }
            snoozesDirty = true;

            ActiveAlert alert {};
            alert.active      = true;
            alert.medIndex    = medIdx;
            alert.alarmIndex  = alarmIdx;
            alert.hour        = origH;
            alert.minute      = origM;
            alert.snoozesUsed = used;
            alert.snoozesLeft = left;
            if (medIdx < kMaxMedications) {
                alert.setMedName(mMedications[medIdx].name);
            } else {
                alert.setMedName("Medication");
            }

            mActiveAlert = alert;
            if (mOnAlarm) {
                mOnAlarm(mActiveAlert);
            }
        } else {
            ++it;
        }
    }

    if (snoozesDirty) {
        saveSnoozes();
    }
}

bool PillManager::snoozeActiveAlert(std::time_t nowUtc)
{
    if (!mActiveAlert.active) {
        return false;
    }

    if (mActiveAlert.snoozesLeft == 0) {
        LOG_INFO("Max snoozes (3) reached for %s; dismissing\n", mActiveAlert.medName);
        dismissActiveAlert();
        return false;
    }

    for (auto& s : mSnoozedAlarms) {
        if (s.medIndex == mActiveAlert.medIndex && s.alarmIndex == mActiveAlert.alarmIndex) {
            s.nextTriggerAt = snoozeDeadline(nowUtc);
            saveSnoozes();
            break;
        }
    }

    LOG_INFO("Snoozed %s (%u snooze(s) remaining)\n",
             mActiveAlert.medName,
             static_cast<unsigned>(mActiveAlert.snoozesLeft));
    mActiveAlert.active = false;
    return true;
}

void PillManager::dismissActiveAlert()
{
    if (mActiveAlert.active) {
        const uint8_t medIdx   = mActiveAlert.medIndex;
        const uint8_t alarmIdx = mActiveAlert.alarmIndex;
        LOG_INFO("Dismissed medication alarm: %s\n", mActiveAlert.medName);

        mSnoozedAlarms.erase(
            std::remove_if(mSnoozedAlarms.begin(), mSnoozedAlarms.end(),
                           [medIdx, alarmIdx](const SnoozedEntry& s) {
                               return s.medIndex == medIdx && s.alarmIndex == alarmIdx;
                           }),
            mSnoozedAlarms.end());
        saveSnoozes();
        mActiveAlert = {};
    } else if (!mSnoozedAlarms.empty()) {
        mSnoozedAlarms.clear();
        saveSnoozes();
    }
}

void PillManager::stopAll()
{
    LOG_INFO("Stopping all pending snoozes\n");
    mSnoozedAlarms.clear();
    mActiveAlert = {};
    saveSnoozes();
}

} // namespace PillSync
