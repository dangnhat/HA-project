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
#include "ha_node_glb.h"
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

const char linear_sensor_usage[] =
        "Usage:\n"
                "senlnr -e [EP id], set end point id.\n"
                "senlnr -e [EP id] -p [port], set port.\n"
                "senlnr -e [EP id] -n [pin], set pin.\n"
                "senlnr -e [EP id] -a [adc], set adc.\n"
                "senlnr -e [EP id] -c [channel], set channel.\n"
                "senlnr -e [EP id] -t [equa-type], choose equation type (linear or rational).\n"
                "senlnr -e [EP id] -k [a_factor], set A factor for equation.\n"
                "senlnr -e [EP id] -b [b_constant], set B constant for equation.\n"
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
static void gpio_common_config(int argc, char** argv, int *endpoint_id);

/**
 * @brief configure pure ADC devices (port/pin and adc/adc_channel).
 *
 * @details
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
static void adc_common_config(int argc, char** argv, int *endpoint_id);

/**
 * @brief configure pure PWM devices (port/pin and timer/pwm_channel).
 *
 * @details
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
static void pwm_common_config(int argc, char** argv, int *endpoint_id);

static bool check_port(char src, char* port);

static void modify_dev_list_file(uint8_t ep_id, uint8_t dev_type);

/* ------Implementation------ */

void button_config(int argc, char** argv)
{
    int ep_id = -1;

    gpio_common_config(argc, argv, &ep_id);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::BUTTON);

    return;
}

void switch_config(int argc, char** argv)
{
    int ep_id = -1;

    gpio_common_config(argc, argv, &ep_id);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::SWITCH);

    return;
}

void on_off_bulb_config(int argc, char** argv)
{
    int ep_id = -1;

    gpio_common_config(argc, argv, &ep_id);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::ON_OFF_BULB);

    return;
}

void sensor_event_config(int argc, char** argv)
{
    int ep_id = -1;

    gpio_common_config(argc, argv, &ep_id);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::EVT_SENSOR);

    return;
}

void dimmer_config(int argc, char** argv)
{
    int ep_id = -1;

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
    int ep_id = -1;

    pwm_common_config(argc, argv, &ep_id);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::LEVEL_BULB);

    return;
}

void servo_config(int argc, char** argv)
{
    int ep_id = -1;

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
    UINT byte_read;
    char f_name[8];

    int ep_id = -1;
    int dev_type = ha_ns::RGB_LED;

    char Rport = '0';
    int Rpin = 0;
    int Rtimer_x = 0;
    int Rchannel = 0;

    char Gport = '0';
    int Gpin = 0;
    int Gtimer_x = 0;
    int Gchannel = 0;

    char Bport = '0';
    int Bpin = 0;
    int Btimer_x = 0;
    int Bchannel = 0;

    int red_at_wp = 0;
    int green_at_wp = 0;
    int blue_at_wp = 0;

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
                f_res = f_open(&fil, argv[count], FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_res = f_read(&fil, config_str,
                        ha_node_ns::dev_pattern_maxsize, &byte_read);
                if (f_res != FR_OK) {
                    printf("Error on reading file, byte_read %u\n", byte_read);
                    print_ferr(f_res);
                    return;
                }
                sscanf(config_str, ha_node_ns::rgb_config_pattern,
                        &Rport, &Rpin, &Rtimer_x, &Rchannel, &Gport, &Gpin,
                        &Gtimer_x, &Gchannel, &Bport, &Bpin, &Btimer_x,
                        &Bchannel, &red_at_wp, &green_at_wp, &blue_at_wp);
                /* close opened file */
                f_close(&fil);
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
                if (Rtimer_x < 0 || Rtimer_x > 8 || Rtimer_x == 5
                        || Rtimer_x == 6) {
                    printf("ERR: invalid timer value\n");
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
                if (Gtimer_x < 0 || Gtimer_x > 8 || Gtimer_x == 5
                        || Gtimer_x == 6) {
                    printf("ERR: invalid timer value\n");
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
                if (Btimer_x < 0 || Btimer_x > 8 || Btimer_x == 5
                        || Btimer_x == 6) {
                    printf("ERR: invalid timer value\n");
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

    if(ep_id < 0) {
        printf("ERR: missing -e option.\n");
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

    int byte_written = f_printf(&fil, ha_node_ns::rgb_config_pattern,
            Rport, Rpin, Rtimer_x, Rchannel, Gport, Gpin, Gtimer_x, Gchannel,
            Bport, Bpin, Btimer_x, Bchannel, red_at_wp, green_at_wp,
            blue_at_wp);
    f_sync(&fil);

    if (byte_written < 0) {
        printf("Error on writing configuration to file\n");
        print_ferr(f_res);
        f_close(&fil);
        return;
    }

    f_close(&fil);

    /* read back */
    f_open(&fil, f_name, FA_READ);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s\n", config_str);
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
    UINT byte_read;
    char f_name[8];

    int ep_id = -1;
    int dev_type = ha_ns::LIN_SENSOR;

    char port = '0';
    int pin = 0;
    int adc_x = 0;
    int channel = 0;

    char e_type = '0';
    char* a_factor;
    char* b_constant;

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
                f_res = f_read(&fil, config_str,
                        ha_node_ns::dev_pattern_maxsize, &byte_read);
                if (f_res != FR_OK) {
                    printf("Error on reading file, byte_read %u\n", byte_read);
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                sscanf(config_str, ha_node_ns::senlnr_config_pattern,
                        &port, &pin, &adc_x, &channel, &e_type, a_factor,
                        b_constant, &filter_thres, &under_thres, &over_thres);
                /* close opened file */
                f_close(&fil);
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
            case 't':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                e_type = argv[count][0];
                if (e_type != 'l' || e_type != 'r') {
                    printf("ERR: invalid type, linear equation as default\n");
                    e_type = 'l';
                }
                break;
            case 'k':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                a_factor = argv[count];
                break;
            case 'b':
                count++;
                if (count > argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                b_constant = argv[count];
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

    if(ep_id < 0) {
        printf("ERR: missing -e option.\n");
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

    int byte_written = f_printf(&fil, ha_node_ns::senlnr_config_pattern,
            port, pin, adc_x, channel, e_type, a_factor, b_constant,
            filter_thres, under_thres, over_thres);
    f_sync(&fil);

    if (byte_written < 0) {
        printf("Error on writing configuration to file\n");
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_open(&fil, f_name, FA_READ);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s\n", config_str);
    }
    f_close(&fil);

    /* modify device list file */
    modify_dev_list_file(ep_id, dev_type);

    return;
}

static void gpio_common_config(int argc, char** argv, int *endpoint_id)
{
    FRESULT f_res;
    FIL fil;
    UINT byte_read;
    char f_name[8];

    *endpoint_id = -1;
    int ep_id = -1;
    char port = '0';
    int pin = 0;

    char config_str[ha_node_ns::gpio_pattern_size];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from file */
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
                ep_id = atoi(argv[count]);
                if (ep_id < 0 || ep_id > ha_node_ns::max_end_point) {
                    printf("ERR: invalid endpoint id value\n");
                    return;
                }
                /* get file name from ep id */
                snprintf(f_name, sizeof(f_name), "%x", ep_id);
                /* open "ep id" file */
                f_res = f_open(&fil, f_name, FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_res = f_read(&fil, config_str, ha_node_ns::gpio_pattern_size,
                        &byte_read);
                if (f_res != FR_OK) {
                    printf("Error on reading file, byte_read %u\n", byte_read);
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                sscanf(config_str, ha_node_ns::gpio_dev_config_pattern, &port,
                        &pin);
                /* close opened file */
                f_close(&fil);
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
            default:
                printf("Unknown option.\n");
                return;
            }
        } else {
            printf("Unknown option.\n");
            return;
        }
    }

    if(ep_id < 0) {
        printf("ERR: missing -e option.\n");
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

    int byte_written = f_printf(&fil, ha_node_ns::gpio_dev_config_pattern, port,
            pin);
    f_sync(&fil);

    if (byte_written < 0) {
        printf("Error on writing configuration to file\n");
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    *endpoint_id = ep_id;

    /* read back */
    f_open(&fil, f_name, FA_READ);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s\n", config_str);
    }
    f_close(&fil);

    return;
}

static void adc_common_config(int argc, char** argv, int *endpoint_id)
{
    FRESULT f_res;
    FIL fil;
    UINT byte_read;
    char f_name[16];

    *endpoint_id = -1;
    int ep_id = -1;
    char port = '0';
    int pin = 0;
    int adc_x = 0;
    int channel = 0;

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
                ep_id = atoi(argv[count]);
                if (ep_id < 0 || ep_id > ha_node_ns::max_end_point) {
                    printf("ERR: invalid endpoint id value\n");
                    return;
                }
                /* get file name from ep id */
                snprintf(f_name, sizeof(f_name), "%x", ep_id);
                /* open "ep id" file */
                f_res = f_open(&fil, f_name, FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_res = f_read(&fil, config_str,
                        ha_node_ns::adc_pwm_pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    printf("Error on reading file, byte_read %u\n", byte_read);
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                sscanf(config_str, ha_node_ns::adc_dev_config_pattern,
                        &port, &pin, &adc_x, &channel);
                /* close opened file */
                f_close(&fil);
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

    if(ep_id < 0) {
        printf("ERR: missing -e option.\n");
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

    int byte_written = f_printf(&fil, ha_node_ns::adc_dev_config_pattern,
            port, pin, adc_x, channel);
    f_sync(&fil);

    if (byte_written < 0) {
        printf("Error on writing configuration to file\n");
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    *endpoint_id = ep_id;

    /* read back */
    f_open(&fil, f_name, FA_READ);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s\n", config_str);
    }
    f_close(&fil);

    return;
}

static void pwm_common_config(int argc, char** argv, int *endpoint_id)
{
    FRESULT f_res;
    FIL fil;
    UINT byte_read;
    char f_name[16];

    *endpoint_id = -1;
    int ep_id = -1;
    char port = '0';
    int pin = 0;
    int timer_x = 0;
    int channel = 0;

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
                ep_id = atoi(argv[count]);
                if (ep_id < 0 || ep_id > ha_node_ns::max_end_point) {
                    printf("ERR: invalid endpoint id value\n");
                    return;
                }
                /* get file name from ep id */
                snprintf(f_name, sizeof(f_name), "%x", ep_id);
                /* open "ep id" file */
                f_res = f_open(&fil, f_name, FA_READ | FA_OPEN_ALWAYS);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_res = f_read(&fil, config_str,
                        ha_node_ns::adc_pwm_pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    printf("Error on reading file, byte_read %u\n", byte_read);
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                sscanf(config_str, ha_node_ns::adc_dev_config_pattern,
                        &port, &pin, &timer_x, &channel);
                /* close opened file */
                f_close(&fil);
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

    if(ep_id < 0) {
        printf("ERR: missing -e option.\n");
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

    int byte_written = f_printf(&fil, ha_node_ns::pwm_dev_config_pattern,
            port, pin, timer_x, channel);
    f_sync(&fil);

    if (byte_written < 0) {
        printf("Error on writing configuration to file\n");
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    *endpoint_id = ep_id;

    /* read back */
    f_open(&fil, f_name, FA_READ);
    while (f_gets(config_str, sizeof(config_str), &fil)) {
        printf("%s\n", config_str);
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
    int ep_arr[ha_node_ns::max_end_point];
    uint32_t dev_list[ha_node_ns::max_end_point];

    f_res = f_open(&fil, ha_node_ns::ha_dev_list_file,
    FA_READ | FA_OPEN_ALWAYS);
    if (f_res != FR_OK) {
        printf("Error on opening device list file\n");
        print_ferr(f_res);
        return;
    }

    char line[64];
    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        if (f_gets(line, sizeof(line), &fil)) {
            sscanf(line, ha_node_ns::dev_list_pattern, &ep_arr[i],
                    &dev_list[i]);
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
        printf("Error on opening device list file\n");
        print_ferr(f_res);
        return;
    }

    for (uint8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        snprintf(line, sizeof(line), ha_node_ns::dev_list_pattern, i,
                dev_list[i]);
        f_puts(line, &fil);
    }
    f_close(&fil);

    return;
}
