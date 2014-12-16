/**
 * @file device_id.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 3-Nov-2014
 * @brief This is the header file for device id definitions.
 *
 * Device id : 32 bit
 * MSB                                      LSB
 * 31           16          8               0
 * |-- Node id --|-- EP id -|- device type -|
 *
 * Node id: 16 bit
 * MSB                                  LSB
 * 15            8                      0
 * |-- Zone id --|-- Node id in zone ---|
 */

#ifndef DEVICE_ID_H_
#define DEVICE_ID_H_

#include <stdint.h>

namespace ha_ns {

enum device_type_common_e
    : uint8_t {
        NO_DEVICE = 0x00,
    SWITCH = 0x01,          //00|000|---
    BUTTON = 0x02,          //00|000|---
    DIMMER = 0x03,          //00|000|---
    ADC_SENSOR = 0x30,      //00|110|---
    EVT_SENSOR = 0x38,      //00|111|---

    ON_OFF_OPUT = 0x78,     //01|111|---
    LEVEL_BULB = 0x42,      //01|000|---
    RGB_LED = 0x43,         //01|000|---
    SERVO_SG90 = 0x44       //01|000|---
};//common type

enum lin_ss_subtype_e
    : uint8_t {
        TEMP = 0x00,
    LUMI = 0x01,
    GAS = 0x02,
    MOISTURE = 0x03
};

enum evt_ss_subtype_e
    : uint8_t {
        PIR = 0x00,
};

enum on_off_subtype_e
    : uint8_t {
        ON_OFF_BULB = 0x00,
    BUZZER = 0x01,
};

}

#endif /* DEVICE_ID_H_ */
