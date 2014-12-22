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

/* common functions */
/**
 * @brief Read configuration from EP-file into a string.
 *
 * @param[in] dev_id Device ID used to open config file.
 * @param[out] config_str The pointer to string that contains device configuration.
 * @param[in] str_len Size of config_str.
 *
 * @return true if success, otherwise false.
 */
static bool read_config_file(uint32_t dev_id, char *config_str,
        uint8_t str_len);

/**
 * @brief return a port_t from port_c parsed from file.
 *
 * @param[in] port_c Parsed from file.
 *
 * @return A port has type port_t.
 */
static port_t get_port(char port_c);

/**
 * @brief return a pwm_timer_t from a number saved in file.
 *
 * @param[in] timer Parsed from file.
 *
 * @return A timer has type pwm_timer_t.
 */
static pwm_timer_t get_pwm_timer(int timer);

/**
 * @brief Get common device type from device ID.
 *
 * @param[in] dev_id Device ID.
 *
 * @return The defined device common subtype.
 */
static uint8_t get_dev_common_subtype(uint32_t dev_id);

/**
 * @brief Compare device type in message value with the given device type.
 *
 * @param[in] msg_value Containing device type.
 * @param[in] dev_type The given dev_type used to check dev_type in msg_value.
 *
 * @return true if dev_type in msg_value is match the given dev_type, otherwise false.
 */
static bool check_dev_type_value(uint32_t msg_value, uint8_t dev_type);

/**
 * @brief Forward data needed to send to 6loWPAN thread.
 *
 * @param[in] cmd       Command.
 * @param[in] dev_id    Device ID.
 * @param[in] value     Device value.
 */
static void forward_data_msg_to_6lowpan(uint16_t cmd, uint32_t dev_id,
        uint16_t value);

/* functions for ADC_sensors */
/**
 * @brief Read configuration for ADC sensor device.
 *
 * @param[in] dev_id Device ID.
 * @param[out] adc_ss The pointer to instance of an ADC-sensor device.
 * @param[out] num_equation Number of equation types.
 * @param[out] num_params Number of equation parameters.
 *
 * @return true if success, other false.
 */
static bool sensor_read_config(uint32_t dev_id, adc_sensor_instance* adc_ss,
        uint16_t *num_equation, uint16_t *num_params);

/**
 * @brief Read equation type and parameter from file.
 *
 * @param[in] dev_id Device ID.
 * @param[out] equa_type_buff The given buffer to save equation types.
 * @param[in] e_type_buff_len The size of equa_type_buff.
 * @param[out] equa_params_buff The given buffer to save equation parameters.
 * @param[in] e_params_buff_len The size of equa_params_buff.
 *
 * @return true if success, other false.
 */
static bool sensor_read_equa_type_and_params(uint32_t dev_id,
        char* equa_type_buff, uint8_t e_type_buff_len, float* equa_params_buff,
        uint8_t e_params_buff_len);

/*---------------------Implementation-----------------------*/

void* end_point_handler(void* arg)
{
    msg_t msg_q[queue_handler_size];

    /* Initialize msg queue */
    msg_init_queue(msg_q, queue_handler_size);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == ha_host_ns::NEW_DEVICE) {
            switch (get_dev_common_subtype(msg.content.value)) {
            case ha_ns::ADC_SENSOR:
                adc_sensor_handler(msg.content.value);
                break;
            case ha_ns::EVT_SENSOR:
                event_sensor_handler(msg.content.value);
                break;
            case ha_ns::ON_OFF_OPUT:
                on_off_output_handler(msg.content.value);
                break;
            default:
                break;
            }
            switch (parse_devtype_deviceid(msg.content.value)) {
            case ha_ns::NO_DEVICE:
                HA_NOTIFY("-EP%d: No device!!.\n",
                        (uint8_t )(msg.content.value >> 8));
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

    /* send first value to CC */
    forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
            ha_ns::btn_no_pressed);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case btn_sw_ns::BTN_SW_MSG:
            if (msg.content.value == btn_sw_ns::btn_no_pressed) {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::btn_no_pressed);
            } else if (msg.content.value == btn_sw_ns::btn_pressed) {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::btn_pressed);
            } else if (msg.content.value == btn_sw_ns::btn_on_hold) {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::btn_on_hold);
            }
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
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

    /* send first value to CC */
    forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
            sw.get_status() == btn_sw_ns::sw_on ?
                    ha_ns::switch_on : ha_ns::switch_off);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case btn_sw_ns::BTN_SW_MSG:
            if (msg.content.value == btn_sw_ns::sw_on) {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::switch_on);
            } else {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::switch_off);
            }
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
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

    /* send first value to CC */
    forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
            (on_off_dev.dev_get_state() == on_off_dev_ns::dev_on) ?
                    ha_ns::output_on : ha_ns::output_off);

    uint8_t old_set_dev_val = 0;
    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
            if (get_dev_common_subtype(msg.content.value >> 16)
                    == (uint8_t) ha_ns::ON_OFF_OPUT) {
                if ((uint16_t) msg.content.value == ha_ns::output_on) {
                    on_off_dev.dev_turn_on();
                } else if ((uint16_t) msg.content.value == ha_ns::output_off) {
                    on_off_dev.dev_turn_off();
                } else if ((uint16_t) msg.content.value == ha_ns::toggle) {
                    on_off_dev.dev_toggle();
                } else if ((uint16_t) msg.content.value > 100) { //blink
                    old_set_dev_val = (uint8_t) msg.content.value;
                    on_off_dev.dev_blink(old_set_dev_val - 100);
                }
            }
            /* feedback to CC */
            if (on_off_dev.dev_get_state() == on_off_dev_ns::dev_blink) {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        (uint16_t) old_set_dev_val);
            } else {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        (on_off_dev.dev_get_state() == on_off_dev_ns::dev_on) ?
                                ha_ns::output_on : ha_ns::output_off);
            }
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
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
            /* dimmer'll send first value to CC when it's started */
            forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                    (uint16_t) msg.content.value);
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
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

    /* send first value to CC */
    uint8_t old_set_dev_val = level_bulb.get_percent_intensity();
    forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
            (uint16_t) old_set_dev_val);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
            if (check_dev_type_value(msg.content.value,
                    (uint8_t) ha_ns::LEVEL_BULB)) {
                old_set_dev_val = (uint8_t) msg.content.value;

                if ((uint8_t) msg.content.value <= 100) { //set level intensity
                    level_bulb.set_percent_intensity(old_set_dev_val);
                } else { //blink
                    level_bulb.blink(old_set_dev_val - 100);
                }
            }
            /* feedback to CC */
            if (level_bulb.bulb_is_blink()) {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        (uint16_t) old_set_dev_val);
            } else {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        (uint16_t) level_bulb.get_percent_intensity());
            }
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
            level_bulb.set_level_intensity(0); //turn off before removing device
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

    /* send first value to CC */
    forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
            (uint16_t) sg90.get_angle());

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
            if (check_dev_type_value(msg.content.value,
                    (uint8_t) ha_ns::SERVO_SG90)) {
                sg90.set_angle((uint8_t) (msg.content.value));
            }
            /* feedback to CC */
            forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                    (uint16_t) sg90.get_angle());
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
            sg90.stop();
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void adc_sensor_handler(uint32_t dev_id)
{
    /* create and configure linear sensor instance */
    adc_sensor_instance adc_sensor;
    uint16_t num_equation = 0;
    uint16_t num_params = 0;
    if (!sensor_read_config(dev_id, &adc_sensor, &num_equation, &num_params)) {
        return;
    }

    char e_type[num_equation];
    float params[num_params];
    sensor_read_equa_type_and_params(dev_id, e_type, num_equation, params,
            num_params);

    adc_sensor.set_equation_type(e_type, num_equation);
    adc_sensor.set_equation_params(params, num_params);

    adc_sensor.start_sensor();

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case adc_sensor_ns::ADC_SENSOR_MSG:
            printf("ss: %d\n", (uint16_t) msg.content.value);
            forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                    (uint16_t) msg.content.value);
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
            msg_send_to_self(&msg);
            return;
        default:
            break;
        }
    }
}

void event_sensor_handler(uint32_t dev_id)
{
    /* get event sensor configuration */
    gpio_config_params_t gpio_params;
    if (!gpio_common_get_config(dev_id, &gpio_params)) {
        return;
    }

    /* create and configure on-off bulb instance */
    sensor_event_instance sensor_event;
    sensor_event.device_configure(&gpio_params);

    /* send first value to CC */
    forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
            sensor_event.is_detected() ? ha_ns::detected : ha_ns::no_detected);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case sensor_event_ns::SEN_EVT_MSG:
            if (msg.content.value == sensor_event_ns::high_level) {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::detected);
            } else if (msg.content.value == sensor_event_ns::low_level) {
                forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                        ha_ns::no_detected);
            }
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
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

    rgb_led.set_color_model(rgb_ns::model_16bits_555);

    /* send first value to CC */
    forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
            (uint16_t) rgb_led.get_current_color());

    uint8_t basic_color = (uint8_t) rgb_ns::white;
    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch (msg.type) {
        case ha_ns::SET_DEV_VAL:
            if (check_dev_type_value(msg.content.value,
                    (uint8_t) ha_ns::RGB_LED)) {
                if (((uint16_t) msg.content.value) >> 15 == 0) {
                    rgb_led.rgb_set_color((uint16_t) msg.content.value);
                } else {
                    basic_color = (basic_color + 1) % rgb_ns::max_basic_color;
                    rgb_led.rgb_set_color((rgb_ns::basic_color_t) basic_color);
                }
            }
            /* feedback to CC */
            forward_data_msg_to_6lowpan(ha_ns::SET_DEV_VAL, dev_id,
                    (uint16_t) rgb_led.get_current_color());
            break;
        case ha_host_ns::SEND_ALIVE:
            forward_data_msg_to_6lowpan(ha_ns::ALIVE, dev_id, 0);
            break;
        case ha_host_ns::NEW_DEVICE:
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
    char config_str[sizeof(ha_host_ns::gpio_dev_config_pattern)];
    if (!read_config_file(dev_id, config_str,
            sizeof(ha_host_ns::gpio_dev_config_pattern))) {
        return false;
    }

    char port_c = '0';
    uint16_t pin = 0;
    char mode_c = '0';

    sscanf(config_str, ha_host_ns::gpio_dev_config_pattern, &port_c, &pin,
            &mode_c);

    gpio_params->device_port = get_port(port_c);
    gpio_params->device_pin = pin;

    uint8_t mode = (uint8_t) gpio_ns::out_push_pull;
    if (mode_c == 'p') {
        mode = (uint8_t) gpio_ns::out_push_pull;
    } else if (mode_c == 'o') {
        mode = (uint8_t) gpio_ns::out_open_drain;
    }
    gpio_params->mode = mode;

    return true;
}

bool adc_common_get_config(uint32_t dev_id, adc_config_params_t *adc_params)
{
    char config_str[sizeof(ha_host_ns::adc_dev_config_pattern)];
    if (!read_config_file(dev_id, config_str,
            sizeof(ha_host_ns::adc_dev_config_pattern))) {
        return false;
    }

    char port_c = '0';
    uint16_t pin = 0;
    uint16_t adc = 0;
    uint16_t chann = 0;

    sscanf(config_str, ha_host_ns::adc_dev_config_pattern, &port_c, &pin, &adc,
            &chann);

    adc_params->device_port = get_port(port_c);
    adc_params->device_pin = pin;
    adc_params->adc_x = (adc_t) (adc - 1);
    adc_params->adc_channel = chann;

    return true;
}

bool pwm_common_get_config(uint32_t dev_id, pwm_config_params_t *pwm_params)
{
    char config_str[sizeof(ha_host_ns::pwm_dev_config_pattern)];
    if (!read_config_file(dev_id, config_str,
            sizeof(ha_host_ns::pwm_dev_config_pattern))) {
        return false;
    }

    char port_c = '0';
    uint16_t pin = 0;
    uint16_t timer = 0;
    uint16_t chann = 0;

    sscanf(config_str, ha_host_ns::pwm_dev_config_pattern, &port_c, &pin,
            &timer, &chann);

    pwm_params->device_port = get_port(port_c);
    pwm_params->device_pin = pin;
    pwm_params->timer_x = get_pwm_timer(timer);
    pwm_params->pwm_channel = chann;

    return true;
}

bool rgb_get_config(uint32_t dev_id, rgb_instance *rgb)
{
    char config_str[sizeof(ha_host_ns::rgb_config_pattern)];
    if (!read_config_file(dev_id, config_str,
            sizeof(ha_host_ns::rgb_config_pattern))) {
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

    sscanf(config_str, ha_host_ns::rgb_config_pattern, &r_port_c, &r_pin,
            &r_timer, &r_chnn, &g_port_c, &g_pin, &g_timer, &g_chnn, &b_port_c,
            &b_pin, &b_timer, &b_chnn, &red_at_wp, &green_at_wp, &blue_at_wp);

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

static void forward_data_msg_to_6lowpan(uint16_t cmd, uint32_t dev_id,
        uint16_t value)
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

static uint8_t get_dev_common_subtype(uint32_t dev_id)
{
    return ((uint8_t) dev_id) & 0xF8;
}

static bool sensor_read_config(uint32_t dev_id, adc_sensor_instance* adc_ss,
        uint16_t *num_equation, uint16_t *num_params)
{
    char config_str[sizeof(ha_host_ns::adc_sensor_config_pattern)];
    if (!read_config_file(dev_id, config_str,
            sizeof(ha_host_ns::adc_sensor_config_pattern))) {
        return false;
    }

    char port_c = '0';
    uint16_t pin = 0;
    uint16_t adc = 0;
    uint16_t chann = 0;

    int filter_thres = 0;
    int under_thres = 0;
    int over_thres = 0;

    sscanf(config_str, ha_host_ns::adc_sensor_config_pattern, &port_c, &pin,
            &adc, &chann, &filter_thres, &under_thres, &over_thres,
            num_equation, num_params);

    adc_config_params_t adc_params;

    adc_params.device_port = get_port(port_c);
    adc_params.device_pin = pin;
    adc_params.adc_x = (adc_t) (adc - 1);
    adc_params.adc_channel = chann;

    adc_ss->set_delta_threshold(filter_thres);
    adc_ss->set_underflow_threshold(under_thres);
    adc_ss->set_overflow_threshold(over_thres);
    adc_ss->device_configure(&adc_params);

    return true;
}

static bool sensor_read_equa_type_and_params(uint32_t dev_id,
        char* equa_type_buff, uint8_t e_type_buff_len, float* equa_params_buff,
        uint8_t e_params_buff_len)
{
    char f_name[4];
    get_file_name_from_dev_id(dev_id, f_name);

    FIL fil;
    if (f_open(&fil, f_name, FA_READ)) {
        HA_DEBUG("Error on opening config file\n");
        return false;
    }
    f_sync(&fil);

    uint8_t equa_line = 0;
    uint8_t index = 0;

    /* read equation type */
    char config_str[ha_host_ns::dev_pattern_maxsize];
    while (f_gets(config_str, ha_host_ns::dev_pattern_maxsize, &fil)) {
        if (equa_line >= 3) {
            equa_type_buff[index] = config_str[0];
            index++;
        }
        equa_line++;
        if (index == e_type_buff_len) {
            break;
        }
    }

    /* continue reading parameter of equations */
    for (index = 0; index < e_params_buff_len; index++) {
        if (f_gets(config_str, ha_host_ns::dev_pattern_maxsize, &fil)) {
            equa_params_buff[index] = strtof(config_str, NULL);
        } else {
            break;
        }
    }
    f_close(&fil);

    return true;
}
