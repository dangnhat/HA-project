/**
 * @file button_switch_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for button device instance in HA system.
 */
#include "button_switch_driver.h"

using namespace btn_sw_ns;

/* configurable variables */
const static uint8_t max_btn_sw = 16;
const static uint8_t btn_sw_active_state = 0;
const static uint8_t btn_sw_sampling_time_cycle = 10;
const static uint16_t btn_hold_time = 100;

/* button&switch table and pid table */
static button_switch_instance* btn_sw_table[max_btn_sw];
#if SND_MSG
static kernel_pid_t pid_table[max_btn_sw];
#endif

static uint8_t time_cycle_count = 0;
static bool table_init = false;

static void btn_sw_table_init(void);

button_switch_instance::button_switch_instance(btn_or_sw_t type) :
        gpio_dev_class(true)
{
    this->dev_type = type;

    this->new_state_reg_1 = !btn_sw_active_state;
    this->new_state_reg_2 = !btn_sw_active_state;
    this->new_state_reg_3 = !btn_sw_active_state;
    this->old_state_reg = !btn_sw_active_state;

    this->hold_time_count = 0;

    if (this->dev_type == btn) {
        this->current_status = btn_no_pressed;
        this->old_status = btn_no_pressed;
    } else {
        this->current_status = sw_off;
        this->old_status = sw_off;
    }
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
    if(!table_init) {
        table_init = true;
        btn_sw_table_init();
    }
    this->assign_btn_sw();
}

btn_sw_status_t button_switch_instance::get_status(void)
{
    btn_sw_status_t status = current_status;

    if (current_status == btn_pressed) {
        current_status = btn_no_pressed;
    }

    old_status = current_status;

    return status;
}

bool button_switch_instance::is_changed_status(void)
{
    if (current_status != old_status) {
        return true;
    }
    return false;
}

void button_switch_instance::btn_sw_processing(void)
{
    if (this->dev_type == btn) {
        button_processing();
    } else {
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
            } else { //change from active->inactive
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
            } else { //change from active->inactive
                current_status = sw_off;
            }
        } // end if()
    } // end if()
}

void button_switch_instance::assign_btn_sw(void)
{
    for (int i = 0; i < max_btn_sw; i++) {
        if (btn_sw_table[i] == NULL) {
            btn_sw_table[i] = this;
#if SND_MSG
            pid_table[i] = thread_getpid();
#endif
            return;
        }
    }
}

void button_switch_instance::remove_btn_sw(void)
{
    for (int i = 0; i < max_btn_sw; i++) {
        if (btn_sw_table[i] == this) {
            btn_sw_table[i] = NULL;
#if SND_MSG
            pid_table[i] = KERNEL_PID_UNDEF; //redundant
#endif
            return;
        }
    }
}

static void btn_sw_table_init(void)
{
    for (int i = 0; i < max_btn_sw; i++) {
        btn_sw_table[i] = NULL;
    }
}

void btn_sw_callback_timer_isr(void)
{
    time_cycle_count = (time_cycle_count + 1) % btn_sw_sampling_time_cycle;

    if (time_cycle_count == (btn_sw_sampling_time_cycle - 1)) {
        for (int i = 0; i < max_btn_sw; i++) {
            if (btn_sw_table[i] != NULL) {
                btn_sw_table[i]->btn_sw_processing();
#if SND_MSG
                if (btn_sw_table[i]->is_changed_status()) {
                    msg_t msg;
                    msg.type = BTN_SW_MSG;
                    msg.content.value =
                            (uint32_t) btn_sw_table[i]->get_status();
                    msg_send(&msg, pid_table[i], false);
                }
#endif
            }
        } //end for()
    } //end if()
}
