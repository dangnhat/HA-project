/**
 * @file sensor_linear_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 30-10-2014
 * @brief This is source file for ADC-sensors instance having linear graph in HA system.
 */
#include "sensor_linear_driver.h"

sensor_linear_instance::sensor_linear_instance(void)
{

}

void sensor_linear_instance::device_configure(
        adc_config_params_t *adc_config_params)
{
    adc_dev_configure(adc_config_params->device_port,
            adc_config_params->device_pin, adc_config_params->adc_x,
            adc_config_params->adc_channel);
}
