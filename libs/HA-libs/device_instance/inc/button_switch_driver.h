/**
 * @file button_switch_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for button & switch device instance in HA system.
 */
#ifndef __HA_BUTTON_SWITCH_DRIVER_H_
#define __HA_BUTTON_SWITCH_DRIVER_H_

/* 0 if you want to poll manually */
#define SND_MSG (1)

/* RIOT's include */
#if SND_MSG
extern "C" {
#include "thread.h"
#include "msg.h"
}
#endif

#include "GPIO_device.h"

namespace btn_sw_ns {
typedef enum
    : uint8_t {
        /* button status */
        btn_no_pressed = 0x01,
    btn_pressed = 0x02,
    btn_on_hold = 0x03,
    /* switch status */
    sw_on = 0x10,
    sw_off = 0x20
} btn_sw_status_t;

typedef enum {
    btn,    //button
    sw      //switch
} btn_or_sw_t;

#if SND_MSG
enum {
    BTN_SW_MSG
};
#endif
}

class button_switch_instance: public gpio_dev_class {
public:
    button_switch_instance(btn_sw_ns::btn_or_sw_t type);
    ~button_switch_instance(void);

    void device_configure(gpio_config_params_t *gpio_config_params);
    btn_sw_ns::btn_sw_status_t get_status(void);
    bool is_changed_status(void);
    void btn_sw_processing(void);
private:
    btn_sw_ns::btn_or_sw_t dev_type;
    btn_sw_ns::btn_sw_status_t current_status, old_status;

    uint8_t new_state_reg_1;
    uint8_t new_state_reg_2;
    uint8_t new_state_reg_3;
    uint8_t old_state_reg;

    uint16_t hold_time_count; //button's var.

    void button_processing(void);
    void switch_processing(void);
    void assign_btn_sw(void);
    void remove_btn_sw(void);
};

void btn_sw_callback_timer_isr(void);

#endif //__HA_BUTTON_SWITCH_DRIVER_H_
