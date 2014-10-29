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

using namespace std;

/*******************************************************************************
 * Variables & Data Buffer
 *
 ******************************************************************************/
const 	uint16_t	usart_rec_stack_size	= 512;
char				usart_rec_stack[usart_rec_stack_size];
int16_t				usart_rec_pid;

uint8_t 			rxBuf[MAX_BUF_SIZE]		= "\0";	// hold usart3 rx data
uint8_t 			idxBuf					= 0;
ble_serv_stt_s 		BTFlags;
uint8_t				attBuf[MAX_MSGBUF_SIZE];
uint8_t				msg[]	= "Trinh Van Anh 1992 cocacola";
/*******************************************************************************
 * Private Functions
 *
 ******************************************************************************/
//void *usart_receving(void *arg){
//
//	usart3_receive();
//	while(1){
//	}
//	return NULL;
//}


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

//	usart_rec_pid = thread_create(usart_rec_stack,
//									usart_rec_stack_size,
//									PRIORITY_MAIN-1,
//									,
//									usart_receving,
//									NULL,
//									"usart3 receive thread");
//	while(BTFlags.ready != 0x01){
//
//	}
//	printf("%d\n", BTFlags.ready);
//	printf("ready\n");

//	ble_cmd_attributes_write(0x000b, 0, 31, msgBuf);

	while(1){

		if(newKey == MB1_usrBtn0.pressedKey_get()){
			/* Set BLE device discoverable */
			ble_cmd_gap_set_mode(gap_general_discoverable, gap_undirected_connectable);
		}

		if(newKey == MB1_usrBtn1.pressedKey_get()){
			/* Write data to BLE */
			ble_cmd_attributes_write(0x000b, 0, 30, msg);
		}

	}
}
