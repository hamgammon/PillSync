/**
 ******************************************************************************
 * @file    ConfigurationSimulator.hpp
 * @brief   Simulated sensor settings for the PillSync LVGL PC simulator.
 ******************************************************************************
 */

#ifndef CONFIG_SIMULATOR_HPP
#define CONFIG_SIMULATOR_HPP

#include <iostream>

// GPS sensor
#define GSP_SIM_ENABLE               0
#define GSP_SIM_SPEED_MIN            15
#define GPS_SIM_SPEED_BASE           20
#define GPS_SIM_SPEED_MAX            25
#define GPS_SIM_TIME_SEACH_SATELLITE 4

// Heart rate sensor
#define HEAT_RATE_SIM_ENABLE        0
#define HEAT_RATE_SIM_MIN_HR        120
#define HEAT_RATE_SIM_MAX_HR        180
#define HEAT_RATE_SIM_TYPE_TRAINING 2

// Pressure sensor
#define PRESSURE_SIM_ENABLE       0
#define PRESSURE_SIM_PRESS_VALLUE 1020.2

// Battery level sensor
#define BATT_LEVEL_SIM_ENABLE      1
#define BATT_LEVEL_SIM_START_VALUE 100
#define BATT_LEVEL_SIM_STEP_VALUE  0.2

// IMU wrist sensor
#define IMU_WRIST_SIM_ENABLE           1
#define IMU_WRIST_SIM_WRIST_DETECT_KEY '5'

// IMU step counter
#define IMU_STEP_COUNTER_SIM_ENABLE        0
#define IMU_STEP_COUNTER_SIM_STRIDE_LENGTH 1

#endif /* CONFIG_SIMULATOR_HPP */

