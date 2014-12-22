/**
 * @file ha_device_handler.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 03-11-2014
 * @brief This is header file for device handlers in HA system.
 */
#ifndef __HA_DEVICE_HANDLER_H_
#define __HA_DEVICE_HANDLER_H_

#include "ha_host_msg_id.h"
#include "ha_host_glb.h"
#include "button_switch_driver.h"
#include "on_off_output_driver.h"
#include "level_bulb_driver.h"
#include "rgb_driver.h"
#include "servo_sg90_driver.h"
#include "dimmer_driver.h"
#include "adc_sensor_driver.h"
#include "sensor_event_driver.h"

namespace ha_host_ns {
const uint8_t dev_pattern_maxsize = 110;

const char ha_dev_list_file_name[] = "dev_list";

const char dev_list_pattern[] = "dID: 0x%lx\n";

const char gpio_dev_config_pattern[] = "P%c%hu M:%c\n";

const char adc_dev_config_pattern[] = "P%c%hu A%hu_IN%hu\n";

const char pwm_dev_config_pattern[] = "P%c%hu T%hu_CH%hu\n";

const char rgb_config_pattern[] = "R: P%c%hu T%hu_CH%hu\n"
        "G: P%c%hu T%hu_CH%hu\n"
        "B: P%c%hu T%hu_CH%hu\n"
        "Cal: %%R=%hu, %%G=%hu, %%B=%hu\n";

const char adc_sensor_config_pattern[] = "P%c%hu A%hu_IN%hu\n"
        "Thr: F=%d U=%d O=%d\n" //F: filter, U:underflow, O=overflow
        "E:%hu P:%hu\n";

const char sensor_equa_type[] = "%c\n";

const char sensor_equa_params[] = "%g\n";
}

/**
 * @brief It's a callback function of EndPoint threads.
 *
 * @param[in] arg The pointer to argument passed into callback function.
 */
void* end_point_handler(void* arg);

/**
 * @brief The function manages button device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void button_handler(uint32_t dev_id);

/**
 * @brief The function manages switch device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void switch_handler(uint32_t dev_id);

/**
 * @brief The function manages On/Off Output device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void on_off_output_handler(uint32_t dev_id);

/**
 * @brief The function manages level bulb device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void level_bulb_handler(uint32_t dev_id);

/**
 * @brief The function manages RGB-led device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void rgb_led_handler(uint32_t dev_id);

/**
 * @brief The function manages dimmer device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void dimmer_handler(uint32_t dev_id);

/**
 * @brief The function manages servo SG90 device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void servo_sg90_handler(uint32_t dev_id);

/**
 * @brief The function manages ADC sensor device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void adc_sensor_handler(uint32_t dev_id);

/**
 * @brief The function manages event sensor device. It's called by end_point_handler.
 *
 * @param[in] dev_id Device ID.
 */
void event_sensor_handler(uint32_t dev_id);

/**
 * @brief Parse device ID to get EP ID. The EP ID is name of configuration file.
 *
 * @param[in] dev_id Device ID.
 * @param[out] file_name The pointer to file name string.
 *
 * @return a negative number if an encoding error occurs, otherwise a positive number is returned.
 */
int get_file_name_from_dev_id(uint32_t dev_id, char* file_name);

/**
 * @brief Get configuration for pure GPIO device from file.
 *
 * @param[in] dev_id Device ID used to open config file.
 * @param[out] gpio_params The pointer contains device configuration.
 *
 * @return true if success, otherwise false.
 */
bool gpio_common_get_config(uint32_t dev_id, gpio_config_params_t *gpio_params);

/**
 * @brief Get configuration for pure ADC device from file.
 *
 * @param[in] dev_id Device ID used to open config file.
 * @param[out] gpio_params The pointer contains device configuration.
 *
 * @return true if success, otherwise false.
 */
bool adc_common_get_config(uint32_t dev_id, adc_config_params_t *adc_params);

/**
 * @brief Get configuration for pure PWM device from file.
 *
 * @param[in] dev_id Device ID used to open config file.
 * @param[out] gpio_params The pointer contains device configuration.
 *
 * @return true if success, otherwise false.
 */
bool pwm_common_get_config(uint32_t dev_id, pwm_config_params_t *pwm_params);

/**
 * @brief Get configuration for RGB-led device from file.
 *
 * @param[in] dev_id Device ID used to open config file.
 * @param[out] gpio_params The pointer contains device configuration.
 *
 * @return true if success, otherwise false.
 */
bool rgb_get_config(uint32_t dev_id, rgb_instance *rgb);

#endif //__HA_DEVICE_HANDLER_H_
