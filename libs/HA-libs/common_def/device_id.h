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

enum device_type_e
    : uint8_t {
        SWITCH = 0x01,
    BUTTON = 0x02,
    DIMMER = 0x03,
    LIN_SENSOR = 0x04,
    EVT_SENSOR = 0x05,

    ON_OFF_BULB = 0x41,
    LEVEL_BULB = 0x42,
    RGB_LED = 0x43,
    SERVO_SG90 = 0x44
};

}

#endif /* DEVICE_ID_H_ */
