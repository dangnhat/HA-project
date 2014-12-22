/**
 * @file level_bulb_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 22-10-2014
 * @brief This is source file for multi-level bulb device instance for HA system.
 */
#include "level_bulb_driver.h"
#include "ha_host_glb.h"

const static uint16_t max_level_intensity = 65535;
const static uint32_t output_freq = 200; //Hz

const static uint8_t timer_period = 1; //ms

static bool table_init = false;
static level_bulb_instance* table_bulb[ha_host_ns::max_end_point];

static void bulb_table_init(void);

level_bulb_instance::level_bulb_instance(void)
{
    this->is_blink = false;
    /* active in low-level as default */
    this->active_level = 0;
    this->percent_intensity = 100;
    this->level_intensity = max_level_intensity;
    if (!table_init) {
        table_init = true;
        bulb_table_init();
    }
}

level_bulb_instance::~level_bulb_instance(void)
{
    if (is_blink) {
        remove_bulb();
    }
}

void level_bulb_instance::device_configure(
        pwm_config_params_t *pwm_config_params)
{
    pwm_dev_configure(pwm_config_params->device_port,
            pwm_config_params->device_pin, pwm_config_params->timer_x,
            pwm_config_params->pwm_channel);
    pwm_dev_period_setup(max_level_intensity);
    set_percent_intensity(percent_intensity);
    pwm_dev_output_frequency_setup(output_freq);
}

void level_bulb_instance::set_output_freq(uint32_t freq_in_hz)
{
    pwm_dev_output_frequency_setup(freq_in_hz);
}

void level_bulb_instance::set_max_level_intensity(uint16_t max_level_intensity)
{
    pwm_dev_period_setup(max_level_intensity);
    set_level_intensity(this->level_intensity);
}

void level_bulb_instance::set_percent_intensity(uint8_t percent_intensity)
{
    if (is_blink) {
        remove_bulb();
    }
    this->is_blink = false;
    this->percent_intensity = percent_intensity;
    if (active_level == 0) {
        pwm_dev_duty_cycle_setup(100 - this->percent_intensity);
    } else {
        pwm_dev_duty_cycle_setup(this->percent_intensity);
    }
    this->level_intensity = timer_pulse_convert(this->percent_intensity);
}

void level_bulb_instance::set_level_intensity(uint16_t level_intensity)
{
    if (is_blink) {
        remove_bulb();
    }
    this->is_blink = false;
    this->level_intensity = level_intensity;
    if (active_level == 0) {
        pwm_dev_level_setup(max_level_intensity - this->level_intensity);
    } else {
        pwm_dev_level_setup(this->level_intensity);
    }
    this->percent_intensity = duty_cycle_convert(this->level_intensity);
}

void level_bulb_instance::set_active_level(uint8_t active_level)
{
    this->active_level = active_level;
}

void level_bulb_instance::restart(void)
{
    pwm_dev_start_stop(true);
}

void level_bulb_instance::stop(void)
{
    if (is_blink) {
        remove_bulb();
    }
    pwm_dev_start_stop(false);
}

void level_bulb_instance::blink(uint8_t freq_in_hz)
{
    remove_bulb();
    this->time_cycle_count = 0;

    this->period_in_ms = 1000 / freq_in_hz; //ms
    if (this->period_in_ms < (2 * timer_period)) {
        return;
    }
    this->is_blink = true;
    assign_bulb();
}

void level_bulb_instance::blink_processing(void)
{
    this->time_cycle_count++;
    if (this->time_cycle_count == (period_in_ms / (2 * timer_period))) {
        this->time_cycle_count = 0;
        /* toggle */
        if (is_on_in_blink) { //turn off
            is_on_in_blink = false;
            if (active_level == 0) {
                pwm_dev_duty_cycle_setup(100);
            } else {
                pwm_dev_duty_cycle_setup(0);
            }
        } else { //turn on
            is_on_in_blink = true;
            if (this->percent_intensity == 0) {
                if (active_level == 0) {
                    pwm_dev_duty_cycle_setup(0);
                } else {
                    pwm_dev_duty_cycle_setup(100);
                }
            } else {
                if (active_level == 0) {
                    pwm_dev_duty_cycle_setup(100 - this->percent_intensity);
                } else {
                    pwm_dev_duty_cycle_setup(this->percent_intensity);
                }
            }
        }
    }
}

bool level_bulb_instance::bulb_is_blink(void)
{
    return this->is_blink;
}

uint16_t level_bulb_instance::get_level_intensity(void)
{
    return this->level_intensity;
}

uint8_t level_bulb_instance::get_percent_intensity(void)
{
    return this->percent_intensity;
}

void level_bulb_instance::assign_bulb(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (table_bulb[i] == NULL) {
            table_bulb[i] = this;
            return;
        }
    }
}

void level_bulb_instance::remove_bulb(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (table_bulb[i] == this) {
            table_bulb[i] = NULL;
            return;
        }
    }
}

static void bulb_table_init(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        table_bulb[i] = NULL;
    }
}

void bulb_blink_callback_timer_isr(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (table_bulb[i] != NULL) {
            table_bulb[i]->blink_processing();
        }
    }
}
