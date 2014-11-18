/**
 * @file rgb_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 22-10-2014
 * @brief This is header file for RGB-led device instance in HA system.
 */
#ifndef __HA_RGB_DRIVER_H_
#define __HA_RGB_DRIVER_H_

#include "PWM_device.h"
#include "level_bulb_driver.h"

namespace rgb_ns {
typedef enum
    : uint8_t {
        model_16bits = 16, //Red:5bits, Green:6bits, Blue:5bits.
    model_24bits = 24
} rgb_color_model_t;
}

class rgb_instance: private pwm_dev_class {
public:
    rgb_instance(void);

    void device_configure(pwm_config_params_t *red_channel_params,
            pwm_config_params_t *green_channel_params,
            pwm_config_params_t *blue_channel_params);
    void restart(void);
    void stop(void);
    void set_white_point(uint8_t red_percent_wp, uint8_t green_percent_wp,
            uint8_t blue_percent_wp);
    void set_color_model(rgb_ns::rgb_color_model_t rgb_color_model);
    void rgb_set_color(uint32_t rgb_color);
    void rgb_set_color(uint8_t red_percent, uint8_t green_percent,
            uint8_t blue_percent);
    uint32_t get_current_color(void);
private:
    uint8_t red_percent_wp;  //at white point
    uint8_t green_percent_wp;  //at white point
    uint8_t blue_percent_wp; //at white point

    uint32_t current_color;
    rgb_ns::rgb_color_model_t color_model;

    level_bulb_instance red_bulb;
    level_bulb_instance green_bulb;
    level_bulb_instance blue_bulb;

    void rgb_calibrate(void);
};

#endif //__HA_RGB_DRIVER_H_
