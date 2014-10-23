#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

extern "C" {
#include "vtimer.h"
}

#include "MB1_System.h"
#include "switch_driver.h"
#include "on_off_bulb_driver.h"
#include "button_driver.h"
#include "dimmer_driver.h"

void sw_worker(void);
//on_off_bulb_instance bulb1;
int main(void)
{
    timex_t time;
    MB1_system_init();

//    switch_instance sw1;
//    config_params_t params;
//    params.device_port = port_B;
//    params.device_pin = 7;
//    sw1.gpio_dev_worker = &sw_worker;
//    sw1.device_configure(&params);
//
//    params.device_port = port_C;
//    params.device_pin = 11;
//    bulb1.device_configure(&params);

    dimmer_instance dimmer1;
    config_params_t params;
    params.device_port = port_C;
    params.device_pin = 0;
    params.adc_x = adc1;
    params.adc_channel = ADC_Channel_10;
    dimmer1.device_configure(&params);

    uint8_t dimmer_percent = 0;
    while(1) {
        dimmer_percent = dimmer1.get_percent();
        printf("dimmer: %d\%\n", dimmer_percent);
        time.seconds = 1;
        vtimer_sleep(time);
    }
}

void sw_worker(void) {
//    printf("sw_worker\n");
//    if(bulb1.bulb_get_state()) {
//        bulb1.bulb_turn_off();
//    }else {
//        bulb1.bulb_turn_on();
//    }
}
