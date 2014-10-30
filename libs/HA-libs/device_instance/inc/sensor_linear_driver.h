/**
 * @file sensor_linear_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 30-10-2014
 * @brief This is header file for ADC-sensors instance having linear graph in HA system.
 */
#ifndef __HA_SENSOR_LINEAR_DRIVER_H_
#define __HA_SENSOR_LINEAR_DRIVER_H_

#include "ADC_device.h"

class sensor_linear_instance: private adc_dev_class {
public:
    sensor_linear_instance(void);

    void device_config(adc_config_params_t *adc_config_params);
    uint32_t get_value(void);
private:
};

#endif //__HA_SENSOR_LINEAR_DRIVER_H_
