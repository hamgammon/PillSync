/**
 ******************************************************************************
 * @file    ModelListener.hpp
 * @brief   Events raised by Model toward the currently active Screen.
 ******************************************************************************
 */

#ifndef MODEL_LISTENER_HPP
#define MODEL_LISTENER_HPP

#include "Medication.hpp"
#include <cstddef>

class Model;

class ModelListener
{
public:
    ModelListener() : model(nullptr) {}
    virtual ~ModelListener() = default;

    void bind(Model* m) { model = m; }

    virtual void onIdleTimeout() {}
    virtual void onSuspend() {}
    virtual void onScheduleUpdated() {}
    virtual void onAlarmActivated(const PillSync::ActiveAlert& alert) { (void)alert; }

protected:
    Model* model;
};

#endif // MODEL_LISTENER_HPP

