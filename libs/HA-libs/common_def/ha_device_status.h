/**
 * @file ha_device_status.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 17-11-2014
 * @brief This is header file for device status in HA system.
 */
#ifndef __HA_DEVICE_STATUS_H
#define __HA_DEVICE_STATUS_H

#include <stdint.h>

namespace ha_ns {
enum btn_status_e
    : uint16_t {
        btn_no_pressed = 0,
    btn_pressed = 1,
    btn_on_hold = 2
};

enum sw_status_e
    : uint16_t {
        switch_off = 0,
    switch_on = 1
};

enum on_off_status_e
    : uint16_t {
        output_off = 0,
    output_on = 1,
    toggle = 2,
};

enum blink_status_e
    : uint16_t {
        //0-100: level intensity
        blink_1hz = 101,
    blink_2hz = 102,
    blink_3hz = 103,
    blink_4hz = 104,
    blink_5hz = 105,
    blink_6hz = 106,
    blink_7hz = 107,
    blink_8hz = 108,
    blink_9hz = 109,
    blink_10hz = 110,
};

enum evt_sensor_status_e
    : uint16_t {
        no_detected = 0,
    detected = 1
};
}

#endif //__HA_DEVICE_STATUS_H
