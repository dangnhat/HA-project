/*******************************************************************************
 * ble_resp.cpp
 *
 *  Created on: Oct 6, 2014
 *      Author: anhtrinh
 ******************************************************************************/

#include "cmd_def.h"
#include "ble_transaction.h"
#include <stdio.h>
#include "controller.h"
#include "gff_mesg_id.h"
#include "ha_gff_misc.h"

extern "C" {
#include "msg.h"
#include "thread.h"
}

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

extern ble_ack_s ble_ack;

/******************************************************************************
 * Private variables and buffers
 ******************************************************************************/
static uint8_t numOfMsg = 0;
static uint8_t msgLen;
static uint8_t totalMsgLen = 0;
static uint8_t ack_index_buf[2];

/* usart receive queue*/
static const uint8_t usart_queue_size = 255;
static uint8_t usart_queue_buf[usart_queue_size];
cir_queue usart_queue(usart_queue_buf, usart_queue_size);
/*******************************************************************************
 * BLE112 Event Callback
 ******************************************************************************/

void ble_evt_system_boot(const struct ble_msg_system_boot_evt_t *msg)
{
    msg_t bleThreadMsg;
    bleThreadMsg.type = ha_cc_ns::BLE_SERVER_RESET;
    msg_send(&bleThreadMsg, ble_thread_ns::ble_thread_pid, false);
}

void ble_rsp_system_reset(const void *nul)
{
    HA_DEBUG("--- system reset ---\n");
}

void ble_rsp_system_hello(const void *nul)
{
    HA_DEBUG("-- hello --\n");
}

void ble_rsp_system_get_info(const struct ble_msg_system_get_info_rsp_t *msg)
{
    HA_DEBUG("-- system get info -- \n");
}

void ble_rsp_hardware_set_soft_timer(
        const struct ble_msg_hardware_set_soft_timer_rsp_t *msg)
{
    HA_DEBUG("-- set soft timer --\n");

}

void ble_rsp_sm_set_bondable_mode(const void *nul)
{
    HA_DEBUG("-- bondable --\n");
}

void ble_rsp_gap_set_mode(const struct ble_msg_gap_set_mode_rsp_t *msg)
{
    HA_DEBUG("-- discoverable --\n");
}

void ble_evt_connection_disconnected(
        const struct ble_msg_connection_disconnected_evt_t *msg)
{
    HA_DEBUG("-- remote device disconnected --\n");
    msg_t bleThreadMsg;
    bleThreadMsg.type = ha_cc_ns::BLE_CLIENT_DISCONNECT;
    msg_send(&bleThreadMsg, ble_thread_ns::ble_thread_pid, false);
}

/**
 * @brief: process data come from Mobile
 */
void ble_evt_attributes_value(const struct ble_msg_attributes_value_evt_t *msg)
{
    msg_t msg_ble_thread;
    numOfMsg++;
    totalMsgLen += msg->value.len;
    if (1 == numOfMsg) {

        ack_index_buf[0] = msg->value.data[1];      // get 2 byte index
        ack_index_buf[1] = msg->value.data[2];      //
        if (msg->value.data[0] == ha_ble_ns::BLE_MSG_ACK) {
            msgLen = msg->value.data[3] + 3;        //plus 3 bytes of header
            /* get index, if true wake-up ble_thread */
            if (( buf2uint16(ack_index_buf) == ble_ack.packet_index)
                    && ble_ack.need_to_wait_ack) {
                ble_ack.need_to_wait_ack = false;
                ble_ack.packet_index++;
                thread_wakeup(ble_thread_ns::ble_thread_pid);
                return;
            }
        } else {          // if message is data
            msgLen = msg->value.data[3] + ha_ns::GFF_CMD_SIZE
                    + ha_ns::GFF_LEN_SIZE + 3;      //plus 3 bytes of header
            /* if message from mobile is data, then send ACK back to mobile */
            send_ack_to_mobile(ack_index_buf);
        }
    }

    usart_queue.add_data((uint8_t*)msg->value.data, msg->value.len);

    /* Consider if received data payload as its length */
    if (totalMsgLen == msgLen) {
        numOfMsg = 0;
        totalMsgLen = 0;
        /* detach message header */
        for(uint8_t i = 0; i < 3; i++){
            usart_queue.get_data();
        }
        /* send message to ble_thread */
        msg_ble_thread.type = ha_cc_ns::BLE_CLIENT_WRITE;
        msg_ble_thread.content.ptr = (char*) (&usart_queue);
        msg_send(&msg_ble_thread, ble_thread_ns::ble_thread_pid, false);
    }
}

void ble_rsp_attributes_write(const struct ble_msg_attributes_write_rsp_t *msg)
{
    HA_DEBUG("-- write local--\n");
}

void ble_evt_connection_status(
        const struct ble_msg_connection_status_evt_t *msg)
{
    HA_DEBUG("-- client connected --\n");
    msg_t bleThreadMsg;
    bleThreadMsg.type = ha_cc_ns::BLE_CLIENT_CONNECT;
    msg_send(&bleThreadMsg, ble_thread_ns::ble_thread_pid, false);
}

