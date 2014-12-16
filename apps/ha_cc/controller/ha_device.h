/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        ha_device.h
 * @brief       Device class.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#ifndef HA_DEVICE_H_
#define HA_DEVICE_H_

#include <cstdint>

namespace ha_device_ns {

enum device_e: uint32_t {
    no_device_id = 0x00,
};

enum device_type_io_e: uint8_t {
    input_device = 0x00,
    output_device = 0x01,
};

}

class ha_device {
public:

    /**
     * @brief   constructor,
     *          New device object with device_id = no_device_id, and value = 0.
     */
    ha_device(void) {device_id = ha_device_ns::no_device_id; value = 0; ttl = 0;} ;
    
    /**
     * @brief   Set device_id.
     *
     * @param[in]   device_id
     */
    void set_device_id(uint32_t id) { device_id = id; };
    
    /**
     * @brief   Get device_id.
     *
     * @return  device_id
     */
    uint32_t get_device_id(void) { return device_id; };

    /**
     * @brief   Set value.
     *
     * @param[in]   value
     */
    void set_value(int16_t val) { value = val; };

    /**
     * @brief   Get value.
     *
     * @return  value
     */
    int16_t get_value(void) { return value; };

    /**
     * @brief   Set to no device.
     */
    void set_to_no_device(void) { device_id = ha_device_ns::no_device_id; };

    /**
     * @brief   Check whether this's no device or not.
     *
     * @return  true if device_id == no device.
     */
    bool is_no_device(void) { return (device_id == ha_device_ns::no_device_id ? true : false); };

    /**
     * @brief   Get node id.
     *
     * @return  node_id.
     */
    uint16_t get_node_id(void) { return (uint16_t)(device_id >> 16); };

    /**
     * @brief   Get zone id.
     *
     * @return  zone_id.
     */
    uint8_t get_zone_id(void) { return (uint8_t)(device_id >> 24); };

    /**
     * @brief   Get device type.
     *
     * @return  device type.
     */
    uint8_t get_device_type(void) { return (uint8_t)(device_id); };

    /**
     * @brief   Get device in/out type.
     *
     * @return  in/out type.
     */
    uint8_t get_io_type(void) { return (get_device_type() >> 6); };

    /**
     * @brief   Get end point id.
     *
     * @return  end point id.
     */
    uint8_t get_end_point(void) { return (uint8_t)(device_id >> 8); };

    /**
     * @brief   Get time to live.
     *
     * @return  time to live
     */
    int16_t get_ttl(void) { return ttl; };

    /**
     * @brief   Set time to live.
     *
     * @param[in]   time, time to live
     */
    void set_ttl(int16_t time) { ttl = (time < 0) ? 0 : time; };

private:
    uint32_t device_id;
    int16_t value;
    int16_t ttl; /* time to live (in seconds) */
};

#endif // HA_DEVICE_H_
