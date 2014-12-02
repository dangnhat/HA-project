/**
 * @file level_bulb_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 22-10-2014
 * @brief This is header file for multi-level bulb device instance for HA system.
 */
#ifndef __HA_LEVEL_BULB_DRIVER_H_
#define __HA_LEVEL_BULB_DRIVER_H_

#include "PWM_device.h"

using namespace dev_param_ns;

class level_bulb_instance: private pwm_dev_class {
public:
    level_bulb_instance(void);
    ~level_bulb_instance(void);

    void device_configure(pwm_config_params_t *pwm_config_params);
    void set_max_level_intensity(uint16_t max_level_intensity);
    void set_percent_intensity(uint8_t percent_intensity);
    void set_level_intensity(uint16_t level_intensity);
    void set_output_freq(uint32_t freq_in_hz);
    void set_active_level(uint8_t active_level);
    void restart(void);
    void stop(void);
    void blink(uint8_t freq_in_hz);
    void blink_processing(void);
    bool bulb_is_blink(void);
    uint16_t get_level_intensity(void);
    uint8_t get_percent_intensity(void);
private:
    void assign_bulb(void);
    void remove_bulb(void);
    uint16_t period_in_ms = 1000; //ms
    uint16_t time_cycle_count = 0;
    bool is_on_in_blink = false;

    bool is_blink;
    uint16_t level_intensity;
    uint8_t percent_intensity;
    uint8_t active_level;
};

void bulb_blink_callback_timer_isr(void);

#endif //__HA_LEVEL_BULB_DRIVER_H_
