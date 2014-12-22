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
#include <ctype.h>

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
        "-e [EP_id] -s [specified subtype], set the specified device subtype.\n"
        "-e [EP_id] -m [mode], set pin mode for output device.\n"
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
                "rgb -e [EP id] -B [port] [pin] [timer] [channel], configure Blue led.\n"
                "rgb -e [EP id] -C [RedAtWp] [GreenAtWp] [BlueAtWp], set calibrating factor.\n"
                "rgb -h, get this help.\n"
                "Note: multiple options can be combined together.\n";

const char adc_sensor_usage[] = "Usage:\n"
        "senadc -e [EP id], set end point id.\n"
        "senadc -e [EP id] -s [specified sensor subtype], set specified sensor subtype.\n" //0=temp, 1=lumi, 2=gas
        "senadc -e [EP id] -p [port], set port.\n"
        "senadc -e [EP id] -n [pin], set pin.\n"
        "senadc -e [EP id] -a [adc], set ADC.\n"//1=ADC1, 2=ADC2 or 3=ADC3
        "senadc -e [EP id] -c [channel], set ADC channel.\n"
        "senadc -e [EP id] -t [equation-type], choose equation type (linear, rational, polynomial or table).\n"
        "senadc -e [EP id] -P [parameters], set parameters for equation (-t option must be entered).\n"
        "senadc -e [EP id] -f [filter threshold], set filter threshold.\n"
        "senadc -e [EP id] -u [underflow threshold], set underflow threshold.\n"
        "senadc -e [EP id] -o [overflow threshold], set overflow threshold.\n"
        "senadc -h, get this help.\n"
        "Note: multiple options can be combined together.\n";

/**
 * @brief configure pure GPIO devices (port/pin).
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
static void gpio_common_config(int argc, char** argv, int8_t *endpoint_id,
        int8_t *specified_subtype);

/**
 * @brief configure pure ADC devices (port/pin and adc/adc_channel).
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
static void adc_common_config(int argc, char** argv, int8_t *endpoint_id);

/**
 * @brief configure pure PWM devices (port/pin and timer/pwm_channel).
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Arguments.
 */
static void pwm_common_config(int argc, char** argv, int8_t *endpoint_id);

/**
 * @brief Check entered port whether it is correct and convert it into an uppercase character.
 *
 * @param[in] src Port in character entered by user.
 * @param[out] port Port converted.
 *
 * @return true if the entered port is correct, otherwise false.
 */
static bool check_port(char src, char* port);

/**
 * @brief Check device ID whether it's correct (only check node ID and EP ID).
 *
 * @param[in] dev_id Device ID needed to check.
 *
 * @return true if dev_id is correct, otherwise false.
 */
static bool check_devid(uint32_t dev_id);

/**
 * @brief Modify dev_list file when an EP has been reconfigured.
 *
 * @param[in] ep_id EP_ID needed to modify.
 * @param[in] dev_type Device type of the new device in EP ID.
 */
static void modify_dev_list_file(uint8_t ep_id, uint8_t dev_type);

/* ------Implementation------ */

void stop_endpoint_callback(int argc, char** argv)
{
    if (argc == 1) {
        printf("ERR: too few arguments.\n");
        return;
    }

    /* get EP ID from command line */
    int ep_id = atoi(argv[1]);
    if (ep_id < 0 || ep_id > ha_host_ns::max_end_point) {
        printf("ERR: invalid endpoint id\n");
        return;
    }

    /* set EP in device list file without device (dev_type = 0x00) */
    modify_dev_list_file(ep_id, ha_ns::NO_DEVICE);

    /* stop EP having the entered ID */
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
    int8_t specified_subtype = -1;

    gpio_common_config(argc, argv, &ep_id, &specified_subtype);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::BUTTON);

    return;
}

void switch_config(int argc, char** argv)
{
    int8_t ep_id = -1;
    int8_t specified_subtype = -1;

    gpio_common_config(argc, argv, &ep_id, &specified_subtype);
    if (ep_id < 0) {
        return;
    }

    modify_dev_list_file(ep_id, (uint8_t) ha_ns::SWITCH);

    return;
}

void on_off_output_config(int argc, char** argv)
{
    int8_t ep_id = -1;
    int8_t specified_subtype = -1;

    gpio_common_config(argc, argv, &ep_id, &specified_subtype);
    if (ep_id < 0 || specified_subtype < 0) {
        printf("ERR: invalid specified sub-type\n");
        return;
    }

    modify_dev_list_file(ep_id,
            combine_dev_type(ha_ns::ON_OFF_OPUT, (uint8_t) specified_subtype));

    return;
}

void event_sensor_config(int argc, char** argv)
{
    int8_t ep_id = -1;
    int8_t specified_subtype = -1;

    gpio_common_config(argc, argv, &ep_id, &specified_subtype);
    if (ep_id < 0 || specified_subtype < 0) {
        printf("ERR: invalid specified sub-type\n");
        return;
    }

    modify_dev_list_file(ep_id,
            combine_dev_type(ha_ns::EVT_SENSOR, (uint8_t) specified_subtype));

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
    char f_name[4];
    uint8_t pattern_size = sizeof(ha_host_ns::rgb_config_pattern);

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

    char config_str[pattern_size];

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
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                /* save endpoint id */
                ep_id = atoi(argv[count]);
                if (ep_id < 0 || ep_id > ha_host_ns::max_end_point) {
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
                UINT byte_read;
                f_res = f_read(&fil, config_str, pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_host_ns::rgb_config_pattern, &Rport,
                        &Rpin, &Rtimer_x, &Rchannel, &Gport, &Gpin, &Gtimer_x,
                        &Gchannel, &Bport, &Bpin, &Btimer_x, &Bchannel,
                        &red_at_wp, &green_at_wp, &blue_at_wp);
                break;
            case 'R': //configure Red channel
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &Rport)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Rpin = atoi(argv[count]);
                if (Rpin < 0 || Rpin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                count++;
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &Gport)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Gpin = atoi(argv[count]);
                if (Rpin < 0 || Rpin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                count++;
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                if (!check_port(argv[count][0], &Bport)) {
                    printf("ERR: invalid port value.\n");
                    return;
                }
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                Bpin = atoi(argv[count]);
                if (Rpin < 0 || Rpin > 15) {
                    printf("ERR: invalid pin value\n");
                    return;
                }
                count++;
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                red_at_wp = atoi(argv[count]);
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                green_at_wp = atoi(argv[count]);
                count++;
                if (count >= argc) {
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

    snprintf(config_str, pattern_size, ha_host_ns::rgb_config_pattern, Rport,
            Rpin, Rtimer_x, Rchannel, Gport, Gpin, Gtimer_x, Gchannel, Bport,
            Bpin, Btimer_x, Bchannel, red_at_wp, green_at_wp, blue_at_wp);

    UINT byte_written;
    f_res = f_write(&fil, config_str, pattern_size, &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_res = f_open(&fil, f_name, FA_READ);
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
    char f_name[4];
    uint8_t pattern_size = sizeof(ha_host_ns::adc_sensor_config_pattern);
    uint8_t first_equa_type = 0;
    uint8_t first_params = 0;
    uint16_t num_params = 0;
    uint16_t num_equation = 0;

    int8_t ep_id = -1;
    int8_t specified_subtype = -1;

    char port = '0';
    uint16_t pin = 0;
    uint16_t adc_x = 0;
    uint16_t channel = 0;

    int filter_thres = 0;
    int under_thres = 0;
    int over_thres = 0;

    char config_str[pattern_size];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from shell */
    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(adc_sensor_usage);
                return;
            case 's':
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                specified_subtype = atoi(argv[count]);
                break;
            case 'e': //set endpoint id
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                /* save endpoint id */
                ep_id = atoi(argv[count]);
                if (ep_id < 0 || ep_id > ha_host_ns::max_end_point) {
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

                UINT byte_read;
                f_res = f_read(&fil, config_str, pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_host_ns::adc_sensor_config_pattern, &port,
                        &pin, &adc_x, &channel, &filter_thres, &under_thres,
                        &over_thres, &num_equation, &num_params);
                break;
            case 'p': //set port
                count++;
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                first_equa_type = count;
                num_equation = 0;
                while (count < argc && argv[count][0] != '-') {
                    num_equation++;
                    count++;
                }
                if (num_equation == 0) {
                    printf("ERR: missing parameters in -t option.\n");
                    return;
                }
                count--;
                break;
            case 'P':
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                first_params = count;
                num_params = 0;
                while (count < argc) {
                    if (argv[count][0] == '-') {
                        if (!isdigit(argv[count][1])) {
                            break;
                        }
                    }
                    num_params++;
                    count++;
                }
                count--;
                break;
            case 'f':
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                filter_thres = atoi(argv[count]);
                break;
            case 'u':
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                under_thres = atoi(argv[count]);
                break;
            case 'o':
                count++;
                if (count >= argc) {
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

    if (specified_subtype < 0) {
        printf("ERR: missing -s option.\n");
        return;
    }

    if (first_equa_type == 0 || first_params == 0) {
        printf("ERR: missing -t or -P option.\n");
        return;
    }

    f_res = f_unlink(f_name);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }

    /* Write configurations to file */
    f_res = f_open(&fil, f_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        printf("Error on opening file to write configuration\n");
        return;
    }
    f_sync(&fil);

    snprintf(config_str, pattern_size, ha_host_ns::adc_sensor_config_pattern,
            port, pin, adc_x, channel, filter_thres, under_thres, over_thres,
            num_equation, num_params);
    f_puts(config_str, &fil);
    f_sync(&fil);

    for (uint8_t count = first_equa_type;
            count < first_equa_type + num_equation; count++) {
        snprintf(config_str, pattern_size, ha_host_ns::sensor_equa_type,
                argv[count][0]);
        f_puts(config_str, &fil);
        f_sync(&fil);
    }

    float param = 0.0f;
    for (uint8_t count = first_params; count < first_params + num_params;
            count++) {
        param = strtof(argv[count], NULL);
        snprintf(config_str, pattern_size, ha_host_ns::sensor_equa_params,
                param);
        f_puts(config_str, &fil);
        f_sync(&fil);
    }
    f_close(&fil);

    /* read back */
    f_res = f_open(&fil, f_name, FA_READ);
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
            combine_dev_type(ha_ns::ADC_SENSOR, (uint8_t) specified_subtype));

    return;
}

static void gpio_common_config(int argc, char** argv, int8_t *endpoint_id,
        int8_t *specified_subtype)
{
    FRESULT f_res;
    FIL fil;
    char f_name[4];
    uint8_t pattern_size = sizeof(ha_host_ns::gpio_dev_config_pattern);

    *endpoint_id = -1;

    char port = '0';
    uint16_t pin = 0;
    char mode = '0';

    char config_str[pattern_size];

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
            case 's':
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                *specified_subtype = atoi(argv[count]);
                break;
            case 'e': //set endpoint id
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                /* save endpoint id */
                *endpoint_id = atoi(argv[count]);
                if (*endpoint_id < 0
                        || *endpoint_id > ha_host_ns::max_end_point) {
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
                UINT byte_read;
                f_res = f_read(&fil, config_str, pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_host_ns::gpio_dev_config_pattern, &port,
                        &pin, &mode);
                break;
            case 'p':
                count++;
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
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

    snprintf(config_str, pattern_size, ha_host_ns::gpio_dev_config_pattern,
            port, pin, mode);

    UINT byte_written;
    f_res = f_write(&fil, config_str, pattern_size, &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_res = f_open(&fil, f_name, FA_READ);
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
    uint8_t pattern_size = sizeof(ha_host_ns::adc_dev_config_pattern);
    char f_name[4];

    *endpoint_id = -1;
    char port = '0';
    uint16_t pin = 0;
    uint16_t adc_x = 0;
    uint16_t channel = 0;

    char config_str[pattern_size];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from shell */
    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(adc_usage, argv[0]);
                return;
            case 'e': //set endpoint id
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                /* save endpoint id */
                *endpoint_id = atoi(argv[count]);
                if (*endpoint_id < 0
                        || *endpoint_id > ha_host_ns::max_end_point) {
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
                UINT byte_read;
                f_res = f_read(&fil, config_str, pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_host_ns::adc_dev_config_pattern, &port,
                        &pin, &adc_x, &channel);
                break;
            case 'p':
                count++;
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
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

    snprintf(config_str, pattern_size, ha_host_ns::adc_dev_config_pattern, port,
            pin, adc_x, channel);

    UINT byte_written;
    f_res = f_write(&fil, config_str, pattern_size, &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_res = f_open(&fil, f_name, FA_READ);
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
    uint8_t pattern_size = sizeof(ha_host_ns::pwm_dev_config_pattern);
    char f_name[4];

    *endpoint_id = -1;
    char port = '0';
    uint16_t pin = 0;
    uint16_t timer_x = 0;
    uint16_t channel = 0;

    char config_str[pattern_size];

    if (argc <= 1) {
        printf("ERR: too few argument. Try -h to get help.\n");
        return;
    }

    /* Read all configuration from shell */
    for (uint8_t count = 1; count < argc; count++) {
        if (argv[count][0] == '-') {
            switch (argv[count][1]) {
            case 'h': //get help
                printf(pwm_usage, argv[0]);
                break;
            case 'e': //set endpoint id
                count++;
                if (count >= argc) {
                    printf("ERR: too few argument. Try -h to get help.\n");
                    return;
                }
                *endpoint_id = atoi(argv[count]);
                if (*endpoint_id < 0
                        || *endpoint_id > ha_host_ns::max_end_point) {
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
                UINT byte_read;
                f_res = f_read(&fil, config_str, pattern_size, &byte_read);
                if (f_res != FR_OK) {
                    print_ferr(f_res);
                    f_close(&fil);
                    return;
                }
                f_close(&fil);
                sscanf(config_str, ha_host_ns::adc_dev_config_pattern, &port,
                        &pin, &timer_x, &channel);
                break;
            case 'p': //set port
                count++;
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
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
                if (count >= argc) {
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

    snprintf(config_str, pattern_size, ha_host_ns::pwm_dev_config_pattern, port,
            pin, timer_x, channel);

    UINT byte_written;
    f_res = f_write(&fil, config_str, pattern_size, &byte_written);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        f_close(&fil);
        return;
    }
    f_close(&fil);

    /* read back */
    f_res = f_open(&fil, f_name, FA_READ);
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
    uint32_t dev_list[ha_host_ns::max_end_point];

    memset(dev_list, 0, sizeof(dev_list));

    f_res = f_open(&fil, ha_host_ns::ha_dev_list_file_name,
    FA_READ | FA_OPEN_ALWAYS);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }

    char line[64];
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (f_gets(line, sizeof(line), &fil)) {
            sscanf(line, ha_host_ns::dev_list_pattern, &dev_list[i]);
        } else {
            break;
        }
    }
    f_close(&fil);

    dev_list[ep_id] = ((uint32_t) ha_ns::sixlowpan_node_id << 16)
            | ((uint32_t) ep_id << 8) | (uint32_t) dev_type;

    f_res = f_open(&fil, ha_host_ns::ha_dev_list_file_name,
    FA_WRITE | FA_CREATE_ALWAYS);
    if (f_res != FR_OK) {
        print_ferr(f_res);
        return;
    }
    f_sync(&fil);

    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        snprintf(line, sizeof(line), ha_host_ns::dev_list_pattern, dev_list[i]);
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
    if (ep_id > ha_host_ns::max_end_point) {
        return false;
    }

    return true;
}

void run_endpoint(int8_t ep_id)
{
    FIL fil;
    uint32_t dev_list[ha_host_ns::max_end_point];

    memset(dev_list, 0, sizeof(dev_list));

    if (ep_id < 0 || ep_id > ha_host_ns::max_end_point) {
        printf("ERR: invalid input endpoint id.\n");
        return;
    }

    if (f_open(&fil, ha_host_ns::ha_dev_list_file_name,
            FA_READ | FA_OPEN_ALWAYS)) {
        printf("Error on opening device list file.\n");
        return;
    }

    char line[24];
    for (uint8_t i = 0; i < ha_host_ns::max_end_point; i++) {
        if (f_gets(line, sizeof(line), &fil)) {
            sscanf(line, ha_host_ns::dev_list_pattern, &dev_list[i]);
        } else {
            break;
        }
    }
    f_close(&fil);

    if (!check_devid(dev_list[ep_id])) {
        printf("-EP%d: No device!!\n", ep_id);
        return;
    }

    msg_t msg;
    msg.type = ha_host_ns::NEW_DEVICE;
    msg.content.value = dev_list[ep_id];
    msg_send(&msg, ha_host_ns::end_point_pid[ep_id], false);
}
