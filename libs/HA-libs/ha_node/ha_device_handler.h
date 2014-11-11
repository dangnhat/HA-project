/**
 * @file ha_device_handler.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 03-11-2014
 * @brief This is header file for device handlers in HA system.
 */
#ifndef __HA_DEVICE_HANDLER_H_
#define __HA_DEVICE_HANDLER_H_

#include "ha_node_msg_id.h"
#include "button_switch_driver.h"
#include "on_off_bulb_driver.h"
#include "level_bulb_driver.h"
#include "rgb_driver.h"
#include "servo_sg90_driver.h"
#include "dimmer_driver.h"
#include "sensor_linear_driver.h"

void* end_point_handler(void* arg);

void button_handler(uint32_t dev_id);
void switch_handler(uint32_t dev_id);
void on_off_bulb_handler(uint32_t dev_id);
void level_bulb_handler(uint32_t dev_id);
void rgb_led_handler(uint32_t dev_id);
void dimmer_handler(uint32_t dev_id);
void servo_sg90_handler(uint32_t dev_id);
void sensor_linear_handler(uint32_t dev_id);

int get_file_name_from_dev_id(uint32_t dev_id, char* file_name);

uint8_t get_configuration_common(uint32_t dev_id, uint8_t* buff,
        float* fl_buff, uint8_t offset, uint8_t size_buff, bool is_float);

#endif //__HA_DEVICE_HANDLER_H_
