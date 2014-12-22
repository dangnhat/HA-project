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

/**
 * @brief Stop the running End Point.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void stop_endpoint_callback(int argc, char** argv);

/**
 * @brief Run an End Point with a configured device.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void rst_endpoint_callback(int argc, char** argv);

/**
 * @brief Configure a button device for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void button_config(int argc, char** argv);

/**
 * @brief Configure a switch device for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void switch_config(int argc, char** argv);

/**
 * @brief Configure an on/off output device for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void on_off_output_config(int argc, char** argv);

/**
 * @brief Configure an event sensor device for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void event_sensor_config(int argc, char** argv);

/**
 * @brief Configure a dimmer device for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void dimmer_config(int argc, char** argv);

/**
 * @brief Configure a level bulb device for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void level_bulb_config(int argc, char** argv);

/**
 * @brief Configure a servo device for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void servo_config(int argc, char** argv);

/**
 * @brief configure RGB led with calibrated intensity for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void rgb_led_config(int argc, char** argv);

/**
 * @brief configure ADC sensors for the specified EndPoint.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
void adc_sensor_config(int argc, char** argv);

/**
 * @brief get dev_id in dev_list file and send to end point having id = ep_id.
 *
 * @param[in] ep_id Target endpoint id.
 */
void run_endpoint(int8_t ep_id);

#endif //__SHELL_CMDS_DEV_CONFIG_H_
