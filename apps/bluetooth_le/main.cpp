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
uint8_t 			rxBuf[MAX_BUF_SIZE]		= "\0";	// hold usart3 rx data
uint8_t 			index					= 0;
ble_serv_stt_s 		BTFlags;
uint8_t 			msgBuf[]				=  "This is default data of BLE112A";
uint8_t				attBuf[MAX_MSGBUF_SIZE];


/*******************************************************************************
 * Private Functions
 *
 ******************************************************************************/




/* main */
int main() {
	/* Initial MBoard-1 system */
	MB1_system_init();
	/* Initial BLE interface */
	ble_init();


//	while(BTFlags.ready != 0x01){
//
//	}
//	ble_cmd_attributes_write(0x000b, 0, 31, msgBuf);
	while(1){

		if(newKey == MB1_usrBtn0.pressedKey_get()){
			MB1_Led_green.toggle();	//DEBUG
			/* Set BLE device discoveryable */
			ble_cmd_gap_set_mode(gap_general_discoverable, gap_undirected_connectable);
		}

		if(newKey == MB1_usrBtn1.pressedKey_get()){
			ble_cmd_attributes_write(0x000b, 0, 31, msgBuf);
		}

	}
}
