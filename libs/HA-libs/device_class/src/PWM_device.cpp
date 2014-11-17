/**
 * @file PWM_device.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 22-10-2014
 * @brief This is source file for PWM device class for HA system.
 */
#include "PWM_device.h"

const uint32_t sys_core_clock = 72000000;

pwm_dev_class::pwm_dev_class(void)
{

}

void pwm_dev_class::pwm_dev_configure(port_t port, uint8_t pin,
        pwm_timer_t timer, uint8_t pwm_channel)
{
    gpio_params_t gpio_params;

    gpio_params.port = port;
    gpio_params.pin = pin;
    gpio_params.mode = af_push_pull;
    gpio_params.gpio_speed = speed_50MHz;
    gpio_init(&gpio_params);

    pwm_params_t pwm_params;
    pwm_params.timer = timer;
    pwm_params.pwm_channel = pwm_channel;
    pwm_params.pwm_mode = TIM_OCMode_PWM1;
    pwm_params.period = this->period;
    pwm_params.prescaler = this->prescaler;
    pwm_params.pulse_width = 0;
    pwm_init(&pwm_params);
}

void pwm_dev_class::pwm_dev_start_stop(bool is_started)
{
    if (is_started) {
        pwm_oc_enable();
    } else {
        pwm_oc_disable();
    }
}

void pwm_dev_class::pwm_dev_period_setup(uint16_t period)
{
    pwm_period_setup(period);
}

void pwm_dev_class::pwm_dev_level_setup(uint16_t level)
{
    if (level > this->period) {
        return;
    }
    pwm_pulse_width_setup(level);
}

void pwm_dev_class::pwm_dev_timer_frequency_setup(uint32_t freq_in_hz)
{
    uint16_t prescaler;

    prescaler = sys_core_clock / freq_in_hz - 1;
    pwm_prescaler_setup(prescaler);
}

void pwm_dev_class::pwm_dev_output_frequency_setup(uint32_t freq_in_hz)
{
    uint32_t freq_timer_clock;

    freq_timer_clock = (this->period + 1) * freq_in_hz;
    if (freq_timer_clock > sys_core_clock) {
        return;
    }
    pwm_dev_timer_frequency_setup(freq_timer_clock);
}

void pwm_dev_class::pwm_dev_duty_cycle_setup(uint8_t duty_cycle)
{
    uint16_t timer_pulses;
    timer_pulses = timer_pulse_convert(duty_cycle);
    if (timer_pulses > this->period) {
        return;
    }
    pwm_pulse_width_setup(timer_pulses);
}

uint16_t pwm_dev_class::timer_pulse_convert(uint8_t duty_cycle)
{
    uint16_t ccr_value;
    uint8_t mod;

    ccr_value = (duty_cycle * (this->period)) / 100;
    mod = (duty_cycle * (this->period)) % 100;

    if (mod >= 50) {
        ccr_value++;
    }

    return ccr_value;
}

uint8_t pwm_dev_class::duty_cycle_convert(uint16_t timer_pulses)
{
    uint8_t duty_cycle;
    uint16_t mod;

    duty_cycle = (timer_pulses * 100) / (this->period);
    mod = (timer_pulses * 100) % (this->period);

    if (mod >= (this->period / 2)) {
        duty_cycle++;
    }

    return duty_cycle;
}
