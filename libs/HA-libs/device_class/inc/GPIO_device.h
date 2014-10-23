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

using namespace ISRMgr_ns;

class gpio_dev_class: private gpio {
public:
    /**
     * @brief The callback function that will be assign into exti handler. It must be none-null.
     *  Assign a function pointer to it before calling gpio_dev_configure() method.
     */
    void (*gpio_dev_worker)(void);
protected:
    /**
     * @brief Constructor.
     *
     * @param[in] input_device Is an input device?
     */
    gpio_dev_class(bool input_device);

    /**
     * @brief
     *
     * @param[in] port
     * @param[in] pin
     */
    void gpio_dev_configure(port_t port, uint8_t pin);

    /**
     * @brief
     */
    void gpio_dev_int_both_edge(void);

    /**
     * @brief
     */
    void gpio_dev_int_rising_edge(void);

    /**
     * @brief
     */
    void gpio_dev_int_falling_edge(void);

    /**
     * @brief
     */
    void gpio_dev_on(void);

    /**
     * @brief
     */
    void gpio_dev_off(void);
private:
    bool in_dev;
    uint8_t exti_type;
};

#endif //__HA_GPIO_DEVICE_H_
