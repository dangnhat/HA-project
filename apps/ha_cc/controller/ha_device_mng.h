/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        device_mng.h
 * @brief       Device managing classes.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#ifndef DEVICE_MNG_H_
#define DEVICE_MNG_H_

#include <cstdint>
#include "ha_device.h"

namespace ha_device_mng_ns {

enum errcode_e: int8_t {
    FAILED = -1,
    SUCCESS = 0,
    TTL_IS_ZERO,
};

}

class ha_device_mng {
public:
    /**
     * @brief   constructor,
     *          User must provide buffer to hold devices and size of this buffer.
     *          All devices in buffer will be clear with no_device ids.
     *
     * @param[in]   devices_buffer, pointer to buffer holding devices.
     * @param[in]   num_of_dev, number of devices in device_buffer
     * @param[in]   devices_list_filename, file name will hold list of devices.
     *              NULL to disable save/restore operations.
     */
    ha_device_mng(ha_device *devices_buffer, uint16_t num_of_dev, const char *devices_list_filename);
    
    /**
     * @brief   Find a device with device_id and set its value. If this device id
     *          didn't exist in devices list of manager, new device will be added.
     *
     * @param[in]   device_id.
     * @param[in]   value.
     *
     * @return      0 on success, -1 if buffer was full and new device was not added.
     */
    int8_t set_dev_val(uint32_t device_id, int16_t value);

    /**
     * @brief   Find and get value from a device.
     *
     * @param[in]   device_id.
     * @param[out]  value.
     *
     * @return      0, if device has been found. -1 if device didn't exist.
     */
    int8_t get_dev_val(uint32_t device_id, int16_t &value);

    /**
     * @brief   Get device id and value at a index position in devices buffer.
     *
     * @param[in]   index
     * @param[out]  device_id.
     * @param[out]  value.
     *
     * @return      0,
     */
    int8_t get_dev_val_with_index(uint16_t index, uint32_t &device_id, int16_t &value);

    /**
     * @brief   Find and set TTL of a device.
     *
     * @param[in]   device_id.
     * @param[in]   ttl.
     *
     * @return      0, if device has been found. -1 if device didn't exist.
     */
    int16_t set_dev_ttl(uint32_t device_id, int16_t ttl);

    /**
     * @brief   Find and get TTL of a device.
     *
     * @param[in]   device_id.
     * @param[out]  ttl.
     *
     * @return      0, if device has been found. -1 if device didn't exist.
     */
    int16_t get_dev_ttl(uint32_t device_id, int16_t &ttl);

    /**
     * @brief   Find and increase / decrease TTL of a device.
     *
     * @param[in]   device_id.
     * @param[in]   val.
     *
     * @return      0 on success and device's ttl is not zero. -1 if device didn't exist.
     *              TTL_IS_ZERO if ttl value is zero.
     */
    int16_t chag_dev_ttl(uint32_t device_id, int16_t val);

    /**
     * @brief   Decrease all device's ttls by one, and remove device(s) if TTL_IS_ZERO.
     */
    void dec_all_devs_ttl(void);

    /**
     * @brief   Remove a device from devices buffer. (i.e. set id to no device)
     *
     * @param[in]   device_id.
     *
     * @return      0, if device has been found and removed. -1 if device didn't exist.
     */
    int8_t remove_device(uint32_t device_id);

    /**
     * @brief   Re-order devices buffer to get a contiguous block.
     */
    void reorder(void);

    /**
     * @brief   Get current number of devices in buffer.
     *
     * @return  current number of devices.
     */
    uint16_t get_current_numofdev(void) {return cur_size;};

    /**
     * @brief   Print all device via HA_NOTIFY.
     */
    void print_all_devices(void);

    /**
     * @brief   Save current list of devices to file.
     */
    void save(void);

    /**
     * @brief   Read device list file and restore devices buffer.
     */
    void restore(void);

private:
    /*----------------------------- Methods ----------------------------------*/
    /**
     * @brief   Find a device with device_id. NOTE: address of a device will be changed
     *          after reorder method is called.
     *
     * @param[in]   device_id.
     *
     * @return      pointer to a ha_device object, NULL if not found.
     */
    ha_device *find_device(uint32_t device_id);

    /**
     * @brief   Find an empty slot in devices buffer.
     *
     * @return      pointer to a ha_device object with no device id (empty slot),
     *              NULL if not found.
     */
    ha_device *find_an_empty_slot(void);

    /**
     * @brief   Find a empty device in buffer and add new device. NOTE: this function
     *          will not check whether this device has existed in buffer or not.
     *          Current size of devices buffer will be updated.
     *
     * @param[in]   device_id.
     *
     * @return      pointer to newly added ha_device object, NULL if devices buffer
     *              was full and new device was not added.
     */
    ha_device *add_device(uint32_t device_id);

    /**
     * @brief   Set all devices in devices buffer to no device.
     */
    void clear_all_devices(void);

    /*----------------------------- Variables --------------------------------*/
    uint16_t cur_size;

    uint16_t max_num_of_dev;
    ha_device *devices_buffer;

    const char *devices_list_file;
};

#endif // DEVICE_MNG_H_
