#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "MB1_System.h"

#define AVG_SLOPE 5
#define V25 1750

using namespace adc_ns;

int main(void)
{
    MB1_system_init();

    ADC_TempSensorVrefintCmd(ENABLE);
    adc_params_t adc_params;

    adc_params.adc = adc1;
    adc_params.adc_mode = independent;
    adc_params.conv_mode = continuous_mode;
    adc_params.channel_type = regular_channel;
    adc_params.data_access = dma_request;
    adc_params.option = no_option;
    adc_params.adc_sample_time = ADC_SampleTime_41Cycles5;
    MB1_ADC1_IN16.adc_init(&adc_params);

    uint16_t adc_value = MB1_ADC1_IN16.adc_convert();

    uint16_t t0C = ((V25 - adc_value)/AVG_SLOPE + 25);

    printf("temp: %d%c\n", t0C, 176);

    while(1);
}
