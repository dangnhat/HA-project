/**
 * @file ha_device_handler.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 03-11-2014
 * @brief This is source file for device handlers in HA system.
 */
extern "C" {
#include "msg.h"
}
#include <stdio.h>
#include "ha_device_handler.h"

void* end_point_handler(void* arg)
{
    uint8_t queue_size = 64;
    msg_t msg_q[queue_size];

    /* Initialize msg queue */
    msg_init_queue(msg_q, queue_size);

//    msg_t msg;
    while (1) {
//        msg_receive(&msg);
//        if (msg.type == 1) {
//            switch(msg.content.value) {
//            case 0x01:
//                break;
//            case 0x02:
//                break;
//            case 0x03:
//                break;
//            case 0x04:
//                break;
//            default:
//                break;
//            }
//        }
//        button_handler();
//        switch_handler();
        dimmer_handler();
    }

    return NULL;
}

void button_handler(void)
{
    /* read configuration of button */

    /* create and configure button instance */
    button_switch_instance btn(btn_sw_ns::btn);
    gpio_config_params_t btn_params;
    btn_params.device_port = port_A;
    btn_params.device_pin = 1;
    btn.device_configure(&btn_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == btn_sw_ns::BTN_SW_MSG) {
            switch (msg.content.value) {
            case btn_sw_ns::btn_no_pressed:
                printf("no pressed\n");
                break;
            case btn_sw_ns::btn_pressed:
                printf("pressed\n");
                break;
            case btn_sw_ns::btn_on_hold:
                printf("on hold\n");
                break;
            default:
                break;
            }
        }
    }
}

void switch_handler(void)
{
    /* read configuration of switch */

    /* create and configure switch instance */
    button_switch_instance sw(btn_sw_ns::sw);
    gpio_config_params_t sw_params;
    sw_params.device_port = port_A;
    sw_params.device_pin = 1;
    sw.device_configure(&sw_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == btn_sw_ns::BTN_SW_MSG) {
            switch (msg.content.value) {
            case btn_sw_ns::sw_on:
                printf("sw on\n");
                break;
            case btn_sw_ns::sw_off:
                printf("sw off\n");
                break;
            default:
                break;
            }
        }
    }
}

void on_off_bulb_handler(void)
{
    /* read configuration of on-off bulb */

    /* create and configure on-off bulb instance */
    on_off_bulb_instance on_off_bulb;
    gpio_config_params_t gpio_params;
    gpio_params.device_port = port_B;
    gpio_params.device_pin = 7;
    on_off_bulb.device_configure(&gpio_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == btn_sw_ns::BTN_SW_MSG) {
            switch (msg.content.value) {
            case btn_sw_ns::sw_on:
                printf("sw on\n");
                break;
            case btn_sw_ns::sw_off:
                printf("sw off\n");
                break;
            default:
                break;
            }
        }
    }
}

void dimmer_handler(void)
{
    /* read configuration of dimmer from flash */

    /* create and configure dimmer instance */
    dimmer_instance dimmer;
    adc_config_params_t adc_params;
    adc_params.device_port = port_C;
    adc_params.device_pin = 0;
    adc_params.adc_x = adc1;
    adc_params.adc_channel = ADC_Channel_10;
    dimmer.device_configure(&adc_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == dimmer_ns::DIMMER_MSG) {
            printf("percent dimmer: %d\n", (uint8_t) msg.content.value);
        }
    }
}

void level_bulb_handler(void)
{
    /* read configuration of level bulb from flash */

    /* create and configure level bulb instance */
    level_bulb_instance level_bulb;
    pwm_config_params_t lv_bulb_params;
    lv_bulb_params.device_port = port_A;
    lv_bulb_params.device_pin = 1;
    lv_bulb_params.timer_x = gp_timer5;
    lv_bulb_params.pwm_channel = 2;
    level_bulb.device_configure(&lv_bulb_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
    }
}

void servo_sg90_handler(void)
{
    /* read configuration of servo sg90 from flash */

    /* create and configure servo sg90 instance */
    servo_sg90_instance sg90;
    pwm_config_params_t servo_params;
    servo_params.device_port = port_A;
    servo_params.device_pin = 1;
    servo_params.timer_x = gp_timer5;
    servo_params.pwm_channel = 2;
    sg90.device_configure(&servo_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
    }
}

void sensor_linear_handler(void)
{
    /* read configuration of linear sensor from flash */

    /* create and configure linear sensor instance */
    sensor_linear_instance sensor_linear;
    adc_config_params_t adc_params;
    adc_params.device_port = port_C;
    adc_params.device_pin = 0;
    adc_params.adc_x = adc1;
    adc_params.adc_channel = ADC_Channel_10;
    sensor_linear.device_configure(&adc_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
    }
}

void rgb_led_handler(void)
{
    /* read configuration of rgb-led from flash */

    /* create and configure rgb-led instance */
    rgb_instance rgb_led;

    pwm_config_params_t red_pwm_params;
    pwm_config_params_t green_pwm_params;
    pwm_config_params_t blue_pwm_params;

    red_pwm_params.device_port = port_B;
    red_pwm_params.device_pin = 7;
    red_pwm_params.timer_x = gp_timer4;
    red_pwm_params.pwm_channel = 2;

    green_pwm_params.device_port = port_B;
    green_pwm_params.device_pin = 8;
    green_pwm_params.timer_x = gp_timer4;
    green_pwm_params.pwm_channel = 3;

    blue_pwm_params.device_port = port_B;
    blue_pwm_params.device_pin = 9;
    blue_pwm_params.timer_x = gp_timer4;
    blue_pwm_params.pwm_channel = 4;
    rgb_led.device_configure(&red_pwm_params, &green_pwm_params,
            &blue_pwm_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);

    }
}
