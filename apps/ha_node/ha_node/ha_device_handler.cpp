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
#include <stdarg.h>
#include "ha_device_handler.h"
#include "ff.h"
#include "device_id.h"
#include "gff_mesg_id.h"

static port_t get_port(char port_c);
static adc_t get_adc(int adc);
static pwm_timer_t get_pwm_timer(int timer);

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
            case ha_ns::LIN_SENSOR:
                sensor_linear_handler(msg.content.value);
                break;
            case ha_ns::EVT_SENSOR:
                sensor_event_handler(msg.content.value);
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
    /* get button configuration */
    gpio_config_params_t gpio_params;
    if (!gpio_common_get_config(dev_id, &gpio_params)) {
        return;
    }

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
    /* get switch configuration */
    gpio_config_params_t gpio_params;
    if (!gpio_common_get_config(dev_id, &gpio_params)) {
        return;
    }

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
    /* get on-off bulb configuration */
    gpio_config_params_t gpio_params;
    if (!gpio_common_get_config(dev_id, &gpio_params)) {
        return;
    }

    /* create and configure on-off bulb instance */
    on_off_bulb_instance on_off_bulb;
    on_off_bulb.device_configure(&gpio_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
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

void dimmer_handler(uint32_t dev_id)
{
    /* get dimmer configuration */
    adc_config_params_t adc_params;
    if (!adc_common_get_config(dev_id, &adc_params)) {
        return;
    }

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
    /* get level bulb configuration */
    pwm_config_params_t pwm_params;
    if (!pwm_common_get_config(dev_id, &pwm_params)) {
        return;
    }

    /* create and configure level bulb instance */
    level_bulb_instance level_bulb;
    level_bulb.device_configure(&pwm_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
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

void servo_sg90_handler(uint32_t dev_id)
{
    /* get servo sg90 configuration */
    pwm_config_params_t pwm_params;
    if (!pwm_common_get_config(dev_id, &pwm_params)) {
        return;
    }

    /* create and configure servo sg90 instance */
    servo_sg90_instance sg90;
    sg90.device_configure(&pwm_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
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

void sensor_linear_handler(uint32_t dev_id)
{
    /* create and configure linear sensor instance */
    sensor_linear_instance sensor_linear;
    if(!sensor_linear_get_config(dev_id, &sensor_linear)) {
        return;
    }

    sensor_linear.start_sensor();

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case sensor_linear_ns::SEN_LINEAR_MSG:
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

void sensor_event_handler(uint32_t dev_id)
{
    /* get event sensor configuration */
    gpio_config_params_t gpio_params;
    if (!gpio_common_get_config(dev_id, &gpio_params)) {
        return;
    }

    /* create and configure on-off bulb instance */
    sensor_event_instance sensor_event;
    sensor_event.device_configure(&gpio_params);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case sensor_event_ns::SEN_EVT_MSG:
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

void rgb_led_handler(uint32_t dev_id)
{
    /* create and configure rgb-led instance */
    rgb_instance rgb_led;
    if(!rgb_get_config(dev_id, &rgb_led)) {
        return;
    }

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
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

int get_file_name_from_dev_id(uint32_t dev_id, char* file_name)
{
    uint8_t endpoint_id = (dev_id >> 8) & 0xFF;
    return snprintf(file_name, sizeof(file_name), "%x", endpoint_id);
}

bool gpio_common_get_config(uint32_t dev_id, gpio_config_params_t *gpio_params)
{
    FIL fil;
    FRESULT f_res;
    UINT byte_read;
    char f_name[8];
    get_file_name_from_dev_id(dev_id, f_name);

    char port_c = '0';
    int pin = 0;

    if (f_open(&fil, f_name, FA_READ)) {
        return false;
    }

    char config_str[ha_node_ns::gpio_pattern_size];
    f_res = f_read(&fil, config_str, ha_node_ns::gpio_pattern_size, &byte_read);
    if (f_res != FR_OK) {
        f_close(&fil);
        return false;
    }

    sscanf(config_str, ha_node_ns::gpio_dev_config_pattern, &port_c, &pin);

    port_t port = get_port(port_c);

    gpio_params->device_port = port;
    gpio_params->device_pin = pin;

    return true;
}

bool adc_common_get_config(uint32_t dev_id, adc_config_params_t *adc_params)
{
    FIL fil;
    FRESULT f_res;
    UINT byte_read;
    char f_name[8];
    get_file_name_from_dev_id(dev_id, f_name);

    char port_c = '0';
    int pin = 0;
    int adc = 0;
    int channel = 0;

    if (f_open(&fil, f_name, FA_READ)) {
        return false;
    }

    char config_str[ha_node_ns::adc_pwm_pattern_size];
    f_res = f_read(&fil, config_str, ha_node_ns::adc_pwm_pattern_size,
            &byte_read);
    if (f_res != FR_OK) {
        f_close(&fil);
        return false;
    }

    sscanf(config_str, ha_node_ns::adc_dev_config_pattern, &port_c, &pin, &adc,
            &channel);

    port_t port = get_port(port_c);
    adc_t adc_x = get_adc(adc);

    adc_params->device_port = port;
    adc_params->device_pin = pin;
    adc_params->adc_x = adc_x;
    adc_params->adc_channel = channel;

    return true;
}

bool pwm_common_get_config(uint32_t dev_id, pwm_config_params_t *pwm_params)
{
    FIL fil;
    FRESULT f_res;
    UINT byte_read;
    char f_name[8];
    get_file_name_from_dev_id(dev_id, f_name);

    char port_c = '0';
    int pin = 0;
    int timer = 0;
    int channel = 0;

    if (f_open(&fil, f_name, FA_READ)) {
        return false;
    }

    char config_str[ha_node_ns::adc_pwm_pattern_size];
    f_res = f_read(&fil, config_str, ha_node_ns::adc_pwm_pattern_size,
            &byte_read);
    if (f_res != FR_OK) {
        f_close(&fil);
        return false;
    }

    sscanf(config_str, ha_node_ns::pwm_dev_config_pattern, &port_c, &pin,
            &timer, &channel);

    port_t port = get_port(port_c);
    pwm_timer_t timer_x = get_pwm_timer(timer);

    pwm_params->device_port = port;
    pwm_params->device_pin = pin;
    pwm_params->timer_x = timer_x;
    pwm_params->pwm_channel = channel;

    return true;
}

bool rgb_get_config(uint32_t dev_id, rgb_instance *rgb)
{
    FIL fil;
    FRESULT f_res;
    UINT byte_read;
    char f_name[8];
    get_file_name_from_dev_id(dev_id, f_name);

    char r_port_c = '0';
    int r_pin = 0;
    int r_timer = 0;
    int r_channel = 0;

    char g_port_c = '0';
    int g_pin = 0;
    int g_timer = 0;
    int g_channel = 0;

    char b_port_c = '0';
    int b_pin = 0;
    int b_timer = 0;
    int b_channel = 0;

    int red_at_wp = 0;
    int green_at_wp = 0;
    int blue_at_wp = 0;

    if (f_open(&fil, f_name, FA_READ)) {
        return false;
    }

    char config_str[ha_node_ns::dev_pattern_maxsize];
    f_res = f_read(&fil, config_str, ha_node_ns::dev_pattern_maxsize,
            &byte_read);
    if (f_res != FR_OK) {
        f_close(&fil);
        return false;
    }

    sscanf(config_str, ha_node_ns::rgb_config_pattern, &r_port_c, &r_pin,
            &r_timer, &r_channel, &g_port_c, &g_pin, &g_timer, &g_channel,
            &b_port_c, &b_pin, &b_timer, &b_channel, &red_at_wp, &green_at_wp,
            &blue_at_wp);

    port_t r_port = get_port(r_port_c);
    port_t g_port = get_port(g_port_c);
    port_t b_port = get_port(b_port_c);
    pwm_timer_t r_timer_x = get_pwm_timer(r_timer);
    pwm_timer_t g_timer_x = get_pwm_timer(g_timer);
    pwm_timer_t b_timer_x = get_pwm_timer(b_timer);

    pwm_config_params_t red_params;
    pwm_config_params_t green_params;
    pwm_config_params_t blue_params;

    red_params.device_port = r_port;
    red_params.device_pin = r_pin;
    red_params.timer_x = r_timer_x;
    red_params.pwm_channel = r_channel;

    green_params.device_port = g_port;
    green_params.device_pin = g_pin;
    green_params.timer_x = g_timer_x;
    green_params.pwm_channel = g_channel;

    blue_params.device_port = b_port;
    blue_params.device_pin = b_pin;
    blue_params.timer_x = b_timer_x;
    blue_params.pwm_channel = b_channel;

    rgb->set_white_point(red_at_wp, green_at_wp, blue_at_wp);
    rgb->device_configure(&red_params, &green_params, &blue_params);

    return true;
}

bool sensor_linear_get_config(uint32_t dev_id, sensor_linear_instance *senlnr)
{
    FIL fil;
    FRESULT f_res;
    UINT byte_read;
    char f_name[8];
    get_file_name_from_dev_id(dev_id, f_name);

    char port_c = '0';
    int pin = 0;
    int adc = 0;
    int channel = 0;

    char e_type = '0';
    char* a = NULL;
    char* b = NULL;

    int filter_thres = 0;
    int under_thres = 0;
    int over_thres = 0;

    if (f_open(&fil, f_name, FA_READ)) {
        return false;
    }

    char config_str[ha_node_ns::dev_pattern_maxsize];
    f_res = f_read(&fil, config_str, ha_node_ns::dev_pattern_maxsize,
            &byte_read);
    if (f_res != FR_OK) {
        f_close(&fil);
        return false;
    }

    sscanf(config_str, ha_node_ns::senlnr_config_pattern, &port_c, &pin, &adc,
            &channel, &e_type, a, b, &filter_thres, &under_thres, &over_thres);

    port_t port = get_port(port_c);
    adc_t adc_x = get_adc(adc);
    float a_factor = strtof(a, NULL);
    float b_constant = strtof(b, NULL);

    adc_config_params_t adc_params;
    adc_params.device_port = port;
    adc_params.device_pin = pin;
    adc_params.adc_x = adc_x;
    adc_params.adc_channel = channel;

    if (e_type == 'l') {
        senlnr->set_equation(sensor_linear_ns::linear, a_factor, b_constant);
    } else {
        senlnr->set_equation(sensor_linear_ns::rational, a_factor, b_constant);
    }

    senlnr->set_delta_threshold(filter_thres);
    senlnr->set_underflow_threshold(under_thres);
    senlnr->set_overflow_threshold(over_thres);
    senlnr->device_configure(&adc_params);

    return true;
}

static port_t get_port(char port_c)
{
    switch (port_c) {
    case 'A':
        return port_A;
    case 'B':
        return port_B;
    case 'C':
        return port_C;
    case 'D':
        return port_D;
    case 'E':
        return port_E;
    case 'F':
        return port_F;
    case 'G':
        return port_G;
    default:
        break;
    }
    return port_A;
}

static adc_t get_adc(int adc)
{
    switch (adc) {
    case 1:
        return adc1;
    case 2:
        return adc2;
    case 3:
        return adc3;
    default:
        break;
    }

    return adc1;
}

static pwm_timer_t get_pwm_timer(int timer)
{
    switch (timer) {
    case 1:
        return adv_timer1;
    case 8:
        return adv_timer8;
    case 2:
        return gp_timer2;
    case 3:
        return gp_timer3;
    case 4:
        return gp_timer4;
    case 5:
        return gp_timer5;
    default:
        break;
    }

    return adv_timer1;
}
