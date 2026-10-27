/**
 ******************************************************************************
 * @file    Screen.hpp
 * @brief   Base class for all PillSync LVGL screens.
 ******************************************************************************
 */

#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <cstdint>

#include "lvgl.h"
#include "gui/model/Model.hpp"
#include "gui/model/ModelListener.hpp"

enum class ScreenId : uint8_t {
    Main,
    MedDetail,
    AlarmAction,
    TimePicker,
    Saved,
    Ringing,
};

class Screen : public ModelListener
{
public:
    explicit Screen(Model& model);
    ~Screen() override;

    Screen(const Screen&)            = delete;
    Screen& operator=(const Screen&) = delete;

    void create();
    void destroy();

    lv_obj_t* root() const { return mRoot; }

    virtual void onShow() {}
    virtual void onHide() {}
    virtual void onKey(uint8_t code) { (void)code; }

    void onIdleTimeout() override;
    void onAlarmActivated(const PillSync::ActiveAlert& alert) override;

protected:
    virtual void build() = 0;

    Model&    mModel;
    lv_obj_t* mRoot = nullptr;

private:
    static void keyEventCb(lv_event_t* e);
};

#endif // SCREEN_HPP

