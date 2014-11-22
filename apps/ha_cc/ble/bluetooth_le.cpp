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
}

#include "gff_mesg_id.h"
#include "ha_gff_misc.h"

#include "controller.h"
#include "ha_device_mng.h"
#include "cc_msg_id.h"

#include "ble_transaction.h"
#include "gff_mesg_id.h"

/******************************************************************************
 * Variables and buffers
 ******************************************************************************/

/* Message queue */
static const uint16_t ble_message_queue_size = 64;
static msg_t ble_message_queue[ble_message_queue_size];

/* Controller thread stack */
static const uint16_t ble_thread_stack_size = 1024;
static char ble_thread_stack[ble_thread_stack_size];
static const char ble_thread_prio = PRIORITY_MAIN - 1;
static void *ble_transaction(void *arg);

namespace ble_thread_ns {
int16_t ble_thread_pid;

/* USART3 receive buffer */
uint16_t idxBuf = 0;
uint8_t usart3_rec_buf[MAX_BUF_SIZE];

/*controller message queue */
const uint16_t from_ctlr_queue_size = 1024;
uint8_t from_ctlr_queue_buf[from_ctlr_queue_size];
cir_queue from_ctlr_queue = cir_queue(
        from_ctlr_queue_buf, from_ctlr_queue_size);
}

/*******************************************************************************
 * Private functions declare
 ******************************************************************************/
static void ble_write_att(uint8_t *dataBuf, uint8_t len);
static void receive_msg_from_controller(cir_queue* mCirQueue);
/*******************************************************************************
 * Public functions
 ******************************************************************************/

/*
 * @brief: Create ble thread
 */

void ble_thread_start(void)
{
    ble_thread_ns::ble_thread_pid = thread_create(ble_thread_stack,
            ble_thread_stack_size, ble_thread_prio, CREATE_STACKTEST,
            ble_transaction,
            NULL, "ble thread");
}

/*******************************************************************************
 * Private functions implementation
 ******************************************************************************/

/*
 * @brief: ble thread implementation
 */

void *ble_transaction(void *arg)
{

    msg_t msg;
    uint8_t numOfMsg = 0;
    uint8array* usartMsgPtr;
    cir_queue* ctlrMsgPtr;
    uint8_t msgLen;
    uint8_t msgBuf[256];
    uint8_t sumOfMsgLen = 0;

    msg_init_queue(ble_message_queue, ble_message_queue_size);
    while (1) {

        msg_receive(&msg);
        switch (msg.type) {

        case ha_cc_ns::BLE_SERVER_RESET:
            HA_NOTIFY("--- server reset ---\n");
            // Make BLE device discoverable
            ble_cmd_gap_set_mode(gap_general_discoverable,
                    gap_undirected_connectable);
            ble_cmd_sm_set_bondable_mode(1);
            break;
        case ha_cc_ns::BLE_CLIENT_WRITE:
            HA_NOTIFY("--- client write ---\n");
            //get bluetooth message from Mobile
            usartMsgPtr = reinterpret_cast<uint8array*>(msg.content.ptr);
            numOfMsg++;
            sumOfMsgLen += usartMsgPtr->len;
            HA_DEBUG("cnt = %d\n", numOfMsg);
            HA_DEBUG("clen = %d\n", usartMsgPtr->len);
            if (1 == numOfMsg) {
                //the first byte of message is length of data
                msgLen = usartMsgPtr->data[0] + ha_ns::GFF_CMD_SIZE
                        + ha_ns::GFF_LEN_SIZE;          //plus 3 bytes of header
                HA_DEBUG("len= %d\n", msgLen);
            }

            if (sumOfMsgLen == msgLen) {
                HA_DEBUG("end of packet\n");
                numOfMsg = 0;
                sumOfMsgLen = 0;
                //TODO Send data to Controller
                msg_t bleThreadMsg;
                bleThreadMsg.type = ha_cc_ns::BLE_GFF_PENDING;
                bleThreadMsg.content.ptr =
                        (char*) &controller_ns::ble_to_controller_queue;
                msg_send(&bleThreadMsg, controller_ns::controller_pid, false);
            }

        case ha_ns::GFF_PENDING:
            // Get message from thread Controller, and send to Mobile
            receive_msg_from_controller((cir_queue *) msg.content.ptr);
            break;
        default:
            HA_DEBUG("tao lao\n");
            break;
        }
    }

    return NULL;
}

/**
 * @brief: Receive message from controller thread
 */
void receive_msg_from_controller(cir_queue* mCirQueue)
{
    uint8_t bufLen = mCirQueue->preview_data(false) + ha_ns::GFF_CMD_SIZE
            + ha_ns::GFF_LEN_SIZE;

    if (bufLen == mCirQueue->get_size()) {
        ble_write_att(
                (uint8_t*)mCirQueue->get_data(
                        ble_thread_ns::from_ctlr_queue_buf, bufLen),
                bufLen);
    } else {
        HA_DEBUG("Frame error\n");
    }

}

/**
 * @brief: Write data to bluetooth LE attribute
 */
void ble_write_att(uint8_t *dataBuf, uint8_t len)
{
    ble_cmd_attributes_write(ATT_WRITE_ADDR, 0, len, dataBuf);
}

