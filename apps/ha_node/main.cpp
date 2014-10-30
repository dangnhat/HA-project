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
#include "level_bulb_driver.h"
#include "rgb_driver.h"
#include "servo_sg90_driver.h"
#include "button_driver.h"
#include "dimmer_driver.h"

void button_handler(void);
void switch_handler(void);
void on_off_bulb_handler(void);
void level_bulb_handler(void);
void rgb_led_handler(void);
void dimmer_handler(void);
void servo_sg90_handler(void);
void sensor_lv1_handler(void);

int main(void)
{
    MB1_system_init();

//    button_instance btn;
//    gpio_config_params_t btn_params;
//    btn_params.device_port = port_A;
//    btn_params.device_pin = 1;
//    btn.device_configure(&btn_params);

//    dimmer_instance dimmer1;
//    adc_config_params_t adc_params;
//    adc_params.device_port = port_C;
//    adc_params.device_pin = 0;
//    adc_params.adc_x = adc1;
//    adc_params.adc_channel = ADC_Channel_10;
//    dimmer1.device_configure(&adc_params);
//
    rgb_instance rgb_led;
    pwm_config_params_t red_pwm_params;
    red_pwm_params.device_port = port_B;
    red_pwm_params.device_pin = 7;
    red_pwm_params.timer_x = gp_timer4;
    red_pwm_params.pwm_channel = 2;

    pwm_config_params_t green_pwm_params;
    green_pwm_params.device_port = port_B;
    green_pwm_params.device_pin = 8;
    green_pwm_params.timer_x = gp_timer4;
    green_pwm_params.pwm_channel = 3;

    pwm_config_params_t blue_pwm_params;
    blue_pwm_params.device_port = port_B;
    blue_pwm_params.device_pin = 9;
    blue_pwm_params.timer_x = gp_timer4;
    blue_pwm_params.pwm_channel = 4;
    rgb_led.device_configure(&red_pwm_params, &green_pwm_params, &blue_pwm_params);

//    servo_sg90_instance sg90;
//    pwm_config_params_t servo_params;
//    servo_params.device_port = port_A;
//    servo_params.device_pin = 1;
//    servo_params.timer_x = gp_timer5;
//    servo_params.pwm_channel = 2;
//    sg90.device_configure(&servo_params);

//    uint8_t dimmer_percent = 0;
//    uint8_t red_percent = 100;
//    uint8_t green_percent = 0;
//    uint8_t blue_percent = 0;
//    rgb_led.rgb_set_color(red_percent, green_percent, blue_percent);
    while (1) {
//        dimmer_percent = dimmer1.get_percent();
//        printf("dimmer: %d\n", dimmer_percent);
//        sg90.set_angle(dimmer_percent);
//        dimmer_percent = dimmer_percent + 10;
//        dimmer_percent > 180 ? dimmer_percent = 0 : dimmer_percent;
//        vtimer_usleep(500000);
        rgb_led.rgb_set_color(rgb_ns::white);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::red);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::green);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::blue);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::yellow);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::magenta);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::cyan);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::orange);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::rose);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::chartreuse);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::aquamarine);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::violet);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::azure);
        vtimer_usleep(2000000);
        rgb_led.rgb_set_color(rgb_ns::pink);
        vtimer_usleep(2000000);
    }
}

void button_handler(void)
{
//    button_instance btn;
//    gpio_config_params_t btn_params;
//    btn_params.device_port = port_A;
//    btn_params.device_pin = 1;
//    btn.device_configure(&btn_params);
    while (1) {

    }
}

void switch_handler(void)
{

}

void on_off_bulb_handler(void)
{

}

void dimmer_handler(void)
{
//    dimmer_instance dimmer1;
//    adc_config_params_t adc_params;
//    adc_params.device_port = port_C;
//    adc_params.device_pin = 0;
//    adc_params.adc_x = adc1;
//    adc_params.adc_channel = ADC_Channel_10;
//    dimmer1.device_configure(&adc_params);
    while (1) {

    }
}

void level_bulb_handler(void)
{

}

void servo_sg90_handler(void)
{
//    servo_sg90_instance sg90;
//    pwm_config_params_t servo_params;
//    servo_params.device_port = port_A;
//    servo_params.device_pin = 1;
//    servo_params.timer_x = gp_timer5;
//    servo_params.pwm_channel = 2;
//    sg90.device_configure(&servo_params);
    while (1) {
        //block in recv_msg_queue.
    }
}

void rgb_led_handler(void)
{
//    rgb_instance rgb_led;
//
//    pwm_config_params_t red_pwm_params;
//    pwm_config_params_t green_pwm_params;
//    pwm_config_params_t blue_pwm_params;

//    red_pwm_params.device_port = port_B;
//    red_pwm_params.device_pin = 7;
//    red_pwm_params.timer_x = gp_timer4;
//    red_pwm_params.pwm_channel = 2;
//
//    green_pwm_params.device_port = port_B;
//    green_pwm_params.device_pin = 8;
//    green_pwm_params.timer_x = gp_timer4;
//    green_pwm_params.pwm_channel = 3;
//
//    blue_pwm_params.device_port = port_B;
//    blue_pwm_params.device_pin = 9;
//    blue_pwm_params.timer_x = gp_timer4;
//    blue_pwm_params.pwm_channel = 4;
//    rgb_led.device_configure(&red_pwm_params, &green_pwm_params,
//            &blue_pwm_params);
    while (1) {
        //block in recv_msg queue.

    }
}
