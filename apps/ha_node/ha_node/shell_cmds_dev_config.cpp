/**
 * @file shell_cmds_dev_config.cpp
 * @author  Nguyen Van Hien  <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 10-Nov-2014
 * @brief Implementation for device configuration related shell commands.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell_cmds_dev_config.h"
#include "shell_cmds_fatfs.h"
#include "ha_sixlowpan.h"
#include "ha_gff_misc.h"
#include "ff.h"
#include "device_id.h"

const char gpio_usage[] = "Usage:\n"
        "%s -e [EP id] -e [EP id], set end point id.\n"
        "-e [EP id] -p [port], set port.\n"
        "-e [EP id] -n [pin], set pin.\n"
        "-h, get this help.\n"
        "Note: multiple options can be combined together.\n";

const char adc_usage[] = "Usage:\n"
        "%s -e [EP id], set end point id.\n"
        "-e [EP id] -p [port], set port.\n"
        "-e [EP id] -n [pin], set pin.\n"
        "-e [EP id] -a [adc], set adc.\n"
        "-e [EP id] -c [channel], set adc channel.\n"
        "-h, get this help.\n"
        "Note: multiple options can be combined together.\n";

const char pwm_usage[] = "Usage:\n"
        "%s -e [EP id] -e [EP id], set end point id.\n"
        "-e [EP id] -p [port], set port.\n"
        "-e [EP id] -n [pin], set pin.\n"
        "-e [EP id] -t [timer], set timer.\n"
        "-e [EP id] -c [channel], set timer channel.\n"
        "-h, get this help.\n"
        "Note: multiple options can be combined together.\n";

const char rgb_usage[] =
        "Usage:\n"
                "rgb -e [EP id], set end point id.\n"
                "rgb -e [EP id] -R [port] [pin] [timer] [channel], configure Red led.\n"
                "rgb -e [EP id] -G [port] [pin] [timer] [channel], configure Green led.\n"
                "rgb -e [EP id] -B [port] [pin] [timer] [channel], configure for Blue led.\n"
                "rgb -e [EP id] -C [RedAtWp] [GreenAtWp] [BlueAtWp], set calibrating factor.\n"
                "rgb -h, get this help.\n"
                "Note: multiple options can be combined together.\n";

const char linear_sensor_usage[] = "Usage:\n"
        "senlnr -e [EP id], set end point id.\n"
        "senlnr -e [EP id] -s [sensor type], set sensor type.\n" //0=temp, 1=lumi
        "senlnr -e [EP id] -p [port], set port.\n"
        "senlnr -e [EP id] -n [pin], set pin.\n"
        "senlnr -e [EP id] -a [adc], set adc.\n"
        "senlnr -e [EP id] -c [channel], set channel.\n"
        "senlnr -e [EP id] -t [equa-type], choose equation type (linear or rational).\n"
        "senlnr -e [EP id] -k [a_factor], set A factor for equation (-t must be entered).\n"
        "senlnr -e [EP id] -b [b_constant], set B constant for equation (-t must be entered).\n"
        "senlnr -e [EP id] -f [filter thres], set filter threshold.\n"
        "senlnr -e [EP id] -u [under thres], set underflow threshold.\n"
        "senlnr -e [EP id] -o [over thres], set overflow threshold.\n"
        "senlnr -h, get this help.\n"
        "Note: multiple options can be combined together.\n";

/**
 * @brief configure pure GPIO devices (port/pin).
 *
 * @details
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
static void gpio_common_config(int argc, char** argv, int8_t *endpoint_id,
        int8_t *sub_type);

/**
 * @brief configure pure ADC devices (port/pin and adc/adc_channel).
 *
 * @details
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
static void adc_common_config(int argc, char** argv, int8_t *endpoint_id);

/**
 * @brief configure pure PWM devices (port/pin and timer/pwm_channel).
 *
 * @details
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
static void pwm_common_config(int argc, char** argv, int8_t *endpoint_id);

static bool check_port(char src, char* port);

static bool check_devid(uint32_t dev_id);

static void modify_dev_list_file(uint8_t ep_id, uint8_t dev_type);

/* ------Implementation------ */

void stop_endpoint_callback(int argc, char** argv)
{
    if (argc == 1) {
        printf("ERR: too few arguments.\n");
        return;
    }

    int ep_id = atoi(argv[1]);
    if (ep_id < 0 || ep_id > ha_node_ns::max_end_point) {
        printf("ERR: invalid endpoint id\n");
        return;
    }

    modify_dev_list_file(ep_id, ha_ns::NO_DEVICE);

    run_endpoint(ep_id);
}

void rst_endpoint_callback(int argc, char** argv)
{

    if (argc == 1) {
        printf("ERR: too few arguments.\n");
        return;
    }

    int ep_id = atoi(argv[1]);

    run_endpoint(ep_id);
}

void button_config(int argc, char** argv)
{
    int8_t ep_id = -1;
    int8_t sub_type = -1;

    gpio_common_config(argc, argv, &ep_id, &sub_type);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::BUTTON);

    return;
}

void switch_config(int argc, char** argv)
{
    int8_t ep_id = -1;
    int8_t sub_type = -1;

    gpio_common_config(argc, argv, &ep_id, &sub_type);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::SWITCH);

    return;
}

void on_off_output_config(int argc, char** argv)
{
    int8_t ep_id = -1;
    int8_t sub_type = -1;

    gpio_common_config(argc, argv, &ep_id, &sub_type);
    if (ep_id < 0 || sub_type < 0) {
        printf("ERR: invalid sub type\n");
        return;
    }

    modify_dev_list_file(ep_id,
            combine_dev_type(ha_ns::ON_OFF_OPUT, (uint8_t) sub_type));

    return;
}

void sensor_event_config(int argc, char** argv)
{
    int8_t ep_id = -1;
    int8_t sub_type = -1;

    gpio_common_config(argc, argv, &ep_id, &sub_type);
    if (ep_id < 0 || sub_type < 0) {
        printf("ERR: invalid sub type\n");
        return;
    }

    modify_dev_list_file(ep_id,
            combine_dev_type(ha_ns::EVT_SENSOR, (uint8_t) sub_type));

    return;
}

void dimmer_config(int argc, char** argv)
{
    int8_t ep_id = -1;

    /* write input to "endpoint" file */
    adc_common_config(argc, argv, &ep_id);
    if (ep_id < 0) {
        return;
    }

    /* rewrite device list file */
    modify_dev_list_file(ep_id, (uint8_t) ha_ns::DIMMER);

    return;
}

void level_bulb_config(int argc, char** argv)
{
    int8_t ep_id = -1;

    pwm_common_config(argc, argv, &ep_id);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::LEVEL_BULB);

    return;
}

void servo_config(int argc, char** argv)
{
    int8_t ep_id = -1;

    pwm_common_config(argc, argv, &ep_id);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::SERVO_SG90);

    return;
}

void rgb_led_config(int argc, char** argv)
{
    FRESULT f_res;
    FIL fil;
    UINT byte_read, byte_written;
    char f_name[4];

    int8_t ep_id = -1;
    uint8_t dev_type = (uint8_t) ha_ns::RGB_LED;

    char Rport = '0';
    uint16_t Rpin = 0;
    uint16_t Rtimer_x = 0;
    uint16_t Rchannel = 0;

    char Gport = '0';
    uint16_t Gpin = 0;
    uint16_t Gtimer_x = 0;
    uint16_t Gchannel = 0;

    char Bport = '0';
    uint16_t Bpin = 0;
    uint16_t Btimer_x = 0;
    uint16_t Bchannel = 0;

    uint16_t red_at_wp = 0;
    uint16_t green_at_wp = 0;
    uint16_t blue_at_wp = 0;

    char config_str[ha_node_ns::dev_pattern_maxsize];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from shell */
    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(rgb_usage);
                return;
            case 'e': //set endpoint id
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                /* save endpoint id */
                ep_id = atoi(argv[count]);
                if (ep_id < 0 || ep_id > ha_node_ns::max_end_point) {
                    printf("ERR: invalid endpoint id value\n");
                    return;
                }
                snprintf(f_name, sizeof(f_name), "%x", ep_id);
                f_res = f_open(&fil, f_name, FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_sync(&fil);
                f_res = f_read(&fil, config_str,
                        ha_node_ns::dev_pattern_maxsize, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_node_ns::rgb_config_pattern, &Rport,
                        &Rpin, &Rtimer_x, &Rchannel, &Gport, &Gpin, &Gtimer_x,
                        &Gchannel, &Bport, &Bpin, &Btimer_x, &Bchannel,
                        &red_at_wp, &green_at_wp, &blue_at_wp);
                break;
            case 'R': //configure Red channel
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &Rport)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Rpin = atoi(argv[count]);
                if (Rpin < 0 || Rpin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Rtimer_x = atoi(argv[count]);
                if (Rtimer_x < 0 || Rtimer_x > 8 || Rtimer_x == 6
                        || Rtimer_x == 7) {
                    printf("ERR: invalid timer value for pwm\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Rchannel = atoi(argv[count]);
                if (Rchannel < 0 || Rchannel > 15) {
                    printf("ERR: invalid timer value\n");
                    return;
                }
                break;
            case 'G': //configure Green channel
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &Gport)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Gpin = atoi(argv[count]);
                if (Rpin < 0 || Rpin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Gtimer_x = atoi(argv[count]);
                if (Gtimer_x < 0 || Gtimer_x > 8 || Gtimer_x == 6
                        || Gtimer_x == 7) {
                    printf("ERR: invalid timer value for pwm\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Gchannel = atoi(argv[count]);
                if (Gchannel < 0 || Gchannel > 15) {
                    printf("ERR: invalid timer value\n");
                    return;
                }
                break;
            case 'B': //configure Blue channel
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &Bport)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Bpin = atoi(argv[count]);
                if (Rpin < 0 || Rpin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Btimer_x = atoi(argv[count]);
                if (Btimer_x < 0 || Btimer_x > 8 || Btimer_x == 6
                        || Btimer_x == 7) {
                    printf("ERR: invalid timer value for pwm\n");
                    return;
                }
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Bchannel = atoi(argv[count]);
                if (Bchannel < 0 || Bchannel > 15) {
                    printf("ERR: invalid timer value\n");
                    return;
                }
                break;
            case 'C': //set calibrating factor
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                red_at_wp = atoi(argv[count]);
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                green_at_wp = atoi(argv[count]);
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                blue_at_wp = atoi(argv[count]);
                if (red_at_wp > 100 || green_at_wp > 100 || blue_at_wp > 100) {
                    printf(
                            "ERR: invalid calibrating factor. Try -h to get help.\n");
                    return;
                }
                break;
            default:
                printf("Unknown option.\n");
                return;
            }
        } else {
            printf("Unknown option.\n");
            return;
        }
    }

    if (ep_id < 0) {
        printf("ERR: missing -e option.\n");
        return;
    }

    /* Write configurations to file */
    f_res = f_open(&fil, f_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);

    snprintf(config_str, ha_node_ns::dev_pattern_maxsize,
            ha_node_ns::rgb_config_pattern, Rport, Rpin, Rtimer_x, Rchannel,
            Gport, Gpin, Gtimer_x, Gchannel, Bport, Bpin, Btimer_x, Bchannel,
            red_at_wp, green_at_wp, blue_at_wp);

    f_res = f_write(&fil, config_str, ha_node_ns::dev_pattern_maxsize,
            &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_open(&fil, f_name, FA_READ);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s", config_str);
    }
    f_close(&fil);

    /* modify device list file */
    modify_dev_list_file(ep_id, dev_type);

    return;
}

void adc_sensor_config(int argc, char** argv)
{
    FRESULT f_res;
    FIL fil;
    UINT byte_read, byte_written;
    char f_name[4];
    uint8_t num_equation = 0;

    int8_t ep_id = -1;
    int8_t sub_type = -1;

    char port = '0';
    uint16_t pin = 0;
    uint16_t adc_x = 0;
    uint16_t channel = 0;

    char e1_type = '0';
    char e2_type = '0';
    char a1_value[6] = {'0', '\0', '\0', '\0', '\0', '\0'};
    char b1_value[6];
    char c1_value[6];
    char a2_value[6];
    char b2_value[6];
    char c2_value[6];

    memcpy(b1_value, a1_value, sizeof(b1_value));
    memcpy(c1_value, a1_value, sizeof(c1_value));
    memcpy(a2_value, a1_value, sizeof(a2_value));
    memcpy(b2_value, a1_value, sizeof(b2_value));
    memcpy(c2_value, a1_value, sizeof(c2_value));

    int filter_thres = 0;
    int under_thres = 0;
    int over_thres = 0;

    char config_str[ha_node_ns::dev_pattern_maxsize];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from shell */
    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(rgb_usage);
                return;
            case 's':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                sub_type = atoi(argv[count]);
                break;
            case 'e': //set endpoint id
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                /* save endpoint id */
                ep_id = atoi(argv[count]);
                if (ep_id < 0 || ep_id > ha_node_ns::max_end_point) {
                    printf("ERR: invalid endpoint id value\n");
                    return;
                }
                snprintf(f_name, sizeof(f_name), "%x", ep_id);
                f_res = f_open(&fil, f_name, FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_sync(&fil);
                f_res = f_read(&fil, config_str,
                        ha_node_ns::dev_pattern_maxsize, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_node_ns::senlnr_config_pattern, &port,
                        &pin, &adc_x, &channel, &e1_type, &e2_type, a1_value,
                        b1_value, c1_value, a2_value, b2_value, c2_value,
                        &filter_thres, &under_thres, &over_thres);
                break;
            case 'p': //set port
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &port)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                break;
            case 'n': //set pin
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                pin = atoi(argv[count]);
                if (pin < 0 || pin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                break;
            case 'a': //set adc
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                adc_x = atoi(argv[count]);
                if (adc_x < 0 || adc_x > 3) {
                    printf("ERR: invalid adc value\n");
                    return;
                }
                break;
            case 'c': //set channel
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                channel = atoi(argv[count]);
                if (channel < 0 || channel > 15) {
                    printf("ERR: invalid channel value\n");
                    return;
                }
                break;
            case 't': //get equation type
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                e1_type = argv[count][0];
                if (e1_type != 'l' && e1_type != 'r' && e1_type != 'p') {
                    printf("ERR: invalid equation type\n");
                    return;
                }
                num_equation++; //number of equation;
                count++;
                if (count > argc) {
                    break;
                }
                if (argv[count][0] == '-') {
                    count--;
                    break;
                }
                e2_type = argv[count][0];
                if (e2_type != 'l' && e2_type != 'r' && e2_type != 'p') {
                    printf("ERR: invalid equation type\n");
                    return;
                }
                num_equation++;
                break;
            case 'A':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                memcpy(a1_value, argv[count], sizeof(a1_value));
                if (num_equation == 2) {
                    count++;
                    if (count > argc) {
                        printf("ERR: too few argument. Try -h to get help.\n");
                        return;
                    }
                    memcpy(a2_value, argv[count], sizeof(a2_value));
                }
                break;
            case 'B':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                memcpy(b1_value, argv[count], sizeof(b1_value));
                if (num_equation == 2) {
                    count++;
                    if (count > argc) {
                        printf("ERR: too few argument. Try -h to get help.\n");
                        return;
                    }
                    memcpy(b2_value, argv[count], sizeof(b2_value));
                }
                break;
            case 'C':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                memcpy(c1_value, argv[count], sizeof(c1_value));
                if (num_equation == 2) {
                    count++;
                    if (count > argc) {
                        printf("ERR: too few argument. Try -h to get help.\n");
                        return;
                    }
                    memcpy(c2_value, argv[count], sizeof(c2_value));
                }
                break;
            case 'f':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                filter_thres = atoi(argv[count]);
                break;
            case 'u':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                under_thres = atoi(argv[count]);
                break;
            case 'o':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                over_thres = atoi(argv[count]);
                break;
            default:
                printf("Unknown option.\n");
                return;
            }
        } else {
            printf("Unknown option.\n");
            return;
        }
    }

    if (ep_id < 0) {
        printf("ERR: missing -e option.\n");
        return;
    }

    if (sub_type < 0) {
        printf("ERR: missing -s option.\n");
        return;
    }

    /* Write configurations to file */
    f_res = f_open(&fil, f_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        printf("Error on opening file to write configuration\n");
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);

    snprintf(config_str, ha_node_ns::dev_pattern_maxsize,
            ha_node_ns::senlnr_config_pattern, port, pin, adc_x, channel,
            e1_type, e2_type, a1_value, b1_value, c1_value, a2_value,
            b2_value, c2_value, filter_thres, under_thres, over_thres);

    f_res = f_write(&fil, config_str, ha_node_ns::dev_pattern_maxsize,
            &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_open(&fil, f_name, FA_READ);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s", config_str);
    }
    f_close(&fil);

    /* modify device list file */
    modify_dev_list_file(ep_id,
            combine_dev_type(ha_ns::LIN_SENSOR, (uint8_t) sub_type));

    return;
}

static void gpio_common_config(int argc, char** argv, int8_t *endpoint_id,
        int8_t *sub_type)
{
    FRESULT f_res;
    FIL fil;
    UINT byte_read;
    char f_name[4];
    UINT byte_written = 0;
    uint8_t count = 0;

    *endpoint_id = -1;
    char port = '0';
    uint16_t pin = 0;
    char mode = '0';

    char config_str[ha_node_ns::gpio_pattern_size];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from file */
    for (count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(gpio_usage, argv[0]);
                return;
            case 's':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                *sub_type = atoi(argv[count]);
                break;
            case 'e': //set endpoint id
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                /* save endpoint id */
                *endpoint_id = atoi(argv[count]);
                if (*endpoint_id < 0
                        || *endpoint_id > ha_node_ns::max_end_point) {
                    printf("ERR: invalid endpoint id value\n");
                    return;
                }
                /* get file name from ep id */
                snprintf(f_name, sizeof(f_name), "%x", *endpoint_id);
                /* open "ep id" file */
                f_res = f_open(&fil, f_name, FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_sync(&fil);
                f_res = f_read(&fil, config_str, ha_node_ns::gpio_pattern_size,
                        &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_node_ns::gpio_dev_config_pattern, &port,
                        &pin, &mode);
                break;
            case 'p':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &port)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                break;
            case 'n':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                pin = atoi(argv[count]);
                if (pin < 0 || pin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                break;
            case 'm':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                mode = argv[count][0];
                break;
            default:
                printf("Unknown option.\n");
                return;
            }
        } else {
            printf("Unknown option.\n");
            return;
        }
    }

    if (*endpoint_id < 0) {
        printf("ERR: missing -e option.\n");
        return;
    }

    /* Write configurations to file */
    f_res = f_open(&fil, f_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);

    snprintf(config_str, ha_node_ns::gpio_pattern_size,
            ha_node_ns::gpio_dev_config_pattern, port, pin, mode);

    f_res = f_write(&fil, config_str, ha_node_ns::gpio_pattern_size,
            &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_open(&fil, f_name, FA_READ);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s", config_str);
    }
    f_close(&fil);

    return;
}

static void adc_common_config(int argc, char** argv, int8_t *endpoint_id)
{
    FRESULT f_res;
    FIL fil;
    UINT byte_read, byte_written;
    char f_name[4];

    *endpoint_id = -1;
    char port = '0';
    uint16_t pin = 0;
    uint16_t adc_x = 0;
    uint16_t channel = 0;

    char config_str[ha_node_ns::adc_pwm_pattern_size];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from shell */
    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(gpio_usage, argv[0]);
                return;
            case 'e': //set endpoint id
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                /* save endpoint id */
                *endpoint_id = atoi(argv[count]);
                if (*endpoint_id < 0
                        || *endpoint_id > ha_node_ns::max_end_point) {
                    printf("ERR: invalid endpoint id value\n");
                    return;
                }
                /* get file name from ep id */
                snprintf(f_name, sizeof(f_name), "%x", *endpoint_id);
                /* open "ep id" file */
                f_res = f_open(&fil, f_name, FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_sync(&fil);
                f_res = f_read(&fil, config_str,
                        ha_node_ns::adc_pwm_pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_node_ns::adc_dev_config_pattern, &port,
                        &pin, &adc_x, &channel);
                break;
            case 'p':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &port)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                break;
            case 'n':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                pin = atoi(argv[count]);
                if (pin < 0 || pin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                break;
            case 'a':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                adc_x = atoi(argv[count]);
                if (adc_x < 0 || adc_x > 3) {
                    printf("ERR: invalid adc value\n");
                    return;
                }
                break;
            case 'c':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                channel = atoi(argv[count]);
                if (channel < 0 || channel > 15) {
                    printf("ERR: invalid channel value\n");
                    return;
                }
                break;
            default:
                printf("Unknown option.\n");
                return;
            }
        } else {
            printf("Unknown option.\n");
            return;
        }
    }

    if (*endpoint_id < 0) {
        printf("ERR: missing -e option.\n");
        return;
    }

    /* Write configurations to file */
    f_res = f_open(&fil, f_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);

    snprintf(config_str, ha_node_ns::adc_pwm_pattern_size,
            ha_node_ns::adc_dev_config_pattern, port, pin, adc_x, channel);

    f_res = f_write(&fil, config_str, ha_node_ns::adc_pwm_pattern_size,
            &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_open(&fil, f_name, FA_READ);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s", config_str);
    }
    f_close(&fil);

    return;
}

static void pwm_common_config(int argc, char** argv, int8_t *endpoint_id)
{
    FRESULT f_res;
    FIL fil;
    UINT byte_read, byte_written;
    char f_name[4];

    *endpoint_id = -1;
    char port = '0';
    uint16_t pin = 0;
    uint16_t timer_x = 0;
    uint16_t channel = 0;

    char config_str[ha_node_ns::adc_pwm_pattern_size];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from shell */
    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(gpio_usage, argv[0]);
                break;
            case 'e': //set endpoint id
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                *endpoint_id = atoi(argv[count]);
                if (*endpoint_id < 0
                        || *endpoint_id > ha_node_ns::max_end_point) {
                    printf("ERR: invalid endpoint id value\n");
                    return;
                }
                /* get file name from ep id */
                snprintf(f_name, sizeof(f_name), "%x", *endpoint_id);
                /* open "ep id" file */
                f_res = f_open(&fil, f_name, FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_sync(&fil);
                f_res = f_read(&fil, config_str,
                        ha_node_ns::adc_pwm_pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_node_ns::adc_dev_config_pattern, &port,
                        &pin, &timer_x, &channel);
                break;
            case 'p': //set port
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &port)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                break;
            case 'n': //set pin
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                pin = atoi(argv[count]);
                if (pin < 0 || pin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                break;
            case 't': //set timer
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                timer_x = atoi(argv[count]);
                if (timer_x < 0 || timer_x > 8 || timer_x == 6
                        || timer_x == 7) {
                    printf("ERR: invalid timer value for pwm\n");
                    return;
                }
                break;
            case 'c': //set channel
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                channel = atoi(argv[count]);
                break;
            default:
                printf("Unknown option.\n");
                return;
            }
        } else {
            printf("Unknown option.\n");
            return;
        }
    }

    if (*endpoint_id < 0) {
        printf("ERR: missing -e option.\n");
        return;
    }

    /* Write configurations to file */
    f_res = f_open(&fil, f_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);

    snprintf(config_str, ha_node_ns::adc_pwm_pattern_size,
            ha_node_ns::pwm_dev_config_pattern, port, pin, timer_x, channel);

    f_res = f_write(&fil, config_str, ha_node_ns::adc_pwm_pattern_size,
            &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_open(&fil, f_name, FA_READ);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s", config_str);
    }
    f_close(&fil);

    return;
}

static bool check_port(char src, char* port)
{
    char port_t;

    switch (src) {
    case 'a':
    case 'A':
        port_t = 'A';
        break;
    case 'b':
    case 'B':
        port_t = 'B';
        break;
    case 'c':
    case 'C':
        port_t = 'C';
        break;
    case 'd':
    case 'D':
        port_t = 'D';
        break;
    case 'e':
    case 'E':
        port_t = 'E';
        break;
    case 'f':
    case 'F':
        port_t = 'F';
        break;
    case 'g':
    case 'G':
        port_t = 'G';
        break;
    default:
        return false;
    }

    *port = port_t;

    return true;
}

static void modify_dev_list_file(uint8_t ep_id, uint8_t dev_type)
{
    FIL fil;
    FRESULT f_res;
    uint32_t dev_list[ha_node_ns::max_end_point];

    memset(dev_list, 0, sizeof(dev_list));

    f_res = f_open(&fil, ha_node_ns::ha_dev_list_file,
    FA_READ | FA_OPEN_ALWAYS);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }

    char line[64];
    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        if (f_gets(line, sizeof(line), &fil)) {
            sscanf(line, ha_node_ns::dev_list_pattern, &dev_list[i]);
        } else {
            break;
        }
    }
    f_close(&fil);

    uint32_t dev_id = ((uint32_t) ha_ns::sixlowpan_node_id << 16)
            | ((uint32_t) ep_id << 8) | (uint32_t) dev_type;

    dev_list[ep_id] = dev_id;

    f_res = f_open(&fil, ha_node_ns::ha_dev_list_file,
    FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);

    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        snprintf(line, sizeof(line), ha_node_ns::dev_list_pattern, dev_list[i]);
        f_puts(line, &fil);
    }
    f_close(&fil);

    return;
}

static bool check_devid(uint32_t dev_id)
{
    uint16_t node_id = dev_id >> 16;
    uint8_t ep_id = (dev_id >> 8) & 0xFF;

    if (node_id != ha_ns::sixlowpan_node_id) {
        return false;
    }
    if (ep_id > ha_node_ns::max_end_point) {
        return false;
    }

    return true;
}

void run_endpoint(int8_t ep_id)
{
    FIL fil;
    uint32_t dev_list[ha_node_ns::max_end_point];

    memset(dev_list, 0, sizeof(dev_list));

    if (ep_id < 0 || ep_id > ha_node_ns::max_end_point) {
        printf("ERR: invalid input endpoint id.\n");
        return;
    }

    if (f_open(&fil, ha_node_ns::ha_dev_list_file, FA_READ | FA_OPEN_ALWAYS)) {
        printf("Error on opening device list file.\n");
        return;
    }

    char line[24];
    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        if (f_gets(line, sizeof(line), &fil)) {
            sscanf(line, ha_node_ns::dev_list_pattern, &dev_list[i]);
        } else {
            break;
        }
    }
    f_close(&fil);

    if (!check_devid(dev_list[ep_id])) {
        printf("-EP%d: No device!!\n", ep_id);
        return;
    } else {
        printf("-EP%d: Device id is valid.\n", ep_id);
    }

    msg_t msg;
    msg.type = ha_node_ns::NEW_DEVICE;
    msg.content.value = dev_list[ep_id];
    msg_send(&msg, ha_node_ns::end_point_pid[ep_id], false);
}
