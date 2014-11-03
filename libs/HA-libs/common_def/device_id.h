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

enum device_type_e: uint8_t {
    BUTTON = 0, /* TODO: Need to be changed by Hien */
};

};

#endif /* DEVICE_ID_H_ */
