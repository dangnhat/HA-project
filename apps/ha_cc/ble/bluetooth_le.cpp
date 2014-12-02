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


#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

/******************************************************************************
 * Variables and buffers
 ******************************************************************************/

/* ble message queue */
static const uint16_t ble_message_queue_size = 64;
static msg_t ble_message_queue[ble_message_queue_size];

/* Controller thread stack */
static const uint16_t ble_thread_stack_size = 512;
static char ble_thread_stack[ble_thread_stack_size];
static const char ble_thread_prio = PRIORITY_MAIN - 1;
static void *ble_transaction(void *arg);

static const uint16_t controller_to_ble_msg_queue_size = 1024;
static uint8_t controller_to_ble_msg_queue_buf[controller_to_ble_msg_queue_size];

namespace ble_thread_ns {
int16_t ble_thread_pid;

/*controller message queue */
cir_queue controller_to_ble_msg_queue(
        controller_to_ble_msg_queue_buf, controller_to_ble_msg_queue_size);
}

//test
bool isReadSuccessed = true;
/*******************************************************************************
 * Private functions declare
 ******************************************************************************/
static void ble_write_att(uint8_t *dataBuf, uint8_t len);
static void receive_msg_from_controller(cir_queue* mCirQueue,
                                        bool       mMoblieConnected);
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
    uint8_t msgLen;
    uint8_t sumOfMsgLen = 0;
    bool mConnect = false;

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
            break;
        case ha_cc_ns::BLE_CLIENT_WRITE:
            HA_DEBUG("--- client write ---\n");
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
            break;

        case ha_ns::GFF_PENDING:
            // Get message from thread Controller, and send to Mobile
            receive_msg_from_controller((cir_queue *) msg.content.ptr,
                    mConnect);
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
void receive_msg_from_controller(cir_queue* mCirQueue, bool mMoblieConnected)
{
    HA_DEBUG("from controller\n");
    uint8_t bufLen = mCirQueue->preview_data(false) + ha_ns::GFF_CMD_SIZE
            + ha_ns::GFF_LEN_SIZE;
    HA_DEBUG("previewed queue\n");
    uint8_t dataBuf[ha_ns::GFF_MAX_FRAME_SIZE];

    if (bufLen == mCirQueue->get_size()) {
        mCirQueue->get_data(dataBuf, bufLen);
        for(uint8_t i = 0 ; i < bufLen; i++){
            HA_DEBUG("%x ", dataBuf[i]);
        }
        HA_DEBUG("\n");

        if (mMoblieConnected) {
            isReadSuccessed = false;
            ble_write_att(dataBuf, bufLen);
//            while(isReadSuccessed == false){};
        }

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

