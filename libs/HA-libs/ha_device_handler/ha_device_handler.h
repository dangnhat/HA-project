/**
 * @file ha_device_handler.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 03-11-2014
 * @brief This is header file for device handlers in HA system.
 */
#ifndef __HA_DEVICE_HANDLER_H_
#define __HA_DEVICE_HANDLER_H_

#include "button_switch_driver.h"
#include "on_off_bulb_driver.h"
#include "level_bulb_driver.h"
#include "rgb_driver.h"
#include "servo_sg90_driver.h"
#include "dimmer_driver.h"
#include "sensor_linear_driver.h"

void* end_point_handler(void* arg);

void button_handler(void);
void switch_handler(void);
void on_off_bulb_handler(void);
void level_bulb_handler(void);
void rgb_led_handler(void);
void dimmer_handler(void);
void servo_sg90_handler(void);
void sensor_linear_handler(void);

#endif //__HA_DEVICE_HANDLER_H_
