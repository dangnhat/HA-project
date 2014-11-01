/**
 * @file GPIO_device.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for GPIO device class for HA system.
 */
#include "GPIO_device.h"

ISRMgr *isr_mgr_ptr = &MB1_ISRs;

gpio_dev_class::gpio_dev_class(bool input_device)
{
    this->in_dev = input_device;
    this->exti_type = 0;
}

void gpio_dev_class::gpio_dev_on(void)
{
    gpio_set();
}

void gpio_dev_class::gpio_dev_off(void)
{
    gpio_reset();
}

void gpio_dev_class::gpio_dev_configure(port_t port, uint8_t pin)
{
    this->exti_type = pin;
    gpio_params_t gpio_params;

    gpio_params.port = port;
    gpio_params.pin = pin;
    gpio_params.gpio_speed = speed_10MHz;
    gpio_params.mode = out_push_pull;

    if (this->in_dev) {
        gpio_params.mode = in_floating;
    }

    gpio_init(&gpio_params);
}

void gpio_dev_class::gpio_dev_int_both_edge(void)
{
    exti_init(both_edge);
    isr_mgr_ptr->subISR_assign((ISR_t) exti_type, gpio_dev_worker);
}

void gpio_dev_class::gpio_dev_int_rising_edge(void)
{
    exti_init(rising_edge);
    isr_mgr_ptr->subISR_assign((ISR_t) exti_type, gpio_dev_worker);
}

void gpio_dev_class::gpio_dev_int_falling_edge(void)
{
    exti_init(falling_edge);
    isr_mgr_ptr->subISR_assign((ISR_t) exti_type, gpio_dev_worker);
}

uint8_t gpio_dev_class::gpio_dev_read(void)
{
    return gpio_read();
}
