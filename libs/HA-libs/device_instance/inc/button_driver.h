/**
 * @file button_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for button device instance for HA system.
 */
#ifndef __HA_BUTTON_DRIVER_H_
#define __HA_BUTTON_DRIVER_H_

#include "GPIO_devices.h"
#include "device_param.h"

using namespace dev_param_ns;

class button_instance: public gpio_dev_class {
public:
    button_instance(void);

    void device_configure(config_params_t *config_params);
private:

};

#endif //__HA_BUTTON_DRIVER_H_
