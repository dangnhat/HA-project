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
#include "ha_node_glb.h"
#include "button_switch_driver.h"
#include "on_off_output_driver.h"
#include "level_bulb_driver.h"
#include "rgb_driver.h"
#include "servo_sg90_driver.h"
#include "dimmer_driver.h"
#include "sensor_linear_driver.h"
#include "sensor_event_driver.h"

namespace ha_node_ns {
const uint8_t dev_list_pattern_maxsize = 16;
const uint8_t gpio_pattern_size = 10;
const uint8_t adc_pwm_pattern_size = 20;
const uint8_t dev_pattern_maxsize = 110;
const char ha_dev_list_file[] = "dev_list";

const char dev_list_pattern[dev_list_pattern_maxsize] = "dID: 0x%lx\n";

const char gpio_dev_config_pattern[gpio_pattern_size] = "P%c%hu\n";

const char adc_dev_config_pattern[adc_pwm_pattern_size] = "P%c%hu A%hu_IN%hu\n";

const char pwm_dev_config_pattern[adc_pwm_pattern_size] = "P%c%hu T%hu_CH%hu\n";

const char rgb_config_pattern[dev_pattern_maxsize] = "R: P%c%hu T%hu_CH%hu\n"
        "G: P%c%hu T%hu_CH%hu\n"
        "B: P%c%hu T%hu_CH%hu\n"
        "Cal: %%R=%hu, %%G=%hu, %%B=%hu\n";

const char senlnr_config_pattern[dev_pattern_maxsize] = "P%c%hu A%hu_IN%hu\n"
        "E-type: %c %c\n" //type: l (linear), r (rational), p (polynomial)
        "A1 B1 C1: %s %s %s\n"
        "A2 B2 C2: %s %s %s\n"
        "Thr: F=%d U=%d O=%d\n";//F: filter, U:underflow, O=overflow
}

void* end_point_handler(void* arg);

void button_handler(uint32_t dev_id);
void switch_handler(uint32_t dev_id);
void on_off_output_handler(uint32_t dev_id);
void level_bulb_handler(uint32_t dev_id);
void rgb_led_handler(uint32_t dev_id);
void dimmer_handler(uint32_t dev_id);
void servo_sg90_handler(uint32_t dev_id);
void sensor_linear_handler(uint32_t dev_id);
void sensor_event_handler(uint32_t dev_id);

int get_file_name_from_dev_id(uint32_t dev_id, char* file_name);

bool gpio_common_get_config(uint32_t dev_id, gpio_config_params_t *gpio_params);
bool adc_common_get_config(uint32_t dev_id, adc_config_params_t *adc_params);
bool pwm_common_get_config(uint32_t dev_id, pwm_config_params_t *pwm_params);
bool rgb_get_config(uint32_t dev_id, rgb_instance *rgb);
bool sensor_linear_get_config(uint32_t dev_id, sensor_linear_instance *senlnr);

#endif //__HA_DEVICE_HANDLER_H_
