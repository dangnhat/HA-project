/**
 * @file on_off_bulb_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for on-off bulb device instance for HA system.
 */
#ifndef __HA_ON_OFF_BULB_DRIVER_H_
#define __HA_ON_OFF_BULB_DRIVER_H_

#include "GPIO_device.h"

using namespace dev_param_ns;

namespace on_off_dev_ns {
typedef enum {
    dev_on,
    dev_off,
    dev_blink
} on_off_dev_status_t;
}

class on_off_output_instance: public gpio_dev_class {
public:
    on_off_output_instance(void);
    ~on_off_output_instance(void);

    void device_configure(gpio_config_params_t *gpio_config_params);
    void dev_turn_on(void);
    void dev_turn_off(void);
    void dev_toggle(void);
    void dev_blink(uint8_t freq_in_hz);
    void blink_processing(void);
    void set_active_level(uint8_t active_level);
    on_off_dev_ns::on_off_dev_status_t dev_get_state(void);
private:
    void assign_dev(void);
    void remove_dev(void);
    uint8_t active_level;
    uint16_t period_in_ms = 1000;
    uint16_t time_cycle_count = 0;
    bool is_on_in_blink = false;
    on_off_dev_ns::on_off_dev_status_t dev_status;
};

void on_off_blink_callback_timer_isr(void);

#endif //__HA_ON_OFF_BULB_DRIVER_H_
