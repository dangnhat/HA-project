/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        controller.h
 * @brief       Home automation CC's controller thread.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <cstdint>

extern "C" {
#include "thread.h"
}

#include "cir_queue.h"

namespace controller_ns {

extern kernel_pid_t controller_pid;

/* Data cir_queue */
extern cir_queue slp_to_controller_queue;
extern cir_queue ble_to_controller_queue;

}

/**
 * @brief   Create and start controller thread,
 */
void controller_start(void);

/**
 * @brief   Callback for 1 second interrupt.
 */
void second_int_callback(void);

/**
 * @brief   List devices shell command.
 */
void controller_list_devices(int argc, char** argv);

/**
 * @brief   Scene configuring command.
 *
 * @details Usage:  refer to scene_cmd_usage in scene_mng.cpp.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void controller_scene_cmd(int argc, char** argv);

/**
 * @brief   Zone configuring command.
 *
 * @details Usage:  refer to zone_cmd_usage in zone.cpp.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void controller_zone_cmd(int argc, char** argv);

#endif // CONTROLLER_H_
