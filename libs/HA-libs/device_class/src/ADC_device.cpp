/**
 * @file ADC_device.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 21-10-2014
 * @brief This is source file for ADC device class for HA system.
 */
#include "ADC_device.h"

adc_dev_class::adc_dev_class(void)
{

}

void adc_dev_class::adc_dev_configure(port_t port, uint8_t pin, adc_t adc_x,
        uint8_t channel)
{
    gpio_params_t gpio_params;

    gpio_params.port = port;
    gpio_params.pin = pin;
    gpio_params.mode = in_analog;
    gpio_init(&gpio_params);

    adc_params_t adc_params;

    adc_params.adc = adc_x;
    adc_params.adc_channel = channel;
    adc_params.adc_mode = independent;
    adc_params.channel_type = regular_channel;
    adc_params.conv_mode = continuous_mode;
    adc_params.data_access = poll;
    adc_params.option = no_option;
    adc_params.adc_sample_time = ADC_SampleTime_28Cycles5;
    adc_init(&adc_params);
    adc_start();
}

void adc_dev_class::adc_dev_sampling_time_setup(uint8_t sample_time)
{

}

uint16_t adc_dev_class::adc_dev_get_value(void)
{
    return adc_convert();
}
