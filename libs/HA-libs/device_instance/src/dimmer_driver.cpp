/**
 * @file dimmer_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for dimmer device instance for HA system.
 */
#include "dimmer_driver.h"

dimmer_instance::dimmer_instance(void)
{

}

void dimmer_instance::device_configure(adc_config_params_t *adc_config_params)
{
    adc_dev_configure(adc_config_params->device_port, adc_config_params->device_pin,
            adc_config_params->adc_x, adc_config_params->adc_channel);
}

uint8_t dimmer_instance::get_percent(void)
{
    uint16_t adc_value;

    adc_value = adc_dev_get_value();

    return adc_value * 100 / adc_value_max;
}
