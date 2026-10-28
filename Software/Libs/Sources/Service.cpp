/**
 ******************************************************************************
 * @file    Service.cpp
 * @brief   PillSync background service implementation.
 ******************************************************************************
 */

#include "Service.hpp"
#include "SDK/Messages/MessageGuard.hpp"

#include <ctime>

#define LOG_MODULE_PRX      "Service"
#define LOG_MODULE_LEVEL    LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

namespace
{

struct Instant {
    std::tm     local;
    std::time_t utc;
};

Instant getNow()
{
    Instant now {};
    now.utc = std::time(nullptr);
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&now.local, &now.utc);
#else
    localtime_r(&now.utc, &now.local);
#endif
    return now;
}

} // namespace

Service::Service(SDK::Kernel& kernel)
    : mKernel(SDK::KernelProviderService::GetInstance().getKernel())
    , mConfig(nullptr)
    , mPillManager(mKernel)
{
    (void)kernel;
}

void Service::reloadAppConfig()
{
    mConfig.reset(new SDK::AppConfig(mKernel,
                                     AppConfigSchema::kConfigFile,
                                     AppConfigSchema::kFields,
                                     AppConfigSchema::kFieldCount));
    mPillManager.attachAppConfig(mConfig.get());
}

void Service::refreshTimeFormat()
{
    if (auto msg = SDK::make_msg<SDK::Message::RequestSystemSettings>(mKernel)) {
        if (msg.send(100) && msg.ok()) {
            mTimeFormat12h = msg->timeFormat;
        }
    }
}

void Service::run()
{
    LOG_INFO("PillSync Service thread started\n");

    mPillManager.load();
    reloadAppConfig();

    mPillManager.setCallback([this](const PillSync::ActiveAlert& alert) {
        LOG_INFO("Medication alarm callback: %s (%02u:%02u) snoozesUsed=%u snoozesLeft=%u\n",
                 alert.medName,
                 static_cast<unsigned>(alert.hour),
                 static_cast<unsigned>(alert.minute),
                 static_cast<unsigned>(alert.snoozesUsed),
                 static_cast<unsigned>(alert.snoozesLeft));

        mNeedTrigger    = true;
        mTriggeredAlert = alert;

        if (!mGUIStarted) {
            runGUI();
        } else {
            sendActivatedAlarm();
        }
    });

    while (true) {
        const Instant now       = getNow();
        uint32_t      sleepTime = mPillManager.execute(now.local, now.utc);

        // If no work is scheduled and GUI is not started, wake periodically to evaluate
        // system state; otherwise sleep until the next alarm check or incoming message.
        if (!mGUIStarted && sleepTime == PillSync::PillManager::kNoWork) {
            sleepTime = 60000u;
        }

        SDK::MessageBase* msg = nullptr;
        if (mKernel.comm.getMessage(msg, sleepTime)) {
            switch (msg->getType()) {
                case SDK::MessageType::COMMAND_APP_STOP:
                    LOG_INFO("Force exit from the application\n");
                    mPillManager.setCallback(nullptr);
                    mKernel.comm.releaseMessage(msg);
                    return;

                case SDK::MessageType::COMMAND_APP_NOTIF_GUI_RUN:
                    LOG_INFO("GUI is now running\n");
                    onStartGUI();
                    break;

                case SDK::MessageType::COMMAND_APP_NOTIF_GUI_STOP:
                    LOG_INFO("GUI has stopped\n");
                    onStopGUI();
                    break;

                case CustomMessage::SCHEDULE_LIST: {
                    LOG_DEBUG("Received updated SCHEDULE_LIST from GUI\n");
                    auto* m = static_cast<CustomMessage::ScheduleList*>(msg);
                    mPillManager.saveSchedule(m->meds, m->vibrateOnly);
                } break;

                case CustomMessage::ALARM_ACTIVATE_EFFECT: {
                    LOG_DEBUG("Received ALARM_ACTIVATE_EFFECT\n");
                    auto* m = static_cast<CustomMessage::AlarmActivateEffect*>(msg);
                    playEffect(m->alert);
                } break;

                case CustomMessage::ALARM_SNOOZE: {
                    LOG_INFO("Received ALARM_SNOOZE\n");
                    const Instant cur = getNow();
                    mPillManager.snoozeActiveAlert(cur.utc);
                    stopRinging();
                } break;

                case CustomMessage::ALARM_DISMISS: {
                    LOG_INFO("Received ALARM_DISMISS\n");
                    mPillManager.dismissActiveAlert();
                    stopRinging();
                } break;

                default:
                    break;
            }

            mKernel.comm.releaseMessage(msg);
        }
    }
}

void Service::onStartGUI()
{
    mGUIStarted = true;

    reloadAppConfig();
    refreshTimeFormat();

    if (mNeedTrigger) {
        sendActivatedAlarm();
    }

    sendScheduleList();
}

void Service::onResumeGUI()
{
}

void Service::onSuspendGUI()
{
}

void Service::onStopGUI()
{
    mGUIStarted = false;
}

void Service::sendScheduleList()
{
    SDK::send_msg<CustomMessage::ScheduleList>(
        mKernel, mPillManager.getMedications(), mTimeFormat12h, mPillManager.isVibrateOnly());
}

void Service::sendActivatedAlarm()
{
    mNeedTrigger = false;
    SDK::send_msg<CustomMessage::ActivatedAlarm>(mKernel, mTriggeredAlert);
    mTriggeredAlert = {};
}

void Service::runGUI()
{
    auto* msg = mKernel.comm.allocateMessage<SDK::Message::RequestAppRunGui>();
    if (msg) {
        mKernel.comm.sendMessage(msg);
        mKernel.comm.releaseMessage(msg);
    }
}

void Service::playEffect(const PillSync::ActiveAlert& alert)
{
    if (!alert.active) {
        return;
    }

    auto* backlightMsg = mKernel.comm.allocateMessage<SDK::Message::RequestBacklightSet>();
    if (backlightMsg) {
        backlightMsg->autoOffTimeoutMs = 4000;
        backlightMsg->brightness       = 100;
        mKernel.comm.sendMessage(backlightMsg);
        mKernel.comm.releaseMessage(backlightMsg);
    }

    auto* vibroMsg = mKernel.comm.allocateMessage<SDK::Message::RequestVibroPlay>();
    if (vibroMsg) {
        vibroMsg->notes[0].effect = SDK::Message::RequestVibroPlay::Effect::ALERT_750MS_100;
        vibroMsg->notes[1].pause  = 250;
        vibroMsg->notes[2].effect = SDK::Message::RequestVibroPlay::Effect::ALERT_750MS_100;
        vibroMsg->notes[3].pause  = 250;
        vibroMsg->notes[4].effect = SDK::Message::RequestVibroPlay::Effect::ALERT_750MS_100;
        vibroMsg->notesCount      = 5;
        mKernel.comm.sendMessage(vibroMsg);
        mKernel.comm.releaseMessage(vibroMsg);
    }

    if (!alert.vibrateOnly) {
        auto* buzzerMsg = mKernel.comm.allocateMessage<SDK::Message::RequestBuzzerPlay>();
        if (buzzerMsg) {
            // Melodic 3-tone chime: 1318 Hz (E6), 1568 Hz (G6), 2093 Hz (C7)
            buzzerMsg->notes[0].volume    = 100;
            buzzerMsg->notes[0].time      = 750;
            buzzerMsg->notes[0].frequency = 1318;

            buzzerMsg->notes[1].volume    = 0;
            buzzerMsg->notes[1].time      = 250;
            buzzerMsg->notes[1].frequency = 0;

            buzzerMsg->notes[2].volume    = 100;
            buzzerMsg->notes[2].time      = 750;
            buzzerMsg->notes[2].frequency = 1568;

            buzzerMsg->notes[3].volume    = 0;
            buzzerMsg->notes[3].time      = 250;
            buzzerMsg->notes[3].frequency = 0;

            buzzerMsg->notes[4].volume    = 100;
            buzzerMsg->notes[4].time      = 750;
            buzzerMsg->notes[4].frequency = 2093;

            buzzerMsg->notesCount         = 5;
            mKernel.comm.sendMessage(buzzerMsg);
            mKernel.comm.releaseMessage(buzzerMsg);
        }
    }
}

void Service::stopRinging()
{
    auto* buzzerMsg = mKernel.comm.allocateMessage<SDK::Message::RequestBuzzerPlay>();
    if (buzzerMsg) {
        mKernel.comm.sendMessage(buzzerMsg);
        mKernel.comm.releaseMessage(buzzerMsg);
    }

    auto* vibroMsg = mKernel.comm.allocateMessage<SDK::Message::RequestVibroPlay>();
    if (vibroMsg) {
        mKernel.comm.sendMessage(vibroMsg);
        mKernel.comm.releaseMessage(vibroMsg);
    }
}

