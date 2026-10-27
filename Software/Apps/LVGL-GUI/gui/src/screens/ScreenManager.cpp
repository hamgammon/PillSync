/**
 ******************************************************************************
 * @file    ScreenManager.cpp
 * @brief   Owns the active Screen and switches between them.
 ******************************************************************************
 */

#include "gui/screens/ScreenManager.hpp"

#include "lvgl.h"

#define LOG_MODULE_PRX      "ScreenManager"
#define LOG_MODULE_LEVEL    LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

#include "gui/model/Model.hpp"
#include "gui/screens/Screen.hpp"
#include "gui/screens/MainScreen.hpp"
#include "gui/screens/MedDetailScreen.hpp"
#include "gui/screens/AlarmActionScreen.hpp"
#include "gui/screens/TimePickerScreen.hpp"
#include "gui/screens/SavedScreen.hpp"
#include "gui/screens/RingingScreen.hpp"

ScreenManager& ScreenManager::instance()
{
    static ScreenManager sInstance;
    return sInstance;
}

void ScreenManager::start(Model& model, ScreenId first)
{
    mModel = &model;
    switchNow(first);
}

void ScreenManager::switchTo(ScreenId id)
{
    mPending = id;
    if (!mPendingSet) {
        if (lv_async_call(&ScreenManager::asyncCb, this) != LV_RESULT_OK) {
            LOG_ERROR("Switch to screen %u could not be scheduled\n", static_cast<unsigned>(id));
            return;
        }
        mPendingSet = true;
    }
}

void ScreenManager::asyncCb(void* user)
{
    auto* self        = static_cast<ScreenManager*>(user);
    self->mPendingSet = false;
    self->switchNow(self->mPending);
}

void ScreenManager::switchNow(ScreenId id)
{
    Screen* next = create(id);
    if (!next) {
        LOG_WARNING("No screen for id %u\n", static_cast<unsigned>(id));
        return;
    }

    Screen* old = mCurrent;
    if (old) {
        old->onHide();
        mModel->bind(nullptr);
    }

    next->create();
    lv_screen_load(next->root());
    mCurrent = next;

    if (old) {
        delete old;
    }

    mModel->bind(next);
    next->onShow();
}

Screen* ScreenManager::create(ScreenId id)
{
    Model& m = *mModel;
    switch (id) {
        case ScreenId::Main:        return new MainScreen(m);
        case ScreenId::MedDetail:   return new MedDetailScreen(m);
        case ScreenId::AlarmAction: return new AlarmActionScreen(m);
        case ScreenId::TimePicker:  return new TimePickerScreen(m);
        case ScreenId::Saved:       return new SavedScreen(m);
        case ScreenId::Ringing:     return new RingingScreen(m);
    }
    return nullptr;
}

