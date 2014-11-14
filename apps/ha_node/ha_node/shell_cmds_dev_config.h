/**
 * @file shell_cmds_dev_config.h
 * @author  Nguyen Van Hien  <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 10-Nov-2014
 * @brief Header file for device configuration related shell commands.
 */
#ifndef __SHELL_CMDS_DEV_CONFIG_H_
#define __SHELL_CMDS_DEV_CONFIG_H_

#include "ha_device_handler.h"

void button_config(int argc, char** argv);

void switch_config(int argc, char** argv);

void on_off_bulb_config(int argc, char** argv);

void sensor_event_config(int argc, char** argv);

void dimmer_config(int argc, char** argv);

void level_bulb_config(int argc, char** argv);

void servo_config(int argc, char** argv);

/**
 * @brief configure RGB led with calibrated intensity.
 *
 * @details
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void rgb_led_config(int argc, char** argv);

/**
 * @brief configure ADC sensors that have linear graph.
 *
 * @details
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void adc_sensor_config(int argc, char** argv);

#endif //__SHELL_CMDS_DEV_CONFIG_H_
