/**
 * @file GPIO_device.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for GPIO device class for HA system.
 */
#ifndef __HA_GPIO_DEVICE_H_
#define __HA_GPIO_DEVICE_H_

#include "device_common.h"

using namespace dev_param_ns;
using namespace ISRMgr_ns;

class gpio_dev_class: private gpio {
protected:
    /**
     * @brief The callback function that will be assign into exti handler. It must be none-null.
     *  Assign a function pointer to it before calling gpio_dev_configure() method.
     */
    void (*gpio_dev_worker)(void);

    /**
     * @brief Constructor.
     *
     * @param[in] input_device Is an input device?
     */
    gpio_dev_class(bool input_device);

    /**
     * @brief Initialize GPIO.
     *
     * @param[in] port
     * @param[in] pin
     */
    void gpio_dev_configure(port_t port, uint8_t pin);

    /**
     * @brief Initialize EXTI in falling-rising edge mode.
     */
    void gpio_dev_int_both_edge(void);

    /**
     * @brief Initialize EXTI in rising edge mode.
     */
    void gpio_dev_int_rising_edge(void);

    /**
     * @brief Initialize EXTI in falling edge mode.
     */
    void gpio_dev_int_falling_edge(void);

    /**
     * @brief Set output = 1 (=VCC) on output device;
     */
    void gpio_dev_on(void);

    /**
     * @brief Set output = 0 (=GND) on output device;
     */
    void gpio_dev_off(void);

    uint8_t gpio_dev_read(void);
private:
    bool in_dev;
    uint8_t exti_type;
};

#endif //__HA_GPIO_DEVICE_H_
