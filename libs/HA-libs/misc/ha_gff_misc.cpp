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
    uint16_t ret_val;

    ret_val = (uint16_t)buffer[0] << 8;
    ret_val = ret_val | ((uint16_t)buffer[1]);
    return ret_val;
}

/*----------------------------------------------------------------------------*/
uint32_t buf2uint32(uint8_t* buffer)
{
    uint32_t ret_val;

    ret_val = (uint32_t)buffer[0] << 24;
    ret_val = ret_val | ((uint32_t)buffer[1] << 16);
    ret_val = ret_val | ((uint32_t)buffer[2] << 8);
    ret_val = ret_val | ((uint32_t)buffer[3]);
    return ret_val;
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

uint8_t combine_dev_type(uint8_t dev_type_common, uint8_t sub_type)
{
    return ((dev_type_common & 0xF8) | (sub_type & 0x07));
}

/*----------------------------------------------------------------------------*/
const char sw_name[] = "switch";
const char btn_name[] = "button";
const char dim_name[] = "dimmer";
const char temp_ss_name[] = "temp sensor";
const char lumi_ss_name[] = "lumi sensor";
const char gas_ss_name[] = "gas sensor";
const char moisture_ss_name[] = "moisture sensor";
const char pir_ss_name[] = "pir sensor";
const char on_off_bulb_name[] = "on/off bulb";
const char buzzer_name[] = "buzzer";
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
    case ((ha_ns::ADC_SENSOR & 0xF8) | (ha_ns::TEMP & 0x07)):
        return temp_ss_name;
    case ((ha_ns::ADC_SENSOR & 0xF8) | (ha_ns::LUMI & 0x07)):
        return lumi_ss_name;
    case ((ha_ns::ADC_SENSOR & 0xF8) | (ha_ns::GAS & 0x07)):
        return gas_ss_name;
    case ((ha_ns::ADC_SENSOR & 0xF8) | (ha_ns::MOISTURE & 0x07)):
        return moisture_ss_name;
    case ((ha_ns::EVT_SENSOR & 0xF8) | (ha_ns::PIR & 0x07)):
        return pir_ss_name;
    case ((ha_ns::ON_OFF_OPUT & 0xF8) | (ha_ns::ON_OFF_BULB & 0x07)):
        return on_off_bulb_name;
    case ((ha_ns::ON_OFF_OPUT & 0xF8) | (ha_ns::BUZZER & 0x07)):
        return buzzer_name;
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
