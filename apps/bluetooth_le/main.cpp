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
				puts("usart rec\n");
//				msg_t usartMSg;
//				usartMSg.type = BLE_SERVER_RESET;
//				msg_send_to_self(&usartMSg);
//				receiveBTMessage();
				break;
			case BLE_CLIENT_WRITE:
				puts("client write\n");
				break;

			default:
				break;
		}
	}
	printf("wtf\n");
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

	ble_thread_ns::ble_thread_pid = thread_create(ble_thread_stack,
									ble_thread_stack_size,
									PRIORITY_MAIN-1,
									0,
									usart_receving,
									NULL,
									"ble thread");
	printf("%x\n", ble_thread_ns::ble_thread_pid);

	while(1){
//
//		if(newKey == MB1_usrBtn0.pressedKey_get()){
//			/* Set BLE device discoverable */
//			ble_cmd_gap_set_mode(gap_general_discoverable, gap_undirected_connectable);
//		}
//
//		if(newKey == MB1_usrBtn1.pressedKey_get()){
//			/* Write data to BLE */
//			ble_cmd_attributes_write(0x000b, 0, 30, msg);
//		}

	}
}
