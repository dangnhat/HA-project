/**
 * @file level_bulb_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 22-10-2014
 * @brief This is source file for multi-level bulb device instance for HA system.
 */
#include "level_bulb_driver.h"

const uint16_t max_level_intensity = 65535;
const uint32_t output_freq = 200; //Hz

level_bulb_instance::level_bulb_instance(void)
{
    this->is_blink = false;
    this->percent_intensity = 0;
    this->level_intensity = 0;
}

void level_bulb_instance::device_configure(
        pwm_config_params_t *pwm_config_params)
{
    pwm_dev_configure(pwm_config_params->device_port,
            pwm_config_params->device_pin, pwm_config_params->timer_x,
            pwm_config_params->pwm_channel);
    pwm_dev_period_setup(max_level_intensity);
    pwm_dev_output_frequency_setup(output_freq);
}

void level_bulb_instance::set_output_freq(uint32_t freq_in_hz)
{
    pwm_dev_output_frequency_setup(freq_in_hz);
}

void level_bulb_instance::set_max_level_intensity(uint16_t max_level_intensity)
{
    if (is_blink) {
        blink_off();
    }
    pwm_dev_period_setup(max_level_intensity);
    set_level_intensity(this->level_intensity);
}

void level_bulb_instance::set_percent_intensity(uint8_t percent_intensity)
{
    if (is_blink) {
        blink_off();
    }
    this->percent_intensity = percent_intensity;
    pwm_dev_duty_cycle_setup(this->percent_intensity);
    this->level_intensity = timer_pulse_convert(this->percent_intensity);
}

void level_bulb_instance::set_level_intensity(uint16_t level_intensity)
{
    if (is_blink) {
        blink_off();
    }
    this->level_intensity = level_intensity;
    pwm_dev_level_setup(this->level_intensity);
    this->percent_intensity = duty_cycle_convert(this->level_intensity);
}

void level_bulb_instance::blink_on(void)
{
    this->is_blink = true;
    /* output frequency = 1Hz */
    pwm_dev_period_setup(39999);
    pwm_dev_timer_frequency_setup(40000);
    /* led-on in 0.5s, then led-off in 0.5s */
    pwm_dev_duty_cycle_setup(50);
}

void level_bulb_instance::blink_off(void)
{
    this->is_blink = false;
    pwm_dev_period_setup(max_level_intensity);
    pwm_dev_output_frequency_setup(output_freq);
    pwm_dev_level_setup(this->level_intensity);
}

uint16_t level_bulb_instance::get_level_intensity(void)
{
    return this->level_intensity;
}

uint8_t level_bulb_instance::get_percent_intensity(void)
{
    return this->percent_intensity;
}