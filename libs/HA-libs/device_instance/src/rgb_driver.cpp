/**
 * @file rgb_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 22-10-2014
 * @brief This is source file for RGB-led device instance in HA system.
 */
#include "rgb_driver.h"

using namespace rgb_ns;

const uint16_t max_level = 65535;
const uint8_t rgb_active_level = 0;

uint8_t color_in_percent[][3] = { { 100, 100, 100 },   //white
        { 100, 0, 0 },      //red
        { 0, 100, 0 },      //green
        { 0, 0, 100 },      //blue
        { 100, 100, 0 },    //yellow
        { 100, 0, 100 },    //magenta
        { 0, 100, 100 },    //cyan
        { 100, 50, 0 },     //orange
        { 100, 0, 50 },     //rose
        { 50, 100, 0 },     //chartreuse
        { 0, 100, 50 },     //aquamarine
        { 50, 0, 100 },     //violet
        { 0, 50, 100 },     //azure
        { 100, 75, 80 },    //pink
        };

rgb_instance::rgb_instance(void)
{
    this->red_percent_wp = 100;
    this->green_percent_wp = 100;
    this->blue_percent_wp = 100;
}

void rgb_instance::device_configure(pwm_config_params_t *red_channel_params,
        pwm_config_params_t *green_channel_params,
        pwm_config_params_t *blue_channel_params)
{
    red_bulb.device_configure(red_channel_params);
    green_bulb.device_configure(green_channel_params);
    blue_bulb.device_configure(blue_channel_params);
}

void rgb_instance::set_white_point(uint8_t red_percent_wp,
        uint8_t green_percent_wp, uint8_t blue_percent_wp)
{
    this->red_percent_wp = red_percent_wp;
    this->green_percent_wp = green_percent_wp;
    this->blue_percent_wp = blue_percent_wp;

    rgb_calibrate();
}

void rgb_instance::rgb_set_color(uint32_t rgb_color,
        rgb_color_model_t rgb_color_model)
{
    uint8_t red_percent = 0;
    uint8_t green_percent = 0;
    uint8_t blue_percent = 0;

    if (rgb_color_model == model_16bits) {
        red_percent = ((rgb_color >> 11) & 0x1F) * 100 / 31;
        green_percent = ((rgb_color >> 5) & 0x3F) * 100 / 63;
        blue_percent = (rgb_color & 0x1F) * 100 / 31;
    } else if (rgb_color_model == model_24bits) {
        red_percent = ((rgb_color >> 16) & 0xFF) * 100 / 255;
        green_percent = ((rgb_color >> 8) & 0xFF) * 100 / 255;
        blue_percent = (rgb_color & 0xFF) * 100 / 255;
    }

    rgb_set_color(red_percent, green_percent, blue_percent);
}

void rgb_instance::rgb_set_color(rgb_color_t color)
{
    rgb_set_color(color_in_percent[color][0], color_in_percent[color][1],
            color_in_percent[color][2]);
}

void rgb_instance::rgb_set_color(uint8_t red_percent, uint8_t green_percent,
        uint8_t blue_percent)
{
    if (rgb_active_level == 0) {
        current_red_percent = 100 - red_percent * red_percent_wp / 100;
        current_green_percent = 100 - green_percent * green_percent_wp / 100;
        current_blue_percent = 100 - blue_percent * blue_percent_wp / 100;
    }
    red_bulb.set_percent_intensity(current_red_percent);
    green_bulb.set_percent_intensity(current_green_percent);
    blue_bulb.set_percent_intensity(current_blue_percent);
}

void rgb_instance::rgb_calibrate(void)
{
    rgb_set_color(current_red_percent, current_green_percent,
            current_blue_percent);
}
