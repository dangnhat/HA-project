/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        controller.cpp
 * @brief       Home automation CC's controller thread.
 *
 * @author      DangNhat Pham-Huu <51002279@stu.hcmut.edu.vn>
 */

#include <cstdint>

extern "C" {
#include "thread.h"
#include "msg.h"
}

#include "gff_mesg_id.h"
#include "ha_gff_misc.h"

#include "controller.h"
#include "ha_device_mng.h"
#include "cc_msg_id.h"

/*----------------------------- Configurations -------------------------------*/
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/* Data Cir_queues */
static const uint16_t slp_to_controller_queue_size = 256;
static uint8_t slp_to_controller_queue_buffer[slp_to_controller_queue_size];

static const uint16_t ble_to_controller_queue_size = 1024;
static uint8_t ble_to_controller_queue_buffer[ble_to_controller_queue_size];

/* Message queue */
static const uint16_t controller_message_queue_size = 64;
static msg_t controller_message_queue[controller_message_queue_size];

/* Controller thread stack */
static const uint16_t controller_stack_size = 2048;
static char controller_stack[controller_stack_size];
static const char controller_prio = PRIORITY_MAIN;
static void *controller_func(void *arg);

/* Device management */
static const uint16_t controller_max_num_of_devs = 128;
static ha_device controller_devs_buffer[controller_max_num_of_devs];
static const char controller_dev_list_filename[] = "dev_lst";
static ha_device_mng controller_dev_mng(controller_devs_buffer, controller_max_num_of_devs,
        controller_dev_list_filename);

/* Time to live for every ALIVE messages */
static const int8_t alive_ttl = 30; /* in second */

/* Time period to save device data */
static const uint8_t dev_list_save_period = 5; /* in seconds */

/*----------------------------- Controller namespace -------------------------*/

namespace controller_ns {

kernel_pid_t controller_pid;

/* Data Cir_queues */
cir_queue slp_to_controller_queue(slp_to_controller_queue_buffer, slp_to_controller_queue_size);
cir_queue ble_to_controller_queue(ble_to_controller_queue_buffer, ble_to_controller_queue_size);

}

/*----------------------------- Public functions -----------------------------*/
using namespace controller_ns;

/**
 * @brief   Create and start controller thread,
 */
void controller_start(void)
{
    /* Create controller thread */
    controller_pid = thread_create(controller_stack, controller_stack_size,
            controller_prio, CREATE_STACKTEST, controller_func, NULL, "CC Controller");
    if (controller_pid > 0) {
        HA_NOTIFY("CC Controller thread created.\n");
    }
    else {
        HA_NOTIFY("Can't create CC Controller thread.\n");
    }
}

/*----------------------------- Static functions -----------------------------*/
/* Prototypes */
static void slp_gff_handler(uint8_t *gff_frame, cir_queue *slp_queue, ha_device_mng *dev_mng);
static void ble_gff_handler(uint8_t *gff_frame, cir_queue *ble_queue, ha_device_mng *dev_mng);
void save_dev_list_with_1sec (uint8_t save_period, ha_device_mng *dev_mng);

static void *controller_func(void *) {
    msg_t mesg;
    uint8_t gff_frame[ha_ns::GFF_MAX_FRAME_SIZE];

    /* Init message queue */
    msg_init_queue(controller_message_queue, controller_message_queue_size);

    /* restore old data */
    controller_dev_mng.restore();

    /* Wait for message */
    while (1) {
        msg_receive(&mesg);

        switch (mesg.type) {
        case ha_cc_ns::SLP_GFF_PENDING:
            HA_DEBUG("controller: SLP_GFF_PENDING\n");
            slp_gff_handler(gff_frame, (cir_queue *)mesg.content.ptr, &controller_dev_mng);
            break;
        case ha_cc_ns::BLE_GFF_PENDING:
            HA_DEBUG("controller: BLE_GFF_PENDING\n");
            break;
        case ha_cc_ns::ONE_SEC_INTERRUPT:
            controller_dev_mng.dec_all_devs_ttl();
            save_dev_list_with_1sec(dev_list_save_period, &controller_dev_mng);
            break;
        default:
            HA_DEBUG("controller: Unknown message %d\n", mesg.type);
            break;
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
static void slp_gff_handler(uint8_t *gff_frame, cir_queue *slp_queue, ha_device_mng *dev_mng)
{
    uint8_t data_len;
    uint16_t cmd_id;
    uint32_t device_id;
    int16_t value;

    /* Check data */
    data_len = slp_queue->preview_data(false);
    if ((data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE) > slp_queue->get_size()) {
        HA_DEBUG("slp_gff_handler: Err, data len %hu + cmd_size + len_size != queue_size %ld\n",
                data_len, slp_queue->get_size());
        return;
    }

    /* Get data from queue */
    slp_queue->get_data(gff_frame, data_len + ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE);

    /* parse GFF frame */
    cmd_id = buf2uint16(&gff_frame[ha_ns::GFF_CMD_POS]);

    switch (cmd_id) {
    case ha_ns::SET_DEV_VAL:
        device_id = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS]);
        value = (int16_t)buf2uint16(&gff_frame[ha_ns::GFF_DATA_POS + 4]);
        HA_DEBUG("slp_gff_handler: SET_DEV_VAL (%lx, %d)\n", device_id, value);

        dev_mng->set_dev_val(device_id, value);
        dev_mng->set_dev_ttl(device_id, alive_ttl);
        /* TODO: forward to BLE */
        break;
    case ha_ns::ALIVE:
        device_id = buf2uint32(&gff_frame[ha_ns::GFF_DATA_POS]);
        HA_DEBUG("slp_gff_handler: ALIVE (%lx)\n", device_id);

        dev_mng->set_dev_ttl(device_id, alive_ttl);
        break;
    default:
        HA_DEBUG("slp_gff_handler: unknown cmd id %x\n", cmd_id);
        break;
    }
}

/*----------------------------------------------------------------------------*/
void second_int_callback(void)
{
    msg_t mesg;

    mesg.type = ha_cc_ns::ONE_SEC_INTERRUPT;
    msg_send(&mesg, controller_pid, false);
}

/*----------------------------------------------------------------------------*/
void save_dev_list_with_1sec (uint8_t save_period, ha_device_mng *dev_mng)
{
    static uint8_t time_count = 0;

    time_count++;
    if (time_count > save_period) {
        time_count = 0;
        HA_DEBUG("save_dev_list: Saving devices list\n");
        dev_mng->save();
        HA_DEBUG("save_dev_list: Saved\n");
    }

}

/*----------------------------------------------------------------------------*/
void controller_list_devices(int argc, char** argv)
{
    controller_dev_mng.print_all_devices();
}
