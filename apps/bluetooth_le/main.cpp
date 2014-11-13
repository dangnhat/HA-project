#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

extern "C" {
#include "transceiver.h"
#include "board_uart0.h"
#include "thread.h"
#include "board.h"
#include "hwtimer.h"
#include "vtimer.h"
#include "msg.h"
#include "cc110x_ng.h"

#include "cc110x_reconfig.h"
}

#include "MB1_System.h"
#include "ble_transaction.h" //used by ANH

using namespace Btn_ns;
using namespace ble_message_ns;

/*******************************************************************************
 * Variables & Data Buffer
 *
 ******************************************************************************/
const 	uint16_t	ble_thread_stack_size	= 512;
char				ble_thread_stack[ble_thread_stack_size];
//int16_t				ble_thread_pid;

uint8_t 			rxBuf[MAX_BUF_SIZE]		= "\0";	// hold usart3 rx data
uint8_t 			idxBuf					= 0;
ble_serv_stt_s 		BTFlags;
uint8_t				attBuf[MAX_MSGBUF_SIZE];
uint8_t				msg[]	= "Trinh Van Anh 1992 cocacola";

/*******************************************************************************
 * Private Functions
 *
 ******************************************************************************/


void *usart_receving(void *arg){

//	uint8_t msg_queue_size = 64;
//	msg_t 	msg_queue[msg_queue_size];
//	msg_init_queue(msg_queue, msg_queue_size);

	// Store message received from other threads
	msg_t msg;
	uint8array* client_msg;

	while(1){
		msg_receive(&msg);

		switch (msg.type) {

			case BLE_SERVER_RESET:
				puts("sys reset\n");

				ble_cmd_sm_set_bondable_mode(1);
				// Make BLE device discoverable
				ble_cmd_gap_set_mode(gap_general_discoverable,
										gap_undirected_connectable);
				break;
			case BLE_USART_REC:
				puts("usart rec");
				break;
			case BLE_CLIENT_WRITE:
				puts("client write");
				client_msg = (uint8array*) msg.content.ptr;
				printf("len = %d\n", client_msg->len);
				for(uint8_t i = 0; i < client_msg->len; i++){
					printf("data = %x\n", client_msg->data[i]);
				}
				break;

			default:
				break;
		}
	}

	return NULL;
}





/*******************************************************************************
 * Main Function
 *
 ******************************************************************************/

/* main */
int main() {
	/* Initial MBoard-1 system */
	MB1_system_init();
	/* Initial BLE interface */
	ble_init();

	devInfo devList[] = init_ble_database();
	uint8_t dataBuf[] = getMsgData(devList);

	ble_thread_ns::ble_thread_pid = thread_create(ble_thread_stack,
									ble_thread_stack_size,
									PRIORITY_MAIN-1,
									0,
									usart_receving,
									NULL,
									"ble thread");
	printf("%x\n", ble_thread_ns::ble_thread_pid);

	while(1){

//		if(newKey == MB1_usrBtn0.pressedKey_get()){
//			/* Set BLE device discoverable */
//			ble_cmd_gap_set_mode(gap_general_discoverable, gap_undirected_connectable);
//		}

		if(newKey == MB1_usrBtn1.pressedKey_get()){
			/* Write data to BLE */
//			ble_cmd_attributes_write(CHARA_WRITE_ADDR, 0, 30, msg);
			for(uint8_t i = 0; i < sizeof(dataBuf); i++){
				printf("data[%x] = %b\n", i, dataBuf[i]);
			}
		}

	}
}
