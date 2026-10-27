/**
 ******************************************************************************
 * @file    ScreenManager.hpp
 * @brief   Owns the active Screen and switches between them.
 ******************************************************************************
 */

#ifndef SCREEN_MANAGER_HPP
#define SCREEN_MANAGER_HPP

#include "gui/screens/Screen.hpp"

class Model;

class ScreenManager
{
public:
    static ScreenManager& instance();

    void start(Model& model, ScreenId first);
    void switchTo(ScreenId id);

private:
    ScreenManager() = default;

    static void asyncCb(void* user);
    void        switchNow(ScreenId id);
    Screen*     create(ScreenId id);

    Model*   mModel      = nullptr;
    Screen*  mCurrent    = nullptr;
    ScreenId mPending    = ScreenId::Main;
    bool     mPendingSet = false;
};

#endif // SCREEN_MANAGER_HPP

