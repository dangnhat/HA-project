/**
 * @file button_switch_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for button device instance in HA system.
 */
#include "button_switch_driver.h"

using namespace btn_sw_ns;

const uint8_t max_btn_sw = 16;

static button_switch_instance* btn_sw_table[max_btn_sw];

const uint8_t btn_sw_active_state = 0;
const uint16_t btn_hold_time = 200;

const uint8_t btn_sw_sampling_time_cycle = 5;
uint8_t time_cycle_count = 0;

button_switch_instance::button_switch_instance(btn_or_sw_t type) :
        gpio_dev_class(true)
{
    this->dev_type = type;

    new_state_reg_1 = !btn_sw_active_state;
    new_state_reg_2 = !btn_sw_active_state;
    new_state_reg_3 = !btn_sw_active_state;
    old_state_reg = !btn_sw_active_state;

    hold_time_count = 0;

    if (this->dev_type == btn) {
        current_status = btn_no_pressed;
    } else {
        current_status = sw_off;
    }

    this->assign_btn_sw();
}

button_switch_instance::~button_switch_instance(void)
{
    this->remove_btn_sw();
}

void button_switch_instance::device_configure(
        gpio_config_params_t *gpio_config_params)
{
    gpio_dev_configure(gpio_config_params->device_port,
            gpio_config_params->device_pin);
}

btn_sw_status_t button_switch_instance::get_status(void)
{
    btn_sw_status_t status = current_status;

    if (current_status == btn_pressed) {
        current_status = btn_no_pressed;
    }

    return status;
}

void button_switch_instance::btn_sw_processing(void)
{
    if (this->dev_type == btn) {
        button_processing();
    }else {
        switch_processing();
    }
}

void button_switch_instance::button_processing(void)
{
    /* sampling */
    new_state_reg_3 = new_state_reg_2;
    new_state_reg_2 = new_state_reg_1;
    new_state_reg_1 = gpio_dev_read();

    /* update hold_time_count value */
    if (hold_time_count != 0) {
        hold_time_count--;
        if (hold_time_count == 0) { //time out, button is hold.
            current_status = btn_on_hold;
        }
    }

    /* processing */
    if ((new_state_reg_1 == new_state_reg_2)
            && (new_state_reg_2 == new_state_reg_3)) { //new stable state

        if (new_state_reg_1 != old_state_reg) { //change state
            old_state_reg = new_state_reg_1;

            if (new_state_reg_1 == btn_sw_active_state) { //change from inactive->active
                hold_time_count = btn_hold_time; //set time out value
            }

            if (new_state_reg_1 == !btn_sw_active_state) { //change from active->inactive
                if (hold_time_count > 0) {  //button is pressed
                    current_status = btn_pressed;
                } else {    //button is un-hold
                    current_status = btn_no_pressed;
                }
                hold_time_count = 0;
            }
        } // end if()
    } // end if()
}

void button_switch_instance::switch_processing(void)
{
    /* sampling */
    new_state_reg_3 = new_state_reg_2;
    new_state_reg_2 = new_state_reg_1;
    new_state_reg_1 = gpio_dev_read();

    /* processing */
    if ((new_state_reg_1 == new_state_reg_2)
            && (new_state_reg_2 == new_state_reg_3)) { //new stable state

        if (new_state_reg_1 != old_state_reg) { //change state
            old_state_reg = new_state_reg_1;

            if (new_state_reg_1 == btn_sw_active_state) { //change from inactive->active
                current_status = sw_on;
            }

            if (new_state_reg_1 == !btn_sw_active_state) { //change from active->inactive
                current_status = sw_off;
            }
        } // end if()
    } // end if()
}

void btn_sw_callback_timer_isr(void)
{
    time_cycle_count = (time_cycle_count + 1) % btn_sw_sampling_time_cycle;

    if (time_cycle_count == (btn_sw_sampling_time_cycle - 1)) {
        for (int i = 0; i < max_btn_sw; i++) {
            if (btn_sw_table[i] != NULL) {
                btn_sw_table[i]->btn_sw_processing();
            }
        }
    }
}

void button_switch_instance::assign_btn_sw(void)
{
    for (int i = 0; i < max_btn_sw; i++) {
        if (btn_sw_table[i] == NULL) {
            btn_sw_table[i] = this;
        }
    }
}

void button_switch_instance::remove_btn_sw(void)
{
    for (int i = 0; i < max_btn_sw; i++) {
        if (btn_sw_table[i] == this) {
            btn_sw_table[i] = NULL;
        }
    }
}

void btn_sw_table_init(void)
{
    for (int i = 0; i < max_btn_sw; i++) {
        btn_sw_table[i] = NULL;
    }
}
