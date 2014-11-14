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
#include "sensor_event_driver.h"

namespace ha_node_ns {
const uint8_t dev_list_pattern_maxsize = 16;
const uint8_t gpio_pattern_size = 8;
const uint8_t adc_pwm_pattern_size = 20;
const uint8_t dev_pattern_maxsize = 96;
const char ha_dev_list_file[] = "dev_list";

const char dev_list_pattern[dev_list_pattern_maxsize] = "EP%d: 0x%lX\n";

const char gpio_dev_config_pattern[gpio_pattern_size] =
        "P%c%d\n";

const char adc_dev_config_pattern[adc_pwm_pattern_size] =
        "P%c%d A%d_IN%d\n";

const char pwm_dev_config_pattern[adc_pwm_pattern_size] =
        "P%c%d T%d_CH%d\n";

const char rgb_config_pattern[dev_pattern_maxsize] =
        "R: P%c%d T%d_CH%d\n"
        "G: P%c%d T%d_CH%d\n"
        "B: P%c%d T%d_CH%d\n"
        "Cal: %%R=%d, %%G=%d, %%B=%d\n";

const char senlnr_config_pattern[dev_pattern_maxsize] =
        "P%c%d A%d_IN%d\n"
        "E-type: %c\n" //type: l (linear), r (rational)
        "A B: %s %s\n"
        "Thr: F=%d U=%d O=%d\n"; //F: filter, U:underflow, O=overflow
}

void* end_point_handler(void* arg);

void button_handler(uint32_t dev_id);
void switch_handler(uint32_t dev_id);
void on_off_bulb_handler(uint32_t dev_id);
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
