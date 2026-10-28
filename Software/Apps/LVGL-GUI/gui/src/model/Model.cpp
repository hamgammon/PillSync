/**
 ******************************************************************************
 * @file    Model.cpp
 * @brief   GUI-side state of PillSync and IPC bridge to Service.
 ******************************************************************************
 */

#include "gui/model/Model.hpp"
#include "gui/model/ModelListener.hpp"
#include "gui/screens/ScreenManager.hpp"

#include "SDK/Kernel/KernelProviderGUI.hpp"
#include "SDK/Messages/MessageGuard.hpp"
#include "SDK/Port/LVGL/LvglPort.hpp"

#define LOG_MODULE_PRX      "Model"
#define LOG_MODULE_LEVEL    LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

#include <cstdio>
#include <cstring>

Model::Model()
    : modelListener(nullptr)
    , mKernel(SDK::KernelProviderGUI::GetInstance().getKernel())
{
    SDK::LVGL::Port::GetInstance().setAppLifeCycleCallback(this);
    SDK::LVGL::Port::GetInstance().setCustomMessageHandler(this);

    initDefaultSchedule();
    setCapabilities();
}

void Model::initDefaultSchedule()
{
    static const uint8_t kHours[PillSync::kAlarmsPerMed] = { 9, 10, 18 };
    for (std::size_t i = 0; i < PillSync::kMaxMedications; ++i) {
        char nameBuf[PillSync::kMaxMedNameLen + 1];
        std::snprintf(nameBuf, sizeof(nameBuf), "Medication %zu", i + 1);
        mMedications[i].setName(nameBuf);
        mMedications[i].enabled = (i == 0);
        for (std::size_t j = 0; j < PillSync::kAlarmsPerMed; ++j) {
            mMedications[i].alarms[j].hour    = (i == 0) ? kHours[j] : ((j == 0) ? 9 : (j == 1 ? 13 : 18));
            mMedications[i].alarms[j].minute  = 0;
            mMedications[i].alarms[j].enabled = (i == 0) ? true : (j == 0);
        }
    }
}

void Model::handleKeyEvent(uint8_t key)
{
    LOG_DEBUG("key = %c\n", static_cast<char>(key));
    if (isClick(key)) {
        resetIdleTimer();
        if (!mActiveAlert.active) {
            mStayInApp = true;
        }
    }
}

void Model::resetIdleTimer()
{
    mIdleTimer = App::Config::kScreenTimeoutSteps;
}

void Model::exitApp()
{
    LOG_INFO("Exiting PillSync GUI\n");
    SDK::LVGL::Port::GetInstance().setAppLifeCycleCallback(nullptr);
    SDK::LVGL::Port::GetInstance().setCustomMessageHandler(nullptr);
    mKernel.sys.exit();
}

void Model::switchToNextPriorityScreen()
{
    if (mActiveAlert.active) {
        ScreenManager::instance().switchTo(ScreenId::Ringing);
        return;
    }

    if (mStayInApp) {
        mStayInApp = false;
        ScreenManager::instance().switchTo(ScreenId::Main);
        return;
    }

    exitApp();
}

void Model::saveMedications()
{
    SDK::send_msg<CustomMessage::ScheduleList>(mKernel, mMedications, mTimeFormat12h, mVibrateOnly);
    if (modelListener) {
        modelListener->onScheduleUpdated();
    }
}

void Model::setVibrateOnly(bool vibrateOnly)
{
    mVibrateOnly = vibrateOnly;
    saveMedications();
}

void Model::toggleVibrateOnly()
{
    setVibrateOnly(!mVibrateOnly);
}

void Model::setSelectedMedIndex(std::size_t idx)
{
    if (idx < PillSync::kMaxMedications) {
        mSelectedMedIndex = idx;
    }
}

void Model::setSelectedAlarmIndex(std::size_t idx)
{
    if (idx < PillSync::kAlarmsPerMed) {
        mSelectedAlarmIndex = idx;
    }
}

void Model::playAlarmEffect()
{
    SDK::send_msg<CustomMessage::AlarmActivateEffect>(mKernel, mActiveAlert);
}

void Model::snoozeAlarm()
{
    SDK::send_msg<CustomMessage::AlarmSnooze>(mKernel);
    mActiveAlert = {};
}

void Model::dismissAlarm()
{
    SDK::send_msg<CustomMessage::AlarmDismiss>(mKernel);
    mActiveAlert = {};
}

void Model::formatTime(uint8_t hour, uint8_t minute, char* buf, std::size_t bufSize) const
{
    if (!buf || bufSize == 0) {
        return;
    }
    if (!mTimeFormat12h) {
        std::snprintf(buf, bufSize, "%02u:%02u",
                      static_cast<unsigned>(hour % 24),
                      static_cast<unsigned>(minute % 60));
    } else {
        const bool    pm  = (hour >= 12);
        const uint8_t h12 = (hour % 12 == 0) ? 12 : (hour % 12);
        std::snprintf(buf, bufSize, "%u:%02u %s",
                      static_cast<unsigned>(h12),
                      static_cast<unsigned>(minute % 60),
                      pm ? "PM" : "AM");
    }
}

void Model::decIdleTimer()
{
    if (mIdleTimer > 0) {
        if (--mIdleTimer == 0 && modelListener) {
            modelListener->onIdleTimeout();
        }
    }
}

void Model::setCapabilities()
{
    auto* msg = mKernel.comm.allocateMessage<SDK::Message::RequestSetCapabilities>();
    if (msg) {
        msg->enMusicControl      = true;
        msg->enUsbChargingScreen = true;
        mKernel.comm.sendMessage(msg);
        mKernel.comm.releaseMessage(msg);
    }
}

bool Model::isClick(uint8_t key)
{
    return key == SDK::GUI::Button::L1 ||
           key == SDK::GUI::Button::L2 ||
           key == SDK::GUI::Button::R1 ||
           key == SDK::GUI::Button::R2;
}

void Model::onStart()
{
    LOG_INFO("Started\n");
}

void Model::onFrame()
{
    if (mIsRunning) {
        decIdleTimer();
    }
}

void Model::onResume()
{
    mIsRunning = true;
    resetIdleTimer();
}

void Model::onSuspend()
{
    mIsRunning = false;
    if (modelListener) {
        modelListener->onSuspend();
    }
}

void Model::onStop()
{
    LOG_INFO("Force exit from PillSync GUI\n");
}

bool Model::customMessageHandler(SDK::MessageBase* msg)
{
    static ModelListener sNullListener;
    ModelListener* listener = modelListener ? modelListener : &sNullListener;

    switch (msg->getType()) {
        case CustomMessage::SCHEDULE_LIST: {
            LOG_DEBUG("SCHEDULE_LIST received\n");
            auto* m = static_cast<CustomMessage::ScheduleList*>(msg);
            std::memcpy(mMedications, m->meds, sizeof(mMedications));
            mTimeFormat12h = m->timeFormat12h;
            mVibrateOnly   = m->vibrateOnly;
            listener->onScheduleUpdated();
        } break;

        case CustomMessage::ACTIVATED_ALARM: {
            LOG_INFO("ACTIVATED_ALARM received: %s\n",
                     static_cast<CustomMessage::ActivatedAlarm*>(msg)->alert.medName);
            auto* m      = static_cast<CustomMessage::ActivatedAlarm*>(msg);
            mActiveAlert = m->alert;
            listener->onAlarmActivated(mActiveAlert);
        } break;

        default:
            break;
    }

    return true;
}

