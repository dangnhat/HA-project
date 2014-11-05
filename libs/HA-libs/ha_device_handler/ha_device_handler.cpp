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
#include <stdlib.h>
#include "ha_device_handler.h"
#include "ff.h"
#include "device_id.h"
#include "mesg_id.h"

static uint8_t get_configuration_common(uint32_t dev_id, uint8_t* buff,
        float* fl_buff, uint8_t offset, uint8_t size_buff, bool is_float);

void* end_point_handler(void* arg)
{
    uint8_t queue_size = 64;
    msg_t msg_q[queue_size];

    /* Initialize msg queue */
    msg_init_queue(msg_q, queue_size);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == ha_node_ns::NEW_DEVICE) {
            switch (msg.content.value & 0xFF) {
            case ha_ns::SWITCH:
                switch_handler(msg.content.value);
                break;
            case ha_ns::BUTTON:
                button_handler(msg.content.value);
                break;
            case ha_ns::DIMMER:
                dimmer_handler(msg.content.value);
                break;
            case ha_ns::SENSOR_GEN:
                sensor_linear_handler(msg.content.value);
                break;
            case ha_ns::ON_OFF_BULB:
                on_off_bulb_handler(msg.content.value);
                break;
            case ha_ns::LEVEL_BULB:
                level_bulb_handler(msg.content.value);
                break;
            case ha_ns::RGB_LED:
                rgb_led_handler(msg.content.value);
                break;
            case ha_ns::SERVO_SG90:
                servo_sg90_handler(msg.content.value);
                break;
            default:
                break;
            }
        }
    }

    return NULL;
}

void button_handler(uint32_t dev_id)
{
    uint8_t num_line = 2;
    uint8_t config_buff[num_line];
    if (get_configuration_common(dev_id, config_buff, NULL, 0, num_line, false)
            < num_line) {
        return;
    }

    gpio_config_params_t gpio_params;
    gpio_params.device_port = (port_t) config_buff[0];
    gpio_params.device_pin = config_buff[1];

    /* create and configure button instance */
    button_switch_instance btn(btn_sw_ns::btn);
    btn.device_configure(&gpio_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case btn_sw_ns::BTN_SW_MSG:
            break;
        case ha_node_ns::SEND_ALIVE:
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void switch_handler(uint32_t dev_id)
{
    uint8_t num_line = 2;
    uint8_t config_buff[num_line];
    if (get_configuration_common(dev_id, config_buff, NULL, 0, num_line, false)
            < num_line) {
        return;
    }

    gpio_config_params_t gpio_params;
    gpio_params.device_port = (port_t) config_buff[0];
    gpio_params.device_pin = config_buff[1];

    /* create and configure switch instance */
    button_switch_instance sw(btn_sw_ns::sw);
    sw.device_configure(&gpio_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case btn_sw_ns::BTN_SW_MSG:
            break;
        case ha_node_ns::SEND_ALIVE:
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void on_off_bulb_handler(uint32_t dev_id)
{
    uint8_t num_line = 2;
    uint8_t config_buff[num_line];
    if (get_configuration_common(dev_id, config_buff, NULL, 0, num_line, false)
            < num_line) {
        return;
    }

    gpio_config_params_t gpio_params;
    gpio_params.device_port = (port_t) config_buff[0];
    gpio_params.device_pin = config_buff[1];

    /* create and configure on-off bulb instance */
    on_off_bulb_instance on_off_bulb;
    on_off_bulb.device_configure(&gpio_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_node_ns::SEND_ALIVE:
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void dimmer_handler(uint32_t dev_id)
{
    uint8_t num_line = 2;
    uint8_t config_buff[num_line];
    if (get_configuration_common(dev_id, config_buff, NULL, 0, num_line, false)
            < num_line) {
        return;
    }

    adc_config_params_t adc_params;
    adc_params.device_port = (port_t) config_buff[0];
    adc_params.device_pin = config_buff[1];
    adc_params.adc_x = (adc_t) config_buff[2];
    adc_params.adc_channel = config_buff[3];

    /* create and configure dimmer instance */
    dimmer_instance dimmer;
    dimmer.device_configure(&adc_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_node_ns::SEND_ALIVE:
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void level_bulb_handler(uint32_t dev_id)
{
    uint8_t num_line = 2;
    uint8_t config_buff[num_line];
    if (get_configuration_common(dev_id, config_buff, NULL, 0, num_line, false)
            < num_line) {
        return;
    }

    pwm_config_params_t pwm_params;
    pwm_params.device_port = (port_t) config_buff[0];
    pwm_params.device_pin = config_buff[1];
    pwm_params.timer_x = (pwm_timer_t) config_buff[2];
    pwm_params.pwm_channel = config_buff[3];

    /* create and configure level bulb instance */
    level_bulb_instance level_bulb;
    level_bulb.device_configure(&pwm_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_node_ns::SEND_ALIVE:
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void servo_sg90_handler(uint32_t dev_id)
{
    uint8_t num_line = 2;
    uint8_t config_buff[num_line];
    if (get_configuration_common(dev_id, config_buff, NULL, 0, num_line, false)
            < num_line) {
        return;
    }

    pwm_config_params_t pwm_params;
    pwm_params.device_port = (port_t) config_buff[0];
    pwm_params.device_pin = config_buff[1];
    pwm_params.timer_x = (pwm_timer_t) config_buff[2];
    pwm_params.pwm_channel = config_buff[3];

    /* create and configure servo sg90 instance */
    servo_sg90_instance sg90;
    sg90.device_configure(&pwm_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_node_ns::SEND_ALIVE:
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void sensor_linear_handler(uint32_t dev_id)
{
    uint8_t num_line = 2;
    uint8_t config_buff[num_line];
    if (get_configuration_common(dev_id, config_buff, NULL, 0, num_line, false)
            < num_line) {
        return;
    }

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
        switch (msg.type) {
        case ha_node_ns::SEND_ALIVE:
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void rgb_led_handler(uint32_t dev_id)
{
    uint8_t num_line = 12;
    uint8_t config_buff[num_line];
    if (get_configuration_common(dev_id, config_buff, NULL, 0, num_line, false)
            < num_line) {
        return;
    }

    pwm_config_params_t red_pwm_params;
    red_pwm_params.device_port = (port_t) config_buff[0];
    red_pwm_params.device_pin = config_buff[1];
    red_pwm_params.timer_x = (pwm_timer_t) config_buff[2];
    red_pwm_params.pwm_channel = config_buff[3];

    pwm_config_params_t green_pwm_params;
    green_pwm_params.device_port = (port_t) config_buff[0];
    green_pwm_params.device_pin = config_buff[1];
    green_pwm_params.timer_x = (pwm_timer_t) config_buff[2];
    green_pwm_params.pwm_channel = config_buff[3];

    pwm_config_params_t blue_pwm_params;
    blue_pwm_params.device_port = (port_t) config_buff[0];
    blue_pwm_params.device_pin = config_buff[1];
    blue_pwm_params.timer_x = (pwm_timer_t) config_buff[2];
    blue_pwm_params.pwm_channel = config_buff[3];

    /* create and configure rgb-led instance */
    rgb_instance rgb_led;
    rgb_led.device_configure(&red_pwm_params, &green_pwm_params,
            &blue_pwm_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_node_ns::SEND_ALIVE:
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

int get_file_name_from_dev_id(uint32_t dev_id, char* file_name)
{
    uint8_t endpoint_id = (dev_id >> 8) & 0xFF;
    return snprintf(file_name, sizeof(file_name), "%x", endpoint_id);
}

uint8_t get_configuration_common(uint32_t dev_id, uint8_t* buff, float* fl_buff,
        uint8_t offset, uint8_t size_buff, bool is_float)
{
    char f_name[3];
    get_file_name_from_dev_id(dev_id, f_name);

    FIL fil;
    if (f_open(&fil, f_name, FA_READ)) {
        return 0;
    }

    if (offset > 0) {
        if (offset > fil.fsize) {
            return 0;
        }
        f_lseek(&fil, offset);
    }

    char line[16];
    uint8_t num_read_line;
    for (num_read_line = 0; num_read_line < size_buff; num_read_line++) {
        if (f_gets(line, sizeof(line), &fil)) {
            if (is_float) {
                fl_buff[num_read_line] = strtof(line, NULL);
            } else {
                buff[num_read_line] = strtol(line, NULL, 10);
            }
        } else {
            break;
        }
    }

    return num_read_line;
}
