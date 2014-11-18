/**
 * @file ha_gff_misc.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 11-Nov-2014
 * @brief This contains implementations of some functions to manipulate data in GFF format.
 */

#include <stdint.h>
#include <stddef.h>

#include "ha_gff_misc.h"
#include "device_id.h"

/*----------------------------------------------------------------------------*/
uint16_t buf2uint16(uint8_t* buffer)
{
    return (uint16_t)(buffer[0] << 8 | buffer[1]);
}

/*----------------------------------------------------------------------------*/
uint32_t buf2uint32(uint8_t* buffer)
{
    return (uint32_t)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
}

/*----------------------------------------------------------------------------*/
void uint162buf(uint16_t value, uint8_t* buffer)
{
    buffer[0] = (uint8_t)(value >> 8);
    buffer[1] = (uint8_t)value;
}

/*----------------------------------------------------------------------------*/
void uint322buf(uint32_t value, uint8_t* buffer)
{
    buffer[0] = (uint8_t)(value >> 24);
    buffer[1] = (uint8_t)(value >> 16);
    buffer[2] = (uint8_t)(value >> 8);
    buffer[3] = (uint8_t)value;
}

/*----------------------------------------------------------------------------*/
uint8_t parse_zone_deviceid(uint32_t device_id)
{
    uint16_t node_id = parse_node_deviceid(device_id);
    return (uint8_t)(node_id >> 8);
}

/*----------------------------------------------------------------------------*/
uint16_t parse_node_deviceid(uint32_t device_id)
{
    return (uint16_t)(device_id >> 16);
}

/*----------------------------------------------------------------------------*/
uint8_t parse_ep_deviceid(uint32_t device_id)
{
    return (uint8_t)(device_id >> 8);
}

/*----------------------------------------------------------------------------*/
uint8_t parse_devtype_deviceid(uint32_t device_id)
{
    return (uint8_t)(device_id);
}

/*----------------------------------------------------------------------------*/
const char sw_name[] = "switch";
const char btn_name[] = "button";
const char dim_name[] = "dimmer";
const char lin_sensor_name[] = "linear sensor";
const char evt_sensor_name[] = "event sensor";
const char on_off_bulb_name[] = "on/off bulb";
const char level_bulb_name[] = "level bulb";
const char rgb_led_name[] = "rgb led";
const char servo_name[] = "servo";

const char* device_type_to_name(uint8_t device_type)
{
    switch (device_type) {
    case ha_ns::SWITCH:
        return sw_name;
    case ha_ns::BUTTON:
        return btn_name;
    case ha_ns::DIMMER:
        return dim_name;
    case ha_ns::LIN_SENSOR:
        return lin_sensor_name;
    case ha_ns::EVT_SENSOR:
        return evt_sensor_name;
    case ha_ns::ON_OFF_BULB:
        return on_off_bulb_name;
    case ha_ns::LEVEL_BULB:
        return level_bulb_name;
    case ha_ns::RGB_LED:
        return rgb_led_name;
    case ha_ns::SERVO_SG90:
        return servo_name;
    default:
        return NULL;
    }
}
