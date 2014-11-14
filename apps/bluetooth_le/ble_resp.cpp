/*******************************************************************************
 * ble_resp.cpp
 *
 *  Created on: Oct 6, 2014
 *      Author: anhtrinh
 ******************************************************************************/


#include "cmd_def.h"
#include "ble_transaction.h"
#include <stdio.h>

extern "C"{
#include "msg.h"
#include "thread.h"
}

void ble_evt_system_boot(const struct ble_msg_system_boot_evt_t *msg)
{
	printf("--- system boot event ---%x\n", thread_getpid());

	msg_t bleThreadMsg;
	bleThreadMsg.type =  ble_message_ns::BLE_SERVER_RESET;
	msg_send(&bleThreadMsg, ble_thread_ns::ble_thread_pid, false);
}

void ble_rsp_system_reset(const void *nul)
{
	printf("--- system reset ---\n");
}

void ble_rsp_system_hello(const void *nul)
{
	printf("-- hello --\n");
}

void ble_rsp_system_get_info(const struct ble_msg_system_get_info_rsp_t *msg)
{
	printf("-- system get info -- \n");
}

void ble_rsp_hardware_set_soft_timer(const struct ble_msg_hardware_set_soft_timer_rsp_t *msg)
{
	printf("-- set soft timer --\n");

}

void ble_rsp_sm_set_bondable_mode(const void *nul)
{
	printf("-- set bondable mode --\n");
}

void ble_rsp_gap_set_mode(const struct ble_msg_gap_set_mode_rsp_t *msg)
{
	printf("-- discoverable --\n");
}


void ble_evt_connection_disconnected(const struct ble_msg_connection_disconnected_evt_t *msg)
{
	printf("-- remote device disconnected --\n");
	BTFlags.connected = 0;
	ble_cmd_gap_set_mode(gap_general_discoverable, gap_undirected_connectable);
}

void ble_evt_attributes_value(const struct ble_msg_attributes_value_evt_t *msg)
{
	msg_t msg_ble_thread;
	msg_ble_thread.type = ble_message_ns::BLE_CLIENT_WRITE;
	msg_ble_thread.content.ptr 		= (char*) (&msg->value);
	msg_send(&msg_ble_thread, ble_thread_ns::ble_thread_pid, false);
}

void ble_rsp_attributes_write(const struct ble_msg_attributes_write_rsp_t *msg)
{
	printf("-- write local--\n");
}

void ble_evt_connection_status(const struct ble_msg_connection_status_evt_t *msg)
{
	printf("-- connection establish --\n");
	BTFlags.connected = 1;
}




