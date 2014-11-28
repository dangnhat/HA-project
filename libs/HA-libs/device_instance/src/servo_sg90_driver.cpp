/**
 * @file servo_sg90_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 27-10-2014
 * @brief This is source file for servo Tower Pro SG90 device instance in HA system.
 */
#include "servo_sg90_driver.h"

const uint16_t pos_0_degree = 600; //us
const uint16_t pos_180_degree = 2400; //us

servo_sg90_instance::servo_sg90_instance(void)
{
    this->angle = 0;
}

void servo_sg90_instance::device_configure(
        pwm_config_params_t *pwm_config_params)
{
    pwm_dev_configure(pwm_config_params->device_port,
            pwm_config_params->device_pin, pwm_config_params->timer_x,
            pwm_config_params->pwm_channel);

    /* 20ms */
    pwm_dev_period_setup(19999);
    /* control servo sg90 in 50Hz (20ms/1period) */
    pwm_dev_output_frequency_setup(50);

    set_angle(this->angle);
}

void servo_sg90_instance::set_angle(uint8_t angle)
{
    if (angle > 180) {
        return;
    }

    this->angle = angle;

    uint16_t pulse_width;
    uint8_t delta_pulse = (pos_180_degree - pos_0_degree) / 180;

    pulse_width = angle * delta_pulse + pos_0_degree;
    pwm_dev_level_setup(pulse_width);
}

void servo_sg90_instance::restart(void)
{
    pwm_dev_start_stop(true);
}

void servo_sg90_instance::stop(void)
{
    pwm_dev_start_stop(false);
}

uint8_t servo_sg90_instance::get_angle(void)
{
    return this->angle;
}
