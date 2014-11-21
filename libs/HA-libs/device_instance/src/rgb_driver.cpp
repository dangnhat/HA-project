/**
 * @file rgb_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 22-10-2014
 * @brief This is source file for RGB-led device instance in HA system.
 */
#include "rgb_driver.h"

using namespace rgb_ns;

const static uint8_t rgb_active_level = 0;

rgb_instance::rgb_instance(void)
{
    this->red_percent_wp = 100;
    this->green_percent_wp = 100;
    this->blue_percent_wp = 100;
    /* 16bits as default */
    this->color_model = model_16bits;
    this->current_color = 0xFFFF;
}

void rgb_instance::device_configure(pwm_config_params_t *red_channel_params,
        pwm_config_params_t *green_channel_params,
        pwm_config_params_t *blue_channel_params)
{
    red_bulb.device_configure(red_channel_params);
    green_bulb.device_configure(green_channel_params);
    blue_bulb.device_configure(blue_channel_params);

    red_bulb.set_active_level(rgb_active_level);
    green_bulb.set_active_level(rgb_active_level);
    blue_bulb.set_active_level(rgb_active_level);
}

void rgb_instance::restart(void)
{
    red_bulb.restart();
    green_bulb.restart();
    blue_bulb.restart();
}

void rgb_instance::stop(void)
{
    red_bulb.stop();
    green_bulb.stop();
    blue_bulb.stop();
}

void rgb_instance::set_white_point(uint8_t red_percent_wp,
        uint8_t green_percent_wp, uint8_t blue_percent_wp)
{
    this->red_percent_wp = red_percent_wp;
    this->green_percent_wp = green_percent_wp;
    this->blue_percent_wp = blue_percent_wp;

    rgb_calibrate();
}

void rgb_instance::set_color_model(rgb_color_model_t rgb_color_model)
{
    this->color_model = rgb_color_model;
}

void rgb_instance::rgb_set_color(uint32_t rgb_color)
{
    uint8_t red_percent = 0;
    uint8_t green_percent = 0;
    uint8_t blue_percent = 0;

    this->current_color = rgb_color;

    if (this->color_model == model_16bits) { //565
        red_percent = ((rgb_color >> 11) & 0x1F) * 100 / 31;
        green_percent = ((rgb_color >> 5) & 0x3F) * 100 / 63;
        blue_percent = (rgb_color & 0x1F) * 100 / 31;
    } else if (this->color_model == model_24bits) {
        red_percent = ((rgb_color >> 16) & 0xFF) * 100 / 255;
        green_percent = ((rgb_color >> 8) & 0xFF) * 100 / 255;
        blue_percent = (rgb_color & 0xFF) * 100 / 255;
    }

    rgb_set_color(red_percent, green_percent, blue_percent);
}

void rgb_instance::rgb_set_color(uint8_t red_percent, uint8_t green_percent,
        uint8_t blue_percent)
{
    red_percent = red_percent * red_percent_wp / 100;
    green_percent = green_percent * green_percent_wp / 100;
    blue_percent = blue_percent * blue_percent_wp / 100;

    red_bulb.set_percent_intensity(red_percent);
    green_bulb.set_percent_intensity(green_percent);
    blue_bulb.set_percent_intensity(blue_percent);
}

void rgb_instance::rgb_calibrate(void)
{
    rgb_set_color(this->current_color);
}

uint32_t rgb_instance::get_current_color(void)
{
    return this->current_color;
}
