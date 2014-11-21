/**
 * @file ha_device_handler.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 03-11-2014
 * @brief This is source file for device handlers in HA system.
 */
#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "msg.h"
}

#include "ha_device_handler.h"
#include "ha_device_status.h"
#include "ha_sixlowpan.h"
#include "ha_gff_misc.h"
#include "ff.h"
#include "device_id.h"
#include "gff_mesg_id.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

static const uint8_t queue_handler_size = 16;
static char config_str[ha_node_ns::dev_pattern_maxsize];

static bool read_config_file(uint32_t dev_id, char *config_str,
        uint8_t str_len);
static port_t get_port(char port_c);
static pwm_timer_t get_pwm_timer(int timer);
static bool check_dev_type_value(uint32_t msg_value, uint8_t dev_type);
static void send_data_over_air(uint16_t cmd, uint32_t dev_id, uint16_t value);
static uint8_t get_dev_type_common(uint32_t dev_id);

void* end_point_handler(void* arg)
{
    msg_t msg_q[queue_handler_size];

    /* Initialize msg queue */
    msg_init_queue(msg_q, queue_handler_size);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == ha_node_ns::NEW_DEVICE) {
            switch (get_dev_type_common(msg.content.value)) {
            case ha_ns::LIN_SENSOR:
                sensor_linear_handler(msg.content.value);
                break;
            case ha_ns::EVT_SENSOR:
                sensor_event_handler(msg.content.value);
                break;
            case ha_ns::ON_OFF_OPUT:
                on_off_output_handler(msg.content.value);
                break;
            default:
                break;
            }
            switch (parse_devtype_deviceid(msg.content.value)) {
            case ha_ns::NO_DEVICE:
                HA_NOTIFY("No device.\n");
                break;
            case ha_ns::SWITCH:
                switch_handler(msg.content.value);
                break;
            case ha_ns::BUTTON:
                button_handler(msg.content.value);
                break;
            case ha_ns::DIMMER:
                dimmer_handler(msg.content.value);
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

    send_data_over_air(ha_ns::SET_DEV_VAL, dev_id, ha_ns::btn_no_pressed);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case btn_sw_ns::BTN_SW_MSG:
            if (msg.content.value == btn_sw_ns::btn_no_pressed) {
                send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::btn_no_pressed);
            } else if (msg.content.value == btn_sw_ns::btn_pressed) {
                send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::btn_pressed);
            } else if (msg.content.value == btn_sw_ns::btn_on_hold) {
                send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::btn_on_hold);
            }
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
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

    send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
            sw.get_status() == btn_sw_ns::sw_on ?
                    ha_ns::switch_on : ha_ns::switch_off);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case btn_sw_ns::BTN_SW_MSG:
            if (msg.content.value == btn_sw_ns::sw_on) {
                send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::switch_on);
            } else {
                send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::switch_off);
            }
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_node_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void on_off_output_handler(uint32_t dev_id)
{
    /* get on-off bulb configuration */
    gpio_config_params_t gpio_params;
    if (!gpio_common_get_config(dev_id, &gpio_params)) {
        return;
    }

    /* create and configure on-off bulb instance */
    on_off_output_instance on_off_dev;
    on_off_dev.device_configure(&gpio_params);

    send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
            on_off_dev.dev_get_state() ? ha_ns::output_on : ha_ns::output_off);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
            if (get_dev_type_common(msg.content.value >> 16)
                    == (uint8_t) ha_ns::ON_OFF_OPUT) {
                printf("%d\n", (uint16_t) msg.content.value);
                if ((uint16_t) msg.content.value == ha_ns::output_on) {
                    on_off_dev.dev_turn_on();
                } else if ((uint16_t) msg.content.value == ha_ns::output_off) {
                    on_off_dev.dev_turn_off();
                }
            }
            send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                    on_off_dev.dev_get_state() ?
                            ha_ns::output_on : ha_ns::output_off);
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_node_ns::NEW_DEVICE:
            on_off_dev.dev_turn_off();
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
        case dimmer_ns::DIMMER_MSG:
            send_data_over_air(ha_ns::SET_DEV_VAL, dev_id, msg.content.value);
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
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

    send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
            (uint16_t) level_bulb.get_percent_intensity());

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
            if (check_dev_type_value(msg.content.value,
                    (uint8_t) ha_ns::LEVEL_BULB)) {
                level_bulb.set_percent_intensity((uint16_t) msg.content.value);
            }
            send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                    (uint16_t) level_bulb.get_percent_intensity());
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_node_ns::NEW_DEVICE:
            level_bulb.set_level_intensity(0);
            level_bulb.stop();
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

    send_data_over_air(ha_ns::SET_DEV_VAL, dev_id, (uint16_t) sg90.get_angle());

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
            if (check_dev_type_value(msg.content.value,
                    (uint8_t) ha_ns::SERVO_SG90)) {
                if ((uint16_t) (msg.content.value) <= 180) {
                    sg90.set_angle((uint8_t) (msg.content.value));
                }
            }
            send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                    (uint16_t) sg90.get_angle());
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_node_ns::NEW_DEVICE:
            sg90.stop();
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
    if (!sensor_linear_get_config(dev_id, &sensor_linear)) {
        return;
    }

    sensor_linear.start_sensor();

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case sensor_linear_ns::SEN_LINEAR_MSG:
            send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                    (uint16_t) msg.content.value);
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
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

    send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
            sensor_event.is_detected() ? ha_ns::detected : ha_ns::no_detected);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case sensor_event_ns::SEN_EVT_MSG:
            if (msg.content.value == sensor_event_ns::high_level) {
                send_data_over_air(ha_ns::SET_DEV_VAL, dev_id, ha_ns::detected);
            } else if (msg.content.value == sensor_event_ns::low_level) {
                send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::no_detected);
            }
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
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
    if (!rgb_get_config(dev_id, &rgb_led)) {
        return;
    }

    send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
            (uint16_t) rgb_led.get_current_color());

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
            if (check_dev_type_value(msg.content.value,
                    (uint8_t) ha_ns::RGB_LED)) {
                rgb_led.rgb_set_color(msg.content.value);
            }
            send_data_over_air(ha_ns::SET_DEV_VAL, dev_id,
                    (uint16_t) rgb_led.get_current_color());
            break;
        case ha_node_ns::SEND_ALIVE:
            send_data_over_air(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_node_ns::NEW_DEVICE:
            rgb_led.rgb_set_color(0x0000);
            rgb_led.stop();
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
//    char config_str[ha_node_ns::gpio_pattern_size];
    if (!read_config_file(dev_id, config_str, ha_node_ns::gpio_pattern_size)) {
        return false;
    }

    char port_c = '0';
    uint16_t pin = 0;

    sscanf(config_str, ha_node_ns::gpio_dev_config_pattern, &port_c, &pin);

    gpio_params->device_port = get_port(port_c);
    gpio_params->device_pin = pin;

    return true;
}

bool adc_common_get_config(uint32_t dev_id, adc_config_params_t *adc_params)
{
//    char config_str[ha_node_ns::adc_pwm_pattern_size];
    if (!read_config_file(dev_id, config_str,
            ha_node_ns::adc_pwm_pattern_size)) {
        return false;
    }

    char port_c = '0';
    uint16_t pin = 0;
    uint16_t adc = 0;
    uint16_t chann = 0;

    sscanf(config_str, ha_node_ns::adc_dev_config_pattern, &port_c, &pin, &adc,
            &chann);

    adc_params->device_port = get_port(port_c);
    adc_params->device_pin = pin;
    adc_params->adc_x = (adc_t) (adc - 1);
    adc_params->adc_channel = chann;

    return true;
}

bool pwm_common_get_config(uint32_t dev_id, pwm_config_params_t *pwm_params)
{
//    char config_str[ha_node_ns::adc_pwm_pattern_size];
    if (!read_config_file(dev_id, config_str,
            ha_node_ns::adc_pwm_pattern_size)) {
        return false;
    }

    char port_c = '0';
    uint16_t pin = 0;
    uint16_t timer = 0;
    uint16_t chann = 0;

    sscanf(config_str, ha_node_ns::pwm_dev_config_pattern, &port_c, &pin,
            &timer, &chann);

    pwm_params->device_port = get_port(port_c);
    pwm_params->device_pin = pin;
    pwm_params->timer_x = get_pwm_timer(timer);
    pwm_params->pwm_channel = chann;

    return true;
}

bool rgb_get_config(uint32_t dev_id, rgb_instance *rgb)
{
//    char config_str[ha_node_ns::dev_pattern_maxsize];
    if (!read_config_file(dev_id, config_str,
            ha_node_ns::dev_pattern_maxsize)) {
        return false;
    }

    char r_port_c = '0';
    uint16_t r_pin = 0;
    uint16_t r_timer = 0;
    uint16_t r_chnn = 0;

    char g_port_c = '0';
    uint16_t g_pin = 0;
    uint16_t g_timer = 0;
    uint16_t g_chnn = 0;

    char b_port_c = '0';
    uint16_t b_pin = 0;
    uint16_t b_timer = 0;
    uint16_t b_chnn = 0;

    uint16_t red_at_wp = 0;
    uint16_t green_at_wp = 0;
    uint16_t blue_at_wp = 0;

    sscanf(config_str, ha_node_ns::rgb_config_pattern, &r_port_c, &(r_pin),
            &r_timer, &(r_chnn), &g_port_c, &(g_pin), &g_timer, &(g_chnn),
            &b_port_c, &(b_pin), &b_timer, &(b_chnn), &red_at_wp, &green_at_wp,
            &blue_at_wp);

    pwm_config_params_t red_params;
    pwm_config_params_t green_params;
    pwm_config_params_t blue_params;

    red_params.device_port = get_port(r_port_c);
    red_params.device_pin = r_pin;
    red_params.timer_x = get_pwm_timer(r_timer);
    red_params.pwm_channel = r_chnn;

    green_params.device_port = get_port(g_port_c);
    green_params.device_pin = g_pin;
    green_params.timer_x = get_pwm_timer(g_timer);
    green_params.pwm_channel = g_chnn;

    blue_params.device_port = get_port(b_port_c);
    blue_params.device_pin = b_pin;
    blue_params.timer_x = get_pwm_timer(b_timer);
    blue_params.pwm_channel = b_chnn;

    rgb->set_white_point(red_at_wp, green_at_wp, blue_at_wp);
    rgb->device_configure(&red_params, &green_params, &blue_params);

    return true;
}

bool sensor_linear_get_config(uint32_t dev_id, sensor_linear_instance *senlnr)
{
//    char config_str[ha_node_ns::dev_pattern_maxsize];
    if (!read_config_file(dev_id, config_str,
            ha_node_ns::dev_pattern_maxsize)) {
        return false;
    }

    char port_c = '0';
    uint16_t pin = 0;
    uint16_t adc = 0;
    uint16_t chann = 0;

    char e1_type = '0';
    char a1[6];
    char b1[6];
    char c1[6];
    char e2_type = '0';
    char a2[6];
    char b2[6];
    char c2[6];

    int filter_thres = 0;
    int under_thres = 0;
    int over_thres = 0;

    sscanf(config_str, ha_node_ns::senlnr_config_pattern, &port_c, &pin, &adc,
            &chann, &e1_type, &e2_type, a1, b1, c1, a2, b2, c2, &filter_thres,
            &under_thres, &over_thres);

    uint8_t num_equation = 0;
    float a1_value, b1_value, c1_value;

    if (e1_type == 'l' || e1_type == 'r' || e1_type == 'p') {
        num_equation++;
        a1_value = strtof(a1, NULL);
        b1_value = strtof(b1, NULL);
        c1_value = strtof(c1, NULL);
    }
    float a2_value, b2_value, c2_value;
    if (e2_type == 'l' || e2_type == 'r' || e2_type == 'p') {
        num_equation++;
        a2_value = strtof(a2, NULL);
        b2_value = strtof(b2, NULL);
        c2_value = strtof(c2, NULL);
    }

    if (num_equation == 0) {
        return false;
    }

    adc_config_params_t adc_params;

    adc_params.device_port = get_port(port_c);
    adc_params.device_pin = pin;
    adc_params.adc_x = (adc_t) (adc - 1);
    adc_params.adc_channel = chann;

    if (e1_type == 'l') {
        senlnr->set_equation(sensor_linear_ns::linear, 1, a1_value,
                b1_value, c1_value);
    } else if (e1_type == 'r') {
        senlnr->set_equation(sensor_linear_ns::rational, 1, a1_value,
                b1_value, c1_value);
    } else if (e1_type == 'p') {
        senlnr->set_equation(sensor_linear_ns::polynomial, 1, a1_value,
                b1_value, c1_value);
    }

    if (e2_type == 'l') {
        senlnr->set_equation(sensor_linear_ns::linear, 2, a2_value,
                b2_value, c2_value);
    } else if (e2_type == 'r') {
        senlnr->set_equation(sensor_linear_ns::rational, 2, a2_value,
                b2_value, c2_value);
    } else if (e2_type == 'p') {
        senlnr->set_equation(sensor_linear_ns::polynomial, 2, a2_value,
                b2_value, c2_value);
    }

    senlnr->set_num_equation(num_equation);
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

static bool read_config_file(uint32_t dev_id, char *config_string,
        uint8_t str_len)
{
    char f_name[4];
    get_file_name_from_dev_id(dev_id, f_name);

    FIL fil;
    if (f_open(&fil, f_name, FA_READ)) {
        HA_DEBUG("Error on opening config file\n");
        return false;
    }
    f_sync(&fil);

    UINT byte_read;
    if (f_read(&fil, config_string, str_len, &byte_read)) {
        f_close(&fil);
        HA_DEBUG("Error on reading config file\n");
        return false;
    }
    f_close(&fil);

    return true;
}

static void send_data_over_air(uint16_t cmd, uint32_t dev_id, uint16_t value)
{
    uint8_t frame_buff_size = ha_ns::GFF_LEN_SIZE + ha_ns::GFF_CMD_SIZE;
    switch (cmd) {
    case ha_ns::SET_DEV_VAL:
        frame_buff_size += ha_ns::SET_DEV_VAL_DATA_LEN;
        break;
    case ha_ns::ALIVE:
        frame_buff_size += ha_ns::ALIVE_DATA_LEN;
        break;
    default:
        return;
    }

    uint8_t frame_buff[frame_buff_size];

    switch (cmd) {
    case ha_ns::SET_DEV_VAL:
        frame_buff[0] = ha_ns::SET_DEV_VAL_DATA_LEN;
        uint162buf(cmd, &frame_buff[ha_ns::GFF_CMD_POS]);
        uint322buf(dev_id, &frame_buff[ha_ns::GFF_DATA_POS]);
        uint162buf(value, &frame_buff[ha_ns::GFF_DATA_POS + 4]);
        break;
    case ha_ns::ALIVE:
        frame_buff[0] = ha_ns::ALIVE_DATA_LEN;
        uint162buf(cmd, &frame_buff[ha_ns::GFF_CMD_POS]);
        uint322buf(dev_id, &frame_buff[ha_ns::GFF_DATA_POS]);
        break;
    default:
        return;
    }

    ha_ns::sixlowpan_sender_gff_queue.add_data(frame_buff, frame_buff_size);

    msg_t gff_msg;
    gff_msg.type = ha_ns::GFF_PENDING;
    gff_msg.content.ptr = (char *) &ha_ns::sixlowpan_sender_gff_queue;
    msg_send(&gff_msg, ha_ns::sixlowpan_sender_pid, false);
}

static bool check_dev_type_value(uint32_t msg_value, uint8_t dev_type)
{
    uint8_t device = (msg_value >> 16) & 0xFF;
    if (device == dev_type) {
        return true;
    }
    return false;
}

static uint8_t get_dev_type_common(uint32_t dev_id)
{
    return ((uint8_t) dev_id) & 0xF8;
}
