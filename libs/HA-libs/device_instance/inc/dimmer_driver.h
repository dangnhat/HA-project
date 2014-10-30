/**
 * @file dimmer_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for dimmer device instance for HA system.
 */
#ifndef __HA_DIMMER_DRIVER_H_
#define __HA_DIMMER_DRIVER_H_

#include "ADC_device.h"

class dimmer_instance: private adc_dev_class {
public:
    dimmer_instance(void);

    void device_configure(adc_config_params_t *adc_config_params);
    uint8_t get_percent(void);
private:
};

#endif //__HA_DIMMER_DRIVER_H_
