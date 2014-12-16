/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        ha_device_mng.cpp
 * @brief       Home automation devices manager.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 * @}
 */

#include <stddef.h>
#include <string.h>
#include "ha_device_mng.h"
#include "ha_gff_misc.h"
#include "ff.h"
#include "shell_cmds_fatfs.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace ha_device_mng_ns;

/* print list of devices */
static const char print_line_pattern[] = "| %-3d | %08lx | %-8d | %-3d | %-4d | %-17s |\n";

/* save and restore */
static const char devices_list_line_pattern[] = "%lx %d %d\n";

/*----------------------------------------------------------------------------*/
ha_device_mng::ha_device_mng(ha_device *devices_buffer, uint16_t num_of_dev,
        const char *devices_list_filename)
{
    cur_size = 0;

    max_num_of_dev = num_of_dev;
    this->devices_buffer = devices_buffer;

    devices_list_file = devices_list_filename;

    /* Clear all devices */
    clear_all_devices();
}

/*----------------------------------------------------------------------------*/
int8_t ha_device_mng::set_dev_val(uint32_t device_id, int16_t value)
{
    ha_device *device_p;

    /* find device */
    device_p = find_device(device_id);

    if (device_p != NULL) { /* device found */
        device_p->set_value(value);
        return 0;
    }

    /* device not found */
    device_p = add_device(device_id);
    if (device_p == NULL) { /* buffer was full */
        return -1;
    }

    device_p->set_value(value);
    cur_size++;
    return 0;
}

/*----------------------------------------------------------------------------*/
int8_t ha_device_mng::get_dev_val(uint32_t device_id, int16_t &value)
{
    ha_device *device_p;

    device_p = find_device(device_id);
    if (device_p == NULL) {
        return -1;
    }

    value = device_p->get_value();
    return 0;
}

/*----------------------------------------------------------------------------*/
int8_t ha_device_mng::get_dev_val_with_index(uint16_t index, uint32_t &device_id, int16_t &value)
{
    device_id = devices_buffer[index].get_device_id();
    value = devices_buffer[index].get_value();

    return 0;
}

/*----------------------------------------------------------------------------*/
int16_t ha_device_mng::set_dev_ttl(uint32_t device_id, int16_t ttl)
{
    ha_device *device_p;

    device_p = find_device(device_id);
    if (device_p == NULL) {
        return -1;
    }

    device_p->set_ttl(ttl);
    return 0;
}

/*----------------------------------------------------------------------------*/
int16_t ha_device_mng::get_dev_ttl(uint32_t device_id, int16_t &ttl)
{
    ha_device *device_p;

    device_p = find_device(device_id);
    if (device_p == NULL) {
        return -1;
    }

    ttl = device_p->get_ttl();
    return 0;
}

/*----------------------------------------------------------------------------*/
int16_t ha_device_mng::chag_dev_ttl(uint32_t device_id, int16_t val)
{
    ha_device *device_p;

    device_p = find_device(device_id);
    if (device_p == NULL) {
        return -1;
    }

    device_p->set_ttl(device_p->get_ttl() + val);
    if (device_p->get_ttl() == 0) {
        return TTL_IS_ZERO;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
void ha_device_mng::dec_all_devs_ttl(void)
{
    uint16_t count;

    for (count = 0; count < max_num_of_dev; count++) {
        if (!devices_buffer[count].is_no_device()) {
            devices_buffer[count].set_ttl(devices_buffer[count].get_ttl() - 1);
            if (devices_buffer[count].get_ttl() == 0) {
                devices_buffer[count].set_to_no_device();
                cur_size--;
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
int8_t ha_device_mng::remove_device(uint32_t device_id)
{
    ha_device *device_p;

    device_p = find_device(device_id);
    if (device_p == NULL) {
        return -1;
    }

    device_p->set_to_no_device();
    cur_size--;
    return 0;
}

/*----------------------------------------------------------------------------*/
void ha_device_mng::reorder(void)
{
    ha_device *empty_dev_p = NULL;
    ha_device *last_dev_p = NULL;
    uint16_t count;

    if (cur_size < 2) {
        return;
    }

    while (1) {
        /* find an empty slot */
        empty_dev_p = find_an_empty_slot();
        if (empty_dev_p == NULL) {
            /* No empty slot */
            return;
        }

        /* find the last device in the buffer */
        for (count = max_num_of_dev - 1; count >= 0; count--) {
            if (!devices_buffer[count].is_no_device()) {
                /* the last device in buffer */
                last_dev_p = &devices_buffer[count];
                memcpy(empty_dev_p, last_dev_p, sizeof(ha_device));
                last_dev_p->set_to_no_device();
                break;
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
void ha_device_mng::print_all_devices(void)
{
    uint16_t num_of_dev_count = 0;
    uint16_t count;

    HA_NOTIFY("| %-3s | %-8s | %-8s | %-3s | %-4s | %-17s |\n", "#", "id",
            "value", "i/o", "ttl", "name");
    HA_NOTIFY("-----\n");

    for (count = 0; count < max_num_of_dev; count++) {
        if (!devices_buffer[count].is_no_device()) {
            /* not empty device */
            HA_NOTIFY(print_line_pattern,
                    count,
                    devices_buffer[count].get_device_id(),
                    devices_buffer[count].get_value(),
                    devices_buffer[count].get_io_type(),
                    devices_buffer[count].get_ttl(),
                    device_type_to_name(devices_buffer[count].get_device_type()));

            num_of_dev_count++;
            if (num_of_dev_count == cur_size) {
                break;
            }
        }/* end a device */
    }/* end for */

    HA_NOTIFY("Total num of devs: %hu\n", cur_size);
}

/*----------------------------------------------------------------------------*/
void ha_device_mng::save(void)
{
    FIL file;
    FRESULT fres;
    uint16_t count;
    uint16_t num_of_dev_count;

    if (devices_list_file == NULL) {
        return;
    }

    /* open file */
    fres = f_open(&file, devices_list_file, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("ha_dev_mng::save: Error when open file %s\n", devices_list_file);
        print_ferr(fres);
        return;
    }
    f_sync(&file);

    /* write data */
    num_of_dev_count = 0;
    for (count = 0; count < max_num_of_dev; count++) {
        if (!devices_buffer[count].is_no_device()) {
            f_printf(&file, devices_list_line_pattern,
                    devices_buffer[count].get_device_id(),
                    devices_buffer[count].get_value(),
                    devices_buffer[count].get_ttl());
            f_sync(&file);

            num_of_dev_count++;
            if (num_of_dev_count == cur_size) {
                f_close(&file);
                return;
            }
        }/* end not empty device */
    }/* end for */

    f_close(&file);
}

/*----------------------------------------------------------------------------*/
void ha_device_mng::restore(void)
{
    FIL file;
    FRESULT fres;
    uint32_t device_id;
    int value_i, ttl_i;
    char line[32];

    if (devices_list_file == NULL) {
        return;
    }

    /* open file */
    fres = f_open(&file, devices_list_file, FA_READ | FA_OPEN_ALWAYS);
    if (fres != FR_OK) {
        HA_DEBUG("ha_device_mng::restore: Error when open file %s\n", devices_list_file);
        print_ferr(fres);
        return;
    }

    /* read list of devices */
    while( f_gets(line, sizeof(line), &file) ){
        sscanf(line, devices_list_line_pattern, &device_id, &value_i, &ttl_i);
        set_dev_val(device_id, (int16_t)value_i);
        set_dev_ttl(device_id, (int8_t)ttl_i);
    }

    f_close(&file);
}

/*----------------------------------------------------------------------------*/
ha_device *ha_device_mng::find_an_empty_slot(void)
{
    uint16_t num_of_dev_count = 0;
    for (uint16_t count = 0; count < max_num_of_dev; count++) {
        if (devices_buffer[count].is_no_device()) {
            /* empty slot here */
            return &devices_buffer[count];
        }

        /* not empty slot */
        num_of_dev_count++;
        if (num_of_dev_count == cur_size) {
            return NULL;
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
ha_device *ha_device_mng::find_device(uint32_t device_id)
{
    for (uint16_t count = 0; count < max_num_of_dev; count++) {
        if (devices_buffer[count].get_device_id() == device_id) {
            return &devices_buffer[count];
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
ha_device *ha_device_mng::add_device(uint32_t device_id)
{
    for (uint16_t count = 0; count < max_num_of_dev; count++) {
        if (devices_buffer[count].is_no_device()) {
            devices_buffer[count].set_device_id(device_id);
            return &devices_buffer[count];
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
void ha_device_mng::clear_all_devices(void)
{
    for (uint16_t count = 0; count < max_num_of_dev; count++) {
        devices_buffer[count].set_to_no_device();
    }

    cur_size = 0;
}
