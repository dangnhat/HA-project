/**
 * @file button_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for button & switch device instance in HA system.
 */
#ifndef __HA_BUTTON_DRIVER_H_
#define __HA_BUTTON_DRIVER_H_

#include "GPIO_device.h"

namespace btn_sw_ns {
typedef enum {
    /* common status */
    no_pressed,
    /* button status */
    pressed,
    on_hold,
    /* switch status */
    sw_on,
    sw_off
} btn_sw_status_t;

typedef enum {
    btn,    //button
    sw      //switch
} btn_or_sw_t;
}

class button_switch_instance: public gpio_dev_class {
public:
    button_switch_instance(btn_sw_ns::btn_or_sw_t type);
    ~button_switch_instance(void);

    void device_configure(gpio_config_params_t *gpio_config_params);
    btn_sw_ns::btn_sw_status_t get_status(void);
    void button_processing(void);
    void switch_processing(void);
private:
    btn_sw_ns::btn_or_sw_t dev_type;
    btn_sw_ns::btn_sw_status_t current_status;
    uint8_t new_state_reg_1;
    uint8_t new_state_reg_2;
    uint8_t new_state_reg_3;
    uint8_t old_state_reg;

    uint16_t hold_time_count; //button var.

    void assign_btn_sw(void);
    void remove_btn_sw(void);
};

void btn_sw_table_init(void);
void btn_sw_callback_timer_isr(void);

#endif //__HA_BUTTON_DRIVER_H_
