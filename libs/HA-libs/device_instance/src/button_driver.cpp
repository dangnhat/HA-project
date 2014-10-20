/**
 * @file button_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for button device instance for HA system.
 */
#include "button_driver.h"

button_instance::button_instance(void) :
        gpio_dev_class(true)
{

}

void button_instance::device_configure(config_params_t *config_params)
{
    gpio_dev_configure(config_params->device_port, config_params->device_pin);
}
