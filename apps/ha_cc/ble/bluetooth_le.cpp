/*
 * Copyright (C) 2014 Ho Chi Minh city University of Technology (HCMUT)
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.
 */

/**
 * @file        bluetooth_le.cpp
 * @brief       Home automation bluetooth LE thread.
 *
 * @author      Anh Trinh <51000131@stu.hcmut.edu.vn>
 */

extern "C" {
#include "thread.h"
#include "msg.h"
#include "vtimer.h"
}

#include "ha_gff_misc.h"

#include "controller.h"
#include "ha_device_mng.h"
#include "cc_msg_id.h"

#include <string.h>
#include "ble_transaction.h"
#include "gff_mesg_id.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/******************************************************************************
 * Variables and buffers
 ******************************************************************************/

/* data structure to process ACK from Mobile */
ble_ack_s ble_ack;

/* ble message queue */
static const uint16_t ble_message_queue_size = 128;
static msg_t ble_message_queue[ble_message_queue_size];

/* bluetooth thread stack */
static const uint16_t ble_thread_stack_size = 1200;
static char ble_thread_stack[ble_thread_stack_size];
static const char ble_thread_prio = PRIORITY_MAIN - 1;
static void *ble_transaction(void *arg);

/* circle queue to save data received from controller thread */
static const uint16_t controller_to_ble_msg_queue_size = 1280;
static uint8_t controller_to_ble_msg_queue_buf[controller_to_ble_msg_queue_size];

/* ble reset pin */
namespace ha_ble_ns {
gpio ble_reset_pin;
}

namespace ble_thread_ns {
kernel_pid_t ble_thread_pid;

/*controller message queue */
cir_queue controller_to_ble_msg_queue(controller_to_ble_msg_queue_buf,
        controller_to_ble_msg_queue_size);
}

// timer 6 timeout
volatile uint16_t ble_ack_timeout_count = 0;

/*******************************************************************************
 * Private functions declare
 ******************************************************************************/

/**
 * @brief: write data to BLE
 */
static void ble_write_att(uint16_t handle, uint8_t *dataBuf, uint8_t len);
/**
 * @brief:  process message from controller
 */
static void receive_msg_from_controller(cir_queue* mCirQueue, uint16_t msgIndex,
bool mMoblieConnected);

/**
 * @brief: thread wait ack from Mobile
 */
void wait_mobile_ack(uint16_t mSec);
/*******************************************************************************
 * Public functions
 ******************************************************************************/

void ble_timeout_TIM6_ISR(void)
{
    if (ble_ack_timeout_count > 0) {
        ble_ack_timeout_count--;
        if (ble_ack_timeout_count == 0) {
            thread_wakeup(ble_thread_ns::ble_thread_pid);
        }
    }
}

/*
 * @brief: Initial ble thread
 */

void ble_thread_start(void)
{
    ble_thread_ns::ble_thread_pid = thread_create(ble_thread_stack,
            ble_thread_stack_size, ble_thread_prio, CREATE_STACKTEST,
            ble_transaction,
            NULL, "ble thread");

    /* init and reset ble */
    ha_ble_ns::ble_reset_pin.gpio_init(&ha_ble_ns::ble_reset_pin_param);
    HA_NOTIFY("Reseting ble module...\n");
    /* Reset */
    ha_ble_ns::ble_reset_pin.gpio_reset();
    /* Hold for 100 ms */
    vtimer_usleep(100);
    /* Release */
    ha_ble_ns::ble_reset_pin.gpio_set();
}

/*******************************************************************************
 * Private functions implementation
 ******************************************************************************/

/*
 * @brief: ble thread function implementation
 */

void *ble_transaction(void *arg)
{

    msg_t msg;
    bool mConnect = false;
    uint16_t usart_msg_len;
    uint8_t usartBuf[ha_ns::GFF_MAX_FRAME_SIZE];
    cir_queue* usartQueue;

    msg_init_queue(ble_message_queue, ble_message_queue_size);
    while (1) {

        msg_receive(&msg);
        switch (msg.type) {

        case ha_cc_ns::BLE_SERVER_RESET:
            HA_DEBUG("--- server reset ---\n");
            // Make BLE device discoverable
            ble_cmd_gap_set_mode(gap_general_discoverable,
                    gap_undirected_connectable);
            ble_cmd_sm_set_bondable_mode(1);
            break;
        case ha_cc_ns::BLE_CLIENT_CONNECT:
            mConnect = true;
            break;
        case ha_cc_ns::BLE_CLIENT_DISCONNECT:
            mConnect = false;
            ble_cmd_gap_set_mode(gap_general_discoverable,
                    gap_undirected_connectable);
            break;
        case ha_cc_ns::BLE_CLIENT_WRITE:

            HA_DEBUG("--- client write ---\n");
            /* get bluetooth message from Mobile */
            usartQueue = (cir_queue*) (msg.content.ptr);
            usart_msg_len = usartQueue->preview_data(false)
                    + ha_ns::GFF_LEN_SIZE + ha_ns::GFF_CMD_SIZE;

            if(usartQueue->get_size() > 30){
                usartQueue->get_data(usartBuf, usartQueue->get_size());
                break;
            }

            if (usart_msg_len <= usartQueue->get_size()) {
                usartQueue->get_data(usartBuf, usart_msg_len);

                /* send ACK to mobile*/
//                send_ack_to_mobile();

                //DEBUG
                for (uint8_t i = 0; i < usart_msg_len; i++) {
                    HA_DEBUG("%d ", usartBuf[i]);
                }HA_DEBUG("\n");

                /* put data to controller's queue */
                controller_ns::ble_to_controller_queue.add_data(usartBuf,
                        usart_msg_len);
                /* Send data to Controller thread */
                msg_t msg_ble_thread;
                msg_ble_thread.type = ha_cc_ns::BLE_GFF_PENDING;
                msg_ble_thread.content.ptr =
                        (char*) &controller_ns::ble_to_controller_queue;
                msg_send(&msg_ble_thread, controller_ns::controller_pid, false);
            } else {
                HA_DEBUG(" usart queue overflow\n");
            }
            break;
        case ha_ns::GFF_PENDING:
            // Get message from thread Controller, and send to Mobile
            receive_msg_from_controller((cir_queue *) msg.content.ptr,
                    ble_ack.packet_index, mConnect);
            break;
        default:
            HA_DEBUG("error\n");
            break;
        }
    }

    return NULL;
}

/**
 * @brief: Receive message from controller thread
 */
void receive_msg_from_controller(cir_queue* mCirQueue, uint16_t msgIndex,
bool mMoblieConnected)
{
    uint8_t bufLen = mCirQueue->preview_data(false) + ha_ns::GFF_CMD_SIZE
            + ha_ns::GFF_LEN_SIZE;
    uint8_t dataBuf[ha_ns::GFF_MAX_FRAME_SIZE];

    uint8_t indexBuf[2];
    int32_t qBufSize;

    uint162buf(msgIndex, indexBuf);
    qBufSize = mCirQueue->get_size();
    HA_DEBUG("len = %d\n", bufLen);HA_DEBUG("qsize = %d\n", qBufSize);
    if (bufLen <= qBufSize) {
        mCirQueue->get_data(dataBuf, bufLen);
        // add header(msg type + index) to message
        add_hdr_to_ble_msg(ha_ble_ns::BLE_MSG_DATA, indexBuf, dataBuf, bufLen);
        if (mMoblieConnected) {
            HA_DEBUG("packet index START\n");
            ble_write_att(ATT_WRITE_ADDR, dataBuf, bufLen + 3);
            // sleep to wait ack from mobile
            if (bufLen >= 27) {
                wait_mobile_ack(400); // 150ms
            } else {
                wait_mobile_ack(300);
            }
            ble_ack.packet_index++;
            ble_ack.need_to_wait_ack = false;
            HA_DEBUG("packet index END\n");
        }

    } else {
        HA_DEBUG("Frame error\n");
    }

}

/**
 * @brief: thread wait ack from Mobile
 */
void wait_mobile_ack(uint16_t mSec)
{
    ble_ack_timeout_count = mSec;
    ble_ack.need_to_wait_ack = true;
    thread_sleep();
}

/**
 * @brief: Write data to bluetooth LE attribute
 */
void ble_write_att(uint16_t handle, uint8_t *dataBuf, uint8_t len)
{
    ble_cmd_attributes_write(handle, 0, len, dataBuf);
}

void add_hdr_to_ble_msg(uint8_t msgType, uint8_t* ack_idx_buf, uint8_t* payload,
        uint8_t bufLen)
{
    uint8_t header[3];
    header[0] = msgType;
    header[1] = ack_idx_buf[0];
    header[2] = ack_idx_buf[1];

    memmove(payload + 3, payload, bufLen);
    mempcpy(payload, header, sizeof(header));
}

void send_ack_to_mobile()
{
    uint32_t timeout = 100000;
    uint8_t msgBuf[4];

    msgBuf[0] = ha_ble_ns::BLE_MSG_ACK;
    msgBuf[1] = 0;
    msgBuf[2] = 0;

//    vtimer_usleep(timeout/2);
    ble_write_att(ATT_WRITE_ADDR, msgBuf, 4);
    vtimer_usleep(timeout);
//    thread_sleep();


}
