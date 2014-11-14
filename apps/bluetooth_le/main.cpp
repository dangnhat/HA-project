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
#include "cir_queue.h"

using namespace Btn_ns;
using namespace ble_message_ns;
using namespace cir_queue_ns;

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

// test data
devInfo devList[4];
uint8_t dataBuf[(sizeof(devList)/sizeof(*devList))*10];
//uint8_t dataBuf[(sizeof(devList))*10];
/*******************************************************************************
 * Private Functions
 *
 ******************************************************************************/


void *ble_transaction(void *arg){

//	uint8_t msg_queue_size = 64;
//	msg_t 	msg_queue[msg_queue_size];
//	msg_init_queue(msg_queue, msg_queue_size);

	// Store message received from other threads
	msg_t 			msg;
	uint8array* 	client_msg;
	uint8_t			cnt = 0;
	uint8_t			msg_len;
	uint8_t			sumOflen = 0;
	uint8_t			msgBuf[msg_len];
	cir_queue		msgQueue 		= new cir_queue();

	while(1){
		msg_receive(&msg);

		switch (msg.type) {

			case BLE_SERVER_RESET:
				printf("--- sys reset ---\n");

				ble_cmd_sm_set_bondable_mode(1);
				// Make BLE device discoverable
				ble_cmd_gap_set_mode(gap_general_discoverable,
										gap_undirected_connectable);
				break;
			case BLE_USART_REC:
				printf("--- usart rec ---\n");
				break;
			case BLE_CLIENT_WRITE:
				printf("--- client write ---\n");
				client_msg = (uint8array*) msg.content.ptr;
				cnt++;
				sumOflen += client_msg->len;

				if(1 == cnt){
					//the first byte of message is length of data
					msg_len = client_msg->data[0] + 3;  //plus 3 bytes of header
				}

				for(uint8_t i = 0; i < client_msg->len; i++){
//					printf("%x\n", client_msg->data[i]);
					msgBuf[]
				}

				client_msg = NULL;


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

	init_ble_database(devList, sizeof(devList)/sizeof(*devList));
	getMsgData(devList, dataBuf, sizeof(dataBuf));

	ble_thread_ns::ble_thread_pid = thread_create(ble_thread_stack,
									ble_thread_stack_size,
									PRIORITY_MAIN-1,
									0,
									ble_transaction,
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
//			printf("devList len = %d\n", sizeof(devList)/sizeof(*devList));
			printf("len databuf = %d\n", sizeof(dataBuf));
			for(uint8_t i = 0; i < sizeof(devList)/sizeof(*devList); i++){
				printf("devIndx = %x\n", devList[i].devIdx);
				printf("devID = %x\n", devList[i].devID);
				printf("devVal = %x\n", devList[i].devVal);
			}
			printf("============================\n");
//			printf("dataBuf[21] = %x\n", dataBuf[21]);
			for(uint8_t i = 0; i < sizeof(dataBuf); i++){
				printf("dataBuf[%d] = %x\n", i, dataBuf[i]);
			}
		}

	}
}
