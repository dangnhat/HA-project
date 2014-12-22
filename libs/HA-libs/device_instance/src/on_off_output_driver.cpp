/**
 * @file on_off_bulb_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for on-off bulb device instance for HA system.
 */
#include "on_off_output_driver.h"
#include "ha_host_glb.h"

using namespace on_off_dev_ns;

static const uint8_t timer_period = 1; //ms

static on_off_output_instance* dev_table[ha_host_ns::max_end_point];

bool table_init = false;
static void dev_table_init(void);

on_off_output_instance::on_off_output_instance(void) :
        gpio_dev_class(false)
{
    this->active_level = 0;
    this->dev_status = on_off_dev_ns::dev_off;
    if (!table_init) {
        table_init = true;
        dev_table_init();
    }
}

on_off_output_instance::~on_off_output_instance(void)
{
    if (this->dev_status == on_off_dev_ns::dev_blink) {
        remove_dev();
    }
}

void on_off_output_instance::device_configure(
        gpio_config_params_t *gpio_config_params)
{
    gpio_dev_configure(gpio_config_params->device_port,
            gpio_config_params->device_pin, gpio_config_params->mode);
    dev_turn_off();
}

void on_off_output_instance::set_active_level(uint8_t active_level)
{
    this->active_level = active_level;
}

void on_off_output_instance::dev_turn_on(void)
{
    if (this->dev_status == on_off_dev_ns::dev_blink) {
        remove_dev();
    }
    this->dev_status = on_off_dev_ns::dev_on;
    (active_level == 0) ? gpio_dev_off() : gpio_dev_on();
}

void on_off_output_instance::dev_turn_off(void)
{
    if (this->dev_status == on_off_dev_ns::dev_blink) {
        remove_dev();
    }
    this->dev_status = on_off_dev_ns::dev_off;
    (active_level == 0) ? gpio_dev_on() : gpio_dev_off();
}

void on_off_output_instance::dev_toggle(void)
{
    if (this->dev_status == on_off_dev_ns::dev_blink) {
        remove_dev();
    }
    if (this->dev_status == on_off_dev_ns::dev_on) {
        dev_turn_off();
    } else {
        dev_turn_on();
    }
}

void on_off_output_instance::dev_blink(uint8_t freq_in_hz)
{
    remove_dev();
    this->time_cycle_count = 0;
    this->period_in_ms = 1000 / freq_in_hz; //ms
    if (this->period_in_ms < (2 * timer_period)) { //can't blink with freq running faster than timer freq 2 times.
        return;
    }
    this->dev_status = on_off_dev_ns::dev_blink;
    assign_dev();
}

void on_off_output_instance::blink_processing(void)
{
    this->time_cycle_count++;
    if (this->time_cycle_count == (this->period_in_ms / (2 * timer_period))) {
        this->time_cycle_count = 0;
        /* toggle */
        if (this->is_on_in_blink) { //dev was turned on. -> turn off
            this->is_on_in_blink = false;
            (active_level == 0) ? gpio_dev_on() : gpio_dev_off();
        } else {
            this->is_on_in_blink = true;
            (active_level == 0) ? gpio_dev_off() : gpio_dev_on();
        }
    }
}

on_off_dev_status_t on_off_output_instance::dev_get_state(void)
{
    return this->dev_status;
}

void on_off_output_instance::assign_dev(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (dev_table[i] == NULL) {
            dev_table[i] = this;
            return;
        }
    }
}

void on_off_output_instance::remove_dev(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (dev_table[i] == this) {
            dev_table[i] = NULL;
            return;
        }
    }
}

static void dev_table_init(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        dev_table[i] = NULL;
    }
}

void on_off_blink_callback_timer_isr(void)
{
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (dev_table[i] != NULL) {
            dev_table[i]->blink_processing();
        }
    }
}
