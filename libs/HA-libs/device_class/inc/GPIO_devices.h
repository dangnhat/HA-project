/**
 * @file GPIO_devices.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for GPIO device class for HA system.
 */
#ifndef __HA_GPIO_DEVICE_H_
#define __HA_GPIO_DEVICE_H_

#include "MB1_System.h"

using namespace gpio_ns;

class gpio_dev_class: private gpio, private ISRMgr {
public:
    void (*gpio_dev_worker)(void);
protected:
    gpio_dev_class(bool input_device);

    void gpio_dev_configure(port_t port, uint8_t pin);
    void gpio_dev_int_both_edge(void);
    void gpio_dev_int_rising_edge(void);
    void gpio_dev_int_falling_edge(void);
    void gpio_dev_on(void);
    void gpio_dev_off(void);
private:
    bool in_dev;
    uint8_t isr_type;
};

#endif //__HA_GPIO_DEVICE_H_
